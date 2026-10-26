#ifndef KEYSEARCHSERVER_H
#define KEYSEARCHSERVER_H

#include <QWidget>
#include <QUdpSocket>
#include <QHostAddress>
#include <QHash>
#include <QStringList>

class QLineEdit;
class QPlainTextEdit;
class QPushButton;
class QLabel;

// ЛР 7. UDP-сервер распределённого поиска ключевой фразы шифра Виженера.
class KeySearchServer : public QWidget
{
    Q_OBJECT
public:
    explicit KeySearchServer(QWidget *parent = nullptr);
    ~KeySearchServer() override;

private slots:
    void onDiscoveryRead();
    void onExchangeRead();
    void onStartClicked();
    void onStopClicked();
    void onLoadKeysClicked();

private:
    struct ClientInfo {
        QHostAddress addr;
        quint16      port   = 0;
        int          taskId = -1;
        int          keyFrom = 0;
        int          keyTo   = 0;
    };

    void log(const QString &text);
    void loadConfig();
    bool loadKeys(const QString &path, QString *error);
    void assignTask(const QString &key);
    void sendLine(const QString &line,
                  const QHostAddress &addr, quint16 port);
    void broadcastStop(const QString &key);
    void updateStatus();
    void handleResult(const QString &rest,
                      const QHostAddress &sender, quint16 senderPort);

    static QString clientKey(const QHostAddress &addr, quint16 port);
    static QString b64(const QString &s);
    static QString unb64(const QString &s);

    QUdpSocket *m_discovery = nullptr;
    QUdpSocket *m_exchange  = nullptr;

    quint16 m_discoverPort;
    quint16 m_exchangePort;
    int     m_chunkSize;
    QString m_keysFile;
    QStringList m_keys;

    // UI
    QPlainTextEdit *m_alphabetEdit = nullptr;
    QPlainTextEdit *m_cipherEdit   = nullptr;
    QLineEdit      *m_keysPathEdit = nullptr;
    QPushButton    *m_loadKeysBtn  = nullptr;
    QPushButton    *m_startBtn     = nullptr;
    QPushButton    *m_stopBtn      = nullptr;
    QLabel         *m_portsLabel   = nullptr;
    QLabel         *m_statusLabel  = nullptr;
    QPlainTextEdit *m_logView      = nullptr;

    // Состояние поиска
    bool    m_searching;
    bool    m_found;
    int     m_nextKey;
    int     m_nextTaskId;
    int     m_checkedKeys;
    QString m_alphabet;
    QString m_cipher;
    QString m_foundKey;

    QHash<QString, ClientInfo> m_clients;
};

#endif // KEYSEARCHSERVER_H
