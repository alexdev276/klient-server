#ifndef SERVER_H
#define SERVER_H

#include <QWidget>
#include <QUdpSocket>
#include <QMap>
#include <QTextEdit>

class ServerSocketAdapter;

class Server : public QWidget
{
    Q_OBJECT
public:
    explicit Server(quint16 port, QWidget *parent = nullptr);
    ~Server();

private slots:
    void on_readyRead();
    void on_message(const QString& msg);
    void on_disconnected();

private:
    QUdpSocket *m_udpSocket;
    QTextEdit *m_log;
    QMap<QString, ServerSocketAdapter*> m_clients; // Ключ: "IP:Порт"
};

#endif // SERVER_H