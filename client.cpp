#include "client.h"
#include <QVBoxLayout>
#include <QHBoxLayout>

Client::Client(QWidget *parent) : QWidget(parent)
{
    setWindowTitle("UDP Chat Client");
    resize(400, 300);

    m_chatLog = new QTextEdit(this);
    m_chatLog->setReadOnly(true);
    m_input = new QLineEdit(this);
    m_sendBtn = new QPushButton("Отправить", this);

    QHBoxLayout *inputLayout = new QHBoxLayout();
    inputLayout->addWidget(m_input);
    inputLayout->addWidget(m_sendBtn);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(m_chatLog);
    mainLayout->addLayout(inputLayout);

    // Подключаемся к серверу на localhost (127.0.0.1) порт 5555
    m_socket = new ClientSocketAdapter(this, QHostAddress("127.0.0.1"), 5555);

    connect(m_socket, &ISocketAdapter::message, this, &Client::on_message);
    connect(m_sendBtn, &QPushButton::clicked, this, &Client::on_sendClicked);
    connect(m_input, &QLineEdit::returnPressed, this, &Client::on_sendClicked);
}

Client::~Client() {}

void Client::on_sendClicked()
{
    QString text = m_input->text().trimmed();
    if (!text.isEmpty()) {
        m_socket->sendString(text);
        m_chatLog->append("Вы: " + text);
        m_input->clear();
    }
}

void Client::on_message(const QString& text)
{
    m_chatLog->append("Сервер: " + text);
}