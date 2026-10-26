#include "clientsocketadapter.h"

ClientSocketAdapter::ClientSocketAdapter(QObject *parent, const QHostAddress &serverAddr, quint16 serverPort)
    : SocketAdapter(parent, new QUdpSocket(), serverAddr, serverPort)
{
    // Привязываем клиентский сокет к случайному свободному порту
    m_udpSocket->bind(QHostAddress::Any, 0);

    // Подключаем сигнал readyRead для приема ответов от сервера
    connect(m_udpSocket, &QUdpSocket::readyRead, this, [this]() {
        while (m_udpSocket->hasPendingDatagrams()) {
            QByteArray datagram;
            datagram.resize(int(m_udpSocket->pendingDatagramSize()));
            QHostAddress sender;
            quint16 senderPort;
            m_udpSocket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);
            processDatagram(datagram);
        }
    });
}

ClientSocketAdapter::~ClientSocketAdapter() {}