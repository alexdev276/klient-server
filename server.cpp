#include "server.h"
#include "serversocketadapter.h"
#include <QVBoxLayout>
#include <QHostAddress>

Server::Server(quint16 port, QWidget *parent) : QWidget(parent)
{
    setWindowTitle("UDP Chat Server");
    resize(400, 300);

    QVBoxLayout *layout = new QVBoxLayout(this);
    m_log = new QTextEdit(this);
    m_log->setReadOnly(true);
    layout->addWidget(m_log);

    m_udpSocket = new QUdpSocket(this);
    if (!m_udpSocket->bind(QHostAddress::Any, port)) {
        m_log->append("Ошибка привязки UDP сервера к порту " + QString::number(port));
        return;
    }

    connect(m_udpSocket, &QUdpSocket::readyRead, this, &Server::on_readyRead);
    m_log->append("UDP Сервер запущен на порту " + QString::number(port));
}

Server::~Server() {}

void Server::on_readyRead()
{
    while (m_udpSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(int(m_udpSocket->pendingDatagramSize()));
        QHostAddress senderAddr;
        quint16 senderPort;
        m_udpSocket->readDatagram(datagram.data(), datagram.size(), &senderAddr, &senderPort);

        QString key = senderAddr.toString() + ":" + QString::number(senderPort);

        // Если от этого клиента еще нет адаптера, создаем его
        if (!m_clients.contains(key)) {
            ServerSocketAdapter *adapter = new ServerSocketAdapter(this, m_udpSocket, senderAddr, senderPort);
            connect(adapter, &ISocketAdapter::message, this, &Server::on_message);
            connect(adapter, &ISocketAdapter::disconnected, this, &Server::on_disconnected);

            m_clients.insert(key, adapter);
            adapter->sendString("Добро пожаловать в UDP чат!");
            m_log->append("Подключился новый клиент: " + key);
        }

        // Передаем данные адаптеру для расшифровки
        m_clients[key]->processDatagram(datagram);
    }
}

void Server::on_message(const QString& msg)
{
    m_log->append("Получено: " + msg);

    // Рассылка сообщения всем клиентам (кроме отправителя, если нужно)
    for (ServerSocketAdapter *client : m_clients) {
        client->sendString(msg);
    }
}

void Server::on_disconnected()
{
    // В UDP нет явного отключения, логика удаления должна быть по таймауту
    // Для простоты лабы оставляем пустым
}