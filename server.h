#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QTcpServer>
#include <QList>
#include "isocketadapter.h"
#include "udpadapter.h"

class Server : public QObject
{
    Q_OBJECT
public:
    explicit Server(int tcpPort, int udpPort,
                    int taskType = 1, double A = 0.0,
                    double B = 4 * M_PI, int steps = 400,
                    QObject *parent = nullptr);
    ~Server();

private slots:
    void on_newConnection();
    void on_disconnected();
    void on_message(QString msg);
    void on_udpMessage(QString msg);

private:
    void sendTo(ISocketAdapter *sock, const QString &msg);
    void broadcast(const QString &msg);
    void distributeTasks(int type, double A, double B, int steps);

    QTcpServer  *m_ptcpServer;
    UdpAdapter  *m_udpAdapter;
    QList<ISocketAdapter*> m_clients;

    int    m_taskType;
    double m_A;
    double m_B;
    int    m_steps;
    bool   m_calculationStarted;

    bool           m_isCalculating;
    int            m_totalTasks;
    int            m_completedTasks;
    double         m_finalArea;
    QList<double>  m_allRoots;
};

#endif // SERVER_H