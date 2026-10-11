#include "client.h"
#include <cmath>
#include <QStringList>
#include <QMessageBox>

Client::Client(QWidget *parent) : QWidget(parent) {
    setWindowTitle("Клиент");
    resize(550, 450);

    m_log = new QTextEdit(this);
    m_log->setReadOnly(true);

    m_input = new QLineEdit(this);
    m_input->setPlaceholderText("Сообщение для чата...");

    QPushButton *btnChat  = new QPushButton("Отправить", this);
    QPushButton *btnCalc1 = new QPushButton("Вариант 1: площадь sin(x), Y>0", this);
    QPushButton *btnCalc4 = new QPushButton("Вариант 4: корни cos³(x)", this);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(m_log);

    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->addWidget(m_input);
    hLayout->addWidget(btnChat);
    layout->addLayout(hLayout);

    layout->addWidget(btnCalc1);
    layout->addWidget(btnCalc4);

    m_pSock = new ClientSocketAdapter(this);
    connect(m_pSock, SIGNAL(message(QString)), SLOT(on_message(QString)));
    connect(btnChat,  SIGNAL(clicked()), SLOT(on_sendChat()));
    connect(btnCalc1, SIGNAL(clicked()), SLOT(onStartCalc1()));
    connect(btnCalc4, SIGNAL(clicked()), SLOT(onStartCalc4()));
}

void Client::on_sendChat() {
    QString text = m_input->text().trimmed();
    if (!text.isEmpty()) {
        m_pSock->sendString(text);
        m_input->clear();
    }
}

void Client::onStartCalc1() {
    // Вариант 1: Y = sin(x), площадь при Y > 0 на [0, 4π]
    double A = 0.0;
    double B = 4 * M_PI;
    int steps = 400;
    m_pSock->sendString(QString("START_CALC:1:%1:%2:%3").arg(A).arg(B).arg(steps));
    m_log->append("<i>Отправлен запрос: Вариант 1 (площадь sin(x), Y>0)...</i>");
}

void Client::onStartCalc4() {
    // Вариант 4: Y = cos³(x), найти корни на [0, 4π]
    double A = 0.0;
    double B = 4 * M_PI;
    int steps = 400;
    m_pSock->sendString(QString("START_CALC:4:%1:%2:%3").arg(A).arg(B).arg(steps));
    m_log->append("<i>Отправлен запрос: Вариант 4 (корни cos³(x))...</i>");
}

void Client::on_message(QString text) {
    // --- Задача на площадь (Вариант 1) ---
    if (text.startsWith("TASK:1:")) {
        QStringList parts = text.split(":"); // TASK:1:ID:A:B
        if (parts.size() == 5) {
            int id = parts[2].toInt();
            double a = parts[3].toDouble();
            double b = parts[4].toDouble();
            double area = calculateAreaSin(a, b);
            m_log->append(QString("<i>[Вар.1]</i> Задача %1: [%2, %3] → S = %4")
                              .arg(id).arg(a, 0, 'f', 3).arg(b, 0, 'f', 3).arg(area, 0, 'f', 6));
            m_pSock->sendString(QString("RESULT:1:%1:%2").arg(id).arg(area));
        }
    }
    // --- Задача на корни (Вариант 4) ---
    else if (text.startsWith("TASK:4:")) {
        QStringList parts = text.split(":"); // TASK:4:ID:A:B
        if (parts.size() == 5) {
            int id = parts[2].toInt();
            double a = parts[3].toDouble();
            double b = parts[4].toDouble();
            QList<double> roots = findRootsCos3(a, b);
            QString rootsStr;
            for (double r : roots) rootsStr += QString::number(r, 'f', 6) + ",";
            if (!rootsStr.isEmpty()) rootsStr.chop(1); // убрать последнюю запятую
            m_log->append(QString("<i>[Вар.4]</i> Задача %1: [%2, %3] → корней: %4")
                              .arg(id).arg(a, 0, 'f', 3).arg(b, 0, 'f', 3).arg(roots.size()));
            m_pSock->sendString(QString("RESULT:4:%1:%2").arg(id).arg(rootsStr));
        }
    }
    // --- Итог по площади ---
    else if (text.startsWith("FINAL:1:")) {
        double val = text.mid(8).toDouble();
        m_log->append(QString("<b style='color:green; font-size:14px;'>[Итог Вар.1] Площадь sin(x) при Y>0 = %1</b>")
                          .arg(val, 0, 'f', 8));
    }
    // --- Итог по корням ---
    else if (text.startsWith("FINAL:4:")) {
        QString rootsStr = text.mid(8);
        m_log->append(QString("<b style='color:green; font-size:14px;'>[Итог Вар.4] Корни cos³(x): %1</b>")
                          .arg(rootsStr));
    }
    // --- Системные / чат ---
    else if (text == "connect") {
        m_log->append("<b style='color:green;'>Подключено к серверу!</b>");
    } else if (text.startsWith("SERVER:")) {
        m_log->append(QString("<b style='color:red;'>%1</b>").arg(text.mid(7)));
    } else {
        m_log->append(QString("<b>[Чат]:</b> %1").arg(text));
    }
}

// Вариант 1: площадь sin(x) при Y > 0, точность 10⁻⁶
double Client::calculateAreaSin(double a, double b) {
    double area = 0.0;
    int localSteps = 100000;
    double dx = (b - a) / localSteps;
    for (int i = 0; i < localSteps; ++i) {
        double x = a + i * dx;
        double y = sin(x);
        if (y > 0) area += y * dx;
    }
    return area;
}

// Вариант 4: корни cos³(x) на [a, b]
// cos³(x) = 0 ⇔ cos(x) = 0 ⇔ x = π/2 + kπ
QList<double> Client::findRootsCos3(double a, double b) {
    QList<double> roots;
    int localSteps = 100000;
    double dx = (b - a) / localSteps;
    double prevY = pow(cos(a), 3);
    for (int i = 1; i <= localSteps; ++i) {
        double x = a + i * dx;
        double y = pow(cos(x), 3);
        // Смена знака ⇒ корень между x-dx и x
        if (prevY * y < 0) {
            // Уточняем корень методом бисекции
            double lo = x - dx, hi = x;
            for (int j = 0; j < 60; ++j) {
                double mid = (lo + hi) / 2;
                double fm = pow(cos(mid), 3);
                if (pow(cos(lo), 3) * fm < 0) hi = mid; else lo = mid;
            }
            roots.append((lo + hi) / 2);
        }
        // Точное попадание в ноль
        else if (qAbs(y) < 1e-12) {
            roots.append(x);
        }
        prevY = y;
    }
    return roots;
}