/*
    SPDX-FileCopyrightText: 2011 Andre Somers

    SPDX-License-Identifier: BSD-3-Clause
*/

#include "debug.h"
#include "o2/o0simplecrypt.h"
#include <QByteArray>
#include <QCryptographicHash>
#include <QDataStream>
#include <QIODevice>
#include <QRandomGenerator>

O0SimpleCrypt::O0SimpleCrypt()
    : m_key(0)
    , m_compressionMode(CompressionAuto)
    , m_protectionMode(ProtectionChecksum)
    , m_lastError(ErrorNoError)
{
}

O0SimpleCrypt::O0SimpleCrypt(quint64 key)
    : m_key(key)
    , m_compressionMode(CompressionAuto)
    , m_protectionMode(ProtectionChecksum)
    , m_lastError(ErrorNoError)
{
    splitKey();
}

void O0SimpleCrypt::setKey(quint64 key)
{
    m_key = key;
    splitKey();
}

void O0SimpleCrypt::splitKey()
{
    m_keyParts.clear();
    m_keyParts.resize(8);
    for (int i = 0; i < 8; i++) {
        quint64 part = m_key;
        for (int j = i; j > 0; j--) {
            part = part >> 8;
        }
        part = part & 0xff;
        m_keyParts[i] = static_cast<char>(part);
    }
}

QByteArray O0SimpleCrypt::encryptToByteArray(const QString &plaintext)
{
    QByteArray plaintextArray = plaintext.toUtf8();
    return encryptToByteArray(plaintextArray);
}

QByteArray O0SimpleCrypt::encryptToByteArray(const QByteArray &plaintext)
{
    if (m_keyParts.isEmpty()) {
        qCWarning(TOMBOYNOTESRESOURCE_LOG) << "No key set.";
        m_lastError = ErrorNoKeySet;
        return {};
    }

    QByteArray ba = plaintext;

    CryptoFlags flags = CryptoFlagNone;
    if (m_compressionMode == CompressionAlways) {
        ba = qCompress(ba, 9); // maximum compression
        flags |= CryptoFlagCompression;
    } else if (m_compressionMode == CompressionAuto) {
        QByteArray compressed = qCompress(ba, 9);
        if (compressed.size() < ba.size()) {
            ba = compressed;
            flags |= CryptoFlagCompression;
        }
    }

    QByteArray integrityProtection;
    if (m_protectionMode == ProtectionChecksum) {
        flags |= CryptoFlagChecksum;
        QDataStream s(&integrityProtection, QIODevice::WriteOnly);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        s << qChecksum(ba.constData(), ba.length());
#else
        s << qChecksum(QByteArrayView(ba.constData(), ba.length()));
#endif
    } else if (m_protectionMode == ProtectionHash) {
        flags |= CryptoFlagHash;
        QCryptographicHash hash(QCryptographicHash::Sha1);
        hash.addData(ba);

        integrityProtection += hash.result();
    }

    // prepend a random char to the string
    char randomChar = static_cast<char>(QRandomGenerator::global()->bounded(0xFF));
    ba = randomChar + integrityProtection + ba;

    int pos(0);
    char lastChar(0);

    int cnt = ba.size();

    while (pos < cnt) {
        ba[pos] = ba.at(pos) ^ m_keyParts.at(pos % 8) ^ lastChar;
        lastChar = ba.at(pos);
        ++pos;
    }

    QByteArray resultArray;
    resultArray.append(char(0x03)); // version for future updates to algorithm
    resultArray.append(char(flags)); // encryption flags
    resultArray.append(ba);

    m_lastError = ErrorNoError;
    return resultArray;
}

QString O0SimpleCrypt::encryptToString(const QString &plaintext)
{
    QByteArray plaintextArray = plaintext.toUtf8();
    QByteArray cypher = encryptToByteArray(plaintextArray);
    QString cypherString = QString::fromLatin1(cypher.toBase64());
    return cypherString;
}

QString O0SimpleCrypt::encryptToString(const QByteArray &plaintext)
{
    QByteArray cypher = encryptToByteArray(plaintext);
    QString cypherString = QString::fromLatin1(cypher.toBase64());
    return cypherString;
}

QString O0SimpleCrypt::decryptToString(const QString &cyphertext)
{
    const QByteArray cyphertextArray = QByteArray::fromBase64(cyphertext.toLatin1());
    const QByteArray plaintextArray = decryptToByteArray(cyphertextArray);
    const QString plaintext = QString::fromUtf8(plaintextArray.constData(), plaintextArray.size());

    return plaintext;
}

QString O0SimpleCrypt::decryptToString(const QByteArray &cypher)
{
    const QByteArray ba = decryptToByteArray(cypher);
    const QString plaintext = QString::fromUtf8(ba.constData(), ba.size());

    return plaintext;
}

QByteArray O0SimpleCrypt::decryptToByteArray(const QString &cyphertext)
{
    const QByteArray cyphertextArray = QByteArray::fromBase64(cyphertext.toLatin1());
    const QByteArray ba = decryptToByteArray(cyphertextArray);

    return ba;
}

QByteArray O0SimpleCrypt::decryptToByteArray(const QByteArray &cypher)
{
    if (m_keyParts.isEmpty()) {
        qCWarning(TOMBOYNOTESRESOURCE_LOG) << "No key set.";
        m_lastError = ErrorNoKeySet;
        return {};
    }

    if (cypher.isEmpty()) {
        m_lastError = ErrorUnknownVersion;
        return {};
    }

    QByteArray ba = cypher;

    char version = ba.at(0);

    if (version != 3) { // we only work with version 3
        m_lastError = ErrorUnknownVersion;
        qCWarning(TOMBOYNOTESRESOURCE_LOG) << "Invalid version or not a cyphertext.";
        return {};
    }

    auto flags = CryptoFlags(ba.at(1));

    ba.remove(0, 2);
    int pos(0);
    int cnt(ba.size());
    char lastChar = 0;

    while (pos < cnt) {
        char currentChar = ba[pos];
        ba[pos] = ba.at(pos) ^ lastChar ^ m_keyParts.at(pos % 8);
        lastChar = currentChar;
        ++pos;
    }

    ba.remove(0, 1); // chop off the random number at the start

    bool integrityOk(true);
    if (flags.testFlag(CryptoFlagChecksum)) {
        if (ba.length() < 2) {
            m_lastError = ErrorIntegrityFailed;
            return {};
        }
        quint16 storedChecksum;
        {
            QDataStream s(&ba, QIODevice::ReadOnly);
            s >> storedChecksum;
        }
        ba.remove(0, 2);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        quint16 checksum = qChecksum(ba.constData(), ba.size());
#else
        quint16 checksum = qChecksum(QByteArrayView(ba.constData(), ba.size()));
#endif
        integrityOk = (checksum == storedChecksum);
    } else if (flags.testFlag(CryptoFlagHash)) {
        if (ba.length() < 20) {
            m_lastError = ErrorIntegrityFailed;
            return {};
        }
        QByteArray storedHash = ba.left(20);
        ba.remove(0, 20);
        QCryptographicHash hash(QCryptographicHash::Sha1);
        hash.addData(ba);
        integrityOk = (hash.result() == storedHash);
    }

    if (!integrityOk) {
        m_lastError = ErrorIntegrityFailed;
        return {};
    }

    if (flags.testFlag(CryptoFlagCompression)) {
        ba = qUncompress(ba);
    }

    m_lastError = ErrorNoError;
    return ba;
}
