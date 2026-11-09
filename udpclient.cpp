// udpclient.cpp
#include "udpclient.h"
#include "workerthread.h"

#include <QCoreApplication>
#include <QNetworkDatagram>
#include <QStringList>
#include <QThreadPool>
#include <QTimer>

#include <iostream>
#include <string>
#include <unistd.h>

using namespace std;

static const int kRetryIntervalMs = 3000;
static const int kMaxAttempts = 5;

UdpClient::UdpClient(int mode, const QString &serverHost, quint16 serverPort, QObject *parent)
    : QObject(parent)
    , udpSocket(new QUdpSocket(this))
    , m_mode(mode)
    , m_stdinNotifier(nullptr)
    , m_serverAddress(serverHost)
    , m_serverPort(serverPort)
    , m_retryTimer(nullptr)
    , m_attempts(0)
{
    if (m_serverPort == 0)
        m_serverPort = (m_mode == 6) ? 5555 : 50050;

    if (!udpSocket->bind(QHostAddress::AnyIPv4, 0))
        cerr << "[Клиент] Не удалось занять локальный UDP-порт" << endl;

    connect(udpSocket, &QUdpSocket::readyRead, this, &UdpClient::readPendingDatagrams);

    if (m_mode == 6) {
        m_stdinNotifier = new QSocketNotifier(STDIN_FILENO, QSocketNotifier::Read, this);
        connect(m_stdinNotifier, &QSocketNotifier::activated, this, &UdpClient::onStdinReady);
        cout << "[ЛР6] Клиент запущен. Сервер: " << m_serverAddress.toString().toStdString()
             << ':' << m_serverPort << endl;
        cout << "[ЛР6] Введите сообщение (exit для выхода):" << endl;
    } else {
        m_retryTimer = new QTimer(this);
        m_retryTimer->setSingleShot(true);
        m_retryTimer->setInterval(kRetryIntervalMs);
        connect(m_retryTimer, &QTimer::timeout, this, &UdpClient::onRetryTimeout);
        cout << "[ЛР7] Клиент запущен. Сервер: " << m_serverAddress.toString().toStdString()
             << ':' << m_serverPort << endl;
    }
}

void UdpClient::sendLab6Message(const QString &msg)
{
    udpSocket->writeDatagram(msg.toUtf8(), m_serverAddress, m_serverPort);
    cout << "[ЛР6] Отправлено: " << msg.toStdString() << endl;
}

void UdpClient::requestLab7Task()
{
    cout << "[ЛР7] Отправка запроса HELLO..." << endl;
    sendRequest("HELLO");
}

void UdpClient::sendRequest(const QString &msg)
{
    udpSocket->writeDatagram(msg.toUtf8(), m_serverAddress, m_serverPort);
    ++m_attempts;

    if (m_retryTimer)
        m_retryTimer->start();
}

void UdpClient::onRetryTimeout()
{
    if (m_attempts >= kMaxAttempts) {
        cout << "[ЛР7] Сервер не отвечает, попытки исчерпаны. Выход." << endl;
        QCoreApplication::quit();
        return;
    }

    cout << "[ЛР7] Нет ответа от сервера, повторный запрос ("
         << m_attempts << '/' << kMaxAttempts << ")..." << endl;
    sendRequest("RETRY");
}

void UdpClient::onStdinReady()
{
    string line;
    if (!getline(cin, line)) {
        if (m_stdinNotifier)
            m_stdinNotifier->setEnabled(false);
        return;
    }

    const QString msg = QString::fromStdString(line).trimmed();
    if (msg == "exit") {
        QCoreApplication::quit();
        return;
    }
    if (msg.isEmpty())
        return;

    sendLab6Message(msg);
}

void UdpClient::reportFound(const QString &key)
{
    if (m_retryTimer)
        m_retryTimer->stop();

    udpSocket->writeDatagram(QString("FOUND: " + key).toUtf8(), m_serverAddress, m_serverPort);
    cout << "[ЛР7] Результат отправлен серверу." << endl;

    QTimer::singleShot(200, [] { QCoreApplication::quit(); });
}

void UdpClient::readPendingDatagrams()
{
    while (udpSocket->hasPendingDatagrams()) {
        const QNetworkDatagram datagram = udpSocket->receiveDatagram();
        const QString msg = QString::fromUtf8(datagram.data()).trimmed();

        if (m_mode == 6) {
            cout << "[ЛР6] Ответ сервера: " << msg.toStdString() << endl;
            continue;
        }

        if (m_retryTimer)
            m_retryTimer->stop();
        m_attempts = 0;

        if (msg == "NO_MORE_TASKS") {
            cout << "[ЛР7] Сервер сообщил, что задания закончились. Выход." << endl;
            QCoreApplication::quit();
            return;
        }

        const QStringList parts = msg.split('|');
        if (parts.size() != 3) {
            cout << "[ЛР7] Некорректное задание от сервера, пропущено." << endl;
            continue;
        }

        bool startOk = false;
        bool endOk = false;
        const QString cipherText = parts[0];
        const int startRange = parts[1].toInt(&startOk);
        const int endRange = parts[2].toInt(&endOk);

        if (!startOk || !endOk || startRange > endRange) {
            cout << "[ЛР7] Некорректный диапазон ключей, задание пропущено." << endl;
            continue;
        }

        cout << "[ЛР7] Получено задание. Диапазон: " << startRange << " - " << endRange << endl;

        auto *worker = new WorkerThread(cipherText, startRange, endRange);
        connect(worker, &WorkerThread::keyFound, this, [this](const QString &key, const QString &text) {
            cout << "[ЛР7] КЛЮЧ НАЙДЕН: " << key.toStdString() << endl;
            cout << "[ЛР7] Текст: " << text.toStdString() << endl;
            reportFound(key);
        });
        connect(worker, &WorkerThread::finishedWithoutResult, this, [this]() {
            cout << "[ЛР7] В данном диапазоне ключ не найден. Запрос следующего." << endl;
            sendRequest("NEXT");
        });
        QThreadPool::globalInstance()->start(worker);
    }
}
