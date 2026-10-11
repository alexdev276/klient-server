#ifndef SERVER_H
#define SERVER_H
#include <QObject>
#include <QTcpServer>
#include <QList>
#include "isocketadapter.h"

class Server : public QObject {
    Q_OBJECT
public:
    explicit Server(int nPort, QObject *parent = nullptr);
    ~Server();
private slots:
    void on_newConnection();
    void on_disconnected();
    void on_message(QString msg);
private:
    void distributeTasks(double A, double B, int steps);
    void broadcast(const QString &msg);

    QTcpServer *m_ptcpServer;
    QList<ISocketAdapter*> m_clients;

    bool m_isCalculating;
    int m_totalTasks, m_completedTasks;
    double m_finalArea;
};
#endif // SERVER_H