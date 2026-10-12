#ifndef SERVERSOCKETADAPTER_H
#define SERVERSOCKETADAPTER_H

#include "isocketadapter.h"
#include <QTcpSocket>
#include <QByteArray>

class ServerSocketAdapter : public ISocketAdapter
{
    Q_OBJECT
public:
    explicit ServerSocketAdapter(QTcpSocket *socket, QObject *parent = nullptr);

    void sendString(const QString &str) override;

private slots:
    void on_readyRead();
    void on_disconnected();

private:
    QTcpSocket *m_pSocket;
    QByteArray  m_buffer;
};

#endif // SERVERSOCKETADAPTER_H