#ifndef CLIENT_H
#define CLIENT_H

#include <QWidget>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QTcpSocket>
#include <QByteArray>

class Client : public QWidget
{
    Q_OBJECT
public:
    explicit Client(QWidget *parent = nullptr);

private slots:
    void on_connected();
    void on_disconnected();
    void on_readyRead();
    void on_socketError(QAbstractSocket::SocketError err);
    void on_ipEntered();

private:
    void log(const QString &msg);
    void connectToServer(const QString &ip);
    void processLine(const QString &line);
    void sendResult(int type, int id, double a, double b);

    QLineEdit      *m_ipEdit;
    QPlainTextEdit *m_logView;
    QTcpSocket     *m_socket;
    QByteArray      m_buffer;
};

#endif // CLIENT_H