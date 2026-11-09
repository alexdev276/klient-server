// udpserver.h
#ifndef UDPSERVER_H
#define UDPSERVER_H

#include <QObject>
#include <QUdpSocket>

class UdpServer : public QObject
{
    Q_OBJECT
public:
    explicit UdpServer(quint16 port, int mode, QObject *parent = nullptr);

private slots:
    void readPendingDatagrams();

private:
    QUdpSocket *udpSocket;
    int m_mode;
    int m_nextKeyStart;
    int m_keyStep;
    int m_maxKey;
};

#endif // UDPSERVER_H