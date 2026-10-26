#include "socketadapter.h"
#include <QDataStream>

SocketAdapter::SocketAdapter(QObject *parent, QUdpSocket *udpSocket, const QHostAddress &peerAddr, quint16 peerPort)
    : ISocketAdapter(parent), m_udpSocket(udpSocket), m_peerAddress(peerAddr), m_peerPort(peerPort), m_msgSize(-1)
{
}

SocketAdapter::~SocketAdapter() {}

void SocketAdapter::sendString(const QString& str)
{
    QByteArray block;
    QDataStream sendStream(&block, QIODevice::ReadWrite);
    sendStream << quint16(0) << str;
    sendStream.device()->seek(0);
    sendStream << (quint16)(block.size() - sizeof(quint16));

    m_udpSocket->writeDatagram(block, m_peerAddress, m_peerPort);
}

void SocketAdapter::processDatagram(const QByteArray& datagram)
{
    QDataStream stream(datagram);
    while (!stream.atEnd()) {
        if (m_msgSize < 0) {
            // Читаем размер сообщения (2 байта)
            if (stream.device()->bytesAvailable() < static_cast<qint64>(sizeof(quint16)))
                break;
            stream >> m_msgSize;
        } else {
            // Читаем само сообщение
            if (stream.device()->bytesAvailable() < static_cast<qint64>(m_msgSize))
                break;

            QByteArray data(m_msgSize, Qt::Uninitialized);
            stream.readRawData(data.data(), m_msgSize);
            emit message(QString::fromUtf8(data));
            m_msgSize = -1; // Сброс для следующего пакета
        }
    }
}