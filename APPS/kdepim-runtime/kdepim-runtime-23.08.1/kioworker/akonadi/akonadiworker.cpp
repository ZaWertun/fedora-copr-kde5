/*
    SPDX-FileCopyrightText: 2006 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "akonadiworker.h"

#include <Akonadi/Collection>
#include <Akonadi/CollectionDeleteJob>
#include <Akonadi/CollectionFetchJob>
#include <Akonadi/EntityDisplayAttribute>
#include <Akonadi/ItemDeleteJob>
#include <Akonadi/ItemFetchJob>
#include <Akonadi/ItemFetchScope>

#include "akonadiworker_debug.h"

#include <KAboutData>
#include <KLocalizedString>
#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>

#ifdef Q_OS_WIN
// see kio/core/src/kioglobal_p.h
#define S_IRUSR 0400
#define S_IRGRP 0040
#define S_IROTH 0004
#endif

// Pseudo plugin class to embed meta data
class KIOPluginForMetaData : public QObject
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.kio.worker.akonadi" FILE "akonadi.json")
};

extern "C" {
int Q_DECL_EXPORT kdemain(int argc, char **argv);
}

int kdemain(int argc, char **argv)
{
    QApplication app(argc, argv);
    KAboutData aboutData(QStringLiteral("kio_akonadi"), QString(), QStringLiteral("0"));
    QCommandLineParser parser;
    KAboutData::setApplicationData(aboutData);
    parser.addOption(QCommandLineOption(QStringList() << QStringLiteral("+protocol"), i18n("Protocol name")));
    parser.addOption(QCommandLineOption(QStringList() << QStringLiteral("+pool"), i18n("Socket name")));
    parser.addOption(QCommandLineOption(QStringList() << QStringLiteral("+app"), i18n("Socket name")));

    aboutData.setupCommandLine(&parser);
    parser.process(app);
    aboutData.processCommandLine(&parser);

    AkonadiWorker worker(parser.positionalArguments().at(1).toLocal8Bit(), parser.positionalArguments().at(2).toLocal8Bit());
    worker.dispatchLoop();

    return 0;
}

using namespace Akonadi;

AkonadiWorker::AkonadiWorker(const QByteArray &pool_socket, const QByteArray &app_socket)
    : KIO::WorkerBase("akonadi", pool_socket, app_socket)
{
    qCDebug(AKONADIWORKER_LOG) << "kio_akonadi starting up";
}

AkonadiWorker::~AkonadiWorker()
{
    qCDebug(AKONADIWORKER_LOG) << "kio_akonadi shutting down";
}

KIO::WorkerResult AkonadiWorker::get(const QUrl &url)
{
    const Item item = Item::fromUrl(url);
    auto job = new ItemFetchJob(item);
    job->fetchScope().fetchFullPayload();

    if (!job->exec()) {
        return KIO::WorkerResult::fail(KIO::ERR_INTERNAL, job->errorString());
    }

    if (job->items().count() != 1) {
        return KIO::WorkerResult::fail(KIO::ERR_DOES_NOT_EXIST, i18n("No such item."));
    } else {
        const Item itemJob = job->items().at(0);
        const QByteArray tmp = itemJob.payloadData();
        data(tmp);
        data(QByteArray());
        return KIO::WorkerResult::pass();
    }
}

KIO::WorkerResult AkonadiWorker::stat(const QUrl &url)
{
    qCDebug(AKONADIWORKER_LOG) << url;

    // Stats for a collection
    if (Collection::fromUrl(url).isValid()) {
        Collection collection = Collection::fromUrl(url);

        if (collection != Collection::root()) {
            // Check that the collection exists.
            auto job = new CollectionFetchJob(collection, CollectionFetchJob::Base);
            if (!job->exec()) {
                return KIO::WorkerResult::fail(KIO::ERR_INTERNAL, job->errorString());
            }

            if (job->collections().count() != 1) {
                return KIO::WorkerResult::fail(KIO::ERR_DOES_NOT_EXIST, i18n("No such item."));
            }

            collection = job->collections().at(0);
        }

        statEntry(entryForCollection(collection));
        return KIO::WorkerResult::pass();
    }
    // Stats for an item
    else if (Item::fromUrl(url).isValid()) {
        auto job = new ItemFetchJob(Item::fromUrl(url));

        if (!job->exec()) {
            return KIO::WorkerResult::fail(KIO::ERR_INTERNAL, job->errorString());
        }

        if (job->items().count() != 1) {
            return KIO::WorkerResult::fail(KIO::ERR_DOES_NOT_EXIST, i18n("No such item."));
        }

        const Item item = job->items().at(0);
        statEntry(entryForItem(item));
        return KIO::WorkerResult::pass();
    }
    return KIO::WorkerResult::fail(KIO::ERR_DOES_NOT_EXIST, url.toString());
}

KIO::WorkerResult AkonadiWorker::del(const QUrl &url, bool isFile)
{
    qCDebug(AKONADIWORKER_LOG) << url;

    if (!isFile) { // It's a directory
        Collection collection = Collection::fromUrl(url);
        auto job = new CollectionDeleteJob(collection);
        if (!job->exec()) {
            return KIO::WorkerResult::fail(KIO::ERR_INTERNAL, job->errorString());
        }
        return KIO::WorkerResult::pass();
    } else { // It's a file
        auto job = new ItemDeleteJob(Item::fromUrl(url));
        if (!job->exec()) {
            return KIO::WorkerResult::fail(KIO::ERR_INTERNAL, job->errorString());
        }
        return KIO::WorkerResult::pass();
    }
}

KIO::WorkerResult AkonadiWorker::listDir(const QUrl &url)
{
    qCDebug(AKONADIWORKER_LOG) << url;

    if (!Collection::fromUrl(url).isValid()) {
        return KIO::WorkerResult::fail(KIO::ERR_DOES_NOT_EXIST, i18n("No such collection."));
    }

    // Fetching collections
    Collection collection = Collection::fromUrl(url);
    if (!collection.isValid()) {
        return KIO::WorkerResult::fail(KIO::ERR_DOES_NOT_EXIST, i18n("No such collection."));
    }
    auto job = new CollectionFetchJob(collection, CollectionFetchJob::FirstLevel);
    if (!job->exec()) {
        return KIO::WorkerResult::fail(KIO::ERR_CANNOT_ENTER_DIRECTORY, job->errorString());
    }

    const Collection::List collections = job->collections();
    for (const Collection &col : collections) {
        listEntry(entryForCollection(col));
    }

    // Fetching items
    if (collection != Collection::root()) {
        auto fjob = new ItemFetchJob(collection);
        if (!fjob->exec()) {
            return KIO::WorkerResult::fail(KIO::ERR_INTERNAL, job->errorString());
        }
        const Item::List items = fjob->items();
        totalSize(collections.count() + items.count());
        for (const Item &item : items) {
            listEntry(entryForItem(item));
        }
    }

    return KIO::WorkerResult::pass();
}

KIO::UDSEntry AkonadiWorker::entryForItem(const Akonadi::Item &item)
{
    KIO::UDSEntry entry;
    entry.reserve(7);
    entry.fastInsert(KIO::UDSEntry::UDS_NAME, QString::number(item.id()));
    entry.fastInsert(KIO::UDSEntry::UDS_MIME_TYPE, item.mimeType());
    entry.fastInsert(KIO::UDSEntry::UDS_FILE_TYPE, S_IFREG);
    entry.fastInsert(KIO::UDSEntry::UDS_URL, item.url().url());
    entry.fastInsert(KIO::UDSEntry::UDS_SIZE, item.size());
    entry.fastInsert(KIO::UDSEntry::UDS_ACCESS, S_IRUSR | S_IRGRP | S_IROTH);
    entry.fastInsert(KIO::UDSEntry::UDS_MODIFICATION_TIME, item.modificationTime().toSecsSinceEpoch());
    return entry;
}

KIO::UDSEntry AkonadiWorker::entryForCollection(const Akonadi::Collection &collection)
{
    KIO::UDSEntry entry;
    entry.reserve(7);
    entry.fastInsert(KIO::UDSEntry::UDS_NAME, collection.name());
    entry.fastInsert(KIO::UDSEntry::UDS_MIME_TYPE, Collection::mimeType());
    entry.fastInsert(KIO::UDSEntry::UDS_FILE_TYPE, S_IFDIR);
    entry.fastInsert(KIO::UDSEntry::UDS_URL, collection.url().url());
    entry.fastInsert(KIO::UDSEntry::UDS_ACCESS, S_IRUSR | S_IRGRP | S_IROTH);
    if (const auto attr = collection.attribute<EntityDisplayAttribute>()) {
        if (!attr->iconName().isEmpty()) {
            entry.fastInsert(KIO::UDSEntry::UDS_ICON_NAME, attr->iconName());
        }
        if (!attr->displayName().isEmpty()) {
            entry.fastInsert(KIO::UDSEntry::UDS_DISPLAY_NAME, attr->displayName());
        }
    }
    return entry;
}

#include "akonadiworker.moc"
