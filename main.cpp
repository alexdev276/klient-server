#include "mainwindow.h"
#include "server.h"

#include <QApplication>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Запуск сервера на порту 1024 (как в методичке)
    try {
        Server server(1024);
        qDebug() << "Сервер инициализирован и слушает порт 1024";
    } catch (const QString &error) {
        qDebug() << "Критическая ошибка сервера:" << error;
        return -1;
    }

    MainWindow w;
    w.show();

    return a.exec();
}