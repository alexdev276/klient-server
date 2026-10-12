#include "server.h"
#include "serversocketadapter.h"
#include <QTextStream>
#include <QTimer>
#include <QtMath>
#include <algorithm>

Server::Server(int tcpPort, int udpPort,
               int taskType, double A, double B, int steps,
               QObject *parent)
    : QObject(parent),
    m_ptcpServer(new QTcpServer(this)),
    m_udpAdapter(new UdpAdapter(this, udpPort)),
    m_taskType(taskType),
    m_A(A), m_B(B), m_steps(steps),
    m_calculationStarted(false),
    m_isCalculating(false),
    m_totalTasks(0), m_completedTasks(0),
    m_finalArea(0.0)
{
    connect(m_ptcpServer, &QTcpServer::newConnection, this, &Server::on_newConnection);
    connect(m_udpAdapter, &UdpAdapter::message,       this, &Server::on_udpMessage);

    if (!m_ptcpServer->listen(QHostAddress::Any, tcpPort)) {
        QString err = m_ptcpServer->errorString();
        m_ptcpServer->close();
        QTextStream(stdout) << "[ОШИБКА] Не удалось запустить TCP сервер: "
                            << err << "\n";
        throw err;
    }

    QTextStream(stdout) << "========================================\n";
    QTextStream(stdout) << "=== СЕРВЕР ЗАПУЩЕН ===\n";
    QTextStream(stdout) << "TCP порт: " << tcpPort << "\n";
    QTextStream(stdout) << "UDP порт: " << udpPort << "\n";
    QTextStream(stdout) << "Задание: Вариант " << taskType << "\n";
    QTextStream(stdout) << "Диапазон: [" << A << ", " << B << "]\n";
    QTextStream(stdout) << "Шагов: " << steps << "\n";
    QTextStream(stdout) << "========================================\n";
}

Server::~Server()
{
    qDeleteAll(m_clients);
    m_clients.clear();
}

void Server::sendTo(ISocketAdapter *sock, const QString &msg)
{
    if (!sock) return;
    sock->sendString(msg + "\n");
}

void Server::broadcast(const QString &msg)
{
    foreach (ISocketAdapter *sock, m_clients)
    sendTo(sock, msg);
}

void Server::on_newConnection()
{
    QTextStream(stdout) << "[СЕРВЕР] Новое TCP подключение!\n";
    QTcpSocket *pClientSock = m_ptcpServer->nextPendingConnection();
    ISocketAdapter *pSockHandle = new ServerSocketAdapter(pClientSock, this);
    m_clients.push_back(pSockHandle);

    sendTo(pSockHandle, "connect");

    connect(pSockHandle, &ISocketAdapter::disconnected,
            this, &Server::on_disconnected);
    connect(pSockHandle, &ISocketAdapter::message,
            this, &Server::on_message);

    if (!m_calculationStarted && m_clients.size() == 1) {
        m_calculationStarted = true;
        QTextStream(stdout) << "[СЕРВЕР] Первый клиент подключен, "
                               "запуск вычислений...\n";
        QTimer::singleShot(50, this, [this](){
            distributeTasks(m_taskType, m_A, m_B, m_steps);
        });
    }
}

void Server::on_disconnected()
{
    QTextStream(stdout) << "[СЕРВЕР] Клиент отключился.\n";
    ISocketAdapter *client = qobject_cast<ISocketAdapter*>(sender());
    if (client) {
        m_clients.removeOne(client);
        client->deleteLater();
    }
}

void Server::on_udpMessage(QString msg)
{
    QTextStream(stdout) << "[СЕРВЕР UDP] Получено: " << msg << "\n";
    if (msg.startsWith("UDP_BROADCAST:"))
        broadcast(msg.mid(14));
}

void Server::on_message(QString msg)
{
    QTextStream(stdout) << "[СЕРВЕР TCP] Получено: " << msg << "\n";

    if (msg.startsWith("RESULT:")) {
        if (!m_isCalculating) return;
        QStringList parts = msg.split(":");
        if (parts.size() < 4) return;

        int type = parts[1].toInt();
        m_completedTasks++;

        if (type == 1) {
            m_finalArea += parts[3].toDouble();
            QTextStream(stdout) << "[Вар.1] Прогресс: "
                                << m_completedTasks << "/" << m_totalTasks << "\n";
            if (m_completedTasks >= m_totalTasks) {
                m_isCalculating = false;
                QString finalMsg = QString("FINAL:1:%1")
                                       .arg(m_finalArea, 0, 'f', 8);
                QTextStream(stdout) << "[Вар.1] Итог: " << m_finalArea << "\n";
                broadcast(finalMsg);
            }
        }
        else if (type == 4) {
            QString rootsStr = parts[3];
            if (!rootsStr.isEmpty()) {
                QStringList list = rootsStr.split(",");
                for (const QString &s : list)
                    m_allRoots.append(s.toDouble());
            }
            QTextStream(stdout) << "[Вар.4] Прогресс: "
                                << m_completedTasks << "/" << m_totalTasks << "\n";
            if (m_completedTasks >= m_totalTasks) {
                m_isCalculating = false;
                std::sort(m_allRoots.begin(), m_allRoots.end());
                QList<double> uniqueRoots;
                for (double r : m_allRoots) {
                    if (uniqueRoots.isEmpty() ||
                        qAbs(r - uniqueRoots.last()) > 1e-4)
                        uniqueRoots.append(r);
                }
                QString rootsOut;
                for (double r : uniqueRoots)
                    rootsOut += QString::number(r, 'f', 6) + ", ";
                if (!rootsOut.isEmpty()) rootsOut.chop(2);
                QString finalMsg = QString("FINAL:4:%1").arg(rootsOut);
                QTextStream(stdout) << "[Вар.4] Итог: " << rootsOut << "\n";
                broadcast(finalMsg);
            }
        }
    }
    else if (msg.startsWith("START_CALC:")) {
        if (m_isCalculating)
            broadcast("SERVER: Вычисления уже запущены!");
        else
            broadcast("SERVER: Параметры заданы при запуске сервера.");
    }
    else {
        // Чат / прочее
        broadcast(msg);
    }
}

void Server::distributeTasks(int type, double A, double B, int steps)
{
    if (m_clients.isEmpty()) {
        QTextStream(stdout) << "[ОШИБКА] Нет клиентов!\n";
        return;
    }

    m_taskType = type;
    m_isCalculating = true;
    m_totalTasks = steps;
    m_completedTasks = 0;
    m_finalArea = 0.0;
    m_allRoots.clear();

    double dx = (B - A) / steps;
    int clientIdx = 0;

    QTextStream(stdout) << "[СЕРВЕР] Распределение " << steps
                        << " задач (тип " << type
                        << ") между " << m_clients.size() << " клиентами...\n";

    for (int i = 0; i < steps; ++i) {
        double a = A + i * dx;
        double b = A + (i + 1) * dx;
        QString task = QString("TASK:%1:%2:%3:%4")
                           .arg(type).arg(i)
                           .arg(a, 0, 'g', 12)
                           .arg(b, 0, 'g', 12);
        sendTo(m_clients[clientIdx], task);
        clientIdx = (clientIdx + 1) % m_clients.size();
    }

    QTextStream(stdout) << "[СЕРВЕР] Все задачи отправлены, ждём результаты...\n";
}