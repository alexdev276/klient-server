#include "udpchat.h"

#include <QCloseEvent>
#include <QDateTime>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>

UdpChat::UdpChat(Mode mode, QWidget *parent)
    : QWidget(parent),
      m_mode(mode),
      m_socket(new QUdpSocket(this)),
      m_port(5555),
      m_serverKnown(false),
      m_serverPort(5555),
      m_regTimer(new QTimer(this))
{
    const bool isServer = (m_mode == ServerMode);
    setWindowTitle(isServer ? "ЛР 6 — UDP сервер" : "ЛР 6 — UDP клиент");
    resize(640, 480);

    m_name = isServer ? "Сервер" : "Клиент";
    m_nameEdit = new QLineEdit(m_name, this);
    m_ipEdit   = new QLineEdit("127.0.0.1", this);
    m_ipEdit->setEnabled(!isServer);
    m_connectBtn = new QPushButton(isServer ? "Слушать UDP :5555"
                                            : "Переподключиться", this);
    m_connectBtn->setEnabled(!isServer);

    m_logView = new QPlainTextEdit(this);
    m_logView->setReadOnly(true);

    m_msgEdit = new QLineEdit(this);
    m_msgEdit->setPlaceholderText("Сообщение...");
    m_sendBtn = new QPushButton("Отправить", this);

    m_statusLabel = new QLabel(this);

    QFormLayout *form = new QFormLayout;
    form->addRow("Имя:", m_nameEdit);
    form->addRow("IP сервера:", m_ipEdit);

    QHBoxLayout *top = new QHBoxLayout;
    top->addLayout(form, 1);
    top->addWidget(m_connectBtn);

    QGroupBox *cfgBox = new QGroupBox("Подключение", this);
    cfgBox->setLayout(top);

    QHBoxLayout *inputRow = new QHBoxLayout;
    inputRow->addWidget(m_msgEdit, 1);
    inputRow->addWidget(m_sendBtn);

    QVBoxLayout *main = new QVBoxLayout(this);
    main->addWidget(cfgBox);
    main->addWidget(m_logView, 1);
    main->addWidget(m_statusLabel);
    main->addLayout(inputRow);

    connect(m_socket, &QUdpSocket::readyRead, this, &UdpChat::onReadyRead);
    connect(m_sendBtn, &QPushButton::clicked, this, &UdpChat::onSendClicked);
    connect(m_msgEdit, &QLineEdit::returnPressed, this, &UdpChat::onSendClicked);
    connect(m_connectBtn, &QPushButton::clicked, this, &UdpChat::onReconnectClicked);
    connect(m_regTimer, &QTimer::timeout, this, &UdpChat::onRegisterTick);

    if (isServer) {
        const bool ok = m_socket->bind(QHostAddress::AnyIPv4, m_port,
                                       QUdpSocket::ShareAddress |
                                           QUdpSocket::ReuseAddressHint);
        if (!ok) {
            log(QString("ОШИБКА: не удалось занять UDP порт %1: %2")
                    .arg(m_port).arg(m_socket->errorString()));
        } else {
            log(QString("UDP сервер слушает порт %1.").arg(m_port));
            log("Ожидание клиентов...");
        }
    } else {
        m_socket->bind(QHostAddress::AnyIPv4, 0);
        log(QString("Локальный UDP порт: %1").arg(m_socket->localPort()));
        log(QString("Ищу сервер на %1:%2 каждые 2 секунды...")
                .arg(m_ipEdit->text()).arg(m_port));
        m_regTimer->start(2000);
        QTimer::singleShot(0, this, &UdpChat::onRegisterTick);
    }
    refreshStatus();
}

void UdpChat::log(const QString &text)
{
    m_logView->appendPlainText(
        QString("[%1] %2")
            .arg(QTime::currentTime().toString("hh:mm:ss"), text));
}

QString UdpChat::peerKey(const QHostAddress &addr, quint16 port) const
{
    return addr.toString() + ":" + QString::number(port);
}

void UdpChat::refreshStatus()
{
    if (m_mode == ServerMode) {
        m_statusLabel->setText(QString("Режим: сервер | клиентов: %1 | порт: %2")
                                   .arg(m_peers.size()).arg(m_port));
    } else {
        m_statusLabel->setText(
            m_serverKnown
                ? QString("Режим: клиент | сервер: %1:%2 | мой порт: %3")
                      .arg(m_serverAddr.toString())
                      .arg(m_serverPort)
                      .arg(m_socket->localPort())
                : QString("Режим: клиент | сервер не найден | мой порт: %1")
                      .arg(m_socket->localPort()));
    }
}

void UdpChat::sendLine(const QString &line,
                       const QHostAddress &addr, quint16 port)
{
    if (addr.isNull() || port == 0) return;
    m_socket->writeDatagram(line.toUtf8(), addr, port);
}

void UdpChat::sendToServer(const QString &line)
{
    if (!m_serverKnown) return;
    sendLine(line, m_serverAddr, m_serverPort);
}

