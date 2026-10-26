#ifndef CLIENT_H
#define CLIENT_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include "clientsocketadapter.h"

class Client : public QWidget
{
    Q_OBJECT
public:
    explicit Client(QWidget *parent = nullptr);
    ~Client();

private slots:
    void on_sendClicked();
    void on_message(const QString& text);

private:
    QTextEdit *m_chatLog;
    QLineEdit *m_input;
    QPushButton *m_sendBtn;
    ClientSocketAdapter *m_socket;
};

#endif // CLIENT_H