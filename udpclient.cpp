// udpclient.cpp
#include "udpclient.h"
#include "workerthread.h"
#include <QNetworkDatagram>
#include <QStringList>
#include <QThreadPool>
#include <QCoreApplication> // <--- ДОБАВЛЕНА ЭТА СТРОКА
#include <iostream>
#include <unistd.h>

using namespace std;

UdpClient::UdpClient(int mode, QObject *parent)
    : QObject(parent), m_mode(mode), m_stdinNotifier(nullptr)
{
    udpSocket = new QUdpSocket(this);
    udpSocket->bind(QHostAddress::AnyIPv4, 0);
    connect(udpSocket, &QUdpSocket::readyRead, this, &UdpClient::readPendingDatagrams);

    if (m_mode == 6) {
        m_stdinNotifier = new QSocketNotifier(STDIN_FILENO, QSocketNotifier::Read, this);
        connect(m_stdinNotifier, &QSocketNotifier::activated, this, &UdpClient::onStdinReady);
        cout << "[ЛР6] Клиент запущен. Введите сообщение (exit для выхода):" << endl;
    } else {
        cout << "[ЛР7] Клиент запущен." << endl;
    }
}

void UdpClient::sendLab6Message(const QString &msg, const QString &host, quint16 port)
{
    QByteArray data = msg.toUtf8();
    udpSocket->writeDatagram(data, QHostAddress(host), port);
    cout << "[ЛР6] Отправлено: " << msg.toStdString() << endl;
}

void UdpClient::requestLab7Task(const QString &host, quint16 serverPort)
{
    m_serverAddress = QHostAddress(host);
    m_serverPort = serverPort;
    cout << "[ЛР7] Отправка запроса HELLO..." << endl;
    udpSocket->writeDatagram("HELLO", m_serverAddress, m_serverPort);
}

void UdpClient::onStdinReady()
{
    string line;
    getline(cin, line);
    if (line == "exit") {
        QCoreApplication::quit();
    } else {
        sendLab6Message(QString::fromStdString(line), "127.0.0.1", 5555);
    }
}

void UdpClient::readPendingDatagrams()
{
    while (udpSocket->hasPendingDatagrams()) {
        QNetworkDatagram datagram = udpSocket->receiveDatagram();
        QString msg = QString::fromUtf8(datagram.data());

        if (m_mode == 6) {
            cout << "[ЛР6] Ответ сервера: " << msg.toStdString() << endl;
        }
        else if (m_mode == 7) {
            if (msg == "NO_MORE_TASKS") {
                cout << "[ЛР7] Сервер сообщил, что задания закончились. Выход." << endl;
                QCoreApplication::quit();
                return;
            }

            QStringList parts = msg.split("|");
            if (parts.size() == 3) {
                QString cipherText = parts[0];
                int startRange = parts[1].toInt();
                int endRange = parts[2].toInt();

                cout << "[ЛР7] Получено задание. Диапазон: " << startRange << " - " << endRange << endl;

                WorkerThread *worker = new WorkerThread(cipherText, startRange, endRange, this);
                connect(worker, &WorkerThread::keyFound, this, [this, datagram](const QString &key, const QString &text){
                    cout << "[ЛР7] КЛЮЧ НАЙДЕН: " << key.toStdString() << endl;
                    cout << "[ЛР7] Текст: " << text.toStdString() << endl;
                    QString resultMsg = "FOUND: " + key;
                    udpSocket->writeDatagram(resultMsg.toUtf8(), datagram.senderAddress(), datagram.senderPort());
                });
                connect(worker, &WorkerThread::finishedWithoutResult, this, [this, datagram](){
                    cout << "[ЛР7] В данном диапазоне ключ не найден. Запрос следующего." << endl;
                    udpSocket->writeDatagram("NEXT", datagram.senderAddress(), datagram.senderPort());
                });
                QThreadPool::globalInstance()->start(worker);
            }
        }
    }
}