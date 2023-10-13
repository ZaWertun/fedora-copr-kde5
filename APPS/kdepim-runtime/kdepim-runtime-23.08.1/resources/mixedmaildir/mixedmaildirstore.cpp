/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
    SPDX-FileContributor: Kevin Krammer <krake@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "mixedmaildirstore.h"

#include "kmindexreader/kmindexreader.h"
#include "mixedmaildir_debug.h"

#include "filestore/collectioncreatejob.h"
#include "filestore/collectiondeletejob.h"
#include "filestore/collectionfetchjob.h"
#include "filestore/collectionmodifyjob.h"
#include "filestore/collectionmovejob.h"
#include "filestore/entitycompactchangeattribute.h"
#include "filestore/itemcreatejob.h"
#include "filestore/itemdeletejob.h"
#include "filestore/itemfetchjob.h"
#include "filestore/itemmodifyjob.h"
#include "filestore/itemmovejob.h"
#include "filestore/storecompactjob.h"

#include "libmaildir/maildir.h"

#include <KMbox/MBox>

#include <KMime/Message>

#include <Akonadi/CachePolicy>
#include <Akonadi/ItemFetchScope>
#include <Akonadi/MessageFlags>
#include <Akonadi/MessageParts>
#include <Akonadi/SpecialCollectionAttribute>

#include <KLocalizedString>

#include "mixedmaildirresource_debug.h"

#include <QDir>
#include <QFileInfo>
#include <QStringList>

using namespace Akonadi;
using KPIM::Maildir;
using namespace KMBox;

static bool fullEntryCompare(const KMBox::MBoxEntry &left, const KMBox::MBoxEntry &right)
{
    return left.messageOffset() == right.messageOffset() && left.separatorSize() == right.separatorSize() && left.messageSize() == right.messageSize();
}

static bool indexFileForFolder(const QFileInfo &folderDirInfo, QFileInfo &indexFileInfo)
{
    indexFileInfo = QFileInfo(folderDirInfo.dir(), QStringLiteral(".%1.index").arg(folderDirInfo.fileName()));

    if (!indexFileInfo.exists() || !indexFileInfo.isReadable()) {
        qCDebug(MIXEDMAILDIR_LOG) << "No index file" << indexFileInfo.absoluteFilePath() << "or not readable";
        return false;
    }

    return true;
}

class MBoxContext
{
public:
    MBoxContext()
    {
    }

    Q_REQUIRED_RESULT QString fileName() const
    {
        return mMBox.fileName();
    }

    bool load(const QString &fileName)
    {
        mModificationTime = QFileInfo(fileName).lastModified();

        // in case of reload, check if anything changed, otherwise keep deleted entries
        if (!mDeletedOffsets.isEmpty() && fileName == mMBox.fileName()) {
            const KMBox::MBoxEntry::List currentEntryList = mMBox.entries();
            if (mMBox.load(fileName)) {
                const KMBox::MBoxEntry::List newEntryList = mMBox.entries();
                if (!std::equal(currentEntryList.begin(), currentEntryList.end(), newEntryList.begin(), fullEntryCompare)) {
                    mDeletedOffsets.clear();
                }
                return true;
            }

            return false;
        }

        mDeletedOffsets.clear();
        return mMBox.load(fileName);
    }

    Q_REQUIRED_RESULT QDateTime modificationTime() const
    {
        return mModificationTime;
    }

    Q_REQUIRED_RESULT KMBox::MBoxEntry::List entryList() const
    {
        KMBox::MBoxEntry::List result;
        result.reserve(mMBox.entries().count());
        const auto entries{mMBox.entries()};
        for (const KMBox::MBoxEntry &entry : entries) {
            if (!mDeletedOffsets.contains(entry.messageOffset())) {
                result << entry;
            }
        }
        return result;
    }

    QByteArray readRawEntry(quint64 offset)
    {
        return mMBox.readRawMessage(KMBox::MBoxEntry(offset));
    }

    QByteArray readEntryHeaders(quint64 offset)
    {
        return mMBox.readMessageHeaders(KMBox::MBoxEntry(offset));
    }

    qint64 appendEntry(const KMime::Message::Ptr &entry)
    {
        const KMBox::MBoxEntry result = mMBox.appendMessage(entry);
        if (result.isValid() && mHasIndexData) {
            mIndexData.insert(result.messageOffset(), KMIndexDataPtr(new KMIndexData));
            Q_ASSERT(mIndexData.value(result.messageOffset())->isEmpty());
        }

        return result.messageOffset();
    }

    void deleteEntry(quint64 offset)
    {
        mDeletedOffsets << offset;
    }

    Q_REQUIRED_RESULT bool isValidOffset(quint64 offset) const
    {
        if (mDeletedOffsets.contains(offset)) {
            return false;
        }
        const KMBox::MBoxEntry::List lstEntry = mMBox.entries();
        for (const KMBox::MBoxEntry &entry : lstEntry) {
            if (entry.messageOffset() == offset) {
                return true;
            }
        }

        return false;
    }

    bool save()
    {
        bool ret = mMBox.save();
        mModificationTime = QDateTime::currentDateTime();
        return ret;
    }

    int purge(QVector<KMBox::MBoxEntry::Pair> &movedEntries)
    {
        const int deleteCount = mDeletedOffsets.count();

        KMBox::MBoxEntry::List deletedEntries;
        deletedEntries.reserve(deleteCount);
        for (quint64 offset : std::as_const(mDeletedOffsets)) {
            deletedEntries << KMBox::MBoxEntry(offset);
        }

        const bool result = mMBox.purge(deletedEntries, &movedEntries);

        if (mHasIndexData) {
            // keep copy of original for lookup
            const IndexDataHash indexData = mIndexData;

            // delete index data for removed entries
            for (quint64 offset : std::as_const(mDeletedOffsets)) {
                mIndexData.remove(offset);
            }

            // delete index data for changed entries
            // re-added below in an extra loop to handled cases where a new index is equal to an
            // old index of a different entry
            for (const KMBox::MBoxEntry::Pair &entry : std::as_const(movedEntries)) {
                mIndexData.remove(entry.first.messageOffset());
            }

            // re-add index data for changed entries at their new position
            for (const KMBox::MBoxEntry::Pair &entry : std::as_const(movedEntries)) {
                const KMIndexDataPtr data = indexData.value(entry.first.messageOffset());
                mIndexData.insert(entry.second.messageOffset(), data);
            }
        }

        mDeletedOffsets.clear();
        mModificationTime = QDateTime::currentDateTime();
        return result ? deleteCount : -1;
    }

    MBox &mbox()
    {
        return mMBox;
    }

    Q_REQUIRED_RESULT const MBox &mbox() const
    {
        return mMBox;
    }

    Q_REQUIRED_RESULT bool hasDeletedOffsets() const
    {
        return !mDeletedOffsets.isEmpty();
    }

    void readIndexData();

    Q_REQUIRED_RESULT KMIndexDataPtr indexData(quint64 offset) const
    {
        if (mHasIndexData) {
            if (!mDeletedOffsets.contains(offset)) {
                IndexDataHash::const_iterator it = mIndexData.constFind(offset);
                if (it != mIndexData.constEnd()) {
                    return it.value();
                }
            }
        }

        return {};
    }

    Q_REQUIRED_RESULT bool hasIndexData() const
    {
        return mHasIndexData;
    }

    void updatePath(const QString &newPath)
    {
        // TODO FIXME there has to be a better of doing that
        mMBox.load(newPath);
    }

public:
    Collection mCollection;
    qint64 mRevision = 0;

private:
    QSet<quint64> mDeletedOffsets;
    MBox mMBox;
    QDateTime mModificationTime;

    using IndexDataHash = QHash<quint64, KMIndexDataPtr>;
    IndexDataHash mIndexData;
    bool mIndexDataLoaded = false;
    bool mHasIndexData = false;
};

using MBoxPtr = QSharedPointer<MBoxContext>;

void MBoxContext::readIndexData()
{
    if (mIndexDataLoaded) {
        return;
    }

    mIndexDataLoaded = true;

    const QFileInfo mboxFileInfo(mMBox.fileName());
    QFileInfo indexFileInfo;
    if (!indexFileForFolder(mboxFileInfo, indexFileInfo)) {
        return;
    }

    if (mboxFileInfo.lastModified() > indexFileInfo.lastModified()) {
        qCDebug(MIXEDMAILDIR_LOG) << "MBox file " << mboxFileInfo.absoluteFilePath() << "newer than the index: mbox modified at" << mboxFileInfo.lastModified()
                                  << ", index modified at" << indexFileInfo.lastModified();
        return;
    }

    KMIndexReader indexReader(indexFileInfo.absoluteFilePath());
    if (indexReader.error() || !indexReader.readIndex()) {
        qCCritical(MIXEDMAILDIRRESOURCE_LOG) << "Index file" << indexFileInfo.path() << "could not be read";
        return;
    }

    mHasIndexData = true;

    const KMBox::MBoxEntry::List entries = mMBox.entries();
    for (const KMBox::MBoxEntry &entry : entries) {
        const quint64 indexOffset = entry.messageOffset() + entry.separatorSize();
        const KMIndexDataPtr data = indexReader.dataByOffset(indexOffset);
        if (data != nullptr) {
            mIndexData.insert(entry.messageOffset(), data);
        }
    }

    qCDebug(MIXEDMAILDIR_LOG) << "Read" << mIndexData.count() << "index entries from" << indexFileInfo.absoluteFilePath();
}

class MaildirContext
{
public:
    MaildirContext(const QString &path, bool isTopLevel)
        : mMaildir(path, isTopLevel)
        , mIndexDataLoaded(false)
        , mHasIndexData(false)
    {
    }

    MaildirContext(const Maildir &md)
        : mMaildir(md)
        , mIndexDataLoaded(false)
        , mHasIndexData(false)
    {
    }

    Q_REQUIRED_RESULT QStringList entryList() const
    {
        return mMaildir.entryList();
    }

    QString addEntry(const QByteArray &data)
    {
        const QString result = mMaildir.addEntry(data);
        if (!result.isEmpty() && mHasIndexData) {
            mIndexData.insert(result, KMIndexDataPtr(new KMIndexData));
            Q_ASSERT(mIndexData.value(result)->isEmpty());
        } else {
            // TODO: use the error string?
            qCWarning(MIXEDMAILDIRRESOURCE_LOG) << mMaildir.lastError();
        }

        return result;
    }

    void writeEntry(const QString &key, const QByteArray &data)
    {
        mMaildir.writeEntry(key, data); // TODO: error handling
        if (mHasIndexData) {
            mIndexData.insert(key, KMIndexDataPtr(new KMIndexData));
        }
    }

    bool removeEntry(const QString &key)
    {
        const bool result = mMaildir.removeEntry(key);
        if (result && mHasIndexData) {
            mIndexData.remove(key);
        }

        return result;
    }

    QString moveEntryTo(const QString &key, MaildirContext &destination)
    {
        const QString result = mMaildir.moveEntryTo(key, destination.mMaildir);
        if (!result.isEmpty()) {
            if (mHasIndexData) {
                mIndexData.remove(key);
            }

            if (destination.mHasIndexData) {
                destination.mIndexData.insert(result, KMIndexDataPtr(new KMIndexData));
            }
        } else {
            // TODO error handling?
            qCWarning(MIXEDMAILDIRRESOURCE_LOG) << mMaildir.lastError();
        }

        return result;
    }

