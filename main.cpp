#include <QCoreApplication>
#include <QString>

#include <iostream>
#include <limits>
#include <string>

#include "udpclient.h"
#include "udpserver.h"

using namespace std;

static quint16 parsePort(const char *arg, quint16 fallback)
{
    bool ok = false;
    const uint value = QString::fromUtf8(arg).toUInt(&ok);
    if (!ok || value == 0 || value > 65535)
        return fallback;
    return static_cast<quint16>(value);
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    string mode = (argc > 1) ? argv[1] : "";

    // Если аргумент не передан — показываем интерактивное меню
    if (mode.empty()) {
        cout << "========================================\n";
        cout << "   Универсальная программа ЛР6 и ЛР7     \n";
        cout << "========================================\n";
        cout << "Выберите режим работы:\n";
        cout << "  1 - server6 (UDP сервер для ЛР6, порт 5555)\n";
        cout << "  2 - client6 (UDP клиент для ЛР6)\n";
        cout << "  3 - server7 (Сервер распределённого поиска ЛР7, порт 50050)\n";
        cout << "  4 - client7 (Клиент распределённого поиска ЛР7)\n";
        cout << "Введите номер: ";
        cout.flush();

        int choice = 0;
        if (!(cin >> choice)) {
            cout << "Ошибка ввода." << endl;
            return 1;
        }
        // Убираем перевод строки, оставшийся после чтения числа,
        // иначе первый getline() в клиенте ЛР6 прочитает пустую строку.
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        switch (choice) {
        case 1: mode = "server6"; break;
        case 2: mode = "client6"; break;
        case 3: mode = "server7"; break;
        case 4: mode = "client7"; break;
        default:
            cout << "Неверный выбор." << endl;
            return 1;
        }
    }

    const bool isServer = (mode == "server6" || mode == "server7");
    const bool isClient = (mode == "client6" || mode == "client7");

    if (!isServer && !isClient) {
        cout << "Неизвестный режим: " << mode << "\n"
             << "Использование: klient-server server6|server7 [порт]\n"
             << "               klient-server client6|client7 [адрес] [порт]" << endl;
        return 1;
    }

    const bool lab6 = (mode == "server6" || mode == "client6");
    const quint16 defaultPort = lab6 ? 5555 : 50050;

    if (isServer) {
        const quint16 port = (argc > 2) ? parsePort(argv[2], defaultPort) : defaultPort;
        UdpServer server(port, lab6 ? 6 : 7);
        if (!server.isListening())
            return 1;
        return a.exec();
    }

    const QString host = (argc > 2) ? QString::fromUtf8(argv[2])
                                    : QStringLiteral("127.0.0.1");
    const quint16 port = (argc > 3) ? parsePort(argv[3], defaultPort) : defaultPort;

    UdpClient client(lab6 ? 6 : 7, host, port);
    if (!lab6)
        client.requestLab7Task();

    return a.exec();
}
