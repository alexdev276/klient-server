// vigenere.h
#ifndef VIGENERE_H
#define VIGENERE_H

#include <QString>

class Vigenere {
public:
    static QString defaultAlphabet();
    static QString encrypt(const QString &text, const QString &key);
    static QString decrypt(const QString &text, const QString &key);
    static bool containsTargetPhrase(const QString &text);
};

#endif // VIGENERE_H
