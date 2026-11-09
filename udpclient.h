// udpclient.h
#ifndef UDPCLIENT_H
#define UDPCLIENT_H

#include <QHostAddress>
#include <QObject>
#include <QSocketNotifier>
#include <QString>
#include <QUdpSocket>

class QTimer;

class UdpClient : public QObject
{
    Q_OBJECT
public:
    explicit UdpClient(int mode,
                       const QString &serverHost = QStringLiteral("192.168.8.159"),
                       quint16 serverPort = 0,
                       const QString &name = QString(),
                       QObject *parent = nullptr);

public slots:
    void sendLab6Message(const QString &msg);
    void requestLab7Task();

private slots:
    void readPendingDatagrams();
    void onStdinReady();
    void onRetryTimeout();
    void onRegisterTick();

private:
    void sendRequest(const QString &msg);
    void reportFound(const QString &key);
    void handleLab6Reply(const QString &msg);
    void sendLab6Hello();
    void sendLab6Bye();

    QUdpSocket *udpSocket;
    int m_mode;
    QSocketNotifier *m_stdinNotifier;
    QHostAddress m_serverAddress;
    quint16 m_serverPort;
    QTimer *m_retryTimer;
    int m_attempts;
    QString m_name;
    QTimer *m_regTimer;
    bool m_serverKnown;
};

#endif // UDPCLIENT_H
