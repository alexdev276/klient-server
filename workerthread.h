// workerthread.h
#ifndef WORKERTHREAD_H
#define WORKERTHREAD_H

#include <QObject>
#include <QRunnable>
#include <QString>

class WorkerThread : public QObject, public QRunnable
{
    Q_OBJECT
public:
    explicit WorkerThread(const QString &cipherText, int startKey, int endKey, QObject *parent = nullptr);
    void run() override;

signals:
    void keyFound(const QString &key, const QString &decryptedText);
    void finishedWithoutResult();

private:
    QString m_cipherText;
    int m_startKey;
    int m_endKey;
};

#endif // WORKERTHREAD_H