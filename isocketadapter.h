#ifndef ISOCKETADAPTER_H
#define ISOCKETADAPTER_H

#include <QObject>

class ISocketAdapter : public QObject
{
    Q_OBJECT
public:
    explicit ISocketAdapter(QObject *parent = nullptr) : QObject(parent) {}
    virtual ~ISocketAdapter() {}

    virtual void sendString(const QString &str) = 0;

signals:
    void message(QString msg);
    void disconnected();
};

#endif // ISOCKETADAPTER_H