#include <QApplication>
#include <QInputDialog>
#include "server.h"
#include "client.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    bool ok;
    QString mode = QInputDialog::getItem(nullptr, "Выбор режима",
                                         "Запустить как:",
                                         QStringList() << "Сервер" << "Клиент",
                                         0, false, &ok);
    if (!ok) return 0;

    if (mode == "Сервер") {
        Server *s = new Server(5555, 5556);
        s->show();
        return a.exec();
    }
    else {
        Client w;
        w.show();
        return a.exec();
    }
}