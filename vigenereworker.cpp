#include "vigenereworker.h"

VigenereWorker::VigenereWorker(const QString &cipherText, const QString &alphabet,
                               int startKey, int endKey, QObject *parent)
    : QObject(parent), QRunnable(), m_cipherText(cipherText), m_alphabet(alphabet),
    m_startKey(startKey), m_endKey(endKey)
{
    setAutoDelete(true); // Автоматическое удаление после выполнения
}

void VigenereWorker::run()
{
    // Перебор ключей в заданном диапазоне
    for (int key = m_startKey; key <= m_endKey; ++key) {
        // В качестве ключа используем число, преобразованное в строку
        QString keyStr = QString::number(key);
        QString decrypted = decrypt(keyStr);

        // Поиск искомой фразы из задания (в методичке это "is120")
        if (decrypted.contains("is120")) {
            emit resultFound(keyStr);
            return;
        }
    }
}

QString VigenereWorker::decrypt(const QString &key)
{
    QString result;
    int keyIndex = 0;
    int alphabetSize = m_alphabet.size();

    for (QChar ch : m_cipherText) {
        int charIndex = m_alphabet.indexOf(ch);
        if (charIndex == -1) {
            result.append(ch); // Символ не из алфавита, пропускаем
            continue;
        }
        int keyCharIndex = m_alphabet.indexOf(key.at(keyIndex % key.size()));
        int decryptedIndex = (charIndex - keyCharIndex + alphabetSize) % alphabetSize;
        result.append(m_alphabet.at(decryptedIndex));
        keyIndex++;
    }
    return result;
}