#ifndef ISOCKETADAPTER_H
#define ISOCKETADAPTER_H

#include <QObject>
#include <QString>

// Абстрактный интерфейс адаптера
class ISocketAdapter : public QObject {
    Q_OBJECT
public:
    explicit ISocketAdapter(QObject *parent) : QObject(parent) {}
    virtual ~ISocketAdapter() {}

    // Метод отправки строки
    virtual void sendString(const QString &str) = 0;

signals:
    void message(QString text);
    void disconnected();
};

#endif // ISOCKETADAPTER_H