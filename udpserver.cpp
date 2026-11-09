// udpserver.cpp
#include "udpserver.h"

#include <QNetworkDatagram>
#include <QNetworkInterface>

#include <iostream>

using namespace std;

// Шифртекст из методических указаний (ЛР 7).
// Сформирован алгоритмом Vigenere::encrypt() из открытого текста
// "Секретный текст is120 для проверки Виженера" ключом из диапазона 1000..10000.
static const QString kCipherText =
    QString::fromUtf8("ГЪЪЯЦзЭйЫpвУЬжвhTgllmpУЪрpЯЯаЧФЯЬЮi3ЪЬФЬЦёП");

UdpServer::UdpServer(quint16 port, int mode, QObject *parent)
    : QObject(parent)
    , udpSocket(new QUdpSocket(this))
    , m_listening(false)
    , m_mode(mode)
    , m_nextKeyStart(1000)
    , m_keyStep(1000)
    , m_maxKey(10000)
    , m_found(false)
{
    m_listening = udpSocket->bind(QHostAddress::Any, port);
    if (!m_listening) {
        cerr << "Не удалось занять UDP-порт " << port << endl;
        return;
    }

    connect(udpSocket, &QUdpSocket::readyRead, this, &UdpServer::readPendingDatagrams);

    if (m_mode == 6) {
        cout << "[ЛР6] UDP Сервер запущен на порту " << port << endl;
    } else {
        cout << "[ЛР7] Сервер распределенных вычислений запущен на порту " << port << endl;
        cout << "[ЛР7] Диапазон ключей: " << m_nextKeyStart << " - " << m_maxKey << endl;
    }
    printLocalAddresses();
}

bool UdpServer::isListening() const
{
    return m_listening;
}

void UdpServer::printLocalAddresses() const
{
    QStringList printed;
    const auto interfaces = QNetworkInterface::allInterfaces();
    for (const QNetworkInterface &iface : interfaces) {
        const auto flags = iface.flags();
        if (!flags.testFlag(QNetworkInterface::IsUp)
            || !flags.testFlag(QNetworkInterface::IsRunning)
            || flags.testFlag(QNetworkInterface::IsLoopBack)) {
            continue;
        }
        const auto entries = iface.addressEntries();
        for (const QNetworkAddressEntry &entry : entries) {
            const QHostAddress ip = entry.ip();
            if (ip.protocol() != QAbstractSocket::IPv4Protocol)
                continue;

            const QString address = ip.toString();
            if (printed.contains(address))
                continue;
            printed.append(address);

            cout << "Адрес для клиентов: " << address.toStdString()
                 << " (" << iface.humanReadableName().toStdString() << ')' << endl;
        }
    }

    if (printed.isEmpty())
        cout << "Адрес для клиентов не найден, используется 127.0.0.1" << endl;
}

void UdpServer::readPendingDatagrams()
{
    while (udpSocket->hasPendingDatagrams()) {
        const QNetworkDatagram datagram = udpSocket->receiveDatagram();
        const QHostAddress sender = datagram.senderAddress();
        const quint16 senderPort = datagram.senderPort();

        if (m_mode == 6)
            handleLab6(QString::fromUtf8(datagram.data()).trimmed(), sender, senderPort);
        else if (m_mode == 7)
            handleLab7(QString::fromUtf8(datagram.data()).trimmed(), sender, senderPort);
    }
}

void UdpServer::handleLab6(const QString &line, const QHostAddress &sender, quint16 senderPort)
{
    const int sep = line.indexOf(':');
    const QString cmd = (sep < 0) ? line : line.left(sep);
    const QString rest = (sep < 0) ? QString() : line.mid(sep + 1);
    const QString id = clientKey(sender, senderPort);

    if (cmd == "HELLO") {
        QString name = rest.trimmed();
        name.remove(':');
        if (name.isEmpty())
            name = QStringLiteral("Клиент");

        const bool isNew = !m_peers.contains(id);
        m_peers.insert(id, qMakePair(sender, senderPort));
        m_peerNames.insert(id, name);

        if (isNew) {
            cout << "[ЛР6] Клиент «" << name.toStdString() << "» зарегистрирован ("
                 << id.toStdString() << ')' << endl;
            broadcastLab6("CHAT:СИСТЕМА:" + name + " подключился", sender, senderPort);
        }
        sendLine("WELCOME:Сервер", sender, senderPort);
        return;
    }

    if (cmd == "BYE") {
        if (m_peers.contains(id)) {
            const QString name = m_peerNames.value(id, QStringLiteral("Клиент"));
            m_peers.remove(id);
            m_peerNames.remove(id);
            cout << "[ЛР6] Клиент «" << name.toStdString() << "» отключился" << endl;
            broadcastLab6("CHAT:СИСТЕМА:" + name + " отключился", sender, senderPort);
        }
        return;
    }

    if (cmd == "MSG") {
        const QString name = m_peerNames.value(id, QStringLiteral("Клиент"));
        cout << "[ЛР6] [" << name.toStdString() << "] " << rest.toStdString() << endl;
        broadcastLab6("CHAT:" + name + ':' + rest, sender, senderPort);
        return;
    }

    cout << "[ЛР6] Неизвестный пакет: " << line.toStdString() << endl;
}

void UdpServer::handleLab7(const QString &msg, const QHostAddress &sender, quint16 senderPort)
{
    const QString id = clientKey(sender, senderPort);

    if (msg == "HELLO" || msg == "NEXT") {
        issueTask(sender, senderPort, id);
    } else if (msg == "RETRY") {
        resendTask(sender, senderPort, id);
    } else if (msg.startsWith("FOUND")) {
        if (m_found)
            return;

        m_found = true;
        QString key = msg.mid(QString("FOUND").size()).trimmed();
        if (key.startsWith(':'))
            key = key.mid(1).trimmed();

        cout << "[ЛР7] Клиент " << id.toStdString() << " нашел ключ: " << key.toStdString() << endl;
        cout << "[ЛР7] Поиск завершен." << endl;
    }
}

void UdpServer::issueTask(const QHostAddress &sender, quint16 senderPort, const QString &clientId)
{
    if (m_found || m_nextKeyStart > m_maxKey) {
        sendLine("NO_MORE_TASKS", sender, senderPort);
        return;
    }

    int end = m_nextKeyStart + m_keyStep - 1;
    if (end > m_maxKey)
        end = m_maxKey;

    const QString task = kCipherText + "|" + QString::number(m_nextKeyStart)
                         + "|" + QString::number(end);
    sendLine(task, sender, senderPort);
    m_lastTask.insert(clientId, qMakePair(m_nextKeyStart, end));

    cout << "[ЛР7] Выдан диапазон: " << m_nextKeyStart << " - " << end << endl;
    m_nextKeyStart += m_keyStep;
}

void UdpServer::resendTask(const QHostAddress &sender, quint16 senderPort, const QString &clientId)
{
    if (m_found) {
        sendLine("NO_MORE_TASKS", sender, senderPort);
        return;
    }

    const auto it = m_lastTask.constFind(clientId);
    if (it == m_lastTask.constEnd()) {
        // Клиент не получил ни одного задания — считаем RETRY повторным запросом.
        issueTask(sender, senderPort, clientId);
        return;
    }

    const QString task = kCipherText + "|" + QString::number(it->first)
                         + "|" + QString::number(it->second);
    sendLine(task, sender, senderPort);
    cout << "[ЛР7] Повторно выдан диапазон: " << it->first << " - " << it->second << endl;
}

void UdpServer::broadcastLab6(const QString &text,
                              const QHostAddress &exceptAddress,
                              quint16 exceptPort)
{
    const auto peers = m_peers;
    for (auto it = peers.constBegin(); it != peers.constEnd(); ++it) {
        const QPair<QHostAddress, quint16> &peer = it.value();
        if (!exceptAddress.isNull()
            && peer.first == exceptAddress
            && peer.second == exceptPort) {
            continue;
        }
        sendLine(text, peer.first, peer.second);
    }
}

void UdpServer::sendLine(const QString &text, const QHostAddress &sender, quint16 senderPort)
{
    udpSocket->writeDatagram(text.toUtf8(), sender, senderPort);
}

QString UdpServer::clientKey(const QHostAddress &sender, quint16 senderPort)
{
    return sender.toString() + ':' + QString::number(senderPort);
}
