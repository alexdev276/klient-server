#include "server.h"
#include "serversocketadapter.h"
#include <QTextStream>
#include <QDebug>
#include <QStringList>

Server::Server(int nPort, QObject *parent)
    : QObject(parent), m_ptcpServer(new QTcpServer(this)),
    m_isCalculating(false), m_totalTasks(0), m_completedTasks(0), m_finalArea(0.0)
{
    connect(m_ptcpServer, SIGNAL(newConnection()), this, SLOT(on_newConnection()));
    if (!m_ptcpServer->listen(QHostAddress::Any, nPort)) {
        m_ptcpServer->close();
        QTextStream(stdout) << "[ОШИБКА] Не удалось запустить сервер: " << m_ptcpServer->errorString() << "\n";
        throw m_ptcpServer->errorString();
    }
    QTextStream(stdout) << "========================================\n";
    QTextStream(stdout) << "=== СЕРВЕР ЗАПУЩЕН НА ПОРТУ " << nPort << " ===\n";
    QTextStream(stdout) << "========================================\n";
}

Server::~Server() {
    qDeleteAll(m_clients);
    m_clients.clear();
}

void Server::on_newConnection() {
    QTextStream(stdout) << "[СЕРВЕР] Новое подключение клиента!\n";
    QTcpSocket *pClientSock = m_ptcpServer->nextPendingConnection();
    ISocketAdapter *pSockHandle = new ServerSocketAdapter(pClientSock, this);
    m_clients.push_back(pSockHandle);
    pSockHandle->sendString("connect");
    connect(pSockHandle, SIGNAL(disconnected()), this, SLOT(on_disconnected()));
    connect(pSockHandle, SIGNAL(message(QString)), this, SLOT(on_message(QString)));
}

void Server::on_disconnected() {
    QTextStream(stdout) << "[СЕРВЕР] Клиент отключился.\n";
    ISocketAdapter *client = qobject_cast<ISocketAdapter*>(sender());
    if (client) {
        m_clients.removeOne(client);
        delete client;
    }
}

void Server::broadcast(const QString &msg) {
    foreach (ISocketAdapter *sock, m_clients) sock->sendString(msg);
}

void Server::on_message(QString msg) {
    QTextStream(stdout) << "[СЕРВЕР] Получено: " << msg << "\n";

    if (msg.startsWith("RESULT:")) {
        if (!m_isCalculating) return;
        QStringList parts = msg.split(":");
        if (parts.size() == 3) {
            m_finalArea += parts[2].toDouble();
            m_completedTasks++;
            QTextStream(stdout) << "[ЛР5] Прогресс: " << m_completedTasks << "/" << m_totalTasks << "\n";
            if (m_completedTasks >= m_totalTasks) {
                m_isCalculating = false;
                QString finalMsg = QString("FINAL:%1").arg(m_finalArea, 0, 'f', 8);
                QTextStream(stdout) << "[ЛР5] Итоговая площадь: " << m_finalArea << "\n";
                broadcast(finalMsg);
            }
        }
    }
    else if (msg.startsWith("START_CALC:")) {
        if (m_isCalculating) {
            broadcast("SERVER: Вычисления уже запущены!");
            return;
        }
        QStringList parts = msg.split(":");
        if (parts.size() == 4) {
            distributeTasks(parts[1].toDouble(), parts[2].toDouble(), parts[3].toInt());
        }
    }
    else {
        broadcast(msg); // ЛР4: Чат
    }
}

void Server::distributeTasks(double A, double B, int steps) {
    if (m_clients.isEmpty()) return;
    m_isCalculating = true;
    m_totalTasks = steps;
    m_completedTasks = 0;
    m_finalArea = 0.0;

    double dx = (B - A) / steps;
    int clientIdx = 0;
    QTextStream(stdout) << "[ЛР5] Распределение " << steps << " задач...\n";
    for (int i = 0; i < steps; ++i) {
        double a = A + i * dx;
        double b = A + (i + 1) * dx;
        QString task = QString("TASK:%1:%2:%3").arg(i).arg(a).arg(b);
        m_clients[clientIdx]->sendString(task);
        clientIdx = (clientIdx + 1) % m_clients.size();
    }
}