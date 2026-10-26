#include "keysearchserver.h"
#include "vigenere.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSettings>
#include <QTextStream>
#include <QTime>
#include <QVBoxLayout>
#include <QFileInfo>
#include <algorithm>

static const quint16 DEFAULT_DISCOVER_PORT = 50050;
static const quint16 DEFAULT_EXCHANGE_PORT = 50055;
static const int     DEFAULT_CHUNK_SIZE    = 200;
static const char   *MARKER               = "is120";

KeySearchServer::KeySearchServer(QWidget *parent)
    : QWidget(parent),
      m_discovery(new QUdpSocket(this)),
      m_exchange(new QUdpSocket(this)),
      m_discoverPort(DEFAULT_DISCOVER_PORT),
      m_exchangePort(DEFAULT_EXCHANGE_PORT),
      m_chunkSize(DEFAULT_CHUNK_SIZE),
      m_searching(false),
      m_found(false),
      m_nextKey(0),
      m_nextTaskId(1),
      m_checkedKeys(0)
{
    setWindowTitle("ЛР 7 — сервер подбора ключа (UDP)");
    resize(820, 720);

    loadConfig();

    m_alphabetEdit = new QPlainTextEdit(this);
    m_alphabetEdit->setPlainText(Vigenere::defaultAlphabet());
    m_alphabetEdit->setMaximumHeight(80);

    m_cipherEdit = new QPlainTextEdit(this);
    m_cipherEdit->setPlaceholderText("Вставьте кодированную комбинацию...");
    m_cipherEdit->setMaximumHeight(90);

    m_keysPathEdit = new QLineEdit(m_keysFile, this);
    m_loadKeysBtn = new QPushButton("Загрузить ключи", this);

    m_portsLabel = new QLabel(this);
    m_portsLabel->setText(QString("Порты из конфигурации: обнаружение %1, обмен %2, "
                                  "ключей в наборе %3")
                              .arg(m_discoverPort).arg(m_exchangePort)
                              .arg(m_keys.size()));

    m_startBtn = new QPushButton("Начать поиск", this);
    m_stopBtn  = new QPushButton("Остановить", this);
    m_stopBtn->setEnabled(false);

    m_statusLabel = new QLabel(this);
    m_logView = new QPlainTextEdit(this);
    m_logView->setReadOnly(true);

    QFormLayout *form = new QFormLayout;
    form->addRow("Алфавит:", m_alphabetEdit);
    form->addRow("Кодированная комбинация:", m_cipherEdit);

    QHBoxLayout *keysRow = new QHBoxLayout;
    keysRow->addWidget(m_keysPathEdit, 1);
    keysRow->addWidget(m_loadKeysBtn);

    QHBoxLayout *buttons = new QHBoxLayout;
    buttons->addWidget(m_startBtn);
    buttons->addWidget(m_stopBtn);
    buttons->addStretch(1);

    QVBoxLayout *cfg = new QVBoxLayout;
    cfg->addLayout(form);
    cfg->addWidget(new QLabel("Файл ключевых фраз:", this));
    cfg->addLayout(keysRow);
    cfg->addWidget(m_portsLabel);
    cfg->addLayout(buttons);

    QGroupBox *cfgBox = new QGroupBox("Параметры поиска", this);
    cfgBox->setLayout(cfg);

    QVBoxLayout *main = new QVBoxLayout(this);
    main->addWidget(cfgBox);
    main->addWidget(m_statusLabel);
    main->addWidget(m_logView, 1);

    connect(m_discovery, &QUdpSocket::readyRead,
            this, &KeySearchServer::onDiscoveryRead);
    connect(m_exchange, &QUdpSocket::readyRead,
            this, &KeySearchServer::onExchangeRead);
    connect(m_startBtn, &QPushButton::clicked,
            this, &KeySearchServer::onStartClicked);
    connect(m_stopBtn, &QPushButton::clicked,
            this, &KeySearchServer::onStopClicked);
    connect(m_loadKeysBtn, &QPushButton::clicked,
            this, &KeySearchServer::onLoadKeysClicked);

    const bool ok1 = m_discovery->bind(QHostAddress::AnyIPv4, m_discoverPort,
                                       QUdpSocket::ShareAddress |
                                           QUdpSocket::ReuseAddressHint);
    const bool ok2 = m_exchange->bind(QHostAddress::AnyIPv4, m_exchangePort,
                                      QUdpSocket::ShareAddress |
                                          QUdpSocket::ReuseAddressHint);

    if (!ok1)
        log(QString("ОШИБКА: порт обнаружения %1 не занят: %2")
                .arg(m_discoverPort).arg(m_discovery->errorString()));
    else if (!ok2)
        log(QString("ОШИБКА: порт обмена %1 не занят: %2")
                .arg(m_exchangePort).arg(m_exchange->errorString()));
    else {
        log("==============================================");
        log("=== ЛР 7: СЕРВЕР ПОДБОРА КЛЮЧА ЗАПУЩЕН ===");
        log(QString("UDP обнаружение: %1, UDP обмен: %2")
                .arg(m_discoverPort).arg(m_exchangePort));
        log(QString("Ключевых фраз загружено: %1").arg(m_keys.size()));
        log("==============================================");
    }
    updateStatus();
}

