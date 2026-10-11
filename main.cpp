#include <QApplication>
#include <QInputDialog>
#include <QMessageBox>
#include "server.h"
#include "client.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    bool ok;
    QString mode = QInputDialog::getItem(nullptr, "Выбор режима", "Запустить как:",
                                         QStringList() << "Сервер" << "Клиент", 0, false, &ok);
    if (!ok) return 0;

    if (mode == "Сервер") {
        try {
            Server s(5555, 5556); // TCP порт 5555, UDP порт 5556
            return a.exec();
        } catch (QString &e) {
            QMessageBox::critical(nullptr, "Ошибка", e);
            return 1;
        }
    } else {
        Client w;
        w.show();
        return a.exec();
    }
}