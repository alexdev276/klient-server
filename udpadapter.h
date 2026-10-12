#ifndef UDPADAPTER_H
#define UDPADAPTER_H

#include <QObject>
#include <QUdpSocket>

class UdpAdapter : public QObject
{
    Q_OBJECT
public:
    explicit UdpAdapter(QObject *parent = nullptr, quint16 port = 5556);
    void sendBroadcast(const QString &msg);

signals:
    void message(QString msg);

private slots:
    void on_readyRead();

private:
    QUdpSocket *m_socket;
};

#endif // UDPADAPTER_H