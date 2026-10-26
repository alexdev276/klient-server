#include "udpserver.h"
#include <QHostAddress>
#include <QDebug>

UdpServer::UdpServer(QWidget *parent) : QWidget(parent)
{
    setWindowTitle(tr("UDP Server (Broadcast)"));
    resize(300, 150);

    TimerLabel = new QLabel(tr("Введите сообщение:"), this);
    TextLineEdit = new QLineEdit(this);
    StartBtn = new QPushButton(tr("Начать рассылку"), this);

    mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(TimerLabel);
    mainLayout->addWidget(TextLineEdit);
    mainLayout->addWidget(StartBtn);

    connect(StartBtn, SIGNAL(clicked()), this, SLOT(StartBtnClicked()));

    port = 5555;
    isStarted = false;
    udpSocket = new QUdpSocket(this);
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(timeout()));
}

UdpServer::~UdpServer() {}

void UdpServer::StartBtnClicked()
{
    if(!isStarted) {
        StartBtn->setText("Остановить");
        timer->start(1000); // Отправка раз в секунду
        isStarted = true;
    } else {
        StartBtn->setText("Начать рассылку");
        isStarted = false;
        timer->stop();
    }
}

void UdpServer::timeout()
{
    QString msg = TextLineEdit->text();
    if (msg.isEmpty()) return;

    QByteArray datagram = msg.toUtf8();
    // Отправка на широковещательный адрес
    qint64 sent = udpSocket->writeDatagram(datagram, QHostAddress::Broadcast, port);
    if (sent != datagram.size()) {
        qDebug() << "Ошибка отправки дейтаграммы";
    }
}