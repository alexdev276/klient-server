#ifndef SERVER_H
#define SERVER_H

#include <QWidget>
#include <QTcpServer>
#include <QList>
#include <QHash>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QPlainTextEdit>
#include <QLabel>
#include "isocketadapter.h"
#include "udpadapter.h"

class Server : public QWidget
{
    Q_OBJECT
public:
    explicit Server(int tcpPort, int udpPort, QWidget *parent = nullptr);
    ~Server();

private slots:
    void on_newConnection();
    void on_disconnected();
    void on_message(QString msg);
    void on_udpMessage(QString msg);
    void onStartClicked();

private:
    void sendTo(ISocketAdapter *sock, const QString &msg);
    void broadcast(const QString &msg);
    void distributeTasks(int type, double A, double B, double dx);
    int  clientId(ISocketAdapter *sock) const;
    void log(const QString &msg);
    void updateStatus();

    // UI
    QComboBox      *m_taskCombo;
    QDoubleSpinBox *m_aSpin;
    QDoubleSpinBox *m_bSpin;
    QDoubleSpinBox *m_dxSpin;
    QPushButton    *m_startBtn;
    QLabel         *m_statusLabel;
    QPlainTextEdit *m_logView;

    // Network
    QTcpServer  *m_ptcpServer;
    UdpAdapter  *m_udpAdapter;
    QList<ISocketAdapter*>      m_clients;
    QHash<ISocketAdapter*, int> m_clientIds;
    int m_nextClientId;
    int m_tcpPort;
    int m_udpPort;

    // Calculation state
    bool           m_isCalculating;
    qint64         m_totalTasks;
    qint64         m_completedTasks;
    double         m_finalArea;
    QList<double>  m_allRoots;
};

#endif // SERVER_H