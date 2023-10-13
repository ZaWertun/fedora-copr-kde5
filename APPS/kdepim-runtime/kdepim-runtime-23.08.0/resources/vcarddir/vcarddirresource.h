/*
    SPDX-FileCopyrightText: 2008 Tobias Koenig <tokoe@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <Akonadi/ResourceBase>

#include <KContacts/Addressee>
#include <KContacts/VCardConverter>

class VCardDirResource : public Akonadi::ResourceBase, public Akonadi::AgentBase::Observer
{
    Q_OBJECT

public:
    explicit VCardDirResource(const QString &id);
    ~VCardDirResource() override;

public Q_SLOTS:
    void aboutToQuit() override;

protected:
    using ResourceBase::retrieveItems; // Suppress -Woverload-virtual

protected Q_SLOTS:
    void retrieveCollections() override;
    void retrieveItems(const Akonadi::Collection &col) override;
    bool retrieveItems(const Akonadi::Item::List &items, const QSet<QByteArray> &parts) override;
    bool retrieveItem(const Akonadi::Item &item, const QSet<QByteArray> &parts) override;

protected:
    void itemAdded(const Akonadi::Item &item, const Akonadi::Collection &collection) override;
    void itemChanged(const Akonadi::Item &item, const QSet<QByteArray> &parts) override;
    void itemRemoved(const Akonadi::Item &item) override;

private:
    void slotReloadConfig();
    void loadAddressees();
    Q_REQUIRED_RESULT QString vCardDirectoryName() const;
    Q_REQUIRED_RESULT QString vCardDirectoryFileName(const QString &file) const;
    void initializeVCardDirectory() const;
    Q_REQUIRED_RESULT bool doRetrieveItem(Akonadi::Item &item);

private:
    QMap<QString, KContacts::Addressee> mAddressees;
    KContacts::VCardConverter mConverter;
};