    Q_REQUIRED_RESULT QByteArray readEntryHeaders(const QString &key) const
    {
        return mMaildir.readEntryHeaders(key);
    }

    Q_REQUIRED_RESULT QByteArray readEntry(const QString &key) const
    {
        return mMaildir.readEntry(key);
    }

    bool isValid(QString &error) const
    {
        bool result = mMaildir.isValid();
        if (!result) {
            error = mMaildir.lastError();
        }
        return result;
    }

    Q_REQUIRED_RESULT bool isValidEntry(const QString &entry) const
    {
        return !mMaildir.findRealKey(entry).isEmpty();
    }

    void readIndexData();

    Q_REQUIRED_RESULT KMIndexDataPtr indexData(const QString &fileName) const
    {
        if (mHasIndexData) {
            IndexDataHash::const_iterator it = mIndexData.constFind(fileName);
            if (it != mIndexData.constEnd()) {
                return it.value();
            }
        }

        return {};
    }

    Q_REQUIRED_RESULT bool hasIndexData() const
    {
        return mHasIndexData;
    }

    void updatePath(const QString &newPath)
    {
        mMaildir = Maildir(newPath, mMaildir.isRoot());
    }

    Q_REQUIRED_RESULT const Maildir &maildir() const
    {
        return mMaildir;
    }

private:
    Maildir mMaildir;

    using IndexDataHash = QHash<QString, KMIndexDataPtr>;
    IndexDataHash mIndexData;
    bool mIndexDataLoaded;
    bool mHasIndexData;
};

void MaildirContext::readIndexData()
{
    if (mIndexDataLoaded) {
        return;
    }

    mIndexDataLoaded = true;

    const QFileInfo maildirFileInfo(mMaildir.path());
    QFileInfo indexFileInfo;
    if (!indexFileForFolder(maildirFileInfo, indexFileInfo)) {
        return;
    }

    const QDir maildirBaseDir(maildirFileInfo.absoluteFilePath());
    const QFileInfo curDirFileInfo(maildirBaseDir, QStringLiteral("cur"));
    const QFileInfo newDirFileInfo(maildirBaseDir, QStringLiteral("new"));

    if (curDirFileInfo.lastModified() > indexFileInfo.lastModified()) {
        qCDebug(MIXEDMAILDIR_LOG) << "Maildir " << maildirFileInfo.absoluteFilePath() << "\"cur\" directory newer than the index: cur modified at"
                                  << curDirFileInfo.lastModified() << ", index modified at" << indexFileInfo.lastModified();
        return;
    }
    if (newDirFileInfo.lastModified() > indexFileInfo.lastModified()) {
        qCDebug(MIXEDMAILDIR_LOG) << "Maildir \"new\" directory newer than the index: cur modified at" << newDirFileInfo.lastModified() << ", index modified at"
                                  << indexFileInfo.lastModified();
        return;
    }

    KMIndexReader indexReader(indexFileInfo.absoluteFilePath());
    if (indexReader.error() || !indexReader.readIndex()) {
        qCCritical(MIXEDMAILDIRRESOURCE_LOG) << "Index file" << indexFileInfo.path() << "could not be read";
        return;
    }

    mHasIndexData = true;

    const QStringList entries = mMaildir.entryList();
    for (const QString &entry : entries) {
        const KMIndexDataPtr data = indexReader.dataByFileName(entry);
        if (data != nullptr) {
            mIndexData.insert(entry, data);
        }
    }

    qCDebug(MIXEDMAILDIR_LOG) << "Read" << mIndexData.count() << "index entries from" << indexFileInfo.absoluteFilePath();
}

using MaildirPtr = QSharedPointer<MaildirContext>;

class MixedMaildirStorePrivate : public FileStore::Job::Visitor
{
    MixedMaildirStore *const q;

public:
    enum FolderType {
        InvalidFolder,
        TopLevelFolder,
        MaildirFolder,
        MBoxFolder,
    };

    explicit MixedMaildirStorePrivate(MixedMaildirStore *parent)
        : q(parent)
    {
    }

    FolderType folderForCollection(const Collection &col, QString &path, QString &errorText) const;

    MBoxPtr getOrCreateMBoxPtr(const QString &path);
    MaildirPtr getOrCreateMaildirPtr(const QString &path, bool isTopLevel);

    void fillMBoxCollectionDetails(const MBoxPtr &mbox, Collection &collection);
    void fillMaildirCollectionDetails(const Maildir &md, Collection &collection);
    void fillMaildirTreeDetails(const Maildir &md, const Collection &collection, Collection::List &collections, bool recurse);
    void listCollection(FileStore::Job *job, MBoxPtr &mbox, const Collection &collection, Item::List &items);
    void listCollection(FileStore::Job *job, MaildirPtr &md, const Collection &collection, Item::List &items);
    bool fillItem(MBoxPtr &mbox, bool includeHeaders, bool includeBody, Item &item) const;
    bool fillItem(const MaildirPtr &md, bool includeHeaders, bool includeBody, Item &item) const;
    void updateContextHashes(const QString &oldPath, const QString &newPath);

public: // visitor interface implementation
    bool visit(FileStore::Job *job) override;
    bool visit(FileStore::CollectionCreateJob *job) override;
    bool visit(FileStore::CollectionDeleteJob *job) override;
    bool visit(FileStore::CollectionFetchJob *job) override;
    bool visit(FileStore::CollectionModifyJob *job) override;
    bool visit(FileStore::CollectionMoveJob *job) override;
    bool visit(FileStore::ItemCreateJob *job) override;
    bool visit(FileStore::ItemDeleteJob *job) override;
    bool visit(FileStore::ItemFetchJob *job) override;
    bool visit(FileStore::ItemModifyJob *job) override;
    bool visit(FileStore::ItemMoveJob *job) override;
    bool visit(FileStore::StoreCompactJob *job) override;

public:
    using MBoxHash = QHash<QString, MBoxPtr>;
    MBoxHash mMBoxes;

    using MaildirHash = QHash<QString, MaildirPtr>;
    MaildirHash mMaildirs;
};

MixedMaildirStorePrivate::FolderType MixedMaildirStorePrivate::folderForCollection(const Collection &col, QString &path, QString &errorText) const
{
    path.clear();
    errorText.clear();

    if (col.remoteId().isEmpty()) {
        errorText = i18nc("@info:status", "Given folder name is empty");
        qCWarning(MIXEDMAILDIRRESOURCE_LOG) << "Incomplete ancestor chain for collection.";
        Q_ASSERT(!col.remoteId().isEmpty()); // abort! Look at backtrace to see where we came from.
        return InvalidFolder;
    }

    if (col.parentCollection() == Collection::root()) {
        path = q->path();
        if (col.remoteId() != path) {
            qCWarning(MIXEDMAILDIRRESOURCE_LOG) << "RID mismatch, is" << col.remoteId() << "expected" << path;
        }
        return TopLevelFolder;
    }

    FolderType type = folderForCollection(col.parentCollection(), path, errorText);
    switch (type) {
    case InvalidFolder:
        return InvalidFolder;

    case TopLevelFolder: // fall through
    case MaildirFolder: {
        const Maildir parentMd(path, type == TopLevelFolder);
        const Maildir subFolder = parentMd.subFolder(col.remoteId());
        if (subFolder.isValid(false)) {
            path = subFolder.path();
            return MaildirFolder;
        }

        const QString subDirPath = (type == TopLevelFolder ? path : Maildir::subDirPathForFolderPath(path));
        QFileInfo fileInfo(QDir(subDirPath), col.remoteId());
        if (fileInfo.isFile()) {
            path = fileInfo.absoluteFilePath();
            return MBoxFolder;
        }

        errorText = i18nc("@info:status", "Folder %1 does not seem to be a valid email folder", fileInfo.absoluteFilePath());
        return InvalidFolder;
    }

    case MBoxFolder: {
        const QString subDirPath = Maildir::subDirPathForFolderPath(path);
        QFileInfo fileInfo(QDir(subDirPath), col.remoteId());

        if (fileInfo.isFile()) {
            path = fileInfo.absoluteFilePath();
            return MBoxFolder;
        }

        const Maildir subFolder(fileInfo.absoluteFilePath(), false);
        if (subFolder.isValid(false)) {
            path = subFolder.path();
            return MaildirFolder;
        }

        errorText = i18nc("@info:status", "Folder %1 does not seem to be a valid email folder", fileInfo.absoluteFilePath());
        return InvalidFolder;
    }
    }
    return InvalidFolder;
}

MBoxPtr MixedMaildirStorePrivate::getOrCreateMBoxPtr(const QString &path)
{
    MBoxPtr mbox;
    const MBoxHash::const_iterator it = mMBoxes.constFind(path);
    if (it == mMBoxes.constEnd()) {
        mbox = MBoxPtr(new MBoxContext);
        mMBoxes.insert(path, mbox);
    } else {
        mbox = it.value();
    }

    return mbox;
}

MaildirPtr MixedMaildirStorePrivate::getOrCreateMaildirPtr(const QString &path, bool isTopLevel)
{
    MaildirPtr md;
    const MaildirHash::const_iterator it = mMaildirs.constFind(path);
    if (it == mMaildirs.constEnd()) {
        md = MaildirPtr(new MaildirContext(path, isTopLevel));
        mMaildirs.insert(path, md);
    } else {
        md = it.value();
    }

    return md;
}

void MixedMaildirStorePrivate::fillMBoxCollectionDetails(const MBoxPtr &mbox, Collection &collection)
{
    collection.setContentMimeTypes(QStringList() << Collection::mimeType() << KMime::Message::mimeType());
    if (collection.name().compare(QLatin1String("inbox"), Qt::CaseInsensitive) == 0) {
        collection.attribute<Akonadi::SpecialCollectionAttribute>(Collection::AddIfMissing)->setCollectionType("inbox");
    }
    const QFileInfo fileInfo(mbox->fileName());
    if (fileInfo.isWritable()) {
        collection.setRights(Collection::CanCreateItem | Collection::CanChangeItem | Collection::CanDeleteItem | Collection::CanCreateCollection
                             | Collection::CanChangeCollection | Collection::CanDeleteCollection);
    } else {
        collection.setRights(Collection::ReadOnly);
    }

    if (mbox->mRevision > 0) {
        collection.setRemoteRevision(QString::number(mbox->mRevision));
    }
}

void MixedMaildirStorePrivate::fillMaildirCollectionDetails(const Maildir &md, Collection &collection)
{
    collection.setContentMimeTypes(QStringList() << Collection::mimeType() << KMime::Message::mimeType());
    if (collection.name().compare(QLatin1String("inbox"), Qt::CaseInsensitive) == 0) {
        collection.attribute<Akonadi::SpecialCollectionAttribute>(Collection::AddIfMissing)->setCollectionType("inbox");
    }

    const QFileInfo fileInfo(md.path());
    if (fileInfo.isWritable()) {
        collection.setRights(Collection::CanCreateItem | Collection::CanChangeItem | Collection::CanDeleteItem | Collection::CanCreateCollection
                             | Collection::CanChangeCollection | Collection::CanDeleteCollection);
    } else {
        collection.setRights(Collection::ReadOnly);
    }
}

