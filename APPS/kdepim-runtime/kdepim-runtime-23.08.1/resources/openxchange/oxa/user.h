/*
    This file is part of oxaccess.

    SPDX-FileCopyrightText: 2009 Tobias Koenig <tokoe@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QString>
#include <QVector>

namespace OXA
{
class User
{
public:
    using List = QVector<User>;

    User();

    Q_REQUIRED_RESULT bool isValid() const;

    void setUid(qlonglong uid);
    Q_REQUIRED_RESULT qlonglong uid() const;

    void setEmail(const QString &email);
    Q_REQUIRED_RESULT QString email() const;

    void setName(const QString &name);
    Q_REQUIRED_RESULT QString name() const;

private:
    qlonglong mUid = -1;
    QString mEmail;
    QString mName;
};
}

Q_DECLARE_TYPEINFO(OXA::User, Q_MOVABLE_TYPE);
