#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QTcpServer>
#include <QUdpSocket>
#include <QList>
#include "isocketadapter.h"
#include "udpadapter.h"

class Server : public QObject {
    Q_OBJECT
public:
    explicit Server(int tcpPort, int udpPort, QObject *parent = nullptr);
    ~Server();

private slots:
    void on_newConnection();
    void on_disconnected();
    void on_message(QString msg);
    void on_udpMessage(QString msg);

private:
    void distributeTasks(int type, double A, double B, int steps);
    void broadcast(const QString &msg);

    QTcpServer *m_ptcpServer;
    UdpAdapter *m_udpAdapter;
    QList<ISocketAdapter*> m_clients;

    // Состояние вычислений
    int m_taskType;
    bool m_isCalculating;
    int m_totalTasks, m_completedTasks;
    double m_finalArea;
    QList<double> m_allRoots;
};

#endif // SERVER_H