#include "udpclient.h"
#include <QMessageBox>

UdpClient::UdpClient(QWidget *parent) : QDialog(parent)
{
    setWindowTitle(tr("UDP Client (Broadcast Receiver)"));
    resize(300, 200);

    ReceiveTextEdit = new QTextEdit(this);
    ReceiveTextEdit->setReadOnly(true);
    CloseBtn = new QPushButton(tr("Закрыть"), this);

    mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(ReceiveTextEdit);
    mainLayout->addWidget(CloseBtn);

    connect(CloseBtn, SIGNAL(clicked()), this, SLOT(CloseBtnClicked()));

    port = 5555;
    udpSocket = new QUdpSocket(this);
    // Привязка к порту для приема широковещательных сообщений
    bool result = udpSocket->bind(port, QUdpSocket::ShareAddress);
    if(!result) {
        QMessageBox::information(this, "Ошибка", "Не удалось привязать UDP сокет! Порт занят?");
        return;
    }
    connect(udpSocket, SIGNAL(readyRead()), this, SLOT(dataReceived()));
}

UdpClient::~UdpClient() {}

void UdpClient::CloseBtnClicked()
{
    close();
}

void UdpClient::dataReceived()
{
    while(udpSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(udpSocket->pendingDatagramSize());
        udpSocket->readDatagram(datagram.data(), datagram.size());
        QString msg = QString::fromUtf8(datagram);
        ReceiveTextEdit->append(msg);
    }
}