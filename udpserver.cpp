// udpserver.cpp
#include "udpserver.h"
#include <QNetworkDatagram>
#include <iostream>

using namespace std;

UdpServer::UdpServer(quint16 port, int mode, QObject *parent)
    : QObject(parent), m_mode(mode)
{
    udpSocket = new QUdpSocket(this);
    udpSocket->bind(QHostAddress::Any, port);
    connect(udpSocket, &QUdpSocket::readyRead, this, &UdpServer::readPendingDatagrams);

    if (m_mode == 6) {
        cout << "[ЛР6] UDP Сервер запущен на порту " << port << endl;
    } else {
        cout << "[ЛР7] Сервер распределенных вычислений запущен на порту " << port << endl;
        m_nextKeyStart = 1000;
        m_keyStep = 1000;
        m_maxKey = 10000;
    }
}

void UdpServer::readPendingDatagrams()
{
    while (udpSocket->hasPendingDatagrams()) {
        QNetworkDatagram datagram = udpSocket->receiveDatagram();
        QString msg = QString::fromUtf8(datagram.data());

        if (m_mode == 6) {
            cout << "[ЛР6] Получено от клиента: " << msg.toStdString() << endl;
            QString reply = "Сервер получил: " + msg;
            udpSocket->writeDatagram(reply.toUtf8(), datagram.senderAddress(), datagram.senderPort());
        }
        else if (m_mode == 7) {
            if (msg == "HELLO" || msg == "NEXT") {
                if (m_nextKeyStart <= m_maxKey) {
                    int end = m_nextKeyStart + m_keyStep - 1;
                    if (end > m_maxKey) end = m_maxKey;
                    QString cipherText = "ДЫввЭфЫdФа13ХІІ2ЫZІПРрggЦУndтЩ0БюggЮхоаЭЩв"; // Пример
                    QString task = cipherText + "|" + QString::number(m_nextKeyStart) + "|" + QString::number(end);
                    udpSocket->writeDatagram(task.toUtf8(), datagram.senderAddress(), datagram.senderPort());
                    cout << "[ЛР7] Выдан диапазон: " << m_nextKeyStart << " - " << end << endl;
                    m_nextKeyStart += m_keyStep;
                } else {
                    udpSocket->writeDatagram("NO_MORE_TASKS", datagram.senderAddress(), datagram.senderPort());
                    cout << "[ЛР7] Задания закончились." << endl;
                }
            }
            else if (msg.startsWith("FOUND")) {
                cout << "[ЛР7] Клиент нашел ключ: " << msg.toStdString() << endl;
            }
        }
    }
}