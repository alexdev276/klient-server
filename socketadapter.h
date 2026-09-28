#ifndef SOCKETADAPTER_H
#define SOCKETADAPTER_H

#include "isocketadapter.h"
#include <QTcpSocket>

class SocketAdapter : public ISocketAdapter {
    Q_OBJECT
public:
    explicit SocketAdapter(QObject *parent = nullptr, QTcpSocket *pSock = nullptr);
    virtual ~SocketAdapter();

    void sendString(const QString &str) override;

protected slots:
    void on_readyRead();
    void on_disconnected();

protected:
    QTcpSocket *m_ptcpSocket;
    int m_msgSize;
};

#endif // SOCKETADAPTER_H