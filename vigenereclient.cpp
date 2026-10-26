#include "vigenereclient.h"
#include "vigenereworker.h"
#include <QDataStream>
#include <QVBoxLayout>

VigenereClient::VigenereClient(QWidget *parent) : QWidget(parent)
{
    setWindowTitle("Vigenere Client (Brute-forcer)");
    resize(400, 300);

    QVBoxLayout *layout = new QVBoxLayout(this);
    m_log = new QTextEdit(this);
    m_log->setReadOnly(true);
    layout->addWidget(m_log);

    m_udpSocket = new QUdpSocket(this);
    // Привязываемся к случайному свободному порту для получения задач
    m_udpSocket->bind(QHostAddress::Any, 0);
    m_listenPort = m_udpSocket->localPort();

    connect(m_udpSocket, SIGNAL(readyRead()), this, SLOT(processPendingDatagrams()));

    // Устанавливаем максимальное количество потоков (по числу ядер)
    m_threadPool.setMaxThreadCount(QThread::idealThreadCount());
    m_log->append(QString("Клиент запущен. Порт: %1. Поиск сервера...").arg(m_listenPort));

    // Отправляем широковещательный запрос на поиск сервера
    QByteArray request = "DISCOVER_SERVER";
    m_udpSocket->writeDatagram(request, QHostAddress::Broadcast, 50050);
}

void VigenereClient::processPendingDatagrams()
{
    while (m_udpSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(m_udpSocket->pendingDatagramSize());
        m_udpSocket->readDatagram(datagram.data(), datagram.size());

        QDataStream in(&datagram, QIODevice::ReadOnly);
        QString alphabet, cipherText;
        int startKey, endKey;
        in >> alphabet >> cipherText >> startKey >> endKey;

        m_log->append(QString("Получено задание: перебор от %1 до %2").arg(startKey).arg(endKey));

        // Создаем задачу для пула потоков
        VigenereWorker *worker = new VigenereWorker(cipherText, alphabet, startKey, endKey);
        connect(worker, &VigenereWorker::resultFound, this, &VigenereClient::onKeyFound);

        // Запускаем в отдельном потоке
        m_threadPool.start(worker);
    }
}

void VigenereClient::onKeyFound(const QString &key)
{
    m_log->append(QString("*** УСПЕХ! Ключ найден: %1 ***").arg(key));
    // Отправляем результат на сервер
    QByteArray result = "KEY_FOUND:" + key.toUtf8();
    m_udpSocket->writeDatagram(result, QHostAddress::Broadcast, 50050);
}