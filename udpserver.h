// udpserver.h
#ifndef UDPSERVER_H
#define UDPSERVER_H

#include <QHash>
#include <QHostAddress>
#include <QObject>
#include <QPair>
#include <QUdpSocket>

class UdpServer : public QObject
{
    Q_OBJECT
public:
    explicit UdpServer(quint16 port, int mode, QObject *parent = nullptr);
    bool isListening() const;

private slots:
    void readPendingDatagrams();

private:
    void handleLab6(const QByteArray &data, const QHostAddress &sender, quint16 senderPort);
    void handleLab7(const QString &msg, const QHostAddress &sender, quint16 senderPort);
    void issueTask(const QHostAddress &sender, quint16 senderPort, const QString &clientId);
    void resendTask(const QHostAddress &sender, quint16 senderPort, const QString &clientId);
    void sendLine(const QString &text, const QHostAddress &sender, quint16 senderPort);

    static QString clientKey(const QHostAddress &sender, quint16 senderPort);

    QUdpSocket *udpSocket;
    bool m_listening;
    int m_mode;
    int m_nextKeyStart;
    int m_keyStep;
    int m_maxKey;
    bool m_found;
    QHash<QString, QPair<int, int>> m_lastTask;
};

#endif // UDPSERVER_H
