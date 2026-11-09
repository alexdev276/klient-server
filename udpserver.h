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
    void handleLab6(const QString &line, const QHostAddress &sender, quint16 senderPort);
    void handleLab7(const QString &msg, const QHostAddress &sender, quint16 senderPort);
    void issueTask(const QHostAddress &sender, quint16 senderPort, const QString &clientId);
    void resendTask(const QHostAddress &sender, quint16 senderPort, const QString &clientId);
    void broadcastLab6(const QString &text, const QHostAddress &exceptAddress, quint16 exceptPort);
    void sendLine(const QString &text, const QHostAddress &sender, quint16 senderPort);
    void printLocalAddresses() const;

    static QString clientKey(const QHostAddress &sender, quint16 senderPort);

    QUdpSocket *udpSocket;
    bool m_listening;
    int m_mode;
    int m_nextKeyStart;
    int m_keyStep;
    int m_maxKey;
    bool m_found;
    QHash<QString, QPair<int, int>> m_lastTask;
    QHash<QString, QPair<QHostAddress, quint16>> m_peers;
    QHash<QString, QString> m_peerNames;
};

#endif // UDPSERVER_H
