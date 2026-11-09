// udpclient.h
#ifndef UDPCLIENT_H
#define UDPCLIENT_H

#include <QObject>
#include <QUdpSocket>
#include <QSocketNotifier>

class UdpClient : public QObject
{
    Q_OBJECT
public:
    explicit UdpClient(int mode, QObject *parent = nullptr);

public slots:
    void sendLab6Message(const QString &msg, const QString &host, quint16 port);
    void requestLab7Task(const QString &host, quint16 serverPort);

private slots:
    void readPendingDatagrams();
    void onStdinReady();

private:
    QUdpSocket *udpSocket;
    int m_mode;
    QSocketNotifier *m_stdinNotifier;
    QHostAddress m_serverAddress;
    quint16 m_serverPort;
};

#endif // UDPCLIENT_H