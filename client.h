#ifndef CLIENT_H
#define CLIENT_H

#include <QWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "clientsocketadapter.h"
#include "udpadapter.h"

class Client : public QWidget {
    Q_OBJECT
public:
    explicit Client(QWidget *parent = nullptr);

private slots:
    void on_sendChat();
    void onStartCalc1();
    void onStartCalc4();
    void on_sendUdp();
    void on_tcpMessage(QString text);
    void on_udpMessage(QString text);

private:
    double calculateAreaSin(double a, double b);
    QList<double> findRootsCos3(double a, double b);

    ClientSocketAdapter *m_pTcpSock;
    UdpAdapter *m_pUdpSock;
    QTextEdit *m_log;
    QLineEdit *m_input;
    QLineEdit *m_udpInput;
};

#endif // CLIENT_H