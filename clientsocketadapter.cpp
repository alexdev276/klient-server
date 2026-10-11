#include "clientsocketadapter.h"

ClientSocketAdapter::ClientSocketAdapter(QObject *parent)
    : SocketAdapter(parent, nullptr)
{
    // Клиент сам создает сокет (в базовом классе) и подключается к серверу
    m_ptcpSocket->connectToHost("127.0.0.1", 1024);
}