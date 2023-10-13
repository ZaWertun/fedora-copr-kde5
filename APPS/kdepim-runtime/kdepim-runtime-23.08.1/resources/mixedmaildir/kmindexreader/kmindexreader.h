/*
 *   SPDX-FileCopyrightText: 2010 Casey Link <unnamedrambler@gmail.com>
 *   SPDX-FileCopyrightText: 2009-2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 *
 */

#pragma once

#include "kmindexreader_export.h"

#include <Akonadi/MessageStatus>
using Akonadi::MessageStatus;

#include <QFile>
#include <QList>
#include <QSharedPointer>
#include <QString>
#include <QStringList>

class KMINDEXREADER_EXPORT KMIndexData
{
    Q_DISABLE_COPY(KMIndexData)
public:
    KMIndexData();
    /** Status object of the message. */
    MessageStatus &status();
    QStringList tagList() const;
    quint64 uid() const;
    bool isEmpty() const;

private:
    QString mCachedStringParts[20];
    unsigned long mCachedLongParts[20];
    bool mPartsCacheBuilt;

    MessageStatus mStatus;
    friend class KMIndexReader;
    friend class TestIdxReader;
};

using KMIndexDataPtr = QSharedPointer<KMIndexData>;

/**
 * @short A class to read legacy kmail (< 4.5) index files
 *
 * This class provides read-only access to legacy kmail index files.
 * It uses old kmfolderindex code, authors attributed as appropriate.
 * @author Casey Link <unnamedrambler@gmail.com>
 */
class KMINDEXREADER_EXPORT KMIndexReader
{
public:
    explicit KMIndexReader(const QString &indexFile);
    ~KMIndexReader();

    bool error() const;

    /**
     * begins the index reading process
     */
    bool readIndex();

    enum MsgPartType {
        MsgNoPart = 0,
        // unicode strings
        MsgFromPart = 1,
        MsgSubjectPart = 2,
        MsgToPart = 3,
        MsgReplyToIdMD5Part = 4,
        MsgIdMD5Part = 5,
        MsgXMarkPart = 6,
        // unsigned long
        MsgOffsetPart = 7,
        MsgLegacyStatusPart = 8,
        MsgSizePart = 9,
        MsgDatePart = 10,
        // unicode string
        MsgFilePart = 11,
        // unsigned long
        MsgCryptoStatePart = 12,
        MsgMDNSentPart = 13,
        // another two unicode strings
        MsgReplyToAuxIdMD5Part = 14,
        MsgStrippedSubjectMD5Part = 15,
        // and another unsigned long
        MsgStatusPart = 16,
        MsgSizeServerPart = 17,
        MsgUIDPart = 18,
        // unicode string
        MsgTagPart = 19
    };

    KMIndexDataPtr dataByOffset(quint64 offset) const;

    KMIndexDataPtr dataByFileName(const QString &fileName) const;

private:
    /**
     * Reads the header of an index
     */
    bool readHeader(int *version);

    /**
     * creates a message object from an old index files
     */
    bool fromOldIndexString(KMIndexData *msg, const QByteArray &str, bool toUtf8);

    bool fillPartsCache(KMIndexData *msg, off_t off, short int len);

    QList<KMIndexDataPtr> messages();

    QString mIndexFileName;
    QFile mIndexFile;
    FILE *mFp = nullptr;

    bool mConvertToUtf8;
    bool mIndexSwapByteOrder; // Index file was written with swapped byte order
    int mIndexSizeOfLong; // Index file was written with longs of this size
    off_t mHeaderOffset;

    bool mError;

    /** list of index entries or messages */
    QList<KMIndexDataPtr> mMsgList;
    QHash<QString, KMIndexDataPtr> mMsgByFileName;
    QHash<quint64, KMIndexDataPtr> mMsgByOffset;
    friend class TestIdxReader;
};
