#include <QApplication>
#include <QInputDialog>
#include <QMessageBox>

#include "udpserver.h"
#include "udpclient.h"
#include "vigenereserver.h"
#include "vigenereclient.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QStringList options;
    options << "ЛР6: UDP Сервер (Широковещательная рассылка)"
            << "ЛР6: UDP Клиент (Прием сообщений)"
            << "ЛР7: Сервер Виженера (Раздача задач)"
            << "ЛР7: Клиент Виженера (Подбор ключа)";

    bool ok;
    QString choice = QInputDialog::getItem(nullptr, "Выбор лабораторной работы",
                                           "Выберите приложение для запуска:",
                                           options, 0, false, &ok);
    if (!ok || choice.isEmpty()) return 0;

    QWidget *window = nullptr;

    if (choice == options[0]) {
        window = new UdpServer();
    } else if (choice == options[1]) {
        window = new UdpClient();
    } else if (choice == options[2]) {
        window = new VigenereServer();
    } else if (choice == options[3]) {
        window = new VigenereClient();
    }

    if (window) {
        window->show();
    }

    return a.exec();
}