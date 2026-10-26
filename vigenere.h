#ifndef VIGENERE_H
#define VIGENERE_H

#include <QString>
#include <QHash>

// Шифр Виженера над произвольным алфавитом.
// Используется в ЛР 7 для перебора ключевых фраз.
class Vigenere
{
public:
    explicit Vigenere(const QString &alphabet = QString());

    QString alphabet() const { return m_alphabet; }
    int     size()     const { return m_alphabet.size(); }
    bool    isValid()  const { return !m_alphabet.isEmpty(); }

    // Расшифровать текст ключевой фразой (символы вне алфавита
    // переносятся без изменений и не сдвигают ключ).
    QString decrypt(const QString &text, const QString &key) const;

    // Зашифровать текст ключевой фразой.
    QString encrypt(const QString &text, const QString &key) const;

    // Алфавит из методических указаний (ЛР 7).
    static QString defaultAlphabet();

    // Удаляет повторяющиеся символы, сохраняя порядок.
    static QString normalizeAlphabet(const QString &raw, QString *duplicates = nullptr);

private:
    QString          m_alphabet;
    QHash<QChar,int> m_index;
};

#endif // VIGENERE_H
