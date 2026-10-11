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
    void onStartCalc1();   // Вариант 1: площадь sin(x)
    void onStartCalc4();   // Вариант 4: корни cos³(x)
    void on_message(QString text);
private:
    double calculateAreaSin(double a, double b);
    QList<double> findRootsCos3(double a, double b);

    ClientSocketAdapter *m_pSock;
    QTextEdit *m_log;
    QLineEdit *m_input;
};
#endif // CLIENT_H