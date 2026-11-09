#include <QCoreApplication>
#include <iostream>
#include <string>
#include "udpserver.h"
#include "udpclient.h"

using namespace std;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    string mode = "";
    if (argc > 1) {
        mode = argv[1];
    }

    // Если аргумент не передан — показываем интерактивное меню
    if (mode.empty()) {
        cout << "========================================\n";
        cout << "   Универсальная программа ЛР6 и ЛР7     \n";
        cout << "========================================\n";
        cout << "Выберите режим работы:\n";
        cout << "  1 - server6 (UDP сервер для ЛР6)\n";
        cout << "  2 - client6 (UDP клиент для ЛР6)\n";
        cout << "  3 - server7 (Сервер распределённого поиска ЛР7)\n";
        cout << "  4 - client7 (Клиент распределённого поиска ЛР7)\n";
        cout << "Введите номер: ";
        cout.flush();

        int choice = 0;
        if (!(cin >> choice)) {
            cout << "Ошибка ввода." << endl;
            return 1;
        }

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

    if (mode == "server6") {
        UdpServer server(5555, 6);
        return a.exec();
    }
    else if (mode == "client6") {
        UdpClient client(6);
        return a.exec();
    }
    else if (mode == "server7") {
        UdpServer server(50050, 7);
        return a.exec();
    }
    else if (mode == "client7") {
        UdpClient client(7);
        client.requestLab7Task("127.0.0.1", 50050);
        return a.exec();
    }
    else {
        cout << "Неизвестный режим: " << mode << endl;
        return 1;
    }
}