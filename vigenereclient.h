#ifndef VIGENERE_CLIENT_H
#define VIGENERE_CLIENT_H

#include <QWidget>
#include <QUdpSocket>
#include <QThreadPool>
#include <QTextEdit>

class VigenereClient : public QWidget
{
    Q_OBJECT
public:
    VigenereClient(QWidget *parent = nullptr);

private slots:
    void processPendingDatagrams();
    void onKeyFound(const QString &key);

private:
    QUdpSocket *m_udpSocket;
    quint16 m_listenPort;
    QThreadPool m_threadPool;
    QTextEdit *m_log;
};

#endif // VIGENERE_CLIENT_H