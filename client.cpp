#include "client.h"
#include <cmath>
#include <QStringList>
#include <QDateTime>

Client::Client(QWidget *parent) : QWidget(parent) {
    setWindowTitle("Клиент (ЛР 4 и ЛР 5)");
    resize(500, 400);

    // Инициализация UI
    m_log = new QTextEdit(this);
    m_log->setReadOnly(true);

    m_input = new QLineEdit(this);
    m_input->setPlaceholderText("Введите сообщение для чата...");

    m_sendBtn = new QPushButton("Отправить в чат (ЛР4)", this);
    m_calcBtn = new QPushButton("Запустить вычисления sin(x) (ЛР5)", this);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(m_log);

    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->addWidget(m_input);
    hLayout->addWidget(m_sendBtn);
    mainLayout->addLayout(hLayout);
    mainLayout->addWidget(m_calcBtn);

    // Инициализация сети
    m_pSock = new ClientSocketAdapter(this);

    connect(m_pSock, SIGNAL(message(QString)), this, SLOT(on_message(QString)));
    connect(m_sendBtn, SIGNAL(clicked()), this, SLOT(on_sendChat()));
    connect(m_calcBtn, SIGNAL(clicked()), this, SLOT(on_startCalc()));

    m_log->append("<i>Попытка подключения к серверу...</i>");
}

void Client::on_sendChat() {
    QString text = m_input->text().trimmed();
    if (!text.isEmpty()) {
        // Формат чата: CHAT:текст
        m_pSock->sendString("CHAT:" + text);
        m_input->clear();
    }
}

void Client::on_startCalc() {
    // Запускаем вычисление площади sin(x) на отрезке [0, 4*PI] (там будет 8 периодов, площадь 8*2=16, но нас интересует только Y>0, т.е. 4 периода по 2 = 8)
    // Разобьем на 400 подзадач для распределения
    double A = 0.0;
    double B = 4 * M_PI;
    int steps = 400;

    QString cmd = QString("CALC_START:%1:%2:%3").arg(A).arg(B).arg(steps);
    m_pSock->sendString(cmd);
    m_log->append("<b style='color:blue;'>[Система]</b> Запрос на распределенные вычисления отправлен на сервер...");
}

void Client::on_message(QString text) {
    // --- Обработка задач от сервера (ЛР 5) ---
    if (text.startsWith("TASK:")) {
        QStringList parts = text.split(":");
        if (parts.size() == 4) {
            int id = parts[1].toInt();
            double a = parts[2].toDouble();
            double b = parts[3].toDouble();

            // Вычисляем площадь на отрезке [a, b]
            double area = calculateArea(a, b);

            m_log->append(QString("<i>[Вычисление]</i> Задача %1: отрезок [%2, %3] -> площадь = %4")
                              .arg(id).arg(a, 0, 'f', 4).arg(b, 0, 'f', 4).arg(area, 0, 'f', 6));

            // Отправляем результат обратно: CALC_RES:ID:VALUE
            m_pSock->sendString(QString("CALC_RES:%1:%2").arg(id).arg(area));
        }
    }
    // --- Обработка итогового результата (ЛР 5) ---
    else if (text.startsWith("CALC_FINAL:")) {
        double finalVal = text.split(":")[1].toDouble();
        m_log->append(QString("<b style='color:green; font-size:14px;'>[ИТОГ ЛР5] Общая площадь фигуры (Y>0) = %1</b>").arg(finalVal, 0, 'f', 6));
    }
    // --- Обработка сообщений чата (ЛР 4) ---
    else if (text.startsWith("CHAT:")) {
        QString chatMsg = text.mid(5); // Убираем "CHAT:"
        m_log->append(QString("<b>[Чат]:</b> %1").arg(chatMsg));
    }
    // --- Системные сообщения ---
    else if (text == "connect") {
        m_log->append("<b style='color:green;'>Успешно подключено к серверу!</b>");
    }
    else if (text.startsWith("SERVER_MSG:")) {
        m_log->append(QString("<b style='color:red;'>[Сервер]:</b> %1").arg(text.mid(11)));
    }
}

// Математическая функция: численное интегрирование sin(x) где Y > 0
double Client::calculateArea(double a, double b) {
    double area = 0.0;
    int localSteps = 1000; // Точность вычислений на каждом подотрезке
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