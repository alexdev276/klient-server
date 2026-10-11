#include <QApplication>
#include <QInputDialog>
#include <QMessageBox>
#include <QTextStream>
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
            Server s(5555);
            return a.exec(); // Event loop крутится, сервер пишет в консоль
        } catch (QString &e) {
            return 1;
        }
    } else {
        Client w;
        w.show();
        return a.exec();
    }
}