void UdpChat::broadcastToPeers(const QString &line,
                               const QHostAddress &exceptAddr,
                               quint16 exceptPort)
{
    for (auto it = m_peers.constBegin(); it != m_peers.constEnd(); ++it) {
        const QPair<QHostAddress, quint16> &p = it.value();
        if (!exceptAddr.isNull() && p.first == exceptAddr && p.second == exceptPort)
            continue;
        sendLine(line, p.first, p.second);
    }
}

void UdpChat::onReconnectClicked()
{
    m_serverKnown = false;
    m_regTimer->start(2000);
    onRegisterTick();
}

void UdpChat::onRegisterTick()
{
    if (m_mode == ServerMode) return;

    const QString name = m_nameEdit->text().trimmed().isEmpty()
                             ? QString("Клиент")
                             : m_nameEdit->text().trimmed();
    m_name = name;

    // HELLO отправляем на указанный адрес и на localhost — удобно
    // и для запуска на одном компьютере, и для работы по сети.
    const QHostAddress target(m_ipEdit->text().trimmed());
    if (!target.isNull()) {
        sendLine("HELLO:" + name, target, m_port);
        if (target != QHostAddress::LocalHost)
            sendLine("HELLO:" + name, QHostAddress::LocalHost, m_port);
    } else {
        sendLine("HELLO:" + name, QHostAddress::LocalHost, m_port);
    }
}

void UdpChat::onSendClicked()
{
    const QString text = m_msgEdit->text().trimmed();
    if (text.isEmpty()) return;
    m_msgEdit->clear();

    const QString name = m_nameEdit->text().trimmed().isEmpty()
                             ? m_name
                             : m_nameEdit->text().trimmed();
    m_name = name;

    if (m_mode == ServerMode) {
        const QString packet = "CHAT:" + name + ":" + text;
        log("[я] " + text);
        broadcastToPeers(packet);
    } else {
        log("[я] " + text);
        sendToServer("MSG:" + text);
    }
}

void UdpChat::onReadyRead()
{
    while (m_socket->hasPendingDatagrams()) {
        QByteArray data;
        data.resize(int(m_socket->pendingDatagramSize()));
        QHostAddress senderAddr;
        quint16 senderPort = 0;
        m_socket->readDatagram(data.data(), data.size(),
                               &senderAddr, &senderPort);
        const QString line = QString::fromUtf8(data).trimmed();
        if (!line.isEmpty())
            processPacket(line, senderAddr, senderPort);
    }
}

void UdpChat::processPacket(const QString &line,
                            const QHostAddress &sender, quint16 senderPort)
{
    const int sep = line.indexOf(':');
    const QString cmd  = (sep < 0) ? line : line.left(sep);
    const QString rest = (sep < 0) ? QString() : line.mid(sep + 1);

    if (m_mode == ServerMode) {
        const QString key = peerKey(sender, senderPort);

        if (cmd == "HELLO") {
            const QString name = rest.isEmpty() ? "Клиент" : rest;
            const bool isNew = !m_peers.contains(key);
            m_peers.insert(key, qMakePair(sender, senderPort));
            m_peerNames.insert(key, name);

            if (isNew) {
                log(QString("Клиент «%1» зарегистрирован (%2:%3)")
                        .arg(name, sender.toString())
                        .arg(senderPort));
                broadcastToPeers("CHAT:СИСТЕМА:" + name + " подключился",
                                 sender, senderPort);
            }
            sendLine("WELCOME:" + m_nameEdit->text(), sender, senderPort);
            refreshStatus();
            return;
        }

        if (cmd == "BYE") {
            if (m_peers.contains(key)) {
                const QString name = m_peerNames.value(key, "Клиент");
                m_peers.remove(key);
                m_peerNames.remove(key);
                log(QString("Клиент «%1» отключился").arg(name));
                broadcastToPeers("CHAT:СИСТЕМА:" + name + " отключился",
                                 sender, senderPort);
                refreshStatus();
            }
            return;
        }

        if (cmd == "MSG") {
            const QString name = m_peerNames.value(key, "Клиент");
            log(QString("[%1] %2").arg(name, rest));
            broadcastToPeers("CHAT:" + name + ":" + rest, sender, senderPort);
            return;
        }

        log("Неизвестный пакет от клиента: " + line);
        return;
    }

    // ---- Клиент ----
    if (cmd == "WELCOME") {
        if (!m_serverKnown) {
            m_serverKnown = true;
            m_serverAddr  = sender;
            m_serverPort  = senderPort;
            m_regTimer->stop();
            log(QString("Сервер найден: %1:%2")
                    .arg(sender.toString()).arg(senderPort));
            refreshStatus();
        }
        return;
    }

    if (cmd == "CHAT") {
        const int p = rest.indexOf(':');
        const QString author = (p < 0) ? "Сервер" : rest.left(p);
        const QString text   = (p < 0) ? rest : rest.mid(p + 1);
        if (author == "СИСТЕМА")
            log("— " + text);
        else
            log(QString("[%1] %2").arg(author, text));
        return;
    }

    log("<- " + line);
}

void UdpChat::closeEvent(QCloseEvent *event)
{
    if (m_mode == ClientMode && m_serverKnown)
        sendToServer("BYE");
    QWidget::closeEvent(event);
}
