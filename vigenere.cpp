#include "vigenere.h"
#include <QSet>

Vigenere::Vigenere(const QString &alphabet)
{
    m_alphabet = normalizeAlphabet(alphabet.isEmpty()
                                       ? defaultAlphabet()
                                       : alphabet);
    for (int i = 0; i < m_alphabet.size(); ++i)
        m_index.insert(m_alphabet.at(i), i);
}

QString Vigenere::defaultAlphabet()
{
    // Алфавит из методических указаний (ЛР 7).
    // В исходной строке буква 'g' и пробел встречаются дважды —
    // normalizeAlphabet() оставит только первые вхождения.
    return QString::fromUtf8(
        "АБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ"
        "абвгдеёжзийклмнопрстуфхцчшщъыьэюя"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghigklmnopqrstuvwxyz"
        " 0123456789 -+=.!?:;,");
}

QString Vigenere::normalizeAlphabet(const QString &raw, QString *duplicates)
{
    QString out;
    QString dup;
    QSet<QChar> seen;
    for (const QChar &ch : raw) {
        if (seen.contains(ch)) {
            if (!dup.contains(ch))
                dup.append(ch);
            continue;
        }
        seen.insert(ch);
        out.append(ch);
    }
    if (duplicates)
        *duplicates = dup;
    return out;
}

QString Vigenere::decrypt(const QString &text, const QString &key) const
{
    if (!isValid() || key.isEmpty())
        return text;

    const int n = m_alphabet.size();
    QString out;
    out.reserve(text.size());

    int j = 0;
    for (const QChar &ch : text) {
        auto it = m_index.constFind(ch);
        if (it == m_index.constEnd()) {
            out.append(ch);          // символ вне алфавита
            continue;
        }
        const QChar k = key.at(j % key.size());
        auto kit = m_index.constFind(k);
        if (kit == m_index.constEnd()) {
            out.append(ch);          // ключ содержит символ вне алфавита
            ++j;
            continue;
        }
        int idx = (it.value() - kit.value()) % n;
        if (idx < 0) idx += n;
        out.append(m_alphabet.at(idx));
        ++j;
    }
    return out;
}

QString Vigenere::encrypt(const QString &text, const QString &key) const
{
    if (!isValid() || key.isEmpty())
        return text;

    const int n = m_alphabet.size();
    QString out;
    out.reserve(text.size());

    int j = 0;
    for (const QChar &ch : text) {
        auto it = m_index.constFind(ch);
        if (it == m_index.constEnd()) {
            out.append(ch);
            continue;
        }
        const QChar k = key.at(j % key.size());
        auto kit = m_index.constFind(k);
        if (kit == m_index.constEnd()) {
            out.append(ch);
            ++j;
            continue;
        }
        out.append(m_alphabet.at((it.value() + kit.value()) % n));
        ++j;
    }
    return out;
}