KeySearchServer::~KeySearchServer() = default;

QString KeySearchServer::clientKey(const QHostAddress &addr, quint16 port)
{
    return addr.toString() + ":" + QString::number(port);
}

QString KeySearchServer::b64(const QString &s)
{
    return QString::fromLatin1(s.toUtf8().toBase64());
}

QString KeySearchServer::unb64(const QString &s)
{
    return QString::fromUtf8(QByteArray::fromBase64(s.toLatin1()));
}

void KeySearchServer::log(const QString &text)
{
    const QString line = QString("[%1] %2")
                             .arg(QTime::currentTime().toString("hh:mm:ss"), text);
    if (m_logView) m_logView->appendPlainText(line);
    QTextStream(stdout) << line << "\n";
}

void KeySearchServer::loadConfig()
{
    const QString path = QDir(QCoreApplication::applicationDirPath())
                             .filePath("kmzi.ini");
    QSettings s(path, QSettings::IniFormat);

    m_discoverPort = quint16(s.value("network/discoverPort",
                                     DEFAULT_DISCOVER_PORT).toUInt());
    m_exchangePort = quint16(s.value("network/exchangePort",
                                     DEFAULT_EXCHANGE_PORT).toUInt());
    m_chunkSize    = s.value("search/chunkSize", DEFAULT_CHUNK_SIZE).toInt();
    if (m_chunkSize < 1) m_chunkSize = 1;

    QString keysName = s.value("search/keysFile", "keys.txt").toString();

    if (!QFileInfo::exists(path)) {
        s.setValue("network/discoverPort", m_discoverPort);
        s.setValue("network/exchangePort", m_exchangePort);
        s.setValue("search/chunkSize", m_chunkSize);
        s.setValue("search/keysFile", keysName);
        s.sync();
    }

    QString full = keysName;
    if (QFileInfo(full).isRelative()) {
        const QString inApp = QDir(QCoreApplication::applicationDirPath())
                                  .filePath(keysName);
        const QString inCwd = QDir::current().filePath(keysName);
        if (QFileInfo::exists(inApp))      full = inApp;
        else if (QFileInfo::exists(inCwd)) full = inCwd;
        else                               full = inApp;
    }
    m_keysFile = full;

    QString err;
    if (!loadKeys(m_keysFile, &err))
        log("Ключи не загружены: " + err);
}

bool KeySearchServer::loadKeys(const QString &path, QString *error)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (error) *error = QString("не удалось открыть %1").arg(path);
        return false;
    }
    m_keys.clear();
    QTextStream in(&f);
    in.setEncoding(QStringConverter::Utf8);
    while (!in.atEnd()) {
        const QString line = in.readLine().trimmed();
        if (line.isEmpty() || line.startsWith('#'))
            continue;
        m_keys.append(line);
    }
    if (m_keys.isEmpty()) {
        if (error) *error = "файл не содержит ключевых фраз";
        return false;
    }
    if (error) error->clear();
    if (m_portsLabel)
        m_portsLabel->setText(QString("Порты из конфигурации: обнаружение %1, "
                                      "обмен %2, ключей в наборе %3")
                                  .arg(m_discoverPort).arg(m_exchangePort)
                                  .arg(m_keys.size()));
    return true;
}

void KeySearchServer::onLoadKeysClicked()
{
    QString err;
    if (loadKeys(m_keysPathEdit->text().trimmed(), &err)) {
        m_keysFile = m_keysPathEdit->text().trimmed();
        log(QString("Загружено ключевых фраз: %1").arg(m_keys.size()));
    } else {
        log("ОШИБКА загрузки ключей: " + err);
    }
}

