#include "serversocketadapter.h"

ServerSocketAdapter::ServerSocketAdapter(QTcpSocket *socket, QObject *parent)
    : ISocketAdapter(parent), m_pSocket(socket)
{
    m_pSocket->setParent(this);
    connect(m_pSocket, &QTcpSocket::readyRead,    this, &ServerSocketAdapter::on_readyRead);
    connect(m_pSocket, &QTcpSocket::disconnected, this, &ServerSocketAdapter::on_disconnected);
}

void ServerSocketAdapter::sendString(const QString &str)
{
    if (!m_pSocket) return;
    m_pSocket->write(str.toUtf8());
    m_pSocket->flush();
}

void ServerSocketAdapter::on_readyRead()
{
    m_buffer.append(m_pSocket->readAll());
    while (m_buffer.contains('\n')) {
        int idx = m_buffer.indexOf('\n');
        QString line = QString::fromUtf8(m_buffer.left(idx)).trimmed();
        m_buffer.remove(0, idx + 1);
        if (!line.isEmpty())
            emit message(line);
    }
}

void ServerSocketAdapter::on_disconnected()
{
    emit disconnected();
}