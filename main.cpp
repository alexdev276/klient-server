#include <QApplication>
#include <QInputDialog>
#include <QMessageBox>
#include "server.h"
#include "client.h"
#include "serversettingsdialog.h"

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
        ServerSettingsDialog dlg;
        if (dlg.exec() != QDialog::Accepted) return 0;

        int    taskType = dlg.taskType();
        double A  = dlg.A();
        double B  = dlg.B();
        double dx = dlg.dx();

        if (B <= A) {
            QMessageBox::warning(nullptr, "Ошибка",
                                 "B должно быть больше A");
            return 1;
        }

        int steps = static_cast<int>((B - A) / dx);
        if (steps < 1) steps = 1;
        if (steps > 10000) {
            QMessageBox::warning(nullptr, "Предупреждение",
                                 "Слишком много шагов, установлено 10000.");
            steps = 10000;
        }

        try {
            Server s(5555, 5556, taskType, A, B, steps);
            return a.exec();
        } catch (QString &e) {
            QMessageBox::critical(nullptr, "Ошибка", e);
            return 1;
        }
    }
    else {
        Client w;
        w.show();
        return a.exec();
    }
}