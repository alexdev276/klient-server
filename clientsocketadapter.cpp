#include "clientsocketadapter.h"
#include <QDebug>

ClientSocketAdapter::ClientSocketAdapter(QObject *parent)
    : SocketAdapter(parent, nullptr)
{
    connect(m_ptcpSocket, &QTcpSocket::errorOccurred, this, [](QAbstractSocket::SocketError error) {
        qDebug() << "[КЛИЕНТ TCP] Ошибка:" << error;
    });

    qDebug() << "[КЛИЕНТ TCP] Подключение к 127.0.0.1:5555...";
    m_ptcpSocket->connectToHost("127.0.0.1", 5555);
}