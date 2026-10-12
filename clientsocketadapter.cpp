#include "clientsocketadapter.h"
#include <QDebug>

ClientSocketAdapter::ClientSocketAdapter(QObject *parent)
    : SocketAdapter(parent, nullptr)
{
    connect(m_ptcpSocket, &QTcpSocket::errorOccurred, this, [](QAbstractSocket::SocketError error) {
        qDebug() << "[КЛИЕНТ TCP] Ошибка:" << error;
    });

    qDebug() << "[КЛИЕНТ TCP] Подключение к 172.20.10.2:5555...";
    m_ptcpSocket->connectToHost("192.168.8.158", 5555);
}