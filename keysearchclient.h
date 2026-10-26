#ifndef KEYSEARCHCLIENT_H
#define KEYSEARCHCLIENT_H

#include <QWidget>
#include <QUdpSocket>
#include <QHostAddress>
#include <QFutureWatcher>
#include <QAtomicInt>
#include <QStringList>

class QLineEdit;
class QPlainTextEdit;
class QPushButton;
class QLabel;
class QProgressBar;
class QTimer;

struct KeySearchResult {
    bool    found   = false;
    QString key;
    QString plain;
    int     checked = 0;
};

// ЛР 7. Клиент-дешифровщик: перебирает ключевые фразы Виженера
// в отдельном потоке и сообщает результат UDP-серверу.
class KeySearchClient : public QWidget
{
    Q_OBJECT
public:
    explicit KeySearchClient(QWidget *parent = nullptr);
    ~KeySearchClient() override;

private slots:
    void onReadyRead();
    void onFindClicked();
    void onStopClicked();
    void onSearchFinished();
    void onDiscoveryTick();

private:
    void log(const QString &text);
    void sendToServer(const QString &line);
    void startSearch(const QString &alphabet, const QString &cipher,
                     const QStringList &keys);
    void updateStatus();

    static QString b64(const QString &s);
    static QString unb64(const QString &s);

    QUdpSocket  *m_socket;
    QHostAddress m_serverAddr;
    quint16      m_serverPort;
    quint16      m_discoverPort;
    bool         m_serverKnown;
    bool         m_searching;
    bool         m_stoppedByServer;
    int          m_taskId;
    int          m_keyFrom;
    int          m_keyTo;
    QString      m_alphabet;
    QString      m_cipher;
    QStringList  m_keys;

    QAtomicInt   m_stopFlag;
    QFutureWatcher<KeySearchResult> *m_watcher;

    QLineEdit      *m_ipEdit      = nullptr;
    QPushButton    *m_findBtn     = nullptr;
    QPushButton    *m_stopBtn     = nullptr;
    QLabel         *m_statusLabel = nullptr;
    QProgressBar   *m_progress    = nullptr;
    QPlainTextEdit *m_logView     = nullptr;
    QTimer         *m_findTimer   = nullptr;
};

#endif // KEYSEARCHCLIENT_H
