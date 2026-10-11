#include "client.h"
#include <cmath>
#include <QStringList>

Client::Client(QWidget *parent) : QWidget(parent) {
    setWindowTitle("КЛИЕНТ (ЛР 4 и ЛР 5, Вариант 1)");
    resize(500, 400);

    m_log = new QTextEdit(this); m_log->setReadOnly(true);
    m_input = new QLineEdit(this); m_input->setPlaceholderText("Сообщение для чата...");

    QPushButton *btnChat = new QPushButton("Отправить (ЛР4)", this);
    QPushButton *btnCalc = new QPushButton("Вычислить sin(x), Y>0 (ЛР5)", this);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(m_log);

    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->addWidget(m_input); hLayout->addWidget(btnChat);
    layout->addLayout(hLayout);
    layout->addWidget(btnCalc);

    m_pSock = new ClientSocketAdapter(this);
    connect(m_pSock, SIGNAL(message(QString)), SLOT(on_message(QString)));
    connect(btnChat, SIGNAL(clicked()), SLOT(on_sendChat()));
    connect(btnCalc, SIGNAL(clicked()), SLOT(on_startCalc()));
}

void Client::on_sendChat() {
    QString text = m_input->text().trimmed();
    if (!text.isEmpty()) {
        m_pSock->sendString(text);
        m_input->clear();
    }
}

void Client::on_startCalc() {
    // Вариант 1: Y = sin(x), Искомая величина: Площадь фигур при Y > 0
    // Интервал [0, 4*PI], разбиваем на 400 подзадач для распределения
    double A = 0.0;
    double B = 4 * M_PI;
    int steps = 400;
    m_pSock->sendString(QString("START_CALC:%1:%2:%3").arg(A).arg(B).arg(steps));
    m_log->append("<i>Запрос на вычисление площади отправлен на сервер...</i>");
}

void Client::on_message(QString text) {
    if (text.startsWith("TASK:")) {
        QStringList parts = text.split(":");
        if (parts.size() == 4) {
            int id = parts[1].toInt();
            double a = parts[2].toDouble();
            double b = parts[3].toDouble();

            double area = calculateAreaVariant1(a, b);
            m_log->append(QString("<i>[Вычисление]</i> Задача %1: [%2, %3] -> S = %4")
                              .arg(id).arg(a, 0, 'f', 3).arg(b, 0, 'f', 3).arg(area, 0, 'f', 6));

            m_pSock->sendString(QString("RESULT:%1:%2").arg(id).arg(area));
        }
    }
    else if (text.startsWith("FINAL:")) {
        double val = text.split(":")[1].toDouble();
        m_log->append(QString("<b style='color:green; font-size:14px;'>[ИТОГ ЛР5] Общая площадь (Y>0) = %1</b>").arg(val, 0, 'f', 8));
    }
    else if (text == "connect") {
        m_log->append("<b style='color:green;'>Успешно подключено к серверу!</b>");
    }
    else if (text.startsWith("SERVER:")) {
        m_log->append(QString("<b style='color:red;'>%1</b>").arg(text));
    }
    else {
        m_log->append(QString("<b>[Чат]:</b> %1").arg(text));
    }
}

// Математика ЛР 5, Вариант 1: Y = sin(x), Площадь при Y > 0, точность 10^-6
double Client::calculateAreaVariant1(double a, double b) {
    double area = 0.0;
    int localSteps = 100000; // 100 000 шагов гарантируют точность 10^-6
    double dx = (b - a) / localSteps;

    for (int i = 0; i < localSteps; ++i) {
        double x = a + i * dx;
        double y = sin(x);
        if (y > 0) {
            area += y * dx; // Метод прямоугольников
        }
    }
    return area;
}