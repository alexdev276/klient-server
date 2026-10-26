#include <QApplication>
#include <QCoreApplication>
#include <QInputDialog>
#include <QTextStream>
#include <QTimer>
#include <QElapsedTimer>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include "server.h"
#include "client.h"
#include "udpchat.h"
#include "keysearchserver.h"
#include "keysearchclient.h"
#include "vigenere.h"

// Автотест алгоритма Виженера (без графического интерфейса):
//   ./klient-server --selftest
static int runSelfTest()
{
    QTextStream out(stdout);
    const QString plain = QString::fromUtf8("Проверка алгоритма is120 Виженера");
    const QString key   = QString::fromUtf8("шифр Виженера");

    Vigenere vig;
    const QString cipher = vig.encrypt(plain, key);
    const QString back   = vig.decrypt(cipher, key);

    const QStringList candidates = {
        QString::fromUtf8("неверный ключ"),
        QString::fromUtf8("Муром"),
        QString::fromUtf8("ключевая фраза"),
        key
    };

    bool found = false;
    QString foundKey;
    for (const QString &k : candidates) {
        if (vig.decrypt(cipher, k).contains("is120")) {
            found = true;
            foundKey = k;
            break;
        }
    }

    const bool ok = (back == plain) && found;
    out << "alphabet size: " << vig.size() << "\n";
    out << "roundtrip: " << (back == plain ? "OK" : "FAIL") << "\n";
    out << "cipher: " << cipher << "\n";
    out << "bruteforce: " << (found ? "OK" : "FAIL")
        << " key=" << foundKey << "\n";
    out << (ok ? "SELFTEST PASSED" : "SELFTEST FAILED") << "\n";
    return ok ? 0 : 1;
}


// ---------------------------------------------------------------
// Автотесты сетевой части (запускаются с QT_QPA_PLATFORM=offscreen):
//   ./klient-server --lab6test
//   ./klient-server --lab7test
// ---------------------------------------------------------------
static int runLab6Test()
{
    QTextStream out(stdout);
    UdpChat server(UdpChat::ServerMode);
    UdpChat client(UdpChat::ClientMode);

    for (QLineEdit *e : client.findChildren<QLineEdit *>()) {
        if (e->text() == "127.0.0.1")
            e->setText("127.0.0.1");
    }

    QElapsedTimer et;
    et.start();
    while (et.elapsed() < 3000) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
        if (client.findChild<QPlainTextEdit *>()->toPlainText().contains("Сервер найден"))
            break;
    }

    QLineEdit *msgEdit = nullptr;
    for (QLineEdit *e : client.findChildren<QLineEdit *>())
        if (e->placeholderText().contains("Сообщение"))
            msgEdit = e;

    if (!msgEdit) {
        out << "LAB6 TEST FAILED: поле ввода не найдено\n";
        return 1;
    }

    msgEdit->setText(QString::fromUtf8("тестовое сообщение"));
    QMetaObject::invokeMethod(&client, "onSendClicked");

    bool received = false;
    et.restart();
    while (et.elapsed() < 4000) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
        if (client.findChild<QPlainTextEdit *>()->toPlainText()
                .contains(QString::fromUtf8("тестовое сообщение"))) {
            received = true;
            break;
        }
    }

    out << "lab6: server/client UDP exchange: "
        << (received ? "OK" : "FAIL") << "\n";
    out << (received ? "LAB6 TEST PASSED" : "LAB6 TEST FAILED") << "\n";
    return received ? 0 : 1;
}

static int runLab7Test()
{
    QTextStream out(stdout);

    KeySearchServer server;
    KeySearchClient client;

    // Заполняем алфавит и шифртекст на сервере.
    const QString plain  = QString::fromUtf8("Тестовый текст is120 для проверки");
    const QString key    = QString::fromUtf8("шифр Виженера");
    Vigenere vig;
    const QString cipher = vig.encrypt(plain, key);

    for (QPlainTextEdit *e : server.findChildren<QPlainTextEdit *>()) {
        if (e->toPlainText().contains(QString::fromUtf8("АБВ")))
            e->setPlainText(Vigenere::defaultAlphabet());
        else if (e->placeholderText().contains(QString::fromUtf8("кодированную")))
            e->setPlainText(cipher);
    }
    for (QLineEdit *e : client.findChildren<QLineEdit *>())
        if (e->placeholderText().contains("IP"))
            e->setText("127.0.0.1");

    QMetaObject::invokeMethod(&server, "onStartClicked");
    QTimer::singleShot(200, &client, [&client]() {
        QMetaObject::invokeMethod(&client, "onFindClicked");
    });

    bool found = false;
    QElapsedTimer et;
    et.start();
    while (et.elapsed() < 10000) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
        for (QLabel *l : server.findChildren<QLabel *>()) {
            if (l->text().contains(QString::fromUtf8("КЛЮЧ НАЙДЕН"))) {
                found = true;
                break;
            }
        }
        if (found) break;
    }

    if (!found) {
        QPlainTextEdit *clientLog = client.findChild<QPlainTextEdit *>();
        if (clientLog)
            out << "lab7 debug: client log:\n" << clientLog->toPlainText() << "\n";
        for (QPlainTextEdit *e : server.findChildren<QPlainTextEdit *>())
            out << "lab7 debug: server edit [" << e->placeholderText() << "]: "
                << e->toPlainText().left(120) << "\n";
    }
    out << "lab7: distributed UDP key search: "
        << (found ? "OK" : "FAIL") << "\n";
    out << (found ? "LAB7 TEST PASSED" : "LAB7 TEST FAILED") << "\n";
    return found ? 0 : 1;
}

int main(int argc, char *argv[])
{
    for (int i = 1; i < argc; ++i) {
        if (QString::fromLocal8Bit(argv[i]) == "--selftest") {
            QCoreApplication app(argc, argv);
            return runSelfTest();
        }
    }

    QApplication a(argc, argv);

    for (int i = 1; i < argc; ++i) {
        const QString arg = QString::fromLocal8Bit(argv[i]);
        if (arg == "--lab6test") return runLab6Test();
        if (arg == "--lab7test") return runLab7Test();
    }

    const QStringList modes = {
        QString::fromUtf8("ЛР 4-5: TCP сервер"),
        QString::fromUtf8("ЛР 4-5: TCP клиент"),
        QString::fromUtf8("ЛР 6: UDP сервер (чат)"),
        QString::fromUtf8("ЛР 6: UDP клиент (чат)"),
        QString::fromUtf8("ЛР 7: сервер подбора ключа"),
        QString::fromUtf8("ЛР 7: клиент-дешифровщик")
    };

    bool ok = false;
    const QString mode = QInputDialog::getItem(
        nullptr,
        QString::fromUtf8("Выбор режима"),
        QString::fromUtf8("Запустить как:"),
        modes, 0, false, &ok);
    if (!ok) return 0;

    const int idx = modes.indexOf(mode);

    switch (idx) {
    case 0: {
        Server *s = new Server(5555, 5556);
        s->show();
        break;
    }
    case 1: {
        Client *c = new Client;
        c->show();
        break;
    }
    case 2: {
        UdpChat *w = new UdpChat(UdpChat::ServerMode);
        w->show();
        break;
    }
    case 3: {
        UdpChat *w = new UdpChat(UdpChat::ClientMode);
        w->show();
        break;
    }
    case 4: {
        KeySearchServer *w = new KeySearchServer;
        w->show();
        break;
    }
    case 5: {
        KeySearchClient *w = new KeySearchClient;
        w->show();
        break;
    }
    default:
        return 0;
    }

    return a.exec();
}
