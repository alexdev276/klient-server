#ifndef UDPADAPTER_H
#define UDPADAPTER_H

#include "isocketadapter.h"
#include <QUdpSocket>

class UdpAdapter : public ISocketAdapter {
    Q_OBJECT
public:
    explicit UdpAdapter(QObject *parent = nullptr, int port = 5556);
    ~UdpAdapter();
    void sendString(const QString &str) override;
    void sendTo(const QString &str, const QHostAddress &addr, int port);

private slots:
    void on_readyRead();

private:
    QUdpSocket *m_udpSocket;
    int m_port;
};

#endif // UDPADAPTER_H