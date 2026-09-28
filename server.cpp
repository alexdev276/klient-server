#include "server.h"
#include "serversocketadapter.h"
#include <QTextStream>
#include <QDebug>

Server::Server(int nPort, QObject *parent)
    : QObject(parent), m_ptcpServer(new QTcpServer(this))
{
    connect(m_ptcpServer, SIGNAL(newConnection()), SLOT(on_newConnection()));

    if (!m_ptcpServer->listen(QHostAddress::Any, nPort)) {
        m_ptcpServer->close();
        qDebug() << "Ошибка запуска сервера:" << m_ptcpServer->errorString();
        throw m_ptcpServer->errorString();
    }
    qDebug() << "Сервер успешно запущен на порту" << nPort;
}

Server::~Server() {
    qDeleteAll(m_clients);
    m_clients.clear();
}

void Server::on_newConnection() {
    QTextStream(stdout) << "Новое подключение клиента" << endl;

    QTcpSocket *pClientSock = m_ptcpServer->nextPendingConnection();
    ISocketAdapter *pSockHandle = new ServerSocketAdapter(pClientSock, this);
    m_clients.push_back(pSockHandle);

    pSockHandle->sendString("connect");

    connect(pSockHandle, SIGNAL(disconnected()), SLOT(on_disconnected()));
    connect(pSockHandle, SIGNAL(message(QString)), SLOT(on_message(QString)));
}

void Server::on_disconnected() {
    QTextStream(stdout) << "Клиент отключился" << endl;
    ISocketAdapter *client = qobject_cast<ISocketAdapter*>(sender());
    if (client) {
        m_clients.removeOne(client);
        delete client;
    }
}

void Server::on_message(QString msg) {
    qDebug() << "Получено сообщение от клиента:" << msg;
    // Рассылка всем клиентам (ЛР4)
    foreach (ISocketAdapter *sock, m_clients) {
        sock->sendString(msg);
    }
}