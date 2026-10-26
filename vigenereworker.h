#ifndef VIGENERE_WORKER_H
#define VIGENERE_WORKER_H

#include <QObject>
#include <QRunnable>
#include <QString>

class VigenereWorker : public QObject, public QRunnable
{
    Q_OBJECT
public:
    VigenereWorker(const QString &cipherText, const QString &alphabet,
                   int startKey, int endKey, QObject *parent = nullptr);

    void run() override;

signals:
    void resultFound(const QString &key);

private:
    QString m_cipherText;
    QString m_alphabet;
    int m_startKey;
    int m_endKey;

    QString decrypt(const QString &key);
};

#endif // VIGENERE_WORKER_H