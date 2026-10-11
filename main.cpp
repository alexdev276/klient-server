#include <QApplication>
#include <QInputDialog>
#include <QMessageBox>
#include "server.h"
#include "client.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    // Определяем режим запуска через аргументы или диалог
    QString mode = "ask";
    if (argc > 1) {
        mode = QString(argv[1]).toLower();
    }

    if (mode == "ask") {
        bool ok;
        QString item = QInputDialog::getItem(nullptr, "Выбор режима запуска",
                                             "Запустить приложение как:",
                                             QStringList() << "Сервер" << "Клиент", 0, false, &ok);
        if (ok) {
            mode = (item == "Сервер") ? "server" : "client";
        } else {
            return 0; // Пользователь отменил
        }
    }

    if (mode == "server") {
        try {
            Server s(1024);
            // Сервер не имеет GUI, просто крутим event loop
            return a.exec();
        } catch (QString &e) {
            QMessageBox::critical(nullptr, "Ошибка", "Не удалось запустить сервер:\n" + e);
            return 1;
        }
    }
    else if (mode == "client") {
        Client w;
        w.show();
        return a.exec();
    }

    return 0;
}