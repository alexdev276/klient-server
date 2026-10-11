#include "udpadapter.h"
#include <QDebug>

UdpAdapter::UdpAdapter(QObject *parent, int port)
    : ISocketAdapter(parent), m_port(port)
{
    m_udpSocket = new QUdpSocket(this);
    bool result = m_udpSocket->bind(QHostAddress::Any, m_port);
    if (result) {
        qDebug() << "[UDP] Привязан к порту" << m_port;
        connect(m_udpSocket, SIGNAL(readyRead()), this, SLOT(on_readyRead()));
    } else {
        qDebug() << "[UDP] Ошибка привязки к порту:" << m_udpSocket->errorString();
    }
}

UdpAdapter::~UdpAdapter() {
    m_udpSocket->close();
}

void UdpAdapter::sendString(const QString &str) {
    QByteArray data = str.toUtf8();
    m_udpSocket->writeDatagram(data, QHostAddress::Broadcast, m_port);
}

void UdpAdapter::sendTo(const QString &str, const QHostAddress &addr, int port) {
    QByteArray data = str.toUtf8();
    m_udpSocket->writeDatagram(data, addr, port);
}

void UdpAdapter::on_readyRead() {
    while (m_udpSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(m_udpSocket->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;
        m_udpSocket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);
        QString msg = QString::fromUtf8(datagram);
        qDebug() << "[UDP] Получено:" << msg << "от" << sender.toString() << ":" << senderPort;
        emit message(msg);
    }
}