void MixedMaildirStorePrivate::fillMaildirTreeDetails(const Maildir &md, const Collection &collection, Collection::List &collections, bool recurse)
{
    if (md.path().isEmpty()) {
        return;
    }

    const QStringList maildirSubFolders = md.subFolderList();
    for (const QString &subFolder : maildirSubFolders) {
        const Maildir subMd = md.subFolder(subFolder);

        if (!mMaildirs.contains(subMd.path())) {
            const MaildirPtr mdPtr = MaildirPtr(new MaildirContext(subMd));
            mMaildirs.insert(subMd.path(), mdPtr);
        }

        Collection col;
        col.setRemoteId(subFolder);
        col.setName(subFolder);
        col.setParentCollection(collection);
        fillMaildirCollectionDetails(subMd, col);
        collections << col;

        if (recurse) {
            fillMaildirTreeDetails(subMd, col, collections, true);
        }
    }

    const QDir dir(md.isRoot() ? md.path() : Maildir::subDirPathForFolderPath(md.path()));
    const QFileInfoList fileInfos = dir.entryInfoList(QDir::Files);
    for (const QFileInfo &fileInfo : fileInfos) {
        if (fileInfo.isHidden() || !fileInfo.isReadable()) {
            continue;
        }

        const QString mboxPath = fileInfo.absoluteFilePath();

        MBoxPtr mbox = getOrCreateMBoxPtr(mboxPath);
        if (mbox->load(mboxPath)) {
            const QString subFolder = fileInfo.fileName();
            Collection col;
            col.setRemoteId(subFolder);
            col.setName(subFolder);
            col.setParentCollection(collection);
            mbox->mCollection = col;

            fillMBoxCollectionDetails(mbox, col);
            collections << col;

            if (recurse) {
                const QString subDirPath = Maildir::subDirPathForFolderPath(fileInfo.absoluteFilePath());
                const Maildir subMd(subDirPath, true);
                fillMaildirTreeDetails(subMd, col, collections, true);
            }
        } else {
            mMBoxes.remove(fileInfo.absoluteFilePath());
        }
    }
}

void MixedMaildirStorePrivate::listCollection(FileStore::Job *job, MBoxPtr &mbox, const Collection &collection, Item::List &items)
{
    mbox->readIndexData();

    QHash<QString, QVariant> uidHash;
    QHash<QString, QVariant> tagListHash;

    const KMBox::MBoxEntry::List entryList = mbox->entryList();
    for (const KMBox::MBoxEntry &entry : entryList) {
        Item item;
        item.setMimeType(KMime::Message::mimeType());
        item.setRemoteId(QString::number(entry.messageOffset()));
        item.setParentCollection(collection);

        if (mbox->hasIndexData()) {
            const KMIndexDataPtr indexData = mbox->indexData(entry.messageOffset());
            if (indexData != nullptr && !indexData->isEmpty()) {
                item.setFlags(indexData->status().statusFlags());

                quint64 uid = indexData->uid();
                if (uid != 0) {
                    qCDebug(MIXEDMAILDIRRESOURCE_LOG) << "item" << item.remoteId() << "has UID" << uid;
                    uidHash.insert(item.remoteId(), QString::number(uid));
                }

                const QStringList tagList = indexData->tagList();
                if (!tagList.isEmpty()) {
                    qCDebug(MIXEDMAILDIRRESOURCE_LOG) << "item" << item.remoteId() << "has" << tagList.count() << "tags:" << tagList;
                    tagListHash.insert(item.remoteId(), tagList);
                }
            } else if (indexData == nullptr) {
                Akonadi::MessageStatus status;
                status.setDeleted(true);
                item.setFlags(status.statusFlags());
                qCDebug(MIXEDMAILDIRRESOURCE_LOG) << "no index for item" << item.remoteId() << "in MBox" << mbox->fileName()
                                                  << "so it has been deleted but not purged. Marking it as" << item.flags();
            }
        }

        items << item;
    }

    if (mbox->hasIndexData()) {
        QVariant var;

        if (!uidHash.isEmpty()) {
            var = QVariant::fromValue<QHash<QString, QVariant>>(uidHash);
            job->setProperty("remoteIdToIndexUid", var);
        }

        if (!tagListHash.isEmpty()) {
            var = QVariant::fromValue<QHash<QString, QVariant>>(tagListHash);
            job->setProperty("remoteIdToTagList", var);
        }
    }
}

void MixedMaildirStorePrivate::listCollection(FileStore::Job *job, MaildirPtr &md, const Collection &collection, Item::List &items)
{
    md->readIndexData();

    QHash<QString, QVariant> uidHash;
    QHash<QString, QVariant> tagListHash;

    const QStringList entryList = md->entryList();
    for (const QString &entry : entryList) {
        Item item;
        item.setMimeType(KMime::Message::mimeType());
        item.setRemoteId(entry);
        item.setParentCollection(collection);

        if (md->hasIndexData()) {
            const KMIndexDataPtr indexData = md->indexData(entry);
            if (indexData != nullptr && !indexData->isEmpty()) {
                item.setFlags(indexData->status().statusFlags());

                const quint64 uid = indexData->uid();
                if (uid != 0) {
                    qCDebug(MIXEDMAILDIRRESOURCE_LOG) << "item" << item.remoteId() << "has UID" << uid;
                    uidHash.insert(item.remoteId(), QString::number(uid));
                }

                const QStringList tagList = indexData->tagList();
                if (!tagList.isEmpty()) {
                    qCDebug(MIXEDMAILDIRRESOURCE_LOG) << "item" << item.remoteId() << "has" << tagList.count() << "tags:" << tagList;
                    tagListHash.insert(item.remoteId(), tagList);
                }
            }
        }
        const Akonadi::Item::Flags flags = md->maildir().readEntryFlags(entry);
        for (const Akonadi::Item::Flag &flag : flags) {
            item.setFlag(flag);
        }

        items << item;
    }

    if (md->hasIndexData()) {
        QVariant var;

        if (!uidHash.isEmpty()) {
            var = QVariant::fromValue<QHash<QString, QVariant>>(uidHash);
            job->setProperty("remoteIdToIndexUid", var);
        }

        if (!tagListHash.isEmpty()) {
            var = QVariant::fromValue<QHash<QString, QVariant>>(tagListHash);
            job->setProperty("remoteIdToTagList", var);
        }
    }
}

bool MixedMaildirStorePrivate::fillItem(MBoxPtr &mbox, bool includeHeaders, bool includeBody, Item &item) const
{
    //  qCDebug(MIXEDMAILDIR_LOG) << "Filling item" << item.remoteId() << "from MBox: includeBody=" << includeBody;
    bool ok = false;
    const quint64 offset = item.remoteId().toULongLong(&ok);
    if (!ok || !mbox->isValidOffset(offset)) {
        return false;
    }

    item.setModificationTime(mbox->modificationTime());

    // TODO: size?

    if (includeHeaders || includeBody) {
        KMime::Message::Ptr messagePtr(new KMime::Message());
        if (includeBody) {
            const QByteArray data = mbox->readRawEntry(offset);
            messagePtr->setContent(KMime::CRLFtoLF(data));
        } else {
            const QByteArray data = mbox->readEntryHeaders(offset);
            messagePtr->setHead(KMime::CRLFtoLF(data));
        }
        messagePtr->parse();

        item.setPayload<KMime::Message::Ptr>(messagePtr);
        Akonadi::MessageFlags::copyMessageFlags(*messagePtr, item);
    }
    return true;
}

bool MixedMaildirStorePrivate::fillItem(const MaildirPtr &md, bool includeHeaders, bool includeBody, Item &item) const
{
    /*  qCDebug(MIXEDMAILDIR_LOG) << "Filling item" << item.remoteId() << "from Maildir: includeBody=" << includeBody;*/

    const qint64 entrySize = md->maildir().size(item.remoteId());
    if (entrySize < 0) {
        return false;
    }

    item.setSize(entrySize);
    item.setModificationTime(md->maildir().lastModified(item.remoteId()));

    if (includeHeaders || includeBody) {
        KMime::Message::Ptr messagePtr(new KMime::Message());
        if (includeBody) {
            const QByteArray data = md->readEntry(item.remoteId());
            if (data.isEmpty()) {
                return false;
            }

            messagePtr->setContent(KMime::CRLFtoLF(data));
        } else {
            const QByteArray data = md->readEntryHeaders(item.remoteId());
            if (data.isEmpty()) {
                return false;
            }

            messagePtr->setHead(KMime::CRLFtoLF(data));
        }
        messagePtr->parse();

        item.setPayload<KMime::Message::Ptr>(messagePtr);
        Akonadi::MessageFlags::copyMessageFlags(*messagePtr, item);
    }
    return true;
}

void MixedMaildirStorePrivate::updateContextHashes(const QString &oldPath, const QString &newPath)
{
    // qCDebug(MIXEDMAILDIRRESOURCE_LOG) << "oldPath=" << oldPath << "newPath=" << newPath;
    const QString oldSubDirPath = Maildir::subDirPathForFolderPath(oldPath);
    const QString newSubDirPath = Maildir::subDirPathForFolderPath(newPath);

    MBoxHash mboxes;
    MBoxHash::const_iterator mboxIt = mMBoxes.constBegin();
    MBoxHash::const_iterator mboxEndIt = mMBoxes.constEnd();
    for (; mboxIt != mboxEndIt; ++mboxIt) {
        QString key = mboxIt.key();
        MBoxPtr mboxPtr = mboxIt.value();

        if (key == oldPath) {
            key = newPath;
        } else if (key.startsWith(oldSubDirPath)) {
            if (mboxPtr->hasIndexData() || mboxPtr->mRevision > 0) {
                key.replace(oldSubDirPath, newSubDirPath);
            } else {
                // if there is no index data yet, just discard this context
                key.clear();
            }
        }

        if (!key.isEmpty()) {
            mboxPtr->updatePath(key);
            mboxes.insert(key, mboxPtr);
        }
    }
    // qCDebug(MIXEDMAILDIRRESOURCE_LOG) << "mbox: old keys=" << mMBoxes.keys() << "new keys" << mboxes.keys();
    mMBoxes = mboxes;

    MaildirHash maildirs;
    MaildirHash::const_iterator mdIt = mMaildirs.constBegin();
    MaildirHash::const_iterator mdEndIt = mMaildirs.constEnd();
    for (; mdIt != mdEndIt; ++mdIt) {
        QString key = mdIt.key();
        MaildirPtr mdPtr = mdIt.value();

        if (key == oldPath) {
            key = newPath;
        } else if (key.startsWith(oldSubDirPath)) {
            if (mdPtr->hasIndexData()) {
                key.replace(oldSubDirPath, newSubDirPath);
            } else {
                // if there is no index data yet, just discard this context
                key.clear();
            }
        }

        if (!key.isEmpty()) {
            mdPtr->updatePath(key);
            maildirs.insert(key, mdPtr);
        }
    }
    // qCDebug(MIXEDMAILDIRRESOURCE_LOG) << "maildir: old keys=" << mMaildirs.keys() << "new keys" << maildirs.keys();
    mMaildirs = maildirs;
}

bool MixedMaildirStorePrivate::visit(FileStore::Job *job)
{
    const QString message = i18nc("@info:status", "Unhandled operation %1", QLatin1String(job->metaObject()->className()));
    qCCritical(MIXEDMAILDIRRESOURCE_LOG) << message;
    q->notifyError(FileStore::Job::InvalidJobContext, message);
    return false;
}

