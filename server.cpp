#include "server.h"
#include "serversocketadapter.h"
#include <QTextStream>
#include <QDebug>
#include <QStringList>

Server::Server(int nPort, QObject *parent)
    : QObject(parent), m_ptcpServer(new QTcpServer(this)),
    m_isCalculating(false), m_totalTasks(0), m_completedTasks(0), m_finalResult(0.0)
{
    connect(m_ptcpServer, SIGNAL(newConnection()), this, SLOT(on_newConnection()));

    if (!m_ptcpServer->listen(QHostAddress::Any, nPort)) {
        m_ptcpServer->close();
        qDebug() << "Ошибка запуска сервера:" << m_ptcpServer->errorString();
        throw m_ptcpServer->errorString();
    }
    qDebug() << "[Сервер] Успешно запущен на порту" << nPort;
}

Server::~Server() {
    qDeleteAll(m_clients);
    m_clients.clear();
}

void Server::on_newConnection() {
    QTcpSocket *pClientSock = m_ptcpServer->nextPendingConnection();
    ISocketAdapter *pSockHandle = new ServerSocketAdapter(pClientSock, this);
    m_clients.push_back(pSockHandle);

    qDebug() << "[Сервер] Новый клиент подключен. Всего клиентов:" << m_clients.size();
    pSockHandle->sendString("connect");

    connect(pSockHandle, SIGNAL(disconnected()), this, SLOT(on_disconnected()));
    connect(pSockHandle, SIGNAL(message(QString)), this, SLOT(on_message(QString)));
}

void Server::on_disconnected() {
    ISocketAdapter *client = qobject_cast<ISocketAdapter*>(sender());
    if (client) {
        m_clients.removeOne(client);
        delete client;
        qDebug() << "[Сервер] Клиент отключился. Осталось клиентов:" << m_clients.size();
    }
}

void Server::broadcastToClients(const QString &msg) {
    foreach (ISocketAdapter *sock, m_clients) {
        sock->sendString(msg);
    }
}

void Server::on_message(QString msg) {
    qDebug() << "[Сервер] Получено:" << msg;

    // --- ЛОГИКА ЛР 5: Запуск вычислений ---
    if (msg.startsWith("CALC_START:")) {
        if (m_isCalculating) {
            broadcastToClients("SERVER_MSG: Вычисления уже запущены!");
            return;
        }
        // Формат: CALC_START:A:B:STEPS
        QStringList parts = msg.split(":");
        if (parts.size() == 4) {
            double A = parts[1].toDouble();
            double B = parts[2].toDouble();
            int steps = parts[3].toInt();
            distributeTasks(A, B, steps);
        }
    }
    // --- ЛОГИКА ЛР 5: Прием результата от клиента ---
    else if (msg.startsWith("CALC_RES:")) {
        if (!m_isCalculating) return;

        // Формат: CALC_RES:ID:VALUE
        QStringList parts = msg.split(":");
        if (parts.size() == 3) {
            double val = parts[2].toDouble();
            m_finalResult += val;
            m_completedTasks++;

            qDebug() << "[Сервер] Получен результат. Прогресс:" << m_completedTasks << "/" << m_totalTasks;

            if (m_completedTasks >= m_totalTasks) {
                m_isCalculating = false;
                QString finalMsg = QString("CALC_FINAL:%1").arg(m_finalResult);
                qDebug() << "[Сервер] Вычисления завершены! Итог:" << m_finalResult;
                broadcastToClients(finalMsg);
            }
        }
    }
    // --- ЛОГИКА ЛР 4: Обычный чат ---
    else if (msg.startsWith("CHAT:")) {
        broadcastToClients(msg);
    }
}

void Server::distributeTasks(double A, double B, int steps) {
    if (m_clients.isEmpty()) {
        qDebug() << "[Сервер] Нет подключенных клиентов для вычислений!";
        return;
    }

    m_isCalculating = true;
    m_totalTasks = steps;
    m_completedTasks = 0;
    m_finalResult = 0.0;

    double dx = (B - A) / steps;
    int clientIndex = 0;

    qDebug() << "[Сервер] Распределение" << steps << "задач между" << m_clients.size() << "клиентами...";

    for (int i = 0; i < steps; ++i) {
        double a = A + i * dx;
        double b = A + (i + 1) * dx;

        // Формируем задачу: TASK:ID:A:B
        QString taskMsg = QString("TASK:%1:%2:%3").arg(i).arg(a).arg(b);

        // Отправляем задачу клиенту (по кругу)
        m_clients[clientIndex]->sendString(taskMsg);
        clientIndex = (clientIndex + 1) % m_clients.size();
    }
}