void KeySearchServer::onStartClicked()
{
    if (m_searching) {
        log("Поиск уже выполняется.");
        return;
    }

    Vigenere vig(m_alphabetEdit->toPlainText());
    if (!vig.isValid()) {
        log("ОШИБКА: пустой алфавит.");
        return;
    }
    m_alphabet = vig.alphabet();

    QString cipher = m_cipherEdit->toPlainText();
    cipher.replace("\r", "");
    if (cipher.trimmed().isEmpty()) {
        log("ОШИБКА: не задана кодированная комбинация.");
        return;
    }
    m_cipher = cipher;

    if (m_keys.isEmpty()) {
        QString err;
        if (!loadKeys(m_keysPathEdit->text().trimmed(), &err)) {
            log("ОШИБКА: нет ключевых фраз: " + err);
            return;
        }
    }

    m_searching   = true;
    m_found       = false;
    m_nextKey     = 0;
    m_nextTaskId  = 1;
    m_checkedKeys = 0;
    m_foundKey.clear();

    log("==============================================");
    log(QString("Старт поиска: ключей %1, размер задания %2, маркер «%3»")
            .arg(m_keys.size()).arg(m_chunkSize).arg(MARKER));
    log(QString("Клиентов на связи: %1").arg(m_clients.size()));

    m_startBtn->setEnabled(false);
    m_stopBtn->setEnabled(true);
    updateStatus();

    const QStringList keys = m_clients.keys();
    for (const QString &k : keys)
        assignTask(k);
}

void KeySearchServer::onStopClicked()
{
    if (!m_searching) return;
    m_searching = false;
    broadcastStop(QString());
    log("Поиск остановлен оператором.");
    m_startBtn->setEnabled(true);
    m_stopBtn->setEnabled(false);
    updateStatus();
}

void KeySearchServer::onDiscoveryRead()
{
    while (m_discovery->hasPendingDatagrams()) {
        QByteArray data;
        data.resize(int(m_discovery->pendingDatagramSize()));
        QHostAddress addr;
        quint16 port = 0;
        m_discovery->readDatagram(data.data(), data.size(), &addr, &port);
        const QString line = QString::fromUtf8(data).trimmed();

        if (!line.startsWith("KMZI_DISCOVER:"))
            continue;
        if (!m_searching) {
            log(QString("Запрос от %1:%2 — поиск ещё не запущен.")
                    .arg(addr.toString()).arg(port));
            continue;
        }

        sendLine(QString("KMZI_SERVER:%1").arg(m_exchangePort), addr, port);
        log(QString("Клиент %1:%2 нашёл сервер, порт обмена %3")
                .arg(addr.toString()).arg(port).arg(m_exchangePort));
    }
}

void KeySearchServer::onExchangeRead()
{
    while (m_exchange->hasPendingDatagrams()) {
        QByteArray data;
        data.resize(int(m_exchange->pendingDatagramSize()));
        QHostAddress addr;
        quint16 port = 0;
        m_exchange->readDatagram(data.data(), data.size(), &addr, &port);
        const QString line = QString::fromUtf8(data).trimmed();
        if (line.isEmpty()) continue;

        const int sep = line.indexOf(':');
        const QString cmd  = (sep < 0) ? line : line.left(sep);
        const QString rest = (sep < 0) ? QString() : line.mid(sep + 1);

        if (cmd == "KMZI_CONFIRM") {
            const QString key = clientKey(addr, port);
            ClientInfo ci;
            ci.addr = addr;
            ci.port = port;
            m_clients.insert(key, ci);
            log(QString("Клиент-дешифровщик %1:%2 подтвердил связь.")
                    .arg(addr.toString()).arg(port));
            if (m_searching && !m_found)
                assignTask(key);
            else if (m_found)
                sendLine(QString("KMZI_STOP:%1").arg(b64(m_foundKey)), addr, port);
            else
                sendLine("KMZI_WAIT", addr, port);
            updateStatus();
            continue;
        }

        if (cmd == "KMZI_ACK") {
            log(QString("Задание %1 принято клиентом %2:%3")
                    .arg(rest).arg(addr.toString()).arg(port));
            continue;
        }

        if (cmd == "KMZI_RESULT") {
            handleResult(rest, addr, port);
            continue;
        }

        log("Неизвестный пакет: " + line);
    }
}

