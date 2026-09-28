#include "socketadapter.h"
#include <QDataStream>
#include <QByteArray>

SocketAdapter::SocketAdapter(QObject *parent, QTcpSocket *pSock)
    : ISocketAdapter(parent), m_msgSize(-1)
{
    if (pSock == nullptr)
        m_ptcpSocket = new QTcpSocket(this);
    else
        m_ptcpSocket = pSock;

    connect(m_ptcpSocket, SIGNAL(readyRead()), this, SLOT(on_readyRead()));
    connect(m_ptcpSocket, SIGNAL(disconnected()), this, SLOT(on_disconnected()));
}

SocketAdapter::~SocketAdapter() {
    if (m_ptcpSocket) {
        m_ptcpSocket->close();
        delete m_ptcpSocket;
    }
}

void SocketAdapter::sendString(const QString &str) {
    QByteArray block;
    QDataStream sendStream(&block, QIODevice::ReadWrite);
    sendStream << quint16(0) << str;
    sendStream.device()->seek(0);
    sendStream << (quint16)(block.size() - sizeof(quint16));
    m_ptcpSocket->write(block);
}

void SocketAdapter::on_readyRead() {
    QString buff;
    QDataStream stream(m_ptcpSocket);

    while (true) {
        if (m_msgSize < 0) {
            if (m_ptcpSocket->bytesAvailable() < (int)sizeof(quint16))
                return;
            quint16 size;
            stream >> size;
            m_msgSize = size;
        } else {
            if (m_ptcpSocket->bytesAvailable() < m_msgSize)
                return;
            stream >> buff;
            emit message(buff);
            m_msgSize = -1;
        }
    }
}

void SocketAdapter::on_disconnected() {
    emit disconnected();
}