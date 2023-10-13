/*
    SPDX-FileCopyrightText: 2015-2017 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QMetaType>
#include <QSharedDataPointer>

#include "ewstypes.h"

class EwsEffectiveRightsPrivate;
class QXmlStreamReader;

class EwsEffectiveRights
{
public:
    typedef QList<EwsEffectiveRights> List;

    EwsEffectiveRights();
    explicit EwsEffectiveRights(QXmlStreamReader &reader);
    EwsEffectiveRights(const EwsEffectiveRights &other);
    EwsEffectiveRights(EwsEffectiveRights &&other);
    virtual ~EwsEffectiveRights();

    EwsEffectiveRights &operator=(const EwsEffectiveRights &other);
    EwsEffectiveRights &operator=(EwsEffectiveRights &&other);

    bool isValid() const;
    bool canCreateAssociated() const;
    bool canCreateContents() const;
    bool canCreateHierarchy() const;
    bool canDelete() const;
    bool canModify() const;
    bool canRead() const;
    bool canViewPrivateItems() const;

protected:
    QSharedDataPointer<EwsEffectiveRightsPrivate> d;
};

Q_DECLARE_METATYPE(EwsEffectiveRights)