void KeySearchServer::handleResult(const QString &rest,
                                   const QHostAddress &sender,
                                   quint16 senderPort)
{
    const QStringList parts = rest.split(':');
    if (parts.size() < 4) {
        log("Некорректный результат: " + rest);
        return;
    }

    const int taskId = parts[0].toInt();
    const bool found = (parts[1] == "1");
    const QString keyPhrase = unb64(parts[2]);
    const QString plain     = unb64(parts[3]);

    const QString ck = clientKey(sender, senderPort);
    auto it = m_clients.find(ck);
    if (it != m_clients.end()) {
        m_checkedKeys += std::max(0, it.value().keyTo - it.value().keyFrom);
        it.value().taskId = -1;
    }

    log(QString("Результат задания %1 от %2:%3 — %4")
            .arg(taskId).arg(sender.toString()).arg(senderPort)
            .arg(found ? "КЛЮЧ НАЙДЕН" : "не найден"));

    if (found && !m_found) {
        m_found      = true;
        m_searching  = false;
        m_foundKey   = keyPhrase;
        broadcastStop(keyPhrase);

        log("==============================================");
        log(QString("=== КЛЮЧЕВАЯ ФРАЗА: %1 ===").arg(keyPhrase));
        log(QString("=== РАСШИФРОВАННЫЙ ТЕКСТ: %1 ===").arg(plain));
        log("==============================================");

        m_startBtn->setEnabled(true);
        m_stopBtn->setEnabled(false);
        updateStatus();
        return;
    }

    if (m_searching) {
        if (it != m_clients.end())
            assignTask(ck);
    }
}

void KeySearchServer::assignTask(const QString &key)
{
    auto it = m_clients.find(key);
    if (it == m_clients.end()) return;
    ClientInfo &ci = it.value();

    if (!m_searching || m_found) {
        sendLine("KMZI_DONE", ci.addr, ci.port);
        return;
    }

    if (m_nextKey >= m_keys.size()) {
        sendLine("KMZI_DONE", ci.addr, ci.port);
        log("Ключевые фразы закончились — поиск завершён без результата.");
        m_searching = false;
        m_startBtn->setEnabled(true);
        m_stopBtn->setEnabled(false);
        updateStatus();
        return;
    }

    const int from = m_nextKey;
    const int to   = std::min<int>(m_nextKey + m_chunkSize, int(m_keys.size()));
    m_nextKey = to;

    ci.taskId  = m_nextTaskId++;
    ci.keyFrom = from;
    ci.keyTo   = to;

    QStringList encodedKeys;
    encodedKeys.reserve(to - from);
    for (int i = from; i < to; ++i)
        encodedKeys.append(b64(m_keys.at(i)));

    const QString packet = QString("KMZI_TASK:%1:%2:%3:%4:%5:%6")
                               .arg(ci.taskId)
                               .arg(b64(m_alphabet))
                               .arg(b64(m_cipher))
                               .arg(from)
                               .arg(to)
                               .arg(encodedKeys.join(','));

    sendLine(packet, ci.addr, ci.port);

    log(QString("Клиенту %1:%2 выдано задание %3: ключи [%4; %5)")
            .arg(ci.addr.toString()).arg(ci.port)
            .arg(ci.taskId).arg(from).arg(to));
}

void KeySearchServer::sendLine(const QString &line,
                               const QHostAddress &addr, quint16 port)
{
    m_exchange->writeDatagram(line.toUtf8(), addr, port);
}

void KeySearchServer::broadcastStop(const QString &key)
{
    const QString packet = key.isEmpty()
                               ? QString("KMZI_STOP")
                               : QString("KMZI_STOP:%1").arg(b64(key));
    for (const ClientInfo &ci : m_clients)
        sendLine(packet, ci.addr, ci.port);
}

void KeySearchServer::updateStatus()
{
    QString state;
    if (m_found)
        state = QString("КЛЮЧ НАЙДЕН: %1").arg(m_foundKey);
    else if (m_searching)
        state = QString("Идёт поиск: проверено %1 из %2 ключей")
                    .arg(m_checkedKeys).arg(m_keys.size());
    else
        state = "Ожидание запуска";

    m_statusLabel->setText(QString("Клиентов: %1 | %2")
                               .arg(m_clients.size()).arg(state));
}