bool MixedMaildirStorePrivate::visit(FileStore::CollectionCreateJob *job)
{
    QString path;
    QString errorText;

    const FolderType folderType = folderForCollection(job->targetParent(), path, errorText);
    if (folderType == InvalidFolder) {
        errorText = i18nc("@info:status", "Cannot create folder %1 inside folder %2", job->collection().name(), job->targetParent().name());
        qCCritical(MIXEDMAILDIRRESOURCE_LOG) << errorText << "FolderType=" << folderType;
        q->notifyError(FileStore::Job::InvalidJobContext, errorText);
        return false;
    }

    const QString collectionName = job->collection().name().remove(QLatin1Char('/'));
    Maildir md;
    if (folderType == MBoxFolder) {
        const QString subDirPath = Maildir::subDirPathForFolderPath(path);
        const QDir dir(subDirPath);
        const QFileInfo dirInfo(dir, collectionName);
        if (dirInfo.exists() && !dirInfo.isDir()) {
            errorText = i18nc("@info:status", "Cannot create folder %1 inside folder %2", job->collection().name(), job->targetParent().name());
            qCCritical(MIXEDMAILDIRRESOURCE_LOG) << errorText << "FolderType=" << folderType << ", dirInfo exists and it not a dir";
            q->notifyError(FileStore::Job::InvalidJobContext, errorText);
            return false;
        }

        if (!dir.mkpath(collectionName)) {
            errorText = i18nc("@info:status", "Cannot create folder %1 inside folder %2", job->collection().name(), job->targetParent().name());
            qCCritical(MIXEDMAILDIRRESOURCE_LOG) << errorText << "FolderType=" << folderType << ", mkpath failed";
            q->notifyError(FileStore::Job::InvalidJobContext, errorText);
            return false;
        }

        md = Maildir(dirInfo.absoluteFilePath(), false);
        if (!md.create()) {
            errorText = i18nc("@info:status", "Cannot create folder %1 inside folder %2", job->collection().name(), job->targetParent().name());
            qCCritical(MIXEDMAILDIRRESOURCE_LOG) << errorText << "FolderType=" << folderType << ", maildir create failed";
            q->notifyError(FileStore::Job::InvalidJobContext, errorText);
            return false;
        }

        const MaildirPtr mdPtr(new MaildirContext(md));
        mMaildirs.insert(md.path(), mdPtr);
    } else {
        Maildir parentMd(path, folderType == TopLevelFolder);
        if (parentMd.addSubFolder(collectionName).isEmpty()) {
            errorText = i18nc("@info:status", "Cannot create folder %1 inside folder %2", job->collection().name(), job->targetParent().name());
            qCCritical(MIXEDMAILDIRRESOURCE_LOG) << errorText << "FolderType=" << folderType;
            q->notifyError(FileStore::Job::InvalidJobContext, errorText);
            return false;
        }

        md = Maildir(parentMd.subFolder(collectionName));
        const MaildirPtr mdPtr(new MaildirContext(md));
        mMaildirs.insert(md.path(), mdPtr);
    }

    Collection collection = job->collection();
    collection.setRemoteId(collectionName);
    collection.setName(collectionName);
    collection.setParentCollection(job->targetParent());
    fillMaildirCollectionDetails(md, collection);

    q->notifyCollectionsProcessed(Collection::List() << collection);
    return true;
}

bool MixedMaildirStorePrivate::visit(FileStore::CollectionDeleteJob *job)
{
    QString path;
    QString errorText;

    const FolderType folderType = folderForCollection(job->collection(), path, errorText);
    if (folderType == InvalidFolder) {
        errorText = i18nc("@info:status", "Cannot remove folder %1 from folder %2", job->collection().name(), job->collection().parentCollection().name());
        qCCritical(MIXEDMAILDIRRESOURCE_LOG) << errorText << "FolderType=" << folderType;
        q->notifyError(FileStore::Job::InvalidJobContext, errorText);
        return false;
    }

    QString parentPath;
    const FolderType parentFolderType = folderForCollection(job->collection().parentCollection(), parentPath, errorText);
    if (parentFolderType == InvalidFolder) {
        errorText = i18nc("@info:status", "Cannot remove folder %1 from folder %2", job->collection().name(), job->collection().parentCollection().name());
        qCCritical(MIXEDMAILDIRRESOURCE_LOG) << errorText << "Parent FolderType=" << parentFolderType;
        q->notifyError(FileStore::Job::InvalidJobContext, errorText);
        return false;
    }

    if (folderType == MBoxFolder) {
        if (!QFile::remove(path)) {
            errorText = i18nc("@info:status", "Cannot remove folder %1 from folder %2", job->collection().name(), job->collection().parentCollection().name());
            qCCritical(MIXEDMAILDIRRESOURCE_LOG) << errorText << "FolderType=" << folderType;
            q->notifyError(FileStore::Job::InvalidJobContext, errorText);
            return false;
        }
    } else {
        if (!QDir(path).removeRecursively()) {
            errorText = i18nc("@info:status", "Cannot remove folder %1 from folder %2", job->collection().name(), job->collection().parentCollection().name());
            qCCritical(MIXEDMAILDIRRESOURCE_LOG) << errorText << "FolderType=" << folderType;
            q->notifyError(FileStore::Job::InvalidJobContext, errorText);
            return false;
        }
    }

    const QString subDirPath = Maildir::subDirPathForFolderPath(path);
    QDir(subDirPath).removeRecursively();

    q->notifyCollectionsProcessed(Collection::List() << job->collection());
    return true;
}

bool MixedMaildirStorePrivate::visit(FileStore::CollectionFetchJob *job)
{
    QString path;
    QString errorText;
    const FolderType folderType = folderForCollection(job->collection(), path, errorText);

    if (folderType == InvalidFolder) {
        qCCritical(MIXEDMAILDIRRESOURCE_LOG) << errorText << "collection:" << job->collection();
        q->notifyError(FileStore::Job::InvalidJobContext, errorText);
        return false;
    }

    Collection::List collections;
    Collection collection = job->collection();
    if (job->type() == FileStore::CollectionFetchJob::Base) {
        collection.setName(collection.remoteId());
        if (folderType == MBoxFolder) {
            MBoxPtr mbox;
            MBoxHash::const_iterator findIt = mMBoxes.constFind(path);
            if (findIt == mMBoxes.constEnd()) {
                mbox = MBoxPtr(new MBoxContext);
                if (!mbox->load(path)) {
                    errorText = i18nc("@info:status", "Failed to load MBox folder %1", path);
                    qCCritical(MIXEDMAILDIRRESOURCE_LOG) << errorText << "collection=" << collection;
                    q->notifyError(FileStore::Job::InvalidJobContext, errorText); // TODO should be a different error code
                    return false;
                }

                mbox->mCollection = collection;
                mMBoxes.insert(path, mbox);
            } else {
                mbox = findIt.value();
            }

            fillMBoxCollectionDetails(mbox, collection);
        } else {
            const Maildir md(path, folderType == TopLevelFolder);
            fillMaildirCollectionDetails(md, collection);
        }
        collections << collection;
    } else {
        // if the base is an mbox, use its sub folder dir like a top level maildir
        if (folderType == MBoxFolder) {
            path = Maildir::subDirPathForFolderPath(path);
        }
        const Maildir md(path, folderType != MaildirFolder);
        fillMaildirTreeDetails(md, collection, collections, job->type() == FileStore::CollectionFetchJob::Recursive);
    }

    if (!collections.isEmpty()) {
        q->notifyCollectionsProcessed(collections);
    }
    return true;
}

static Collection updateMBoxCollectionTree(const Collection &collection, const Collection &oldParent, const Collection &newParent)
{
    if (collection == oldParent) {
        return newParent;
    }

    if (collection == Collection::root()) {
        return collection;
    }

    Collection updatedCollection = collection;
    updatedCollection.setParentCollection(updateMBoxCollectionTree(collection.parentCollection(), oldParent, newParent));

    return updatedCollection;
}

bool MixedMaildirStorePrivate::visit(FileStore::CollectionModifyJob *job)
{
    const Collection collection = job->collection();
    const QString collectionName = collection.name().remove(QLatin1Char('/'));

    // we also only do renames
    if (collection.remoteId() == collection.name()) {
        qCWarning(MIXEDMAILDIRRESOURCE_LOG) << "CollectionModifyJob with name still identical to remoteId. Ignoring";
        return true;
    }

    QString path;
    QString errorText;
    const FolderType folderType = folderForCollection(collection, path, errorText);
    if (folderType == InvalidFolder) {
        errorText = i18nc("@info:status", "Cannot rename folder %1", collection.name());
        qCCritical(MIXEDMAILDIRRESOURCE_LOG) << errorText << "FolderType=" << folderType;
        q->notifyError(FileStore::Job::InvalidJobContext, errorText);
        return false;
    }

    const QFileInfo fileInfo(path);
    const QFileInfo subDirInfo = QFileInfo(Maildir::subDirPathForFolderPath(path));

    QDir parentDir(path);
    parentDir.cdUp();

    const QFileInfo targetFileInfo(parentDir, collectionName);
    const QFileInfo targetSubDirInfo = QFileInfo(Maildir::subDirPathForFolderPath(targetFileInfo.absoluteFilePath()));

    if (targetFileInfo.exists() || (subDirInfo.exists() && targetSubDirInfo.exists())) {
        errorText = i18nc("@info:status", "Cannot rename folder %1", collection.name());
        qCCritical(MIXEDMAILDIRRESOURCE_LOG) << errorText << "FolderType=" << folderType;
        q->notifyError(FileStore::Job::InvalidJobContext, errorText);
        return false;
    }

    // if there is an index, make sure it is read before renaming
    // do not rename index as it could already be out of date
    bool indexInvalidated = false;
    if (folderType == MBoxFolder) {
        // TODO would be nice if getOrCreateMBoxPtr() could be used instead, like below for Maildir
        MBoxPtr mbox;
        MBoxHash::const_iterator findIt = mMBoxes.constFind(path);
        if (findIt == mMBoxes.constEnd()) {
            mbox = MBoxPtr(new MBoxContext);
            if (!mbox->load(path)) {
                qCWarning(MIXEDMAILDIRRESOURCE_LOG) << "Failed to load mbox" << path;
            }

            mbox->mCollection = collection;
            mMBoxes.insert(path, mbox);
        } else {
            mbox = findIt.value();
        }

        mbox->readIndexData();
        indexInvalidated = mbox->hasIndexData();
    } else if (folderType == MaildirFolder) {
        MaildirPtr md = getOrCreateMaildirPtr(path, false);

        md->readIndexData();
        indexInvalidated = md->hasIndexData();
    }

    if (!parentDir.rename(fileInfo.absoluteFilePath(), targetFileInfo.fileName())) {
        errorText = i18nc("@info:status", "Cannot rename folder %1", collection.name());
        qCCritical(MIXEDMAILDIRRESOURCE_LOG) << errorText << "FolderType=" << folderType;
        q->notifyError(FileStore::Job::InvalidJobContext, errorText);
        return false;
    }

    if (subDirInfo.exists()) {
        if (!parentDir.rename(subDirInfo.absoluteFilePath(), targetSubDirInfo.fileName())) {
            errorText = i18nc("@info:status", "Cannot rename folder %1", collection.name());
            qCCritical(MIXEDMAILDIRRESOURCE_LOG) << errorText << "FolderType=" << folderType;
            q->notifyError(FileStore::Job::InvalidJobContext, errorText);

            // try to recover the previous rename
            parentDir.rename(targetFileInfo.absoluteFilePath(), fileInfo.fileName());
            return false;
        }
    }

    // update context hashes
    updateContextHashes(fileInfo.absoluteFilePath(), targetFileInfo.absoluteFilePath());

    Collection renamedCollection = collection;

    // when renaming top level folder, change path of store
    if (folderType == TopLevelFolder) {
        // backup caches, setTopLevelCollection() clears them
        const MBoxHash mboxes = mMBoxes;
        const MaildirHash maildirs = mMaildirs;

        q->setPath(targetFileInfo.absoluteFilePath());

        // restore caches
        mMBoxes = mboxes;
        mMaildirs = maildirs;

        renamedCollection = q->topLevelCollection();
    } else {
        renamedCollection.setRemoteId(collectionName);
        renamedCollection.setName(collectionName);
    }

    // update collections in MBox contexts so they stay usable for purge
    for (const MBoxPtr &mbox : std::as_const(mMBoxes)) {
        if (mbox->mCollection.isValid()) {
            MBoxPtr updatedMBox = mbox;
            updatedMBox->mCollection = updateMBoxCollectionTree(mbox->mCollection, collection, renamedCollection);
        }
    }

    if (indexInvalidated) {
        const QVariant var = QVariant::fromValue<Collection::List>(Collection::List() << renamedCollection);
        job->setProperty("onDiskIndexInvalidated", var);
    }

    q->notifyCollectionsProcessed(Collection::List() << renamedCollection);
    return true;
}

