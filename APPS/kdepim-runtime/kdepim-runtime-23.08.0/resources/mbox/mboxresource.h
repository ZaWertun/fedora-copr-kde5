/*
    SPDX-FileCopyrightText: 2009 Bertjan Broeksem <broeksema@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "settings.h"
#include "singlefileresource.h"

namespace KMBox
{
class MBox;
}

class MboxResource : public Akonadi::SingleFileResource<Settings>
{
    Q_OBJECT

public:
    explicit MboxResource(const QString &id);
    ~MboxResource() override;

protected Q_SLOTS:
    bool retrieveItems(const Akonadi::Item::List &items, const QSet<QByteArray> &parts) override;
    void retrieveItems(const Akonadi::Collection &col) override;

protected:
    void aboutToQuit() override;

    void itemAdded(const Akonadi::Item &item, const Akonadi::Collection &collection) override;
    void itemChanged(const Akonadi::Item &item, const QSet<QByteArray> &parts) override;
    void itemRemoved(const Akonadi::Item &item) override;

    // From SingleFileResourceBase
    void handleHashChange() override;
    bool readFromFile(const QString &fileName) override;
    bool writeToFile(const QString &fileName) override;

    Q_REQUIRED_RESULT Akonadi::Collection rootCollection() const override;

private:
    void onCollectionFetch(KJob *job);
    void onCollectionModify(KJob *job);
    QHash<KJob *, Akonadi::Item> mCurrentItemDeletions;
    KMBox::MBox *mMBox = nullptr;
};
