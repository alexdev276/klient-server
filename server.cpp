#include "server.h"
#include "serversocketadapter.h"
#include <QTextStream>
#include <QStringList>
#include <algorithm>

Server::Server(int nPort, QObject *parent)
    : QObject(parent), m_ptcpServer(new QTcpServer(this)),
    m_taskType(0), m_isCalculating(false),
    m_totalTasks(0), m_completedTasks(0), m_finalArea(0.0)
{
    connect(m_ptcpServer, SIGNAL(newConnection()), SLOT(on_newConnection()));
    if (!m_ptcpServer->listen(QHostAddress::Any, nPort)) {
        m_ptcpServer->close();
        QTextStream(stdout) << "[ОШИБКА] Не удалось запустить сервер: "
                            << m_ptcpServer->errorString() << "\n";
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
    connect(pSockHandle, SIGNAL(disconnected()), SLOT(on_disconnected()));
    connect(pSockHandle, SIGNAL(message(QString)), SLOT(on_message(QString)));
}

void Server::on_disconnected() {
    QTextStream(stdout) << "[СЕРВЕР] Клиент отключился.\n";
    ISocketAdapter *client = qobject_cast<ISocketAdapter*>(sender());
    if (client) {
        m_clients.removeOne(client);
        client->deleteLater();
    }
}

void Server::broadcast(const QString &msg) {
    foreach (ISocketAdapter *sock, m_clients) sock->sendString(msg);
}

void Server::on_message(QString msg) {
    // --- Результат от клиента ---
    if (msg.startsWith("RESULT:")) {
        if (!m_isCalculating) return;
        QStringList parts = msg.split(":"); // RESULT:TYPE:ID:VALUE
        if (parts.size() >= 4) {
            int type = parts[1].toInt();
            // int id = parts[2].toInt(); // можно использовать для контроля
            m_completedTasks++;

            if (type == 1) {
                // Площадь
                m_finalArea += parts[3].toDouble();
                QTextStream(stdout) << "[Вар.1] Прогресс: "
                                    << m_completedTasks << "/" << m_totalTasks << "\n";
                if (m_completedTasks >= m_totalTasks) {
                    m_isCalculating = false;
                    QString finalMsg = QString("FINAL:1:%1").arg(m_finalArea, 0, 'f', 8);
                    QTextStream(stdout) << "[Вар.1] Итоговая площадь: " << m_finalArea << "\n";
                    broadcast(finalMsg);
                }
            }
            else if (type == 4) {
                // Корни (список через запятую)
                QString rootsStr = parts[3];
                if (!rootsStr.isEmpty()) {
                    QStringList list = rootsStr.split(",");
                    for (const QString &s : list) m_allRoots.append(s.toDouble());
                }
                QTextStream(stdout) << "[Вар.4] Прогресс: "
                                    << m_completedTasks << "/" << m_totalTasks << "\n";
                if (m_completedTasks >= m_totalTasks) {
                    m_isCalculating = false;
                    // Сортируем и убираем близкие дубликаты
                    std::sort(m_allRoots.begin(), m_allRoots.end());
                    QList<double> uniqueRoots;
                    for (double r : m_allRoots) {
                        if (uniqueRoots.isEmpty() || qAbs(r - uniqueRoots.last()) > 1e-4)
                            uniqueRoots.append(r);
                    }
                    QString rootsOut;
                    for (double r : uniqueRoots)
                        rootsOut += QString::number(r, 'f', 6) + ", ";
                    if (!rootsOut.isEmpty()) rootsOut.chop(2);
                    QString finalMsg = QString("FINAL:4:%1").arg(rootsOut);
                    QTextStream(stdout) << "[Вар.4] Итоговые корни: " << rootsOut << "\n";
                    broadcast(finalMsg);
                }
            }
        }
    }
    // --- Запуск вычислений ---
    else if (msg.startsWith("START_CALC:")) {
        if (m_isCalculating) {
            broadcast("SERVER: Вычисления уже запущены!");
            return;
        }
        // START_CALC:TYPE:A:B:STEPS
        QStringList parts = msg.split(":");
        if (parts.size() == 5) {
            int type = parts[1].toInt();
            double A = parts[2].toDouble();
            double B = parts[3].toDouble();
            int steps = parts[4].toInt();
            distributeTasks(type, A, B, steps);
        }
    }
    // --- Чат ---
    else {
        broadcast(msg);
    }
}

void Server::distributeTasks(int type, double A, double B, int steps) {
    if (m_clients.isEmpty()) {
        QTextStream(stdout) << "[ОШИБКА] Нет подключенных клиентов!\n";
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
                        << " задач (тип " << type << ")...\n";
    for (int i = 0; i < steps; ++i) {
        double a = A + i * dx;
        double b = A + (i + 1) * dx;
        QString task = QString("TASK:%1:%2:%3:%4").arg(type).arg(i).arg(a).arg(b);
        m_clients[clientIdx]->sendString(task);
        clientIdx = (clientIdx + 1) % m_clients.size();
    }
}