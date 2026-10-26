#include "keysearchclient.h"
#include "vigenere.h"

#include <QAtomicInt>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QProgressBar>
#include <QPushButton>
#include <QTextStream>
#include <QTime>
#include <QTimer>
#include <QVBoxLayout>
#include <QtConcurrent/QtConcurrent>

static const quint16 DEFAULT_DISCOVER_PORT = 50050;
static const char   *MARKER               = "is120";

KeySearchClient::KeySearchClient(QWidget *parent)
    : QWidget(parent),
      m_socket(new QUdpSocket(this)),
      m_serverPort(50055),
      m_discoverPort(DEFAULT_DISCOVER_PORT),
      m_serverKnown(false),
      m_searching(false),
      m_stoppedByServer(false),
      m_taskId(-1),
      m_keyFrom(0),
      m_keyTo(0),
      m_stopFlag(0),
      m_watcher(new QFutureWatcher<KeySearchResult>(this)),
      m_findTimer(new QTimer(this))
{
    setWindowTitle("ЛР 7 — клиент-дешифровщик (UDP)");
    resize(720, 560);

    m_ipEdit = new QLineEdit(this);
    m_ipEdit->setPlaceholderText("IP сервера (пусто = широковещательный поиск)");

    m_findBtn = new QPushButton("Найти сервер", this);
    m_stopBtn = new QPushButton("Остановить перебор", this);
    m_stopBtn->setEnabled(false);

    m_statusLabel = new QLabel(this);
    m_progress = new QProgressBar(this);
    m_progress->setRange(0, 1);
    m_progress->setValue(0);
    m_progress->setFormat("ожидание");

    m_logView = new QPlainTextEdit(this);
    m_logView->setReadOnly(true);

    QHBoxLayout *top = new QHBoxLayout;
    top->addWidget(m_ipEdit, 1);
    top->addWidget(m_findBtn);

    QGroupBox *netBox = new QGroupBox("Связь с сервером", this);
    netBox->setLayout(top);

    QHBoxLayout *buttons = new QHBoxLayout;
    buttons->addWidget(m_stopBtn);
    buttons->addStretch(1);

    QVBoxLayout *main = new QVBoxLayout(this);
    main->addWidget(netBox);
    main->addLayout(buttons);
    main->addWidget(m_statusLabel);
    main->addWidget(m_progress);
    main->addWidget(m_logView, 1);

    connect(m_socket, &QUdpSocket::readyRead, this, &KeySearchClient::onReadyRead);
    connect(m_findBtn, &QPushButton::clicked, this, &KeySearchClient::onFindClicked);
    connect(m_stopBtn, &QPushButton::clicked, this, &KeySearchClient::onStopClicked);
    connect(m_watcher, &QFutureWatcher<KeySearchResult>::finished,
            this, &KeySearchClient::onSearchFinished);
    connect(m_findTimer, &QTimer::timeout, this, &KeySearchClient::onDiscoveryTick);

    m_socket->bind(QHostAddress::AnyIPv4, 0);
    log(QString("Клиент запущен, локальный UDP порт %1")
            .arg(m_socket->localPort()));
    updateStatus();
}

KeySearchClient::~KeySearchClient()
{
    m_stopFlag.storeRelease(1);
    if (m_watcher->isRunning())
        m_watcher->waitForFinished();
}

QString KeySearchClient::b64(const QString &s)
{
    return QString::fromLatin1(s.toUtf8().toBase64());
}

QString KeySearchClient::unb64(const QString &s)
{
    return QString::fromUtf8(QByteArray::fromBase64(s.toLatin1()));
}

void KeySearchClient::log(const QString &text)
{
    const QString line = QString("[%1] %2")
                             .arg(QTime::currentTime().toString("hh:mm:ss"), text);
    m_logView->appendPlainText(line);
    QTextStream(stdout) << line << "\n";
}

