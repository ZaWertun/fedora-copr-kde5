/*
    SPDX-FileCopyrightText: 2009 Bertjan Broeksema <broeksema@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "compactpage.h"

#include <Akonadi/CollectionFetchJob>
#include <Akonadi/CollectionModifyJob>
#include <KLocalizedString>
#include <KMbox/MBox>
#include <QUrl>

#include "deleteditemsattribute.h"

#include <QFileInfo>

using namespace Akonadi;

CompactPage::CompactPage(const QString &collectionId, QWidget *parent)
    : QWidget(parent)
    , mCollectionId(collectionId)
{
    ui.setupUi(this);

    connect(ui.compactButton, &QPushButton::clicked, this, &CompactPage::compact);

    checkCollectionId();
}

void CompactPage::checkCollectionId()
{
    if (!mCollectionId.isEmpty()) {
        Collection collection;
        collection.setRemoteId(mCollectionId);
        auto fetchJob = new CollectionFetchJob(collection, CollectionFetchJob::Base);

        connect(fetchJob, &CollectionFetchJob::result, this, &CompactPage::onCollectionFetchCheck);
    }
}

void CompactPage::compact()
{
    ui.compactButton->setEnabled(false);

    Collection collection;
    collection.setRemoteId(mCollectionId);
    auto fetchJob = new CollectionFetchJob(collection, CollectionFetchJob::Base);

    connect(fetchJob, &CollectionFetchJob::result, this, &CompactPage::onCollectionFetchCompact);
}

void CompactPage::onCollectionFetchCheck(KJob *job)
{
    if (job->error()) {
        // If we cannot fetch the collection, than also disable compacting.
        ui.compactButton->setEnabled(false);
        return;
    }

    auto fetchJob = qobject_cast<CollectionFetchJob *>(job);
    Q_ASSERT(fetchJob);
    Q_ASSERT(fetchJob->collections().size() == 1);

    Collection mboxCollection = fetchJob->collections().at(0);
    auto attr = mboxCollection.attribute<DeletedItemsAttribute>(Akonadi::Collection::AddIfMissing);

    if (!attr->deletedItemOffsets().isEmpty()) {
        ui.compactButton->setEnabled(true);
        ui.messageLabel->setText(i18np("(1 message marked for deletion)", "(%1 messages marked for deletion)", attr->deletedItemOffsets().size()));
    }
}

void CompactPage::onCollectionFetchCompact(KJob *job)
{
    if (job->error()) {
        ui.messageLabel->setText(i18n("Failed to fetch the collection."));
        ui.compactButton->setEnabled(true);
        return;
    }

    auto fetchJob = qobject_cast<CollectionFetchJob *>(job);
    Q_ASSERT(fetchJob);
    Q_ASSERT(fetchJob->collections().size() == 1);

    Collection mboxCollection = fetchJob->collections().at(0);
    auto attr = mboxCollection.attribute<DeletedItemsAttribute>(Akonadi::Collection::AddIfMissing);

    KMBox::MBox mbox;
    // TODO: Set lock method.
    const QString fileName = QUrl::fromLocalFile(mCollectionId).toLocalFile();
    if (!mbox.load(fileName)) {
        ui.messageLabel->setText(i18n("Failed to load the mbox file"));
    } else {
        ui.messageLabel->setText(i18np("(Deleting 1 message)", "(Deleting %1 messages)", attr->offsetCount()));
        // TODO: implement and connect to messageProcessed signal.
        if (mbox.purge(attr->deletedItemEntries()) || (QFileInfo(fileName).size() == 0)) {
            // even if purge() failed but the file is now empty.
            // it was probably deleted/emptied by an external prog. For whatever reason
            // doesn't matter here. We know the file is empty so we can get rid
            // of our stored DeletedItemsAttribute
            mboxCollection.removeAttribute<DeletedItemsAttribute>();
            auto modifyJob = new CollectionModifyJob(mboxCollection);
            connect(modifyJob, &CollectionModifyJob::result, this, &CompactPage::onCollectionModify);
        } else {
            ui.messageLabel->setText(i18n("Failed to compact the mbox file."));
        }
    }
}

void CompactPage::onCollectionModify(KJob *job)
{
    if (job->error()) {
        ui.messageLabel->setText(i18n("Failed to compact the mbox file."));
    } else {
        ui.messageLabel->setText(i18n("MBox file compacted."));
    }
}