bool MixedMaildirStorePrivate::visit(FileStore::CollectionMoveJob *job)
{
    QString errorText;

    const Collection moveCollection = job->collection();
    const Collection targetCollection = job->targetParent();

    QString movePath;
    const FolderType moveFolderType = folderForCollection(moveCollection, movePath, errorText);
    if (moveFolderType == InvalidFolder || moveFolderType == TopLevelFolder) {
        errorText = i18nc("@info:status",
                          "Cannot move folder %1 from folder %2 to folder %3",
                          moveCollection.name(),
                          moveCollection.parentCollection().name(),
                          targetCollection.name());
        qCCritical(MIXEDMAILDIRRESOURCE_LOG) << errorText << "FolderType=" << moveFolderType;
        q->notifyError(FileStore::Job::InvalidJobContext, errorText);
        return false;
    }

    //   qCDebug(MIXEDMAILDIR_LOG) << "moveCollection" << moveCollection.remoteId()
    //                                    << "movePath=" << movePath
    //                                    << "moveType=" << moveFolderType;

    QString targetPath;
    const FolderType targetFolderType = folderForCollection(targetCollection, targetPath, errorText);
    if (targetFolderType == InvalidFolder) {
        errorText = i18nc("@info:status",
                          "Cannot move folder %1 from folder %2 to folder %3",
                          moveCollection.name(),
                          moveCollection.parentCollection().name(),
                          targetCollection.name());
        qCCritical(MIXEDMAILDIRRESOURCE_LOG) << errorText << "FolderType=" << targetFolderType;
        q->notifyError(FileStore::Job::InvalidJobContext, errorText);
        return false;
    }

    //   qCDebug(MIXEDMAILDIR_LOG) << "targetCollection" << targetCollection.remoteId()
    //                                    << "targetPath=" << targetPath
    //                                    << "targetType=" << targetFolderType;

    const QFileInfo targetSubDirInfo(Maildir::subDirPathForFolderPath(targetPath));

    // if target is not the top level folder, make sure the sub folder directory exists
    if (targetFolderType != TopLevelFolder) {
        if (!targetSubDirInfo.exists()) {
            QDir topDir(q->path());
            if (!topDir.mkpath(targetSubDirInfo.absoluteFilePath())) {
                errorText = i18nc("@info:status",
                                  "Cannot move folder %1 from folder %2 to folder %3",
                                  moveCollection.name(),
                                  moveCollection.parentCollection().name(),
                                  targetCollection.name());
                qCCritical(MIXEDMAILDIRRESOURCE_LOG) << errorText << "MoveFolderType=" << moveFolderType << "TargetFolderType=" << targetFolderType;
                q->notifyError(FileStore::Job::InvalidJobContext, errorText);
                return false;
            }
        }
    }

    bool indexInvalidated = false;
    QString movedPath;

    if (moveFolderType == MBoxFolder) {
        // TODO would be nice if getOrCreateMBoxPtr() could be used instead, like below for Maildir
        MBoxPtr mbox;
        MBoxHash::const_iterator findIt = mMBoxes.constFind(movePath);
        if (findIt == mMBoxes.constEnd()) {
            mbox = MBoxPtr(new MBoxContext);
            if (!mbox->load(movePath)) {
                qCWarning(MIXEDMAILDIRRESOURCE_LOG) << "Failed to load mbox" << movePath;
            }

            mbox->mCollection = moveCollection;
            mMBoxes.insert(movePath, mbox);
        } else {
            mbox = findIt.value();
        }

        mbox->readIndexData();
        indexInvalidated = mbox->hasIndexData();

        const QFileInfo moveFileInfo(movePath);
        const QFileInfo moveSubDirInfo(Maildir::subDirPathForFolderPath(movePath));
        const QFileInfo targetFileInfo(targetPath);

        QDir targetDir(targetFolderType == TopLevelFolder ? targetPath : Maildir::subDirPathForFolderPath(targetPath));
        if (targetDir.exists(moveFileInfo.fileName()) || !targetDir.rename(moveFileInfo.absoluteFilePath(), moveFileInfo.fileName())) {
            errorText = i18nc("@info:status",
                              "Cannot move folder %1 from folder %2 to folder %3",
                              moveCollection.name(),
                              moveCollection.parentCollection().name(),
                              targetCollection.name());
            qCCritical(MIXEDMAILDIRRESOURCE_LOG) << errorText << "MoveFolderType=" << moveFolderType << "TargetFolderType=" << targetFolderType;
            q->notifyError(FileStore::Job::InvalidJobContext, errorText);
            return false;
        }

        if (moveSubDirInfo.exists()) {
            if (targetDir.exists(moveSubDirInfo.fileName()) || !targetDir.rename(moveSubDirInfo.absoluteFilePath(), moveSubDirInfo.fileName())) {
                errorText = i18nc("@info:status",
                                  "Cannot move folder %1 from folder %2 to folder %3",
                                  moveCollection.name(),
                                  moveCollection.parentCollection().name(),
                                  targetCollection.name());
                qCCritical(MIXEDMAILDIRRESOURCE_LOG) << errorText << "MoveFolderType=" << moveFolderType << "TargetFolderType=" << targetFolderType;

                // try to revert the other rename
                QDir sourceDir(moveFileInfo.absolutePath());
                sourceDir.cdUp();
                sourceDir.rename(targetFileInfo.absoluteFilePath(), moveFileInfo.fileName());
                q->notifyError(FileStore::Job::InvalidJobContext, errorText);
                return false;
            }
        }

        movedPath = QFileInfo(targetDir, moveFileInfo.fileName()).absoluteFilePath();
    } else {
        MaildirPtr md = getOrCreateMaildirPtr(movePath, false);

        md->readIndexData();
        indexInvalidated = md->hasIndexData();

        Maildir moveMd(movePath, false);

        // for moving purpose we can treat the MBox target's subDirPath like a top level maildir
        Maildir targetMd;
        if (targetFolderType == MBoxFolder) {
            targetMd = Maildir(targetSubDirInfo.absoluteFilePath(), true);
        } else {
            targetMd = Maildir(targetPath, targetFolderType == TopLevelFolder);
        }

        if (!moveMd.moveTo(targetMd)) {
            errorText = i18nc("@info:status",
                              "Cannot move folder %1 from folder %2 to folder %3",
                              moveCollection.name(),
                              moveCollection.parentCollection().name(),
                              targetCollection.name());
            qCCritical(MIXEDMAILDIRRESOURCE_LOG) << errorText << "MoveFolderType=" << moveFolderType << "TargetFolderType=" << targetFolderType;
            q->notifyError(FileStore::Job::InvalidJobContext, errorText);
            return false;
        }

        movedPath = targetMd.subFolder(moveCollection.remoteId()).path();
    }

    // update context hashes
    updateContextHashes(movePath, movedPath);

    Collection movedCollection = moveCollection;
    movedCollection.setParentCollection(targetCollection);

    // update collections in MBox contexts so they stay usable for purge
    for (const MBoxPtr &mbox : std::as_const(mMBoxes)) {
        if (mbox->mCollection.isValid()) {
            MBoxPtr updatedMBox = mbox;
            updatedMBox->mCollection = updateMBoxCollectionTree(mbox->mCollection, moveCollection, movedCollection);
        }
    }

    if (indexInvalidated) {
        const QVariant var = QVariant::fromValue<Collection::List>(Collection::List() << movedCollection);
        job->setProperty("onDiskIndexInvalidated", var);
    }

    q->notifyCollectionsProcessed(Collection::List() << movedCollection);
    return true;
}

bool MixedMaildirStorePrivate::visit(FileStore::ItemCreateJob *job)
{
    QString path;
    QString errorText;

    const FolderType folderType = folderForCollection(job->collection(), path, errorText);
    if (folderType == InvalidFolder || folderType == TopLevelFolder) {
        errorText = i18nc("@info:status", "Cannot add emails to folder %1", job->collection().name());
        qCCritical(MIXEDMAILDIRRESOURCE_LOG) << errorText << "FolderType=" << folderType;
        q->notifyError(FileStore::Job::InvalidJobContext, errorText);
        return false;
    }

    Item item = job->item();

    if (folderType == MBoxFolder) {
        MBoxPtr mbox;
        MBoxHash::const_iterator findIt = mMBoxes.constFind(path);
        if (findIt == mMBoxes.constEnd()) {
            mbox = MBoxPtr(new MBoxContext);
            if (!mbox->load(path)) {
                errorText = i18nc("@info:status", "Cannot add emails to folder %1", job->collection().name());
                qCCritical(MIXEDMAILDIRRESOURCE_LOG) << errorText << "FolderType=" << folderType;
                q->notifyError(FileStore::Job::InvalidJobContext, errorText);
                return false;
            }

            mbox->mCollection = job->collection();
            mMBoxes.insert(path, mbox);
        } else {
            mbox = findIt.value();
        }

        // make sure to read the index (if available) before modifying the data, which would
        // make the index invalid
        mbox->readIndexData();

        // if there is index data now, we let the job creator know that the on-disk index
        // became invalid
        if (mbox->hasIndexData()) {
            const QVariant var = QVariant::fromValue<Collection::List>(Collection::List() << job->collection());
            job->setProperty("onDiskIndexInvalidated", var);
        }

        qint64 result = mbox->appendEntry(item.payload<KMime::Message::Ptr>());
        if (result < 0) {
            errorText = i18nc("@info:status", "Cannot add emails to folder %1", job->collection().name());
            qCCritical(MIXEDMAILDIRRESOURCE_LOG) << errorText << "FolderType=" << folderType;
            q->notifyError(FileStore::Job::InvalidJobContext, errorText);
            return false;
        }
        mbox->save();
        item.setRemoteId(QString::number(result));
    } else {
        MaildirPtr mdPtr;
        MaildirHash::const_iterator findIt = mMaildirs.constFind(path);
        if (findIt == mMaildirs.constEnd()) {
            mdPtr = MaildirPtr(new MaildirContext(path, false));
            mMaildirs.insert(path, mdPtr);
        } else {
            mdPtr = findIt.value();
        }

        // make sure to read the index (if available) before modifying the data, which would
        // make the index invalid
        mdPtr->readIndexData();

        // if there is index data now, we let the job creator know that the on-disk index
        // became invalid
        if (mdPtr->hasIndexData()) {
            const QVariant var = QVariant::fromValue<Collection::List>(Collection::List() << job->collection());
            job->setProperty("onDiskIndexInvalidated", var);
        }

        const QString result = mdPtr->addEntry(item.payload<KMime::Message::Ptr>()->encodedContent());
        if (result.isEmpty()) {
            errorText = i18nc("@info:status", "Cannot add emails to folder %1", job->collection().name());
            qCCritical(MIXEDMAILDIRRESOURCE_LOG) << errorText << "FolderType=" << folderType;
            q->notifyError(FileStore::Job::InvalidJobContext, errorText);
            return false;
        }

        item.setRemoteId(result);
    }

    item.setParentCollection(job->collection());
    q->notifyItemsProcessed(Item::List() << item);
    return true;
}

bool MixedMaildirStorePrivate::visit(FileStore::ItemDeleteJob *job)
{
    const Item item = job->item();
    const Collection collection = item.parentCollection();
    QString path;
    QString errorText;

    const FolderType folderType = folderForCollection(collection, path, errorText);
    if (folderType == InvalidFolder || folderType == TopLevelFolder) {
        errorText = i18nc("@info:status", "Cannot remove emails from folder %1", collection.name());
        qCCritical(MIXEDMAILDIRRESOURCE_LOG) << errorText << "FolderType=" << folderType;
        q->notifyError(FileStore::Job::InvalidJobContext, errorText);
        return false;
    }

    if (folderType == MBoxFolder) {
        MBoxPtr mbox;
        MBoxHash::const_iterator findIt = mMBoxes.constFind(path);
        if (findIt == mMBoxes.constEnd()) {
            mbox = MBoxPtr(new MBoxContext);
            if (!mbox->load(path)) {
                errorText = i18nc("@info:status", "Cannot remove emails from folder %1", collection.name());
                qCCritical(MIXEDMAILDIRRESOURCE_LOG) << errorText << "FolderType=" << folderType;
                q->notifyError(FileStore::Job::InvalidJobContext, errorText);
                return false;
            }

            mMBoxes.insert(path, mbox);
        } else {
            mbox = findIt.value();
        }

        bool ok = false;
        qint64 offset = item.remoteId().toLongLong(&ok);
        if (!ok || offset < 0 || !mbox->isValidOffset(offset)) {
            errorText = i18nc("@info:status", "Cannot remove emails from folder %1", collection.name());
            qCCritical(MIXEDMAILDIRRESOURCE_LOG) << errorText << "FolderType=" << folderType;
            q->notifyError(FileStore::Job::InvalidJobContext, errorText);
            return false;
        }

        mbox->mCollection = collection;
        mbox->deleteEntry(offset);
        job->setProperty("compactStore", true);
    } else {
        MaildirPtr mdPtr;
        MaildirHash::const_iterator findIt = mMaildirs.constFind(path);
        if (findIt == mMaildirs.constEnd()) {
            mdPtr = MaildirPtr(new MaildirContext(path, false));

            mMaildirs.insert(path, mdPtr);
        } else {
            mdPtr = findIt.value();
        }

        // make sure to read the index (if available) before modifying the data, which would
        // make the index invalid
        mdPtr->readIndexData();

        // if there is index data now, we let the job creator know that the on-disk index
        // became invalid
        if (mdPtr->hasIndexData()) {
            const QVariant var = QVariant::fromValue<Collection::List>(Collection::List() << collection);
            job->setProperty("onDiskIndexInvalidated", var);
        }

        if (!mdPtr->removeEntry(item.remoteId())) {
            errorText = i18nc("@info:status", "Cannot remove emails from folder %1", collection.name());
            qCCritical(MIXEDMAILDIRRESOURCE_LOG) << errorText << "FolderType=" << folderType;
            q->notifyError(FileStore::Job::InvalidJobContext, errorText);
            return false;
        }
    }

    q->notifyItemsProcessed(Item::List() << item);
    return true;
}

bool MixedMaildirStorePrivate::visit(FileStore::ItemFetchJob *job)
{
    ItemFetchScope scope = job->fetchScope();
    const bool includeBody = scope.fullPayload() || scope.payloadParts().contains(MessagePart::Body);
    const bool includeHeaders = scope.payloadParts().contains(MessagePart::Header) || scope.payloadParts().contains(MessagePart::Envelope);

    const bool fetchItemsBatch = !job->requestedItems().isEmpty();
    const bool fetchSingleItem = job->collection().remoteId().isEmpty() && !fetchItemsBatch;
    const Collection collection = fetchItemsBatch ? job->requestedItems().at(0).parentCollection()
        : fetchSingleItem                         ? job->item().parentCollection()
                                                  : job->collection();

    QString path;
    QString errorText;
    Q_ASSERT(!collection.remoteId().isEmpty());
    const FolderType folderType = folderForCollection(collection, path, errorText);

    if (folderType == InvalidFolder) {
        qCCritical(MIXEDMAILDIRRESOURCE_LOG) << errorText << "collection:" << job->collection();
        q->notifyError(FileStore::Job::InvalidJobContext, errorText);
        return false;
    }

    if (folderType == MBoxFolder) {
        MBoxHash::iterator findIt = mMBoxes.find(path);
        if (findIt == mMBoxes.end() || (!fetchSingleItem && !fetchItemsBatch)) {
            MBoxPtr mbox = findIt != mMBoxes.end() ? findIt.value() : MBoxPtr(new MBoxContext);
            if (!mbox->load(path)) {
                errorText = i18nc("@info:status", "Failed to load MBox folder %1", path);
                qCCritical(MIXEDMAILDIRRESOURCE_LOG) << errorText << "collection=" << collection;
                q->notifyError(FileStore::Job::InvalidJobContext, errorText); // TODO should be a different error code
                if (findIt != mMBoxes.end()) {
                    mMBoxes.erase(findIt);
                }
                return false;
            }

            if (findIt == mMBoxes.end()) {
                findIt = mMBoxes.insert(path, mbox);
            }
        }

        Item::List items;
        if (fetchSingleItem) {
            items << job->item();
        } else if (fetchItemsBatch) {
            items = job->requestedItems();
        } else {
            listCollection(job, findIt.value(), collection, items);
        }

        Item::List::iterator it = items.begin();
        Item::List::iterator endIt = items.end();
        for (; it != endIt; ++it) {
            if (!fillItem(findIt.value(), includeHeaders, includeBody, *it)) {
                errorText = i18nc("@info:status", "Error while reading mails from folder %1", collection.name());
                q->notifyError(FileStore::Job::InvalidJobContext, errorText); // TODO should be a different error code
                qCCritical(MIXEDMAILDIRRESOURCE_LOG) << "Failed to read item" << (*it).remoteId() << "in MBox file" << path;
                return false;
            }
        }

        if (!items.isEmpty()) {
            q->notifyItemsProcessed(items);
        }
    } else {
        MaildirPtr mdPtr;
        MaildirHash::const_iterator mdIt = mMaildirs.constFind(path);
        if (mdIt == mMaildirs.constEnd()) {
            mdPtr = MaildirPtr(new MaildirContext(path, folderType == TopLevelFolder));
            mMaildirs.insert(path, mdPtr);
        } else {
            mdPtr = mdIt.value();
        }

        if (!mdPtr->isValid(errorText)) {
            errorText = i18nc("@info:status", "Failed to load Maildirs folder %1", path);
            qCCritical(MIXEDMAILDIRRESOURCE_LOG) << errorText << "collection=" << collection;
            q->notifyError(FileStore::Job::InvalidJobContext, errorText); // TODO should be a different error code
            return false;
        }

        Item::List items;
        if (fetchSingleItem) {
            items << job->item();
        } else if (fetchItemsBatch) {
            items = job->requestedItems();
        } else {
            listCollection(job, mdPtr, collection, items);
        }

        Item::List::iterator it = items.begin();
        Item::List::iterator endIt = items.end();
        for (; it != endIt; ++it) {
            if (!fillItem(mdPtr, includeHeaders, includeBody, *it)) {
                const QString errorText = i18nc("@info:status", "Error while reading mails from folder %1", collection.name());
                q->notifyError(FileStore::Job::InvalidJobContext, errorText); // TODO should be a different error code
                qCCritical(MIXEDMAILDIRRESOURCE_LOG) << "Failed to read item" << (*it).remoteId() << "in Maildir" << path;
                return false;
            }
        }

        if (!items.isEmpty()) {
            q->notifyItemsProcessed(items);
        }
    }

    return true;
}

bool MixedMaildirStorePrivate::visit(FileStore::ItemModifyJob *job)
{
    const QSet<QByteArray> parts = job->parts();
    bool payloadChanged = false;
    bool flagsChanged = false;
    for (const QByteArray &part : parts) {
        if (part.startsWith("PLD:")) {
            payloadChanged = true;
        }
        if (part.contains("FLAGS")) {
            flagsChanged = true;
        }
    }

    const bool nothingChanged = (!payloadChanged && !flagsChanged);
    const bool payloadChangedButIgnored = payloadChanged && job->ignorePayload();
    const bool ignoreModifyIfValid = nothingChanged || (payloadChangedButIgnored && !flagsChanged);

    Item item = job->item();
    const Collection collection = item.parentCollection();
    QString path;
    QString errorText;

    const FolderType folderType = folderForCollection(collection, path, errorText);
    if (folderType == InvalidFolder || folderType == TopLevelFolder) {
        errorText = i18nc("@info:status", "Cannot modify emails in folder %1", collection.name());
        qCCritical(MIXEDMAILDIRRESOURCE_LOG) << errorText << "FolderType=" << folderType;
        q->notifyError(FileStore::Job::InvalidJobContext, errorText);
        return false;
    }

    if (folderType == MBoxFolder) {
        MBoxPtr mbox;
        MBoxHash::const_iterator findIt = mMBoxes.constFind(path);
        if (findIt == mMBoxes.constEnd()) {
            mbox = MBoxPtr(new MBoxContext);
            if (!mbox->load(path)) {
                errorText = i18nc("@info:status", "Cannot modify emails in folder %1", collection.name());
                qCCritical(MIXEDMAILDIRRESOURCE_LOG) << errorText << "FolderType=" << folderType;
                q->notifyError(FileStore::Job::InvalidJobContext, errorText);
                return false;
            }

            mMBoxes.insert(path, mbox);
        } else {
            mbox = findIt.value();
        }

        bool ok = false;
        qint64 offset = item.remoteId().toLongLong(&ok);
        if (!ok || offset < 0 || !mbox->isValidOffset(offset)) {
            errorText = i18nc("@info:status", "Cannot modify emails in folder %1", collection.name());
            qCCritical(MIXEDMAILDIRRESOURCE_LOG) << errorText << "FolderType=" << folderType;
            q->notifyError(FileStore::Job::InvalidJobContext, errorText);
            return false;
        }

        // if we can ignore payload, or we have nothing else to change, then we are finished
        if (ignoreModifyIfValid) {
            qCDebug(MIXEDMAILDIR_LOG) << "ItemModifyJob for item" << item.remoteId() << "in collection" << collection.remoteId()
                                      << "skipped: nothing of interest changed (" << nothingChanged << ") or only payload changed but should be ignored ("
                                      << (payloadChanged && !flagsChanged && job->ignorePayload()) << "). Modified parts:" << parts;
            q->notifyItemsProcessed(Item::List() << job->item());
            return true;
        }

        // mbox can only change payload, ignore any other change
        if (!payloadChanged) {
            q->notifyItemsProcessed(Item::List() << item);
            return true;
        }

        // make sure to read the index (if available) before modifying the data, which would
        // make the index invalid
        mbox->readIndexData();

        // if there is index data now, we let the job creator know that the on-disk index
        // became invalid
        if (mbox->hasIndexData()) {
            const QVariant var = QVariant::fromValue<Collection::List>(Collection::List() << collection);
            job->setProperty("onDiskIndexInvalidated", var);
        }

        qint64 newOffset = mbox->appendEntry(item.payload<KMime::Message::Ptr>());
        if (newOffset < 0) {
            errorText = i18nc("@info:status", "Cannot modify emails in folder %1", collection.name());
            qCCritical(MIXEDMAILDIRRESOURCE_LOG) << errorText << "FolderType=" << folderType;
            q->notifyError(FileStore::Job::InvalidJobContext, errorText);
            return false;
        }

        if (newOffset > 0) {
            mbox->mCollection = collection;
            mbox->deleteEntry(offset);
            job->setProperty("compactStore", true);
        }
        mbox->save();
        item.setRemoteId(QString::number(newOffset));
    } else {
        MaildirPtr mdPtr;
        MaildirHash::const_iterator findIt = mMaildirs.constFind(path);
        if (findIt == mMaildirs.constEnd()) {
            mdPtr = MaildirPtr(new MaildirContext(path, false));
            mMaildirs.insert(path, mdPtr);
        } else {
            mdPtr = findIt.value();
        }

        if (!mdPtr->isValidEntry(item.remoteId())) {
            errorText = i18nc("@info:status", "Cannot modify emails in folder %1", collection.name());
            qCCritical(MIXEDMAILDIRRESOURCE_LOG) << errorText << "FolderType=" << folderType;
            q->notifyError(FileStore::Job::InvalidJobContext, errorText);
            return false;
        }

        // if we can ignore payload, or we have nothing else to change, then we are finished
        if (ignoreModifyIfValid) {
            qCDebug(MIXEDMAILDIR_LOG) << "ItemModifyJob for item" << item.remoteId() << "in collection" << collection.remoteId()
                                      << "skipped: nothing of interest changed (" << nothingChanged << ") or only payload changed but should be ignored ("
                                      << (payloadChanged && !flagsChanged && job->ignorePayload()) << "). Modified parts:" << parts;
            q->notifyItemsProcessed(Item::List() << job->item());
            return true;
        }

        // make sure to read the index (if available) before modifying the data, which would
        // make the index invalid
        mdPtr->readIndexData();

        // if there is index data now, we let the job creator know that the on-disk index
        // became invalid
        if (mdPtr->hasIndexData()) {
            const QVariant var = QVariant::fromValue<Collection::List>(Collection::List() << collection);
            job->setProperty("onDiskIndexInvalidated", var);
        }

        QString newKey = item.remoteId();
        if (flagsChanged) {
            Maildir md(mdPtr->maildir());
            newKey = md.changeEntryFlags(item.remoteId(), item.flags());
            if (newKey.isEmpty()) {
                errorText = i18nc("@info:status", "Cannot modify emails in folder %1. %2", collection.name(), md.lastError());
                qCCritical(MIXEDMAILDIRRESOURCE_LOG) << errorText << "FolderType=" << folderType;
                q->notifyError(FileStore::Job::InvalidJobContext, errorText);
                return false;
            }
            item.setRemoteId(newKey);
        }

        if (payloadChanged) {
            mdPtr->writeEntry(newKey, item.payload<KMime::Message::Ptr>()->encodedContent());
        }
    }

    q->notifyItemsProcessed(Item::List() << item);
    return true;
}

bool MixedMaildirStorePrivate::visit(FileStore::ItemMoveJob *job)
{
    QString errorText;

    QString sourcePath;
    const Collection sourceCollection = job->item().parentCollection();
    const FolderType sourceFolderType = folderForCollection(sourceCollection, sourcePath, errorText);
    if (sourceFolderType == InvalidFolder || sourceFolderType == TopLevelFolder) {
        errorText = i18nc("@info:status", "Cannot move emails from folder %1", sourceCollection.name());
        qCCritical(MIXEDMAILDIRRESOURCE_LOG) << errorText << "FolderType=" << sourceFolderType;
        q->notifyError(FileStore::Job::InvalidJobContext, errorText);
        return false;
    }

    //   qCDebug(MIXEDMAILDIR_LOG) << "sourceCollection" << sourceCollection.remoteId()
    //                                    << "sourcePath=" << sourcePath
    //                                    << "sourceType=" << sourceFolderType;

    QString targetPath;
    const Collection targetCollection = job->targetParent();
    const FolderType targetFolderType = folderForCollection(targetCollection, targetPath, errorText);
    if (targetFolderType == InvalidFolder || targetFolderType == TopLevelFolder) {
        errorText = i18nc("@info:status", "Cannot move emails to folder %1", targetCollection.name());
        qCCritical(MIXEDMAILDIRRESOURCE_LOG) << errorText << "FolderType=" << targetFolderType;
        q->notifyError(FileStore::Job::InvalidJobContext, errorText);
        return false;
    }

    //   qCDebug(MIXEDMAILDIR_LOG) << "targetCollection" << targetCollection.remoteId()
    //                                    << "targetPath=" << targetPath
    //                                    << "targetType=" << targetFolderType;

    Item item = job->item();

    if (sourceFolderType == MBoxFolder) {
        /*    qCDebug(MIXEDMAILDIR_LOG) << "source is MBox";*/
        bool ok = false;
        quint64 offset = item.remoteId().toULongLong(&ok);
        if (!ok) {
            errorText = i18nc("@info:status", "Cannot move emails from folder %1", sourceCollection.name());
            qCCritical(MIXEDMAILDIRRESOURCE_LOG) << errorText << "FolderType=" << sourceFolderType;
            q->notifyError(FileStore::Job::InvalidJobContext, errorText);
            return false;
        }

        MBoxPtr mbox;
        MBoxHash::const_iterator findIt = mMBoxes.constFind(sourcePath);
        if (findIt == mMBoxes.constEnd()) {
            mbox = MBoxPtr(new MBoxContext);
            if (!mbox->load(sourcePath)) {
                errorText = i18nc("@info:status", "Cannot move emails to folder %1", sourceCollection.name());
                qCCritical(MIXEDMAILDIRRESOURCE_LOG) << errorText << "FolderType=" << sourceFolderType;
                q->notifyError(FileStore::Job::InvalidJobContext, errorText);
                return false;
            }

            mbox->mCollection = sourceCollection;
            mMBoxes.insert(sourcePath, mbox);
        } else {
            mbox = findIt.value();
        }

        if (!mbox->isValidOffset(offset)) {
            errorText = i18nc("@info:status", "Cannot move emails from folder %1", sourceCollection.name());
            qCCritical(MIXEDMAILDIRRESOURCE_LOG) << errorText << "FolderType=" << sourceFolderType;
            q->notifyError(FileStore::Job::InvalidJobContext, errorText);
            return false;
        }

        if (!item.hasPayload<KMime::Message::Ptr>() || !item.loadedPayloadParts().contains(MessagePart::Body)) {
            if (!fillItem(mbox, true, true, item)) {
                errorText = i18nc("@info:status", "Cannot move email from folder %1", sourceCollection.name());
                qCCritical(MIXEDMAILDIRRESOURCE_LOG) << errorText << "FolderType=" << sourceFolderType;
                q->notifyError(FileStore::Job::InvalidJobContext, errorText);
                return false;
            }
        }

        Collection::List collections;

        // make sure to read the index (if available) before modifying the data, which would
        // make the index invalid
        mbox->readIndexData();

        if (mbox->hasIndexData()) {
            collections << sourceCollection;
        }

        if (targetFolderType == MBoxFolder) {
            /*      qCDebug(MIXEDMAILDIR_LOG) << "target is MBox";*/
            MBoxPtr targetMBox;
            MBoxHash::const_iterator findIt = mMBoxes.constFind(targetPath);
            if (findIt == mMBoxes.constEnd()) {
                targetMBox = MBoxPtr(new MBoxContext);
                if (!targetMBox->load(targetPath)) {
                    errorText = i18nc("@info:status", "Cannot move emails to folder %1", targetCollection.name());
                    qCCritical(MIXEDMAILDIRRESOURCE_LOG) << errorText << "FolderType=" << targetFolderType;
                    q->notifyError(FileStore::Job::InvalidJobContext, errorText);
                    return false;
                }

                targetMBox->mCollection = targetCollection;
                mMBoxes.insert(targetPath, targetMBox);
            } else {
                targetMBox = findIt.value();
            }

            // make sure to read the index (if available) before modifying the data, which would
            // make the index invalid
            targetMBox->readIndexData();

            // if there is index data now, we let the job creator know that the on-disk index
            // became invalid
            if (targetMBox->hasIndexData()) {
                collections << targetCollection;
            }

            qint64 remoteId = targetMBox->appendEntry(item.payload<KMime::Message::Ptr>());
            if (remoteId < 0) {
                errorText = i18nc("@info:status", "Cannot move emails to folder %1", targetCollection.name());
                qCCritical(MIXEDMAILDIRRESOURCE_LOG) << errorText << "FolderType=" << targetFolderType;
                q->notifyError(FileStore::Job::InvalidJobContext, errorText);
                return false;
            }

            if (!targetMBox->save()) {
                errorText = i18nc("@info:status", "Cannot move emails to folder %1", targetCollection.name());
                qCCritical(MIXEDMAILDIRRESOURCE_LOG) << errorText << "FolderType=" << targetFolderType;
                q->notifyError(FileStore::Job::InvalidJobContext, errorText);
                return false;
            }

            item.setRemoteId(QString::number(remoteId));
        } else {
            /*      qCDebug(MIXEDMAILDIR_LOG) << "target is Maildir";*/
            MaildirPtr targetMdPtr = getOrCreateMaildirPtr(targetPath, false);

            // make sure to read the index (if available) before modifying the data, which would
            // make the index invalid
            targetMdPtr->readIndexData();

            // if there is index data now, we let the job creator know that the on-disk index
            // became invalid
            if (targetMdPtr->hasIndexData()) {
                collections << targetCollection;
            }

            const QString remoteId = targetMdPtr->addEntry(mbox->readRawEntry(offset));
            if (remoteId.isEmpty()) {
                errorText = i18nc("@info:status", "Cannot move email from folder %1 to folder %2", sourceCollection.name(), targetCollection.name());
                qCCritical(MIXEDMAILDIRRESOURCE_LOG) << errorText << "SourceFolderType=" << sourceFolderType << "TargetFolderType=" << targetFolderType;
                q->notifyError(FileStore::Job::InvalidJobContext, errorText);
                return false;
            }

            item.setRemoteId(remoteId);
        }

        if (!collections.isEmpty()) {
            const QVariant var = QVariant::fromValue<Collection::List>(collections);
            job->setProperty("onDiskIndexInvalidated", var);
        }

        mbox->mCollection = sourceCollection;
        mbox->deleteEntry(offset);
        job->setProperty("compactStore", true);
    } else {
        /*    qCDebug(MIXEDMAILDIR_LOG) << "source is Maildir";*/
        MaildirPtr sourceMdPtr = getOrCreateMaildirPtr(sourcePath, false);

        if (!sourceMdPtr->isValidEntry(item.remoteId())) {
            errorText = i18nc("@info:status", "Cannot move email from folder %1", sourceCollection.name());
            qCCritical(MIXEDMAILDIRRESOURCE_LOG) << errorText << "FolderType=" << sourceFolderType;
            q->notifyError(FileStore::Job::InvalidJobContext, errorText);
            return false;
        }

        Collection::List collections;

        // make sure to read the index (if available) before modifying the data, which would
        // make the index invalid
        sourceMdPtr->readIndexData();

        // if there is index data now, we let the job creator know that the on-disk index
        // became invalid
        if (sourceMdPtr->hasIndexData()) {
            collections << sourceCollection;
        }

        if (targetFolderType == MBoxFolder) {
            /*      qCDebug(MIXEDMAILDIR_LOG) << "target is MBox";*/
            if (!item.hasPayload<KMime::Message::Ptr>() || !item.loadedPayloadParts().contains(MessagePart::Body)) {
                if (!fillItem(sourceMdPtr, true, true, item)) {
                    errorText = i18nc("@info:status", "Cannot move email from folder %1", sourceCollection.name());
                    qCCritical(MIXEDMAILDIRRESOURCE_LOG) << errorText << "FolderType=" << sourceFolderType;
                    q->notifyError(FileStore::Job::InvalidJobContext, errorText);
                    return false;
                }
            }

            MBoxPtr mbox;
            MBoxHash::const_iterator findIt = mMBoxes.constFind(targetPath);
            if (findIt == mMBoxes.constEnd()) {
                mbox = MBoxPtr(new MBoxContext);
                if (!mbox->load(targetPath)) {
                    errorText = i18nc("@info:status", "Cannot move emails to folder %1", targetCollection.name());
                    qCCritical(MIXEDMAILDIRRESOURCE_LOG) << errorText << "FolderType=" << targetFolderType;
                    q->notifyError(FileStore::Job::InvalidJobContext, errorText);
                    return false;
                }

                mbox->mCollection = targetCollection;
                mMBoxes.insert(targetPath, mbox);
            } else {
                mbox = findIt.value();
            }

            // make sure to read the index (if available) before modifying the data, which would
            // make the index invalid
            mbox->readIndexData();

            // if there is index data now, we let the job creator know that the on-disk index
            // became invalid
            if (mbox->hasIndexData()) {
                collections << targetCollection;
            }

            const qint64 remoteId = mbox->appendEntry(item.payload<KMime::Message::Ptr>());
            if (remoteId < 0) {
                errorText = i18nc("@info:status", "Cannot move emails to folder %1", targetCollection.name());
                qCCritical(MIXEDMAILDIRRESOURCE_LOG) << errorText << "FolderType=" << targetFolderType;
                q->notifyError(FileStore::Job::InvalidJobContext, errorText);
                return false;
            }
            sourceMdPtr->removeEntry(item.remoteId());

            mbox->save();
            item.setRemoteId(QString::number(remoteId));
        } else {
            /*      qCDebug(MIXEDMAILDIR_LOG) << "target is Maildir";*/
            MaildirPtr targetMdPtr = getOrCreateMaildirPtr(targetPath, false);

            // make sure to read the index (if available) before modifying the data, which would
            // make the index invalid
            targetMdPtr->readIndexData();

            // if there is index data now, we let the job creator know that the on-disk index
            // became invalid
            if (targetMdPtr->hasIndexData()) {
                collections << targetCollection;
            }

            const QString remoteId = sourceMdPtr->moveEntryTo(item.remoteId(), *targetMdPtr);
            if (remoteId.isEmpty()) {
                errorText = i18nc("@info:status", "Cannot move email from folder %1 to folder %2", sourceCollection.name(), targetCollection.name());
                qCCritical(MIXEDMAILDIRRESOURCE_LOG) << errorText << "SourceFolderType=" << sourceFolderType << "TargetFolderType=" << targetFolderType;
                q->notifyError(FileStore::Job::InvalidJobContext, errorText);
                return false;
            }

            item.setRemoteId(remoteId);
        }

        if (!collections.isEmpty()) {
            const QVariant var = QVariant::fromValue<Collection::List>(collections);
            job->setProperty("onDiskIndexInvalidated", var);
        }
    }

    item.setParentCollection(targetCollection);
    q->notifyItemsProcessed(Item::List() << item);
    return true;
}

bool MixedMaildirStorePrivate::visit(FileStore::StoreCompactJob *job)
{
    Q_UNUSED(job)

    Collection::List collections;

    MBoxHash::const_iterator it = mMBoxes.constBegin();
    MBoxHash::const_iterator endIt = mMBoxes.constEnd();
    for (; it != endIt; ++it) {
        MBoxPtr mbox = it.value();

        if (!mbox->hasDeletedOffsets()) {
            continue;
        }

        // make sure to read the index (if available) before modifying the data, which would
        // make the index invalid
        mbox->readIndexData();

        QVector<KMBox::MBoxEntry::Pair> movedEntries;
        const int result = mbox->purge(movedEntries);
        if (result > 0) {
            if (!movedEntries.isEmpty()) {
                qint64 revision = mbox->mCollection.remoteRevision().toLongLong();
                qCDebug(MIXEDMAILDIRRESOURCE_LOG) << "purge of" << mbox->mCollection.name() << "caused item move: oldRevision=" << revision << "(stored),"
                                                  << mbox->mRevision << "(local)";
                revision = qMax(revision, mbox->mRevision) + 1;

                const QString remoteRevision = QString::number(revision);

                Collection collection = mbox->mCollection;
                collection.attribute<FileStore::EntityCompactChangeAttribute>(Collection::AddIfMissing)->setRemoteRevision(remoteRevision);

                q->notifyCollectionsProcessed(Collection::List() << collection);

                mbox->mCollection.setRemoteRevision(remoteRevision);
                mbox->mRevision = revision;
            }

            Item::List items;
            items.reserve(movedEntries.count());
            for (const KMBox::MBoxEntry::Pair &offsetPair : std::as_const(movedEntries)) {
                const QString oldRemoteId(QString::number(offsetPair.first.messageOffset()));
                const QString newRemoteId(QString::number(offsetPair.second.messageOffset()));

                Item item;
                item.setRemoteId(oldRemoteId);
                item.setParentCollection(mbox->mCollection);
                item.attribute<FileStore::EntityCompactChangeAttribute>(Item::AddIfMissing)->setRemoteId(newRemoteId);

                items << item;
            }

            // if there is index data, we let the job creator know that the on-disk index
            // became invalid
            if (mbox->hasIndexData()) {
                collections << mbox->mCollection;
            }

            if (!items.isEmpty()) {
                q->notifyItemsProcessed(items);
            }
        }
    }

    if (!collections.isEmpty()) {
        const QVariant var = QVariant::fromValue<Collection::List>(collections);
        job->setProperty("onDiskIndexInvalidated", var);
    }

    return true;
}

MixedMaildirStore::MixedMaildirStore()
    : FileStore::AbstractLocalStore()
    , d(new MixedMaildirStorePrivate(this))
{
}

MixedMaildirStore::~MixedMaildirStore() = default;

void MixedMaildirStore::setTopLevelCollection(const Collection &collection)
{
    QStringList contentMimeTypes;
    contentMimeTypes << Collection::mimeType();

    Collection::Rights rights;
    // TODO check if read-only?
    rights = Collection::CanCreateCollection | Collection::CanChangeCollection | Collection::CanDeleteCollection;

    CachePolicy cachePolicy;
    cachePolicy.setInheritFromParent(false);
    cachePolicy.setLocalParts(QStringList() << QLatin1String(MessagePart::Envelope));
    cachePolicy.setSyncOnDemand(true);
    cachePolicy.setCacheTimeout(1);

    Collection modifiedCollection = collection;
    modifiedCollection.setContentMimeTypes(contentMimeTypes);
    modifiedCollection.setRights(rights);
    modifiedCollection.setParentCollection(Collection::root());
    modifiedCollection.setCachePolicy(cachePolicy);

    // clear caches
    d->mMBoxes.clear();
    d->mMaildirs.clear();

    FileStore::AbstractLocalStore::setTopLevelCollection(modifiedCollection);
}

void MixedMaildirStore::processJob(FileStore::Job *job)
{
    if (!job->accept(d.get())) {
        // check that an error has been set
        if (job->error() == 0 || job->errorString().isEmpty()) {
            qCCritical(MIXEDMAILDIRRESOURCE_LOG) << "visitor did not set either error code or error string when returning false";
            Q_ASSERT(job->error() == 0 || job->errorString().isEmpty());
        }
    } else {
        // check that no error has been set
        if (job->error() != 0 || !job->errorString().isEmpty()) {
            qCCritical(MIXEDMAILDIRRESOURCE_LOG) << "visitor did set either error code or error string when returning true";
            Q_ASSERT(job->error() != 0 || !job->errorString().isEmpty());
        }
    }
}

void MixedMaildirStore::checkCollectionMove(FileStore::CollectionMoveJob *job, int &errorCode, QString &errorText) const
{
    // check if the target is not the collection itself or one if its children
    Collection targetCollection = job->targetParent();
    while (targetCollection.isValid()) {
        if (targetCollection == job->collection()) {
            errorCode = FileStore::Job::InvalidJobContext;
            errorText = i18nc("@info:status", "Cannot move folder %1 into one of its own subfolder tree", job->collection().name());
            return;
        }

        targetCollection = targetCollection.parentCollection();
    }
}

void MixedMaildirStore::checkItemCreate(FileStore::ItemCreateJob *job, int &errorCode, QString &errorText) const
{
    if (!job->item().hasPayload<KMime::Message::Ptr>()) {
        errorCode = FileStore::Job::InvalidJobContext;
        errorText = i18nc("@info:status", "Cannot add email to folder %1 because there is no email content", job->collection().name());
    }
}

void MixedMaildirStore::checkItemModify(FileStore::ItemModifyJob *job, int &errorCode, QString &errorText) const
{
    if (!job->ignorePayload() && !job->item().hasPayload<KMime::Message::Ptr>()) {
        errorCode = FileStore::Job::InvalidJobContext;
        errorText = i18nc("@info:status", "Cannot modify email in folder %1 because there is no email content", job->item().parentCollection().name());
    }
}

void MixedMaildirStore::checkItemFetch(FileStore::ItemFetchJob *job, int &errorCode, QString &errorText) const
{
    Q_UNUSED(errorCode)
    Q_UNUSED(errorText)
    if (!job->requestedItems().isEmpty()) {
        // Requesting items
        const auto items = job->requestedItems();
        for (const Item &item : items) {
            const Collection coll = item.parentCollection();
            Q_ASSERT(!coll.remoteId().isEmpty());
        }
    } else {
        // Requesting an entire collection
        Q_ASSERT(!job->collection().remoteId().isEmpty());
    }
}
