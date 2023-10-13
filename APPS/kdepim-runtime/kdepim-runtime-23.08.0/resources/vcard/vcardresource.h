/*
    SPDX-FileCopyrightText: 2007 Tobias Koenig <tokoe@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "settings.h"
#include "singlefileresource.h"

#include <KContacts/Addressee>
#include <KContacts/VCardConverter>

class VCardResource : public Akonadi::SingleFileResource<Akonadi_VCard_Resource::Settings>
{
    Q_OBJECT

public:
    explicit VCardResource(const QString &id);
    ~VCardResource() override;

protected:
    using ResourceBase::retrieveItems; // Suppress -Woverload-virtual

protected Q_SLOTS:
    bool retrieveItem(const Akonadi::Item &item, const QSet<QByteArray> &parts) override;
    bool retrieveItems(const Akonadi::Item::List &items, const QSet<QByteArray> &parts) override;
    void retrieveItems(const Akonadi::Collection &col) override;

protected:
    bool readFromFile(const QString &fileName) override;
    bool writeToFile(const QString &fileName) override;
    void aboutToQuit() override;

    void itemAdded(const Akonadi::Item &item, const Akonadi::Collection &collection) override;
    void itemChanged(const Akonadi::Item &item, const QSet<QByteArray> &parts) override;
    void itemRemoved(const Akonadi::Item &item) override;

private:
    Q_REQUIRED_RESULT bool doRetrieveItem(Akonadi::Item &item);

private:
    QMap<QString, KContacts::Addressee> mAddressees;
    KContacts::VCardConverter mConverter;
};
