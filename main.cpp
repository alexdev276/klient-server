#include <QApplication>
#include <QInputDialog>
#include "server.h"
#include "client.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QStringList options;
    options << "Сервер (UDP)" << "Клиент (UDP)";

    bool ok;
    QString choice = QInputDialog::getItem(nullptr, "Выбор режима",
                                           "Выберите приложение для запуска:",
                                           options, 0, false, &ok);
    if (!ok || choice.isEmpty()) return 0;

    QWidget *window = nullptr;

    if (choice == options[0]) {
        window = new Server(5555); // Запуск сервера на порту 5555
    } else if (choice == options[1]) {
        window = new Client();     // Запуск клиента
    }

    if (window) {
        window->show();
    }

    return a.exec();
}