void KeySearchClient::updateStatus()
{
    if (m_searching)
        m_statusLabel->setText(
            QString("Перебор ключей %1–%2 (задание %3) в отдельном потоке...")
                .arg(m_keyFrom).arg(m_keyTo - 1).arg(m_taskId));
    else if (m_serverKnown)
        m_statusLabel->setText(
            QString("Подключён к %1:%2 | ожидание задания")
                .arg(m_serverAddr.toString()).arg(m_serverPort));
    else
        m_statusLabel->setText(
            QString("Сервер не найден | мой порт %1").arg(m_socket->localPort()));
}

void KeySearchClient::onFindClicked()
{
    if (m_socket->state() != QAbstractSocket::BoundState) {
        m_socket->bind(QHostAddress::AnyIPv4, 0);
        log(QString("Локальный UDP порт: %1").arg(m_socket->localPort()));
    }

    m_serverKnown = false;
    m_stoppedByServer = false;

    log(QString("Ищу сервер на порту %1...").arg(m_discoverPort));
    onDiscoveryTick();
    m_findTimer->start(2000);
    updateStatus();
}

void KeySearchClient::onDiscoveryTick()
{
    const QString packet = QString("KMZI_DISCOVER:%1").arg(m_socket->localPort());
    const QString ip = m_ipEdit->text().trimmed();

    if (!ip.isEmpty()) {
        QHostAddress target(ip);
        if (!target.isNull())
            m_socket->writeDatagram(packet.toUtf8(), target, m_discoverPort);
    } else {
        m_socket->writeDatagram(packet.toUtf8(),
                                QHostAddress::Broadcast, m_discoverPort);
        m_socket->writeDatagram(packet.toUtf8(),
                                QHostAddress::LocalHost, m_discoverPort);
    }
}

void KeySearchClient::sendToServer(const QString &line)
{
    if (!m_serverKnown) return;
    m_socket->writeDatagram(line.toUtf8(), m_serverAddr, m_serverPort);
}

void KeySearchClient::onReadyRead()
{
    while (m_socket->hasPendingDatagrams()) {
        QByteArray data;
        data.resize(int(m_socket->pendingDatagramSize()));
        QHostAddress addr;
        quint16 port = 0;
        m_socket->readDatagram(data.data(), data.size(), &addr, &port);
        const QString line = QString::fromUtf8(data).trimmed();
        if (line.isEmpty()) continue;

        const int sep = line.indexOf(':');
        const QString cmd  = (sep < 0) ? line : line.left(sep);
        const QString rest = (sep < 0) ? QString() : line.mid(sep + 1);

        if (cmd == "KMZI_SERVER") {
            if (!m_serverKnown) {
                m_serverKnown = true;
                m_serverAddr  = addr;
                m_serverPort  = rest.toUShort();
                m_findTimer->stop();
                log(QString("Сервер найден: %1, порт обмена %2")
                        .arg(addr.toString()).arg(m_serverPort));
                sendToServer(QString("KMZI_CONFIRM:%1").arg(m_socket->localPort()));
                log("Отправлено подтверждение связи, ждём задание.");
                updateStatus();
            }
            continue;
        }

        if (cmd == "KMZI_TASK") {
            const QStringList parts = rest.split(':');
            if (parts.size() < 6) {
                log("Некорректное задание: " + line);
                continue;
            }
            const int taskId = parts[0].toInt();
            const QString alphabet = unb64(parts[1]);
            const QString cipher   = unb64(parts[2]);
            m_keyFrom = parts[3].toInt();
            m_keyTo   = parts[4].toInt();
            const QString keysCsv = parts.mid(5).join(':');

            m_keys.clear();
            if (!keysCsv.isEmpty()) {
                const QStringList encoded = keysCsv.split(',');
                for (const QString &k : encoded)
                    m_keys.append(unb64(k));
            }

            m_taskId = taskId;
            m_stoppedByServer = false;
            m_stopFlag.storeRelease(0);

            log(QString("Задание %1: ключи [%2; %3), %4 шт.")
                    .arg(taskId).arg(m_keyFrom).arg(m_keyTo)
                    .arg(m_keys.size()));

            sendToServer(QString("KMZI_ACK:%1").arg(taskId));
            startSearch(alphabet, cipher, m_keys);
            continue;
        }

        if (cmd == "KMZI_STOP") {
            m_stopFlag.storeRelease(1);
            m_stoppedByServer = true;
            if (rest.isEmpty())
                log("Сервер остановил поиск.");
            else
                log(QString("Сервер сообщил: ключ найден (%1), останавливаю перебор.")
                        .arg(unb64(rest)));
            continue;
        }

        if (cmd == "KMZI_WAIT") {
            log("Сервер пока не начал поиск. Нажмите «Начать поиск» на сервере.");
            m_statusLabel->setText("Подключён, ожидание старта поиска на сервере");
            continue;
        }

        if (cmd == "KMZI_DONE") {
            if (!m_searching) {
                log("Сервер: задания закончились, ключ не найден.");
                m_statusLabel->setText("Поиск завершён: ключ не найден");
                m_progress->setRange(0, 1);
                m_progress->setValue(0);
                m_progress->setFormat("не найден");
                m_stopBtn->setEnabled(false);
            }
            continue;
        }

        log("<- " + line);
    }
}

void KeySearchClient::startSearch(const QString &alphabet,
                                  const QString &cipher,
                                  const QStringList &keys)
{
    if (m_searching) {
        log("Предыдущий перебор ещё идёт — пропускаю задание.");
        return;
    }
    if (keys.isEmpty()) {
        log("Пустой набор ключей.");
        return;
    }

    m_alphabet = alphabet;
    m_cipher   = cipher;
    m_searching = true;
    m_stopBtn->setEnabled(true);
    m_progress->setRange(0, 0);          // «бегущая» полоса
    m_progress->setFormat("перебор...");
    updateStatus();

    const Vigenere vig(alphabet);
    const QString cipherCopy = cipher;
    const QStringList keysCopy = keys;
    QAtomicInt *stop = &m_stopFlag;
    const QString marker = QString::fromLatin1(MARKER);

    log(QString("Запуск перебора %1 ключей в отдельном потоке...")
            .arg(keysCopy.size()));

    m_watcher->setFuture(QtConcurrent::run([vig, cipherCopy, keysCopy, stop, marker]() {
        KeySearchResult res;
        for (const QString &key : keysCopy) {
            if (stop->loadAcquire())
                break;
            const QString plain = vig.decrypt(cipherCopy, key);
            ++res.checked;
            if (plain.contains(marker)) {
                res.found = true;
                res.key   = key;
                res.plain = plain;
                return res;
            }
        }
        return res;
    }));
}

void KeySearchClient::onSearchFinished()
{
    const KeySearchResult res = m_watcher->result();
    m_searching = false;
    m_stopBtn->setEnabled(false);
    m_progress->setRange(0, 1);
    m_progress->setValue(res.found ? 1 : 0);
    m_progress->setFormat(res.found ? "ключ найден" : "диапазон проверен");

    if (res.found) {
        log("==============================================");
        log(QString("НАЙДЕНА КЛЮЧЕВАЯ ФРАЗА: %1").arg(res.key));
        log(QString("РАСШИФРОВАННЫЙ ТЕКСТ: %1").arg(res.plain));
        log("==============================================");
        sendToServer(QString("KMZI_RESULT:%1:1:%2:%3")
                         .arg(m_taskId)
                         .arg(b64(res.key), b64(res.plain)));
        m_statusLabel->setText("Ключ найден и отправлен серверу");
    } else if (m_stoppedByServer) {
        log("Перебор остановлен по команде сервера.");
        updateStatus();
    } else {
        log(QString("Диапазон проверен (%1 ключей), ключ не найден.")
                .arg(res.checked));
        sendToServer(QString("KMZI_RESULT:%1:0::").arg(m_taskId));
        updateStatus();
    }
}

void KeySearchClient::onStopClicked()
{
    if (!m_searching) return;
    m_stopFlag.storeRelease(1);
    m_stoppedByServer = true; // не отправляем «не найдено» по ручной остановке
    log("Остановка перебора по команде оператора.");
}
