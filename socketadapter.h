#ifndef SOCKETADAPTER_H
#define SOCKETADAPTER_H

#include "isocketadapter.h"
#include <QUdpSocket>
#include <QHostAddress>

class SocketAdapter : public ISocketAdapter
{
    Q_OBJECT
public:
    explicit SocketAdapter(QObject *parent, QUdpSocket *udpSocket, const QHostAddress &peerAddr, quint16 peerPort);
    virtual ~SocketAdapter();

    void sendString(const QString& str) override;
    void processDatagram(const QByteArray& datagram);

protected:
    QUdpSocket *m_udpSocket;
    QHostAddress m_peerAddress;
    quint16 m_peerPort;
    int m_msgSize;
};

#endif // SOCKETADAPTER_H