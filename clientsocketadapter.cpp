#include "clientsocketadapter.h"
#include <QTextStream>
#include <QDebug>

ClientSocketAdapter::ClientSocketAdapter(QObject *parent)
    : SocketAdapter(parent, nullptr)
{
    connect(m_ptcpSocket, &QTcpSocket::errorOccurred, this, [](QAbstractSocket::SocketError error) {
        qDebug() << "[КЛИЕНТ] Ошибка сокета:" << error;
    });

    qDebug() << "[КЛИЕНТ] Попытка подключения к 127.0.0.1:5555...";
    m_ptcpSocket->connectToHost("127.0.0.1", 5555); // Порт 5555 вместо 1024!
}