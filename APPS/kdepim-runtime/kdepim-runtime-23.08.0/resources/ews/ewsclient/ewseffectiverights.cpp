/*
    SPDX-FileCopyrightText: 2015-2017 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ewseffectiverights.h"

#include <QBitArray>
#include <QSharedData>
#include <QXmlStreamReader>

#include "ewsclient_debug.h"

class EwsEffectiveRightsPrivate : public QSharedData
{
public:
    enum Right {
        CreateAssociated = 0,
        CreateContents,
        CreateHierarchy,
        Delete,
        Modify,
        Read,
        ViewPrivateItems,
    };

    EwsEffectiveRightsPrivate();
    virtual ~EwsEffectiveRightsPrivate();

    bool readRight(QXmlStreamReader &reader, Right right);

    bool mValid;

    QBitArray mRights;
};

EwsEffectiveRightsPrivate::EwsEffectiveRightsPrivate()
    : mValid(false)
    , mRights(7)
{
}

EwsEffectiveRightsPrivate::~EwsEffectiveRightsPrivate()
{
}

bool EwsEffectiveRightsPrivate::readRight(QXmlStreamReader &reader, Right right)
{
    QString elm = reader.name().toString();
    if (reader.error() != QXmlStreamReader::NoError) {
        qCWarning(EWSCLI_LOG) << QStringLiteral("Failed to read %1 element - invalid %2 element.").arg(QStringLiteral("EffectiveRights"), elm);
        return false;
    }

    const QString text = reader.readElementText();
    if (text == QLatin1String("true")) {
        mRights.setBit(right);
    } else if (text == QLatin1String("false")) {
        mRights.clearBit(right);
    } else {
        qCWarning(EWSCLI_LOG) << QStringLiteral("Failed to read %1 element - invalid %2 element value: %3.").arg(QStringLiteral("EffectiveRights"), elm, text);
        return false;
    }

    return true;
}

EwsEffectiveRights::EwsEffectiveRights()
    : d(new EwsEffectiveRightsPrivate())
{
}

EwsEffectiveRights::EwsEffectiveRights(QXmlStreamReader &reader)
    : d(new EwsEffectiveRightsPrivate())
{
    while (reader.readNextStartElement()) {
        if (reader.namespaceUri() != ewsTypeNsUri) {
            qCWarningNC(EWSCLI_LOG) << QStringLiteral("Unexpected namespace in mailbox element:") << reader.namespaceUri();
            return;
        }
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        const QStringRef readerName = reader.name();
#else
        const QStringView readerName = reader.name();
#endif
        if (readerName == QLatin1String("CreateAssociated")) {
            if (!d->readRight(reader, EwsEffectiveRightsPrivate::CreateAssociated)) {
                return;
            }
        } else if (readerName == QLatin1String("CreateContents")) {
            if (!d->readRight(reader, EwsEffectiveRightsPrivate::CreateContents)) {
                return;
            }
        } else if (readerName == QLatin1String("CreateHierarchy")) {
            if (!d->readRight(reader, EwsEffectiveRightsPrivate::CreateHierarchy)) {
                return;
            }
        } else if (readerName == QLatin1String("Delete")) {
            if (!d->readRight(reader, EwsEffectiveRightsPrivate::Delete)) {
                return;
            }
        } else if (readerName == QLatin1String("Modify")) {
            if (!d->readRight(reader, EwsEffectiveRightsPrivate::Modify)) {
                return;
            }
        } else if (readerName == QLatin1String("Read")) {
            if (!d->readRight(reader, EwsEffectiveRightsPrivate::Read)) {
                return;
            }
        } else if (readerName == QLatin1String("ViewPrivateItems")) {
            if (!d->readRight(reader, EwsEffectiveRightsPrivate::ViewPrivateItems)) {
                return;
            }
        } else {
            qCWarning(EWSCLI_LOG)
                << QStringLiteral("Failed to read %1 element - unknown element: %2.").arg(QStringLiteral("EffectiveRights"), readerName.toString());
            return;
        }
    }

    d->mValid = true;
}

EwsEffectiveRights::EwsEffectiveRights(const EwsEffectiveRights &other)
    : d(other.d)
{
}

EwsEffectiveRights::EwsEffectiveRights(EwsEffectiveRights &&other)
    : d(std::move(other.d))
{
}

EwsEffectiveRights::~EwsEffectiveRights()
{
}

EwsEffectiveRights &EwsEffectiveRights::operator=(const EwsEffectiveRights &other)
{
    d = other.d;
    return *this;
}

EwsEffectiveRights &EwsEffectiveRights::operator=(EwsEffectiveRights &&other)
{
    d = std::move(other.d);
    return *this;
}

bool EwsEffectiveRights::isValid() const
{
    return d->mValid;
}

bool EwsEffectiveRights::canCreateAssociated() const
{
    return d->mRights.testBit(EwsEffectiveRightsPrivate::CreateAssociated);
}

bool EwsEffectiveRights::canCreateContents() const
{
    return d->mRights.testBit(EwsEffectiveRightsPrivate::CreateContents);
}

bool EwsEffectiveRights::canCreateHierarchy() const
{
    return d->mRights.testBit(EwsEffectiveRightsPrivate::CreateHierarchy);
}

bool EwsEffectiveRights::canDelete() const
{
    return d->mRights.testBit(EwsEffectiveRightsPrivate::Delete);
}

bool EwsEffectiveRights::canModify() const
{
    return d->mRights.testBit(EwsEffectiveRightsPrivate::Modify);
}

bool EwsEffectiveRights::canRead() const
{
    return d->mRights.testBit(EwsEffectiveRightsPrivate::Read);
}

bool EwsEffectiveRights::canViewPrivateItems() const
{
    return d->mRights.testBit(EwsEffectiveRightsPrivate::ViewPrivateItems);
}
