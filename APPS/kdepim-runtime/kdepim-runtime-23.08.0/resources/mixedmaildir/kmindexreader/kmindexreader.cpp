/*
 *   SPDX-FileCopyrightText: 2010 Casey Link <unnamedrambler@gmail.com>
 *   SPDX-FileCopyrightText: 2009-2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

 *   This file includes code from old files from previous KDE versions:
 *   SPDX-FileCopyrightText: 2003 Andreas Gungl <a.gungl@gmx.de>
 *   SPDX-FileCopyrightText: 1996-1998 Stefan Taferner <taferner@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 *
 */

#include "kmindexreader.h"
#include "kmindexreader-config.h"

#include "../mixedmaildir_debug.h"
#include "../mixedmaildirresource_debug.h"

using Akonadi::MessageStatus;

#include <qplatformdefs.h>

// BEGIN: Magic definitions from old kmail code
#if HAVE_BYTESWAP_H
#include <byteswap.h>
#endif

static const int INDEX_VERSION = 1506;
const size_t readCount = 1;
#ifndef MAX_LINE
static const int MAX_LINE = 4096;
#endif

// We define functions as kmail_swap_NN so that we don't get compile errors
// on platforms where bswap_NN happens to be a function instead of a define.

/* Swap bytes in 16 bit value.  */
#ifdef bswap_16
#define kmail_swap_16(x) bswap_16(x)
#else
#define kmail_swap_16(x) ((((x) >> 8) & 0xff) | (((x)&0xff) << 8))
#endif

/* Swap bytes in 32 bit value.  */
#ifdef bswap_32
#define kmail_swap_32(x) bswap_32(x)
#else
#define kmail_swap_32(x) ((((x)&0xff000000) >> 24) | (((x)&0x00ff0000) >> 8) | (((x)&0x0000ff00) << 8) | (((x)&0x000000ff) << 24))
#endif

/* Swap bytes in 64 bit value.  */
#ifdef bswap_64
#define kmail_swap_64(x) bswap_64(x)
#else
#define kmail_swap_64(x)                                                                                                                                       \
    ((((x)&0xff00000000000000ull) >> 56) | (((x)&0x00ff000000000000ull) >> 40) | (((x)&0x0000ff0000000000ull) >> 24) | (((x)&0x000000ff00000000ull) >> 8)      \
     | (((x)&0x00000000ff000000ull) << 8) | (((x)&0x0000000000ff0000ull) << 24) | (((x)&0x000000000000ff00ull) << 40) | (((x)&0x00000000000000ffull) << 56))
#endif

/** The old status format, only one at a time possible. Needed
    for upgrade path purposes. */
using KMLegacyMsgStatus = enum {
    KMLegacyMsgStatusUnknown = ' ',
    KMLegacyMsgStatusNew = 'N',
    KMLegacyMsgStatusUnread = 'U',
    KMLegacyMsgStatusRead = 'R',
    KMLegacyMsgStatusOld = 'O',
    KMLegacyMsgStatusDeleted = 'D',
    KMLegacyMsgStatusReplied = 'A',
    KMLegacyMsgStatusForwarded = 'F',
    KMLegacyMsgStatusQueued = 'Q',
    KMLegacyMsgStatusSent = 'S',
    KMLegacyMsgStatusFlag = 'G'
};

// END: Magic definitions from old kmail code

// BEGIN: KMIndexMsg methods

KMIndexData::KMIndexData()
    : mPartsCacheBuilt(false)
{
    const uint count = sizeof(mCachedLongParts) / sizeof(unsigned long);
    for (uint i = 0; i < count; ++i) {
        mCachedLongParts[i] = 0;
    }
}

MessageStatus &KMIndexData::status()
{
    if (mStatus.isOfUnknownStatus()) {
        mStatus.fromQInt32(mCachedLongParts[KMIndexReader::MsgStatusPart]);
        if (mStatus.isOfUnknownStatus()) {
            // We are opening an old index for the first time, get the legacy
            // status and merge it in.
            // This is kept to provide an upgrade path from the old single
            // status to the new multiple status scheme.
            auto legacyMsgStatus = (KMLegacyMsgStatus)mCachedLongParts[KMIndexReader::MsgLegacyStatusPart];
            mStatus.setRead();
            switch (legacyMsgStatus) {
            case KMLegacyMsgStatusUnknown:
                mStatus.clear();
                break;
            case KMLegacyMsgStatusUnread:
                mStatus.setRead(false);
                break;
            case KMLegacyMsgStatusRead:
                mStatus.setRead();
                break;
            case KMLegacyMsgStatusDeleted:
                mStatus.setDeleted();
                break;
            case KMLegacyMsgStatusReplied:
                mStatus.setReplied();
                break;
            case KMLegacyMsgStatusForwarded:
                mStatus.setForwarded();
                break;
            case KMLegacyMsgStatusQueued:
                mStatus.setQueued();
                break;
            case KMLegacyMsgStatusSent:
                mStatus.setSent();
                break;
            case KMLegacyMsgStatusFlag:
                mStatus.setImportant();
                break;
            default:
                break;
            }
        }
    }
    return mStatus;
}

QStringList KMIndexData::tagList() const
{
    return mCachedStringParts[KMIndexReader::MsgTagPart].split(QLatin1Char(','), Qt::SkipEmptyParts);
}

quint64 KMIndexData::uid() const
{
    return mCachedLongParts[KMIndexReader::MsgUIDPart];
}

bool KMIndexData::isEmpty() const
{
    return !mPartsCacheBuilt;
}

// END: KMIndexMsg methods

KMIndexReader::KMIndexReader(const QString &indexFile)
    : mIndexFileName(indexFile)
    , mIndexFile(indexFile)
    , mConvertToUtf8(false)
    , mIndexSwapByteOrder(false)
    , mHeaderOffset(0)
    , mError(false)
{
    if (!mIndexFile.exists()) {
        qCDebug(MIXEDMAILDIR_LOG) << "file doesn't exist";
        mError = true;
    }

    if (!mIndexFile.open(QIODevice::ReadOnly)) {
        qCDebug(MIXEDMAILDIR_LOG) << "file can't be open";
        mError = true;
    }

    mFp = fdopen(mIndexFile.handle(), "r");
}

KMIndexReader::~KMIndexReader()
{
    if (mFp) {
        fclose(mFp);
    }
}

bool KMIndexReader::error() const
{
    return mError;
}

KMIndexDataPtr KMIndexReader::dataByOffset(quint64 offset) const
{
    QHash<quint64, KMIndexDataPtr>::const_iterator it = mMsgByOffset.constFind(offset);
    if (it == mMsgByOffset.constEnd()) {
        return {};
    }

    return it.value();
}

KMIndexDataPtr KMIndexReader::dataByFileName(const QString &fileName) const
{
    QHash<QString, KMIndexDataPtr>::const_iterator it = mMsgByFileName.constFind(fileName);
    if (it == mMsgByFileName.constEnd()) {
        return {};
    }

    return it.value();
}

bool KMIndexReader::readHeader(int *version)
{
    int indexVersion;
    Q_ASSERT(mFp != nullptr);
    mIndexSwapByteOrder = false;
    mIndexSizeOfLong = sizeof(long);

    int ret = fscanf(mFp, "# KMail-Index V%d\n", &indexVersion);
    if (ret == EOF || ret == 0) {
        return false; // index file has invalid header
    }
    if (version) {
        *version = indexVersion;
    }
    if (indexVersion < 1505) {
        if (indexVersion == 1503) {
            qCWarning(MIXEDMAILDIRRESOURCE_LOG) << "Need to convert old index file" << mIndexFileName << "to utf-8";
            mConvertToUtf8 = true;
        }
        return true;
    } else if (indexVersion == 1505) {
    } else if (indexVersion < INDEX_VERSION) {
        qCCritical(MIXEDMAILDIR_LOG) << "Index file" << mIndexFileName << "is out of date. What to do?";
        //       createIndexFromContents();
        return false;
    } else if (indexVersion > INDEX_VERSION) {
        qFatal("index file of newer version");
        return false;
    } else {
        // Header
        quint32 byteOrder = 0;
        quint32 sizeOfLong = sizeof(long); // default

        quint32 header_length = 0;
        QT_FSEEK(mFp, sizeof(char), SEEK_CUR);
        if (fread(&header_length, sizeof(header_length), readCount, mFp) != readCount) {
            qCWarning(MIXEDMAILDIRRESOURCE_LOG) << "Failed to read header_length";
            return false;
        }
        if (header_length > 0xFFFF) {
            header_length = kmail_swap_32(header_length);
        }

        off_t endOfHeader = QT_FTELL(mFp) + header_length;

        bool needs_update = true;
        // Process available header parts
        if (header_length >= sizeof(byteOrder)) {
            if (fread(&byteOrder, sizeof(byteOrder), readCount, mFp) != readCount) {
                qCWarning(MIXEDMAILDIRRESOURCE_LOG) << "Failed to read byteOrder";
                return false;
            }
            mIndexSwapByteOrder = (byteOrder == 0x78563412);
            header_length -= sizeof(byteOrder);

            if (header_length >= sizeof(sizeOfLong)) {
                if (fread(&sizeOfLong, sizeof(sizeOfLong), readCount, mFp) != readCount) {
                    qCWarning(MIXEDMAILDIRRESOURCE_LOG) << "Failed to read sizeOfLong";
                    return false;
                }
                if (mIndexSwapByteOrder) {
                    sizeOfLong = kmail_swap_32(sizeOfLong);
                }
                mIndexSizeOfLong = sizeOfLong;
                header_length -= sizeof(sizeOfLong);
                needs_update = false;
            }
        }
        if (needs_update || mIndexSwapByteOrder || (mIndexSizeOfLong != sizeof(long))) {
            qCDebug(MIXEDMAILDIR_LOG) << "DIRTY!";
            //         setDirty( true );
        }
        // Seek to end of header
        QT_FSEEK(mFp, endOfHeader, SEEK_SET);

        if (mIndexSwapByteOrder) {
            qCDebug(MIXEDMAILDIR_LOG) << "Index File has byte order swapped!";
        }
        if (mIndexSizeOfLong != sizeof(long)) {
            qCDebug(MIXEDMAILDIR_LOG) << "Index File sizeOfLong is" << mIndexSizeOfLong << "while sizeof(long) is" << sizeof(long) << "!";
        }
    }
    return true;
}

bool KMIndexReader::readIndex()
{
    qint32 len;
    KMIndexData *msg = nullptr;

    Q_ASSERT(mFp != nullptr);
    rewind(mFp);

    mMsgList.clear();
    mMsgByFileName.clear();
    mMsgByOffset.clear();

    int version;

    if (!readHeader(&version)) {
        return false;
    }

    mHeaderOffset = QT_FTELL(mFp);

    // loop through the entire index
    while (!feof(mFp)) {
        // qCDebug(MIXEDMAILDIR_LOG) << "NEW MSG!";
        msg = nullptr;
        // check version (parsed by readHeader)
        // because different versions must be
        // parsed differently
        // qCDebug(MIXEDMAILDIR_LOG) << "parsing version" << version;
        if (version >= 1505) {
            // parse versions >= 1505
            if (!fread(&len, sizeof(len), 1, mFp)) {
                break;
            }

            // swap bytes if needed
            if (mIndexSwapByteOrder) {
                len = kmail_swap_32(len);
            }

            off_t offs = QT_FTELL(mFp);
            if (QT_FSEEK(mFp, len, SEEK_CUR)) {
                break;
            }
            msg = new KMIndexData();
            fillPartsCache(msg, offs, len);
        } else {
            //////////////////////
            // BEGIN UNTESTED CODE
            //////////////////////
            // parse versions < 1505
            QByteArray line(MAX_LINE, '\0');
            if (fgets(line.data(), MAX_LINE, mFp) == nullptr) {
                break;
            }
            if (feof(mFp)) {
                break;
            }
            if (*line.data() == '\0') {
                // really, i have no idea when or how this would occur
                // but we probably want to know if it does - Casey
                qCWarning(MIXEDMAILDIRRESOURCE_LOG) << "Unknowable bad occurred";
                qCDebug(MIXEDMAILDIR_LOG) << "fclose(mFp = " << mFp << ")";
                fclose(mFp);
                mFp = nullptr;
                mMsgList.clear();
                mMsgByFileName.clear();
                mMsgByOffset.clear();
                return false;
            }
            off_t offs = QT_FTELL(mFp);
            if (QT_FSEEK(mFp, len, SEEK_CUR)) {
                break;
            }
            msg = new KMIndexData;
            fromOldIndexString(msg, line, mConvertToUtf8);

            fillPartsCache(msg, offs, len);
            //////////////////////
            // END UNTESTED CODE
            //////////////////////
        }
        if (!msg) {
            break;
        }

        if (msg->status().isDeleted()) {
            delete msg; // skip messages that are marked as deleted
            continue;
        }
#ifdef OBSOLETE
//     else if (mi->isNew())
//     {
//       mi->setStatus(KMMsgStatusUnread);
//       mi->setDirty(false);
//     }
#endif
        KMIndexDataPtr msgPtr(msg);
        mMsgList.append(msgPtr);
        const QString fileName = msg->mCachedStringParts[MsgFilePart];
        if (!fileName.isEmpty()) {
            mMsgByFileName.insert(fileName, msgPtr);
        }

        const quint64 offset = msg->mCachedLongParts[MsgOffsetPart];
        if (offset > 0) {
            mMsgByOffset.insert(offset, msgPtr);
        }
    } // end while

    return true;
}

//--- For compatibility with old index files
bool KMIndexReader::fromOldIndexString(KMIndexData *msg, const QByteArray &str, bool toUtf8)
{
    Q_UNUSED(toUtf8)
    //     const char *start, *offset;
    //   msg->modifiers = KMMsgInfoPrivate::ALL_SET;
    //   msg->xmark   = str.mid(33, 3).trimmed();
    //   msg->folderOffset = str.mid(2,9).toULong();
    //   msg->msgSize = str.mid(12,9).toULong();
    //   msg->date = (time_t)str.mid(22,10).toULong();
    //   mStatus.setStatusFromStr( str );
    //   if (toUtf8) {
    //       msg->subject = str.mid(37, 100).trimmed();
    //       msg->from = str.mid(138, 50).trimmed();
    //       msg->to = str.mid(189, 50).trimmed();
    //   } else {
    //       start = offset = str.data() + 37;
    //       while (*start == ' ' && start - offset < 100) start++;
    //       msg->subject = QString::fromUtf8(str.mid(start - str.data(),
    //           100 - (start - offset)), 100 - (start - offset));
    //       start = offset = str.data() + 138;
    //       while (*start == ' ' && start - offset < 50) start++;
    //       msg->from = QString::fromUtf8(str.mid(start - str.data(),
    //           50 - (start - offset)), 50 - (start - offset));
    //       start = offset = str.data() + 189;
    //       while (*start == ' ' && start - offset < 50) start++;
    //       msg->to = QString::fromUtf8(str.mid(start - str.data(),
    //           50 - (start - offset)), 50 - (start - offset));
    //   }
    //   msg->replyToIdMD5 = str.mid(240, 22).trimmed();
    //   msg->msgIdMD5 = str.mid(263, 22).trimmed();
    msg->mStatus.setStatusFromStr(QString::fromUtf8(str));
    return true;
}

//-----------------------------------------------------------------------------

static void swapEndian(QString &str)
{
    QChar *data = str.data();
    while (!data->isNull()) {
        *data = QChar(kmail_swap_16(data->unicode()));
        data++;
    }
}

static int g_chunk_length = 0, g_chunk_offset = 0;
static uchar *g_chunk = nullptr;

namespace
{
template<typename T>
void copy_from_stream(T &x)
{
    if (g_chunk_offset + int(sizeof(T)) > g_chunk_length) {
        g_chunk_offset = g_chunk_length;
        qCWarning(MIXEDMAILDIRRESOURCE_LOG) << "This should never happen..";
        x = 0;
    } else {
        // the memcpy is optimized out by the compiler for the values
        // of sizeof(T) that is called with
        memcpy(&x, g_chunk + g_chunk_offset, sizeof(T));
        g_chunk_offset += sizeof(T);
    }
}
}

bool KMIndexReader::fillPartsCache(KMIndexData *msg, off_t indexOff, short int indexLen)
{
    if (!msg) {
        return false;
    }
    // qCDebug(MIXEDMAILDIR_LOG);
    if (g_chunk_length < indexLen) {
        g_chunk = (uchar *)realloc(g_chunk, g_chunk_length = indexLen);
    }

    off_t first_off = QT_FTELL(mFp);
    QT_FSEEK(mFp, indexOff, SEEK_SET);
    if (fread(g_chunk, indexLen, readCount, mFp) != readCount) {
        qCWarning(MIXEDMAILDIRRESOURCE_LOG) << "Failed to read index";
        return false;
    }
    QT_FSEEK(mFp, first_off, SEEK_SET);

    MsgPartType type;
    quint16 len;
    off_t ret = 0;
    for (g_chunk_offset = 0; g_chunk_offset < indexLen; g_chunk_offset += len) {
        quint32 tmp;
        copy_from_stream(tmp);
        copy_from_stream(len);
        if (mIndexSwapByteOrder) {
            tmp = kmail_swap_32(tmp);
            len = kmail_swap_16(len);
        }
        type = (MsgPartType)tmp;
        if (g_chunk_offset + len > indexLen) {
            qCWarning(MIXEDMAILDIRRESOURCE_LOG) << "g_chunk_offset + len > indexLen"
                                                << "This should never happen..";
            return false;
        }
        // Only try to create strings if the part is really a string part, see declaration of
        // MsgPartType
        if (len && ((type >= 1 && type <= 6) || type == 11 || type == 14 || type == 15 || type == 19)) {
            // This works because the QString constructor does a memcpy.
            // Otherwise we would need to be concerned about the alignment.
            QString str((QChar *)(g_chunk + g_chunk_offset), len / 2);
            msg->mCachedStringParts[type] = str;

            // Normally we need to swap the byte order because the QStrings are written
            // in the style of Qt2 (MSB -> network ordered).
            // QStrings in Qt3 expect host ordering.
            // On e.g. Intel host ordering is LSB, on e.g. Sparc it is MSB.

#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
            // Byte order is little endian (swap is true)
            swapEndian(msg->mCachedStringParts[type]);
#else
            // Byte order is big endian (swap is false)
#endif
        } else if ((type >= 7 && type <= 10) || type == 12 || type == 13 || (type >= 16 && type <= 18)) {
            Q_ASSERT(mIndexSizeOfLong == len);
            if (mIndexSizeOfLong == sizeof(ret)) {
                // qCDebug(MIXEDMAILDIR_LOG) << "mIndexSizeOfLong == sizeof(ret)";
                // this memcpy replaces the original call to copy_from_stream
                // so that g_chunk_offset is not changed
                memcpy(&ret, g_chunk + g_chunk_offset, sizeof(ret));
                if (mIndexSwapByteOrder) {
                    if (sizeof(ret) == 4) {
                        ret = kmail_swap_32(ret);
                    } else {
                        ret = kmail_swap_64(ret);
                    }
                }
            }
            //////////////////////
            // BEGIN UNTESTED CODE
            //////////////////////
            else if (mIndexSizeOfLong == 4) {
                // Long is stored as 4 bytes in index file, sizeof(long) = 8
                quint32 ret_32;
                // this memcpy replaces the original call to copy_from_stream
                // so that g_chunk_offset is not changed
                memcpy(&ret_32, g_chunk + g_chunk_offset, sizeof(quint32));
                if (mIndexSwapByteOrder) {
                    ret_32 = kmail_swap_32(ret_32);
                }
                ret = ret_32;
            } else if (mIndexSizeOfLong == 8) {
                // Long is stored as 8 bytes in index file, sizeof(long) = 4
                quint32 ret_1;
                quint32 ret_2;
                // these memcpys replace the original calls to copy_from_stream
                // so that g_chunk_offset is not changed
                memcpy(&ret_1, g_chunk + g_chunk_offset, sizeof(quint32));
                memcpy(&ret_2, g_chunk + g_chunk_offset, sizeof(quint32));
                if (!mIndexSwapByteOrder) {
                    // Index file order is the same as the order of this CPU.
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
                    // Index file order is little endian
                    ret = ret_1; // We drop the 4 most significant bytes
#else
                    // Index file order is big endian
                    ret = ret_2; // We drop the 4 most significant bytes
#endif
                } else {
                    // Index file order is different from this CPU.
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
                    // Index file order is big endian
                    ret = ret_2; // We drop the 4 most significant bytes
#else
                    // Index file order is little endian
                    ret = ret_1; // We drop the 4 most significant bytes
#endif
                    // We swap the result to host order.
                    ret = kmail_swap_32(ret);
                }
            }
            //////////////////////
            // END UNTESTED CODE
            //////////////////////
            msg->mCachedLongParts[type] = ret;
        }
    } // for loop
    msg->mPartsCacheBuilt = true;
    return true;
}

//-----------------------------------------------------------------------------

QList<KMIndexDataPtr> KMIndexReader::messages()
{
    return mMsgList;
}
