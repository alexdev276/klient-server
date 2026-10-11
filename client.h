#ifndef CLIENT_H
#define CLIENT_H

#include <QWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "clientsocketadapter.h"

class Client : public QWidget {
    Q_OBJECT
public:
    explicit Client(QWidget *parent = nullptr);

private slots:
    void on_sendChat();
    void on_startCalc();
    void on_message(QString text);

private:
    double calculateArea(double a, double b);

    ClientSocketAdapter *m_pSock;
    QTextEdit *m_log;
    QLineEdit *m_input;
    QPushButton *m_sendBtn;
    QPushButton *m_calcBtn;
};

#endif // CLIENT_H