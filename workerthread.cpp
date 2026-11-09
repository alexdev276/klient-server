// workerthread.cpp
#include "workerthread.h"
#include "vigenere.h"

WorkerThread::WorkerThread(const QString &cipherText, int startKey, int endKey, QObject *parent)
    : QObject(parent), m_cipherText(cipherText), m_startKey(startKey), m_endKey(endKey)
{
    setAutoDelete(true);
}

void WorkerThread::run()
{
    for (int keyNum = m_startKey; keyNum <= m_endKey; ++keyNum) {
        QString keyStr = QString::number(keyNum);
        QString decrypted = Vigenere::decrypt(m_cipherText, keyStr);

        if (Vigenere::containsTargetPhrase(decrypted)) {
            emit keyFound(keyStr, decrypted);
            return;
        }
    }
    emit finishedWithoutResult();
}