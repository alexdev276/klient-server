#ifndef UDPCHAT_H
#define UDPCHAT_H

#include <QWidget>
#include <QUdpSocket>
#include <QHostAddress>
#include <QHash>
#include <QPair>

class QLineEdit;
class QPlainTextEdit;
class QPushButton;
class QLabel;
class QTimer;

// ЛР 6. Обмен сообщениями между клиентом и сервером по протоколу UDP.
class UdpChat : public QWidget
{
    Q_OBJECT
public:
    enum Mode { ServerMode, ClientMode };

    explicit UdpChat(Mode mode, QWidget *parent = nullptr);

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onReadyRead();
    void onSendClicked();
    void onRegisterTick();
    void onReconnectClicked();

private:
    void log(const QString &text);
    void processPacket(const QString &line,
                       const QHostAddress &sender, quint16 senderPort);
    void sendLine(const QString &line,
                  const QHostAddress &addr, quint16 port);
    void sendToServer(const QString &line);
    void broadcastToPeers(const QString &line,
                          const QHostAddress &exceptAddr = QHostAddress(),
                          quint16 exceptPort = 0);
    void refreshStatus();
    QString peerKey(const QHostAddress &addr, quint16 port) const;

    Mode           m_mode;
    QUdpSocket    *m_socket;
    quint16        m_port;
    QString        m_name;

    QLineEdit      *m_ipEdit       = nullptr;
    QLineEdit      *m_nameEdit     = nullptr;
    QLineEdit      *m_msgEdit      = nullptr;
    QPlainTextEdit *m_logView      = nullptr;
    QPushButton    *m_sendBtn      = nullptr;
    QPushButton    *m_connectBtn   = nullptr;
    QLabel         *m_statusLabel  = nullptr;
    QTimer         *m_regTimer     = nullptr;

    bool           m_serverKnown;
    QHostAddress   m_serverAddr;
    quint16        m_serverPort;

    // Реестр клиентов (только на сервере).
    QHash<QString, QPair<QHostAddress, quint16>> m_peers;
    QHash<QString, QString>                      m_peerNames;
};

#endif // UDPCHAT_H
