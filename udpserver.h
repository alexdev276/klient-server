#ifndef UDPSERVER_H
#define UDPSERVER_H

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QUdpSocket>
#include <QTimer>

class UdpServer : public QWidget
{
    Q_OBJECT
public:
    UdpServer(QWidget *parent = nullptr);
    ~UdpServer();

private slots:
    void StartBtnClicked();
    void timeout();

private:
    QLabel *TimerLabel;
    QLineEdit *TextLineEdit;
    QPushButton *StartBtn;
    QVBoxLayout *mainLayout;
    int port;
    bool isStarted;
    QUdpSocket *udpSocket;
    QTimer *timer;
};

#endif // UDPSERVER_H