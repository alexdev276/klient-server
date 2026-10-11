#include "clientsocketadapter.h"
#include <QTextStream>

ClientSocketAdapter::ClientSocketAdapter(QObject *parent)
    : SocketAdapter(parent, nullptr)
{
    connect(m_ptcpSocket, &QTcpSocket::errorOccurred, this, [this](QAbstractSocket::SocketError) {
        QTextStream(stdout) << "[КЛИЕНТ ОШИБКА] " << m_ptcpSocket->errorString() << "\n";
    });

    connect(m_ptcpSocket, &QTcpSocket::connected, this, []() {
        QTextStream(stdout) << "[КЛИЕНТ] Успешно подключен к серверу!\n";
    });

    QTextStream(stdout) << "[КЛИЕНТ] Попытка подключения к 127.0.0.1:1024...\n";
    m_ptcpSocket->connectToHost("192.168.0.1", 5555);
}