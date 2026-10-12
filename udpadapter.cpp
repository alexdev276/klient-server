#include "udpadapter.h"

UdpAdapter::UdpAdapter(QObject *parent, quint16 port)
    : QObject(parent), m_socket(new QUdpSocket(this))
{
    m_socket->bind(QHostAddress::AnyIPv4, port,
                   QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);
    connect(m_socket, &QUdpSocket::readyRead, this, &UdpAdapter::on_readyRead);
}

void UdpAdapter::sendBroadcast(const QString &msg)
{
    m_socket->writeDatagram(msg.toUtf8(),
                            QHostAddress::Broadcast, 5556);
}

void UdpAdapter::on_readyRead()
{
    while (m_socket->hasPendingDatagrams()) {
        QByteArray data;
        data.resize(int(m_socket->pendingDatagramSize()));
        m_socket->readDatagram(data.data(), data.size());
        emit message(QString::fromUtf8(data));
    }
}