#include "client.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QTimer>
#include <QtMath>

Client::Client(QWidget *parent)
    : QWidget(parent), m_myId(0)
{
    setWindowTitle("Клиент");
    resize(660, 480);

    QLabel *ipLabel = new QLabel("IP сервера (Enter — переподключиться):");
    m_ipEdit = new QLineEdit("127.0.0.1");

    m_logView = new QPlainTextEdit;
    m_logView->setReadOnly(true);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(ipLabel);
    layout->addWidget(m_ipEdit);
    layout->addWidget(m_logView);

    m_socket = new QTcpSocket(this);
    connect(m_socket, &QTcpSocket::connected,     this, &Client::on_connected);
    connect(m_socket, &QTcpSocket::disconnected,  this, &Client::on_disconnected);
    connect(m_socket, &QTcpSocket::readyRead,     this, &Client::on_readyRead);
    connect(m_socket, &QTcpSocket::errorOccurred, this, &Client::on_socketError);
    connect(m_ipEdit, &QLineEdit::returnPressed,  this, &Client::on_ipEntered);

    QTimer *flushTimer = new QTimer(this);
    connect(flushTimer, &QTimer::timeout, this, [this](){
        if (!m_buffer.isEmpty() && !m_buffer.contains('\n')) {
            QString line = QString::fromUtf8(m_buffer).trimmed();
            m_buffer.clear();
            if (!line.isEmpty()) processLine(line);
        }
    });
    flushTimer->start(150);

    connectToServer(m_ipEdit->text());
}

void Client::log(const QString &msg)
{
    m_logView->appendPlainText(msg);
}

void Client::on_ipEntered()
{
    if (m_socket->state() != QAbstractSocket::UnconnectedState)
        m_socket->abort();
    connectToServer(m_ipEdit->text());
}

void Client::connectToServer(const QString &ip)
{
    log(QString("Подключение к %1:5555...").arg(ip));
    m_socket->connectToHost(ip, 5555);
}

void Client::on_connected()      { log("Подключено к серверу."); }
void Client::on_disconnected()   { log("Соединение с сервером закрыто."); }
void Client::on_socketError(QAbstractSocket::SocketError)
{
    log("Ошибка: " + m_socket->errorString());
}

void Client::on_readyRead()
{
    m_buffer.append(m_socket->readAll());
    while (m_buffer.contains('\n')) {
        int idx = m_buffer.indexOf('\n');
        QString line = QString::fromUtf8(m_buffer.left(idx)).trimmed();
        m_buffer.remove(0, idx + 1);
        if (!line.isEmpty()) processLine(line);
    }
}

void Client::processLine(const QString &line)
{
    if (line.startsWith("WELCOME:")) {
        m_myId = line.mid(8).toInt();
        setWindowTitle(QString("Клиент #%1").arg(m_myId));
        log(QString("Мой номер на сервере: #%1").arg(m_myId));
        return;
    }

    if (line.startsWith("TASK:")) {
        QStringList parts = line.split(":");
        int type = parts.size() >= 2 ? parts[1].toInt() : 0;

        // 5 полей: TASK:type:id:a:b — один шаг
        if (parts.size() == 5) {
            int    type_ = parts[1].toInt();
            qint64 id    = parts[2].toLongLong();
            double a     = parts[3].toDouble();
            double b     = parts[4].toDouble();

            QString name = (type_ == 1) ? "площадь sin(x) [Y>0]"
                                        : "корни cos^3(x)";
            log(QString("Клиент #%1 - шаг %2 : %3  [%4 ; %5]")
                    .arg(m_myId).arg(id).arg(name)
                    .arg(a, 0, 'f', 8)
                    .arg(b, 0, 'f', 8));

            computeSingleStep(type_, id, a, b);
            return;
        }

        // 6 полей: TASK:type:id:aStart:bEnd:dx — чанк
        if (parts.size() >= 6) {
            int    type_  = parts[1].toInt();
            qint64 id     = parts[2].toLongLong();
            double aStart = parts[3].toDouble();
            double bEnd   = parts[4].toDouble();
            double dx     = parts[5].toDouble();

            QString name = (type_ == 1) ? "площадь sin(x) [Y>0]"
                                        : "корни cos^3(x)";
            log(QString("Клиент #%1 - чанк %2 : %3  [%4 ; %5]  dx=%6")
                    .arg(m_myId).arg(id).arg(name)
                    .arg(aStart, 0, 'f', 8)
                    .arg(bEnd, 0, 'f', 8)
                    .arg(dx, 0, 'g', 15));

            computeChunk(type_, id, aStart, bEnd, dx);
            return;
        }

        log("Неизвестный формат TASK: " + line);
        return;
    }

    if (line.startsWith("FINAL:")) {
        log("ИТОГОВЫЙ РЕЗУЛЬТАТ: " + line);
        return;
    }
    log("<- " + line);
}

void Client::computeSingleStep(int type, qint64 id, double a, double b)
{
    double dx = (b - a) / 1000.0;

    if (type == 1) {
        double area = 0.0;
        for (int i = 0; i < 1000; ++i) {
            double x = a + (i + 0.5) * dx;
            double y = qSin(x);
            if (y > 0) area += y * dx;
        }
        sendResult(1, id, QString::number(area, 'f', 12));
    }
    else if (type == 4) {
        auto f = [](double x) { double c = qCos(x); return c * c * c; };
        QStringList roots;
        double xPrev = a, yPrev = f(a);
        for (int i = 1; i <= 1000; ++i) {
            double xCur = a + i * dx;
            double yCur = f(xCur);
            if (yPrev * yCur < 0.0) {
                double lo = xPrev, hi = xCur;
                for (int k = 0; k < 60; ++k) {
                    double mid = 0.5 * (lo + hi);
                    if (f(lo) * f(mid) <= 0) hi = mid;
                    else                     lo = mid;
                }
                roots << QString::number(0.5 * (lo + hi), 'f', 9);
            }
            xPrev = xCur; yPrev = yCur;
        }
        sendResult(4, id, roots.join(","));
    }
}

void Client::computeChunk(int type, qint64 id, double aStart, double bEnd, double dx)
{
    if (dx <= 0.0 || bEnd <= aStart) {
        sendResult(type, id, "");
        return;
    }

    qint64 n = static_cast<qint64>((bEnd - aStart) / dx);
    if (n < 1) n = 1;

    if (type == 1) {
        double area = 0.0;
        for (qint64 i = 0; i < n; ++i) {
            double x = aStart + (i + 0.5) * dx;
            double y = qSin(x);
            if (y > 0) area += y * dx;
        }
        sendResult(1, id, QString::number(area, 'f', 12));
        log(QString("   -> ответ на чанк %1: площадь=%2")
                .arg(id).arg(area, 0, 'f', 12));
    }
    else if (type == 4) {
        auto f = [](double x) { double c = qCos(x); return c * c * c; };
        QStringList roots;

        double xPrev = aStart;
        double yPrev = f(xPrev);
        for (qint64 i = 1; i <= n; ++i) {
            double xCur = aStart + i * dx;
            double yCur = f(xCur);
            if (yPrev == 0.0) {
                roots << QString::number(xPrev, 'f', 9);
            } else if (yPrev * yCur < 0.0) {
                double lo = xPrev, hi = xCur;
                for (int k = 0; k < 60; ++k) {
                    double mid = 0.5 * (lo + hi);
                    if (f(lo) * f(mid) <= 0) hi = mid;
                    else                     lo = mid;
                }
                roots << QString::number(0.5 * (lo + hi), 'f', 9);
            }
            xPrev = xCur; yPrev = yCur;
        }
        sendResult(4, id, roots.join(","));
        log(QString("   -> ответ на чанк %1: корней %2")
                .arg(id).arg(roots.size()));
    }
}

void Client::sendResult(int type, qint64 id, const QString &payload)
{
    QString r = QString("RESULT:%1:%2:%3").arg(type).arg(id).arg(payload);
    m_socket->write((r + "\n").toUtf8());
    m_socket->flush();
}