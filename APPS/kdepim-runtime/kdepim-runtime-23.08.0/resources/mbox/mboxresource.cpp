/*
    SPDX-FileCopyrightText: 2009 Bertjan Broeksem <broeksema@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "mboxresource.h"
#include "mboxresource_debug.h"

#include <Akonadi/AttributeFactory>
#include <Akonadi/ChangeRecorder>
#include <Akonadi/CollectionFetchJob>
#include <Akonadi/CollectionModifyJob>
#include <Akonadi/ItemFetchScope>
#include <Akonadi/MessageFlags>
#include <Akonadi/SpecialCollectionAttribute>

#include <KMbox/MBox>

#include <KMime/Message>

#include "deleteditemsattribute.h"
#include "settingsadaptor.h"

#include <QDBusConnection>

using namespace Akonadi;

static Collection::Id collectionId(const QString &remoteItemId)
{
    // [CollectionId]::[RemoteCollectionId]::[Offset]
    const QStringList lst = remoteItemId.split(QStringLiteral("::"));
    Q_ASSERT(lst.size() == 3);
    return lst.first().toLongLong();
}

static QString mboxFile(const QString &remoteItemId)
{
    // [CollectionId]::[RemoteCollectionId]::[Offset]
    const QStringList lst = remoteItemId.split(QStringLiteral("::"));
    Q_ASSERT(lst.size() == 3);
    return lst.at(1);
}

static quint64 itemOffset(const QString &remoteItemId)
{
    // [CollectionId]::[RemoteCollectionId]::[Offset]
    const QStringList lst = remoteItemId.split(QStringLiteral("::"));
    Q_ASSERT(lst.size() == 3);
    return lst.last().toULongLong();
}

MboxResource::MboxResource(const QString &id)
    : SingleFileResource<Settings>(id)
{
    new SettingsAdaptor(mSettings);
    QDBusConnection::sessionBus().registerObject(QStringLiteral("/Settings"), mSettings, QDBusConnection::ExportAdaptors);

    const QStringList mimeTypes{QStringLiteral("message/rfc822")};
    setSupportedMimetypes(mimeTypes, QStringLiteral("message-rfc822"));
    // Register the list of deleted items as an attribute of the collection.
    AttributeFactory::registerAttribute<DeletedItemsAttribute>();
    setName(mSettings->displayName());
}

MboxResource::~MboxResource()
{
    delete mMBox;
}

Collection MboxResource::rootCollection() const
{
    // Maildir only has a single collection so we treat it as an inbox
    auto col = SingleFileResource<Settings>::rootCollection();
    col.attribute<Akonadi::SpecialCollectionAttribute>(Akonadi::Collection::AddIfMissing)->setCollectionType("inbox");
    return col;
}

void MboxResource::retrieveItems(const Akonadi::Collection &col)
{
    Q_UNUSED(col)
    if (!mMBox) {
        cancelTask();
        return;
    }
    if (mMBox->fileName().isEmpty()) {
        Q_EMIT status(NotConfigured, i18nc("@info:status", "MBox not configured."));
        return;
    }

    reloadFile();

    KMBox::MBoxEntry::List entryList;
    if (const auto attr = col.attribute<DeletedItemsAttribute>()) {
        entryList = mMBox->entries(attr->deletedItemEntries());
    } else { // No deleted items (yet)
        entryList = mMBox->entries();
    }
    mMBox->lock(); // Lock the file so that it doesn't get locked for every
    // readEntryHeaders() call.

    Item::List items;
    const QString colId = QString::number(col.id());
    const QString colRid = col.remoteId();
    double count = 1;
    const int entryListSize(entryList.size());
    items.reserve(entryListSize);
    for (const KMBox::MBoxEntry &entry : std::as_const(entryList)) {
        // TODO: Use cache policy to see what actually has to been set as payload.
        //       Currently most views need a minimal amount of information so the
        //       Items get Envelopes as payload.
        auto mail = new KMime::Message();
        mail->setHead(KMime::CRLFtoLF(mMBox->readMessageHeaders(entry)));
        mail->parse();

        Item item;
        item.setRemoteId(colId + QLatin1String("::") + colRid + QLatin1String("::") + QString::number(entry.messageOffset()));
        item.setMimeType(QStringLiteral("message/rfc822"));
        item.setSize(entry.messageSize());
        item.setPayload(KMime::Message::Ptr(mail));
        Akonadi::MessageFlags::copyMessageFlags(*mail, item);
        Q_EMIT percent(count++ / entryListSize);
        items << item;
    }

    mMBox->unlock(); // Now we have the items, unlock

    itemsRetrieved(items);
}

bool MboxResource::retrieveItems(const Akonadi::Item::List &items, const QSet<QByteArray> &parts)
{
    Q_UNUSED(parts)

    if (!mMBox) {
        Q_EMIT error(i18n("MBox not loaded."));
        return false;
    }
    if (mMBox->fileName().isEmpty()) {
        Q_EMIT status(NotConfigured, i18nc("@info:status", "MBox not configured."));
        return false;
    }

    Akonadi::Item::List rv;
    rv.reserve(items.count());
    for (const auto &item : items) {
        const QString rid = item.remoteId();
        const quint64 offset = itemOffset(rid);
        KMime::Message *mail = mMBox->readMessage(KMBox::MBoxEntry(offset));
        if (!mail) {
            Q_EMIT error(i18n("Failed to read message with uid '%1'.", rid));
            return false;
        }

        Item i(item);
        i.setPayload(KMime::Message::Ptr(mail));
        Akonadi::MessageFlags::copyMessageFlags(*mail, i);
        rv.push_back(i);
    }
    itemsRetrieved(rv);
    return true;
}

void MboxResource::aboutToQuit()
{
    if (!mSettings->readOnly()) {
        writeFile();
    }
    mSettings->save();
}

void MboxResource::itemAdded(const Akonadi::Item &item, const Akonadi::Collection &collection)
{
    if (!mMBox) {
        cancelTask(i18n("MBox not loaded."));
        return;
    }
    if (mMBox->fileName().isEmpty()) {
        Q_EMIT status(NotConfigured, i18nc("@info:status", "MBox not configured."));
        return;
    }

    // we can only deal with mail
    if (!item.hasPayload<KMime::Message::Ptr>()) {
        cancelTask(i18n("Only email messages can be added to the MBox resource."));
        return;
    }

    const KMBox::MBoxEntry entry = mMBox->appendMessage(item.payload<KMime::Message::Ptr>());
    if (!entry.isValid()) {
        cancelTask(i18n("Mail message not added to the MBox."));
        return;
    }

    scheduleWrite();
    const QString rid =
        QString::number(collection.id()) + QLatin1String("::") + collection.remoteId() + QLatin1String("::") + QString::number(entry.messageOffset());

    Item i(item);
    i.setRemoteId(rid);

    changeCommitted(i);
}

void MboxResource::itemChanged(const Akonadi::Item &item, const QSet<QByteArray> &parts)
{
    if (parts.contains("PLD:RFC822")) {
        qCDebug(MBOXRESOURCE_LOG) << itemOffset(item.remoteId());
        // Only complete messages can be stored in a MBox file. Because all messages
        // are stored in one single file we do an ItemDelete and an ItemCreate to
        // prevent that whole file must been rewritten.
        auto fetchJob = new CollectionFetchJob(Collection(collectionId(item.remoteId())), CollectionFetchJob::Base);

        connect(fetchJob, &CollectionFetchJob::result, this, &MboxResource::onCollectionFetch);

        mCurrentItemDeletions.insert(fetchJob, item);

        fetchJob->start();
        return;
    }

    changeProcessed();
}

void MboxResource::itemRemoved(const Akonadi::Item &item)
{
    auto fetchJob = new CollectionFetchJob(Collection(collectionId(item.remoteId())), CollectionFetchJob::Base);

    if (!fetchJob->exec()) {
        cancelTask(i18n("Could not fetch the collection: %1", fetchJob->errorString()));
        return;
    }

    Q_ASSERT(fetchJob->collections().size() == 1);
    Collection mboxCollection = fetchJob->collections().at(0);
    auto attr = mboxCollection.attribute<DeletedItemsAttribute>(Akonadi::Collection::AddIfMissing);

    if (mSettings->compactFrequency() == Settings::per_x_messages && mSettings->messageCount() == static_cast<uint>(attr->offsetCount() + 1)) {
        qCDebug(MBOXRESOURCE_LOG) << "Compacting mbox file";
        mMBox->purge(attr->deletedItemEntries() << KMBox::MBoxEntry(itemOffset(item.remoteId())));
        scheduleWrite();
        mboxCollection.removeAttribute<DeletedItemsAttribute>();
    } else {
        attr->addDeletedItemOffset(itemOffset(item.remoteId()));
    }

    auto modifyJob = new CollectionModifyJob(mboxCollection);
    if (!modifyJob->exec()) {
        cancelTask(modifyJob->errorString());
        return;
    }

    changeProcessed();
}

void MboxResource::handleHashChange()
{
    Q_EMIT warning(
        i18n("The MBox file was changed by another program. "
             "A copy of the new file was made and pending changes "
             "are appended to that copy. To prevent this from happening "
             "use locking and make sure that all programs accessing the mbox "
             "use the same locking method."));
}

bool MboxResource::readFromFile(const QString &fileName)
{
    delete mMBox;
    mMBox = new KMBox::MBox();

    switch (mSettings->lockfileMethod()) {
    case Settings::procmail:
        mMBox->setLockType(KMBox::MBox::ProcmailLockfile);
        mMBox->setLockFile(mSettings->lockfile());
        break;
    case Settings::mutt_dotlock:
        mMBox->setLockType(KMBox::MBox::MuttDotlock);
        break;
    case Settings::mutt_dotlock_privileged:
        mMBox->setLockType(KMBox::MBox::MuttDotlockPrivileged);
        break;
    }

    return mMBox->load(QUrl::fromLocalFile(fileName).toLocalFile());
}

bool MboxResource::writeToFile(const QString &fileName)
{
    if (!mMBox->save(fileName)) {
        Q_EMIT error(i18n("Failed to save mbox file to %1", fileName));
        return false;
    }

    // HACK: When writeToFile is called with another file than with which the mbox
    // was loaded we assume that a backup is made as result of the fileChanged slot
    // in SingleFileResourceBase. The problem is that SingleFileResource assumes that
    // the implementing resources can save/retrieve the data from before the file
    // change we have a problem at this point in the mbox resource. Therefore we
    // copy the original file and append pending changes to it but also add an extra
    // '\n' to make sure that the hashes differ and the user gets notified. Normally
    // if this happens the user should make use of locking in all applications that
    // use the mbox file.
    if (fileName != mMBox->fileName()) {
        QFile file(fileName);
        file.open(QIODevice::WriteOnly);
        file.seek(file.size());
        file.write("\n");
    }

    return true;
}

/// Private slots

void MboxResource::onCollectionFetch(KJob *job)
{
    Q_ASSERT(mCurrentItemDeletions.contains(job));
    const Item item = mCurrentItemDeletions.take(job);

    if (job->error()) {
        cancelTask(job->errorString());
        return;
    }

    auto fetchJob = dynamic_cast<CollectionFetchJob *>(job);
    Q_ASSERT(fetchJob);
    Q_ASSERT(fetchJob->collections().size() == 1);

    Collection mboxCollection = fetchJob->collections().at(0);
    auto attr = mboxCollection.attribute<DeletedItemsAttribute>(Akonadi::Collection::AddIfMissing);
    attr->addDeletedItemOffset(itemOffset(item.remoteId()));

    auto modifyJob = new CollectionModifyJob(mboxCollection);
    mCurrentItemDeletions.insert(modifyJob, item);
    connect(modifyJob, &CollectionModifyJob::result, this, &MboxResource::onCollectionModify);
    modifyJob->start();
}

void MboxResource::onCollectionModify(KJob *job)
{
    Q_ASSERT(mCurrentItemDeletions.contains(job));
    const Item item = mCurrentItemDeletions.take(job);

    if (job->error()) {
        // Failed to store the offset of a deleted item in the DeletedItemsAttribute
        // of the collection. In this case we shouldn't try to store the modified
        // item.
        cancelTask(
            i18n("Failed to update the changed item because the old item "
                 "could not be deleted Reason: %1",
                 job->errorString()));
        return;
    }

    Collection c(collectionId(item.remoteId()));
    c.setRemoteId(mboxFile(item.remoteId()));

    itemAdded(item, c);
}

AKONADI_RESOURCE_MAIN(MboxResource)
