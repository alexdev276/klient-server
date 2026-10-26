#include "vigenereserver.h"
#include <QDataStream>
#include <QVBoxLayout>

VigenereServer::VigenereServer(QWidget *parent) : QWidget(parent), m_nextKeyStart(0)
{
    setWindowTitle("Vigenere Server (Task Distributor)");
    resize(400, 300);

    QVBoxLayout *layout = new QVBoxLayout(this);
    m_log = new QTextEdit(this);
    m_log->setReadOnly(true);
    layout->addWidget(m_log);

    // Данные из методички (алфавит)
    m_alphabet = "АБВГДЕЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯабвгдежзийклмнопрстуфхцчшщъыьэюяABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-+=.!?:;,";
    // Пример зашифрованного текста (для теста можно взять короткую строку)
    m_cipherText = "VI6XT9Й;нm-Ё.pmzЁ+мЧэО!нЫQ1";

    m_udpSocket = new QUdpSocket(this);
    bool ok = m_udpSocket->bind(50050, QUdpSocket::ShareAddress);
    if (!ok) {
        m_log->append("Ошибка: Не удалось привязать серверный сокет на порт 50050");
        return;
    }
    connect(m_udpSocket, SIGNAL(readyRead()), this, SLOT(processPendingDatagrams()));
    m_log->append("Сервер запущен. Ожидание клиентов...");
}

void VigenereServer::processPendingDatagrams()
{
    while (m_udpSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(m_udpSocket->pendingDatagramSize());
        QHostAddress senderAddress;
        quint16 senderPort;
        m_udpSocket->readDatagram(datagram.data(), datagram.size(), &senderAddress, &senderPort);

        QString request = QString::fromUtf8(datagram);

        // Клиент ищет сервер
        if (request == "DISCOVER_SERVER") {
            m_log->append(QString("Подключился клиент: %1:%2").arg(senderAddress.toString()).arg(senderPort));

            // Формируем задание
            QByteArray task;
            QDataStream out(&task, QIODevice::WriteOnly);
            out << m_alphabet << m_cipherText << m_nextKeyStart << (m_nextKeyStart + KEYS_PER_CLIENT);

            m_log->append(QString("Выдано задание: ключи от %1 до %2").arg(m_nextKeyStart).arg(m_nextKeyStart + KEYS_PER_CLIENT));
            m_nextKeyStart += KEYS_PER_CLIENT;

            // Отправляем задание клиенту
            m_udpSocket->writeDatagram(task, senderAddress, senderPort);
        }
        // Клиент нашел ключ
        else if (request.startsWith("KEY_FOUND:")) {
            QString key = request.mid(10);
            m_log->append(QString("!!! КЛЮЧ НАЙДЕН КЛИЕНТОМ %1: %2 !!!").arg(senderAddress.toString()).arg(key));
        }
    }
}