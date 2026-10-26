#include "serversocketadapter.h"

ServerSocketAdapter::ServerSocketAdapter(QObject *parent, QUdpSocket *sharedUdpSocket, const QHostAddress &clientAddr, quint16 clientPort)
    : SocketAdapter(parent, sharedUdpSocket, clientAddr, clientPort)
{
    // Серверный адаптер не создает свой сокет, а использует общий сокет сервера
}

ServerSocketAdapter::~ServerSocketAdapter() {}