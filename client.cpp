#include "client.h"
#include <cmath>
#include <QStringList>

Client::Client(QWidget *parent) : QWidget(parent) {
    setWindowTitle("Клиент (TCP + UDP)");
    resize(600, 500);

    m_log = new QTextEdit(this);
    m_log->setReadOnly(true);

    m_input = new QLineEdit(this);
    m_input->setPlaceholderText("Сообщение для TCP чата...");

    m_udpInput = new QLineEdit(this);
    m_udpInput->setPlaceholderText("Сообщение для UDP...");

    QPushButton *btnChat  = new QPushButton("Отправить (TCP)", this);
    QPushButton *btnCalc1 = new QPushButton("Вариант 1: площадь sin(x), Y>0", this);
    QPushButton *btnCalc4 = new QPushButton("Вариант 4: корни cos³(x)", this);
    QPushButton *btnUdp   = new QPushButton("Отправить (UDP)", this);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(m_log);

    QHBoxLayout *hLayout1 = new QHBoxLayout();
    hLayout1->addWidget(m_input);
    hLayout1->addWidget(btnChat);
    layout->addLayout(hLayout1);

    layout->addWidget(btnCalc1);
    layout->addWidget(btnCalc4);

    QHBoxLayout *hLayout2 = new QHBoxLayout();
    hLayout2->addWidget(m_udpInput);
    hLayout2->addWidget(btnUdp);
    layout->addLayout(hLayout2);

    // TCP сокет
    m_pTcpSock = new ClientSocketAdapter(this);
    connect(m_pTcpSock, SIGNAL(message(QString)), SLOT(on_tcpMessage(QString)));
    connect(btnChat,  SIGNAL(clicked()), SLOT(on_sendChat()));
    connect(btnCalc1, SIGNAL(clicked()), SLOT(onStartCalc1()));
    connect(btnCalc4, SIGNAL(clicked()), SLOT(onStartCalc4()));

    // UDP сокет (порт 5556)
    m_pUdpSock = new UdpAdapter(this, 5556);
    connect(m_pUdpSock, SIGNAL(message(QString)), SLOT(on_udpMessage(QString)));
    connect(btnUdp, SIGNAL(clicked()), SLOT(on_sendUdp()));
}

void Client::on_sendChat() {
    QString text = m_input->text().trimmed();
    if (!text.isEmpty()) {
        m_pTcpSock->sendString(text);
        m_input->clear();
    }
}

void Client::on_sendUdp() {
    QString text = m_udpInput->text().trimmed();
    if (!text.isEmpty()) {
        m_pUdpSock->sendString("UDP_BROADCAST:" + text);
        m_udpInput->clear();
    }
}

void Client::onStartCalc1() {
    double A = 0.0;
    double B = 4 * M_PI;
    int steps = 400;
    m_pTcpSock->sendString(QString("START_CALC:1:%1:%2:%3").arg(A).arg(B).arg(steps));
    m_log->append("<i>Запрос: Вариант 1 (площадь sin(x), Y>0)...</i>");
}

void Client::onStartCalc4() {
    double A = 0.0;
    double B = 4 * M_PI;
    int steps = 400;
    m_pTcpSock->sendString(QString("START_CALC:4:%1:%2:%3").arg(A).arg(B).arg(steps));
    m_log->append("<i>Запрос: Вариант 4 (корни cos³(x))...</i>");
}

void Client::on_tcpMessage(QString text) {
    if (text.startsWith("TASK:1:")) {
        QStringList parts = text.split(":");
        if (parts.size() == 5) {
            int id = parts[2].toInt();
            double a = parts[3].toDouble();
            double b = parts[4].toDouble();
            double area = calculateAreaSin(a, b);
            m_log->append(QString("<i>[Вар.1]</i> Задача %1: [%2, %3] → S = %4")
                              .arg(id).arg(a, 0, 'f', 3).arg(b, 0, 'f', 3).arg(area, 0, 'f', 6));
            m_pTcpSock->sendString(QString("RESULT:1:%1:%2").arg(id).arg(area));
        }
    }
    else if (text.startsWith("TASK:4:")) {
        QStringList parts = text.split(":");
        if (parts.size() == 5) {
            int id = parts[2].toInt();
            double a = parts[3].toDouble();
            double b = parts[4].toDouble();
            QList<double> roots = findRootsCos3(a, b);
            QString rootsStr;
            for (double r : roots) rootsStr += QString::number(r, 'f', 6) + ",";
            if (!rootsStr.isEmpty()) rootsStr.chop(1);
            m_log->append(QString("<i>[Вар.4]</i> Задача %1: [%2, %3] → корней: %4")
                              .arg(id).arg(a, 0, 'f', 3).arg(b, 0, 'f', 3).arg(roots.size()));
            m_pTcpSock->sendString(QString("RESULT:4:%1:%2").arg(id).arg(rootsStr));
        }
    }
    else if (text.startsWith("FINAL:1:")) {
        double val = text.mid(8).toDouble();
        m_log->append(QString("<b style='color:green;'>[Итог Вар.1] Площадь = %1</b>")
                          .arg(val, 0, 'f', 8));
    }
    else if (text.startsWith("FINAL:4:")) {
        QString rootsStr = text.mid(8);
        m_log->append(QString("<b style='color:green;'>[Итог Вар.4] Корни: %1</b>")
                          .arg(rootsStr));
    }
    else if (text == "connect") {
        m_log->append("<b style='color:green;'>Подключено к серверу (TCP)!</b>");
    }
    else if (text.startsWith("SERVER:")) {
        m_log->append(QString("<b style='color:red;'>%1</b>").arg(text.mid(7)));
    }
    else {
        m_log->append(QString("<b>[TCP Чат]:</b> %1").arg(text));
    }
}

void Client::on_udpMessage(QString text) {
    m_log->append(QString("<b style='color:blue;'>[UDP]:</b> %1").arg(text));
}

// Вариант 1: площадь sin(x) при Y > 0
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

// Вариант 4: корни cos³(x)
QList<double> Client::findRootsCos3(double a, double b) {
    QList<double> roots;
    int localSteps = 100000;
    double dx = (b - a) / localSteps;
    double prevY = pow(cos(a), 3);
    for (int i = 1; i <= localSteps; ++i) {
        double x = a + i * dx;
        double y = pow(cos(x), 3);
        if (prevY * y < 0) {
            double lo = x - dx, hi = x;
            for (int j = 0; j < 60; ++j) {
                double mid = (lo + hi) / 2;
                double fm = pow(cos(mid), 3);
                if (pow(cos(lo), 3) * fm < 0) hi = mid; else lo = mid;
            }
            roots.append((lo + hi) / 2);
        }
        else if (qAbs(y) < 1e-12) {
            roots.append(x);
        }
        prevY = y;
    }
    return roots;
}