#ifndef SERVERSOCKETADAPTER_H
#define SERVERSOCKETADAPTER_H

#include "socketadapter.h"

class ServerSocketAdapter : public SocketAdapter
{
    Q_OBJECT
public:
    explicit ServerSocketAdapter(QObject *parent, QUdpSocket *sharedUdpSocket, const QHostAddress &clientAddr, quint16 clientPort);
    ~ServerSocketAdapter();
};

#endif // SERVERSOCKETADAPTER_H