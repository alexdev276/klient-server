// vigenere.cpp
#include "vigenere.h"

static const QString alphabet = "АБВГДЕЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ"
                                "абвгдежзийклмнопрстуфхцчшщъыьэюя"
                                "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                "abcdefghijklmnopqrstuvwxyz"
                                "0123456789-+-=.!?:;, ";

QString Vigenere::decrypt(const QString &text, const QString &key) {
    QString result = text;
    int keyLen = key.length();
    int alphLen = alphabet.length();

    for (int i = 0; i < text.length(); ++i) {
        int textIdx = alphabet.indexOf(text[i]);
        if (textIdx != -1) {
            int keyIdx = alphabet.indexOf(key[i % keyLen]);
            if (keyIdx != -1) {
                int resIdx = (textIdx - keyIdx + alphLen) % alphLen;
                result[i] = alphabet[resIdx];
            }
        }
    }
    return result;
}

bool Vigenere::containsTargetPhrase(const QString &text) {
    return text.contains("is120", Qt::CaseInsensitive);
}