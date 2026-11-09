// vigenere.cpp
#include "vigenere.h"

#include <QSet>

QString Vigenere::defaultAlphabet()
{
    // Алфавит из методических указаний (ЛР 7).
    // В исходной строке буква 'g' и пробел встречаются дважды —
    // duplicates отбрасываются при построении рабочего алфавита.
    return QString::fromUtf8(
        "АБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ"
        "абвгдеёжзийклмнопрстуфхцчшщъыьэюя"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghigklmnopqrstuvwxyz"
        " 0123456789 -+=.!?:;,");
}

static const QString &alphabet()
{
    static const QString normalized = [] {
        QString out;
        QSet<QChar> seen;
        for (const QChar &ch : Vigenere::defaultAlphabet()) {
            if (seen.contains(ch))
                continue;
            seen.insert(ch);
            out.append(ch);
        }
        return out;
    }();
    return normalized;
}

QString Vigenere::encrypt(const QString &text, const QString &key)
{
    const QString &alpha = alphabet();
    if (alpha.isEmpty() || key.isEmpty())
        return text;

    const int n = alpha.size();
    QString result;
    result.reserve(text.size());

    int keyPos = 0;
    for (const QChar &ch : text) {
        const int textIdx = alpha.indexOf(ch);
        if (textIdx < 0) {
            result.append(ch);
            continue;
        }

        const int keyIdx = alpha.indexOf(key.at(keyPos % key.size()));
        ++keyPos;
        if (keyIdx < 0) {
            result.append(ch);
            continue;
        }

        result.append(alpha.at((textIdx + keyIdx) % n));
    }
    return result;
}

QString Vigenere::decrypt(const QString &text, const QString &key)
{
    const QString &alpha = alphabet();
    if (alpha.isEmpty() || key.isEmpty())
        return text;

    const int n = alpha.size();
    QString result;
    result.reserve(text.size());

    int keyPos = 0;
    for (const QChar &ch : text) {
        const int textIdx = alpha.indexOf(ch);
        if (textIdx < 0) {
            result.append(ch);
            continue;
        }

        const int keyIdx = alpha.indexOf(key.at(keyPos % key.size()));
        ++keyPos;
        if (keyIdx < 0) {
            result.append(ch);
            continue;
        }

        int resIdx = (textIdx - keyIdx) % n;
        if (resIdx < 0)
            resIdx += n;
        result.append(alpha.at(resIdx));
    }
    return result;
}

bool Vigenere::containsTargetPhrase(const QString &text)
{
    return text.contains("is120", Qt::CaseInsensitive);
}
