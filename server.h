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
    QTcpServer *m_ptcpServer;
    QList<ISocketAdapter*> m_clients;
};

#endif // SERVER_H