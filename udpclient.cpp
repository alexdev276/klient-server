// udpclient.cpp
#include "udpclient.h"
#include "workerthread.h"

#include <QCoreApplication>
#include <QNetworkDatagram>
#include <QStringList>
#include <QThreadPool>
#include <QTimer>

#include <iostream>
#include <string>
#include <unistd.h>

using namespace std;

static const int kRetryIntervalMs = 3000;
static const int kMaxAttempts = 5;
static const int kRegisterIntervalMs = 2000;

UdpClient::UdpClient(int mode, const QString &serverHost, quint16 serverPort,
                     const QString &name, QObject *parent)
    : QObject(parent)
    , udpSocket(new QUdpSocket(this))
    , m_mode(mode)
    , m_stdinNotifier(nullptr)
    , m_serverAddress(serverHost)
    , m_serverPort(serverPort)
    , m_retryTimer(nullptr)
    , m_attempts(0)
    , m_name(name.trimmed())
    , m_regTimer(nullptr)
    , m_serverKnown(false)
{
    if (m_serverPort == 0)
        m_serverPort = (m_mode == 6) ? 5555 : 50050;
    if (m_name.isEmpty())
        m_name = QStringLiteral("Клиент");

    if (!udpSocket->bind(QHostAddress::AnyIPv4, 0))
        cerr << "[Клиент] Не удалось занять локальный UDP-порт" << endl;

    connect(udpSocket, &QUdpSocket::readyRead, this, &UdpClient::readPendingDatagrams);

    if (m_mode == 6) {
        m_stdinNotifier = new QSocketNotifier(STDIN_FILENO, QSocketNotifier::Read, this);
        connect(m_stdinNotifier, &QSocketNotifier::activated, this, &UdpClient::onStdinReady);

        m_regTimer = new QTimer(this);
        m_regTimer->setInterval(kRegisterIntervalMs);
        connect(m_regTimer, &QTimer::timeout, this, &UdpClient::onRegisterTick);

        cout << "[ЛР6] Клиент «" << m_name.toStdString() << "» запущен. Сервер: "
             << m_serverAddress.toString().toStdString() << ':' << m_serverPort << endl;
        cout << "[ЛР6] Регистрация на сервере..." << endl;
        cout << "[ЛР6] Введите сообщение (exit для выхода):" << endl;

        QTimer::singleShot(0, this, &UdpClient::onRegisterTick);
    } else {
        m_retryTimer = new QTimer(this);
        m_retryTimer->setSingleShot(true);
        m_retryTimer->setInterval(kRetryIntervalMs);
        connect(m_retryTimer, &QTimer::timeout, this, &UdpClient::onRetryTimeout);
        cout << "[ЛР7] Клиент запущен. Сервер: " << m_serverAddress.toString().toStdString()
             << ':' << m_serverPort << endl;
    }
}

void UdpClient::sendLab6Hello()
{
    udpSocket->writeDatagram(("HELLO:" + m_name).toUtf8(), m_serverAddress, m_serverPort);
}

void UdpClient::sendLab6Bye()
{
    if (m_serverKnown)
        udpSocket->writeDatagram("BYE", m_serverAddress, m_serverPort);
}

void UdpClient::onRegisterTick()
{
    if (m_serverKnown)
        return;
    sendLab6Hello();
}

void UdpClient::sendLab6Message(const QString &msg)
{
    // Если сервер ещё не подтвердил регистрацию, отправляем HELLO перед сообщением,
    // чтобы сервер знал имя отправителя.
    if (!m_serverKnown)
        sendLab6Hello();

    cout << "[ЛР6] [я] " << msg.toStdString() << endl;
    udpSocket->writeDatagram(("MSG:" + msg).toUtf8(), m_serverAddress, m_serverPort);
}

void UdpClient::handleLab6Reply(const QString &msg)
{
    const int sep = msg.indexOf(':');
    const QString cmd = (sep < 0) ? msg : msg.left(sep);
    const QString rest = (sep < 0) ? QString() : msg.mid(sep + 1);

    if (cmd == "WELCOME") {
        if (!m_serverKnown) {
            m_serverKnown = true;
            if (m_regTimer)
                m_regTimer->stop();
            cout << "[ЛР6] Сервер найден: " << m_serverAddress.toString().toStdString()
                 << ':' << m_serverPort << endl;
        }
        return;
    }

    if (cmd == "CHAT") {
        const int p = rest.indexOf(':');
        const QString author = (p < 0) ? QStringLiteral("Сервер") : rest.left(p);
        const QString text = (p < 0) ? rest : rest.mid(p + 1);

        if (author == QStringLiteral("СИСТЕМА"))
            cout << "[ЛР6] — " << text.toStdString() << endl;
        else
            cout << "[ЛР6] [" << author.toStdString() << "] " << text.toStdString() << endl;
        return;
    }

    cout << "[ЛР6] Сервер: " << msg.toStdString() << endl;
}

void UdpClient::requestLab7Task()
{
    cout << "[ЛР7] Отправка запроса HELLO..." << endl;
    sendRequest("HELLO");
}

void UdpClient::sendRequest(const QString &msg)
{
    udpSocket->writeDatagram(msg.toUtf8(), m_serverAddress, m_serverPort);
    ++m_attempts;

    if (m_retryTimer)
        m_retryTimer->start();
}

void UdpClient::onRetryTimeout()
{
    if (m_attempts >= kMaxAttempts) {
        cout << "[ЛР7] Сервер не отвечает, попытки исчерпаны. Выход." << endl;
        QCoreApplication::quit();
        return;
    }

    cout << "[ЛР7] Нет ответа от сервера, повторный запрос ("
         << m_attempts << '/' << kMaxAttempts << ")..." << endl;
    sendRequest("RETRY");
}

void UdpClient::onStdinReady()
{
    string line;
    if (!getline(cin, line)) {
        sendLab6Bye();
        QCoreApplication::quit();
        return;
    }

    const QString msg = QString::fromStdString(line).trimmed();
    if (msg == "exit") {
        sendLab6Bye();
        QCoreApplication::quit();
        return;
    }
    if (msg.isEmpty())
        return;

    sendLab6Message(msg);
}

void UdpClient::reportFound(const QString &key)
{
    if (m_retryTimer)
        m_retryTimer->stop();

    udpSocket->writeDatagram(QString("FOUND: " + key).toUtf8(), m_serverAddress, m_serverPort);
    cout << "[ЛР7] Результат отправлен серверу." << endl;

    QTimer::singleShot(200, [] { QCoreApplication::quit(); });
}

void UdpClient::readPendingDatagrams()
{
    while (udpSocket->hasPendingDatagrams()) {
        const QNetworkDatagram datagram = udpSocket->receiveDatagram();
        const QString msg = QString::fromUtf8(datagram.data()).trimmed();

        if (m_mode == 6) {
            handleLab6Reply(msg);
            continue;
        }

        if (m_retryTimer)
            m_retryTimer->stop();
        m_attempts = 0;

        if (msg == "NO_MORE_TASKS") {
            cout << "[ЛР7] Сервер сообщил, что задания закончились. Выход." << endl;
            QCoreApplication::quit();
            return;
        }

        const QStringList parts = msg.split('|');
        if (parts.size() != 3) {
            cout << "[ЛР7] Некорректное задание от сервера, пропущено." << endl;
            continue;
        }

        bool startOk = false;
        bool endOk = false;
        const QString cipherText = parts[0];
        const int startRange = parts[1].toInt(&startOk);
        const int endRange = parts[2].toInt(&endOk);

        if (!startOk || !endOk || startRange > endRange) {
            cout << "[ЛР7] Некорректный диапазон ключей, задание пропущено." << endl;
            continue;
        }

        cout << "[ЛР7] Получено задание. Диапазон: " << startRange << " - " << endRange << endl;

        auto *worker = new WorkerThread(cipherText, startRange, endRange);
        connect(worker, &WorkerThread::keyFound, this, [this](const QString &key, const QString &text) {
            cout << "[ЛР7] КЛЮЧ НАЙДЕН: " << key.toStdString() << endl;
            cout << "[ЛР7] Текст: " << text.toStdString() << endl;
            reportFound(key);
        });
        connect(worker, &WorkerThread::finishedWithoutResult, this, [this]() {
            cout << "[ЛР7] В данном диапазоне ключ не найден. Запрос следующего." << endl;
            sendRequest("NEXT");
        });
        QThreadPool::globalInstance()->start(worker);
    }
}
