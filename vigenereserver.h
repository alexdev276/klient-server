#ifndef VIGENERE_SERVER_H
#define VIGENERE_SERVER_H

#include <QWidget>
#include <QUdpSocket>
#include <QHostAddress>
#include <QList>
#include <QTextEdit>

struct ClientInfo {
    QHostAddress address;
    quint16 port;
};

class VigenereServer : public QWidget
{
    Q_OBJECT
public:
    VigenereServer(QWidget *parent = nullptr);

private slots:
    void processPendingDatagrams();

private:
    QUdpSocket *m_udpSocket;
    QTextEdit *m_log;
    QList<ClientInfo> m_clients;
    QString m_cipherText;
    QString m_alphabet;
    int m_nextKeyStart;
    const int KEYS_PER_CLIENT = 5000; // Количество ключей, выдаваемых одному клиенту
};

#endif // VIGENERE_SERVER_H