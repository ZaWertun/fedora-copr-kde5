/*
    SPDX-FileCopyrightText: 2015-2017 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QList>
#include <QMetaType>
#include <QString>

#include "ewstypes.h"

class QXmlStreamWriter;
class QXmlStreamReader;
/**
 *  @brief  EWS Id wrapper class
 *
 *  This class wraps an EWS folder or item identifier.
 *
 *  In the EWS world an id can come in two forms:
 *   - An "actual" id identified by a unique, server-generated string (actually it's a
 *     base64-encoded internal server structure). Optionally this id is accompanied by a change
 *     key, which acts as a version number of the item. Each time something changes with the
 *     item (either the item itself or folder content - not sure if this applies to subfolders)
 *     the change key is updated. This gives you access to an older version of the item and
 *     allows to quickly find out if the item needs synchronizing.
 *   - A "distinguished" folder id which is a string identifying a list of known root folders
 *     such as 'inbox'. This is necessary for the initial query as there is no way to know the
 *     real folder ids beforehand. This applies only to folder identifiers.
 */
class EwsId
{
public:
    enum Type {
        Distinguished,
        Real,
        Unspecified,
    };

    typedef QVector<EwsId> List;

    explicit EwsId(EwsDistinguishedId did)
        : mType(Distinguished)
        , mDid(did)
    {
    }

    explicit EwsId(const QString &id, const QString &changeKey = QString());
    EwsId(const EwsId &id)
    {
        *this = id;
    }

    EwsId(EwsId &&id)
    {
        *this = std::move(id);
    }

    EwsId()
        : mType(Unspecified)
        , mDid(EwsDIdCalendar)
    {
    }

    explicit EwsId(QXmlStreamReader &reader);

    Type type() const
    {
        return mType;
    }

    QString id() const
    {
        return mId;
    }

    QString changeKey() const
    {
        return mChangeKey;
    }

    EwsDistinguishedId distinguishedId() const
    {
        return mDid;
    }

    EwsId &operator=(const EwsId &other);
    EwsId &operator=(EwsId &&other);
    bool operator==(const EwsId &other) const;
    bool operator<(const EwsId &other) const;

    void writeFolderIds(QXmlStreamWriter &writer) const;
    void writeItemIds(QXmlStreamWriter &writer) const;
    void writeAttributes(QXmlStreamWriter &writer) const;

    friend QDebug operator<<(QDebug debug, const EwsId &id);

private:
    Type mType;
    QString mId;
    QString mChangeKey;
    EwsDistinguishedId mDid;
};

uint qHash(const EwsId &id, uint seed);

QDebug operator<<(QDebug debug, const EwsId &id);

Q_DECLARE_METATYPE(EwsId)
Q_DECLARE_METATYPE(EwsId::List)
