/*
    SPDX-FileCopyrightText: 2008 Bertjan Broeksema <broeksema@kde.org>
    SPDX-FileCopyrightText: 2008 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "singlefileresourcebase.h"

#include <Akonadi/ChangeRecorder>
#include <Akonadi/EntityDisplayAttribute>
#include <Akonadi/ItemFetchScope>

#include <KDirWatch>
#include <KIO/Job>
#include <KLocalizedString>
#include <QDebug>

#include <KConfigGroup>

#include <QCryptographicHash>
#include <QDir>
#include <QStandardPaths>
#include <QTimer>

Q_DECLARE_METATYPE(QEventLoopLocker *)

using namespace Akonadi;

SingleFileResourceBase::SingleFileResourceBase(const QString &id)
    : ResourceBase(id)
{
    connect(this, &SingleFileResourceBase::reloadConfiguration, this, [this]() {
        applyConfigurationChanges();
        reloadFile();
        synchronizeCollectionTree();
    });
    QTimer::singleShot(0, this, [this]() {
        readFile();
    });

    changeRecorder()->itemFetchScope().fetchFullPayload();
    changeRecorder()->fetchCollection(true);

    connect(changeRecorder(), &ChangeRecorder::changesAdded, this, &SingleFileResourceBase::scheduleWrite);

    connect(KDirWatch::self(), &KDirWatch::dirty, this, &SingleFileResourceBase::fileChanged);
    connect(KDirWatch::self(), &KDirWatch::created, this, &SingleFileResourceBase::fileChanged);
}

void SingleFileResourceBase::applyConfigurationChanges()
{
}

KSharedConfig::Ptr SingleFileResourceBase::runtimeConfig() const
{
    return KSharedConfig::openConfig(name() + QLatin1String("rc"), KConfig::SimpleConfig, QStandardPaths::CacheLocation);
}

bool SingleFileResourceBase::readLocalFile(const QString &fileName)
{
    const QByteArray newHash = calculateHash(fileName);
    if (mCurrentHash != newHash) {
        if (!mCurrentHash.isEmpty()) {
            // There was a hash stored in the config file or a cached one from
            // a previous read and it is different from the hash we just read.
            handleHashChange();
        }

        if (!readFromFile(fileName)) {
            mCurrentHash.clear();
            mCurrentUrl = QUrl(); // reset so we don't accidentally overwrite the file
            return false;
        }

        if (mCurrentHash.isEmpty()) {
            // This is the very first time we read the file so make sure to store
            // the hash as writeFile() might not be called at all (e.g in case of
            // read only resources).
            saveHash(newHash);
        }

        // Only synchronize when the contents of the file have changed wrt to
        // the last time this file was read. Before we synchronize first
        // clearCache is called to make sure that the cached items get the
        // actual values as present in the file.
        invalidateCache(rootCollection());
        synchronize();
    } else {
        // The hash didn't change, notify implementing resources about the
        // actual file name that should be used when reading the file is
        // necessary.
        setLocalFileName(fileName);
    }

    mCurrentHash = newHash;
    return true;
}

void SingleFileResourceBase::setLocalFileName(const QString &fileName)
{
    // Default implementation.
    if (!readFromFile(fileName)) {
        mCurrentHash.clear();
        mCurrentUrl = QUrl(); // reset so we don't accidentally overwrite the file
        return;
    }
}

QString SingleFileResourceBase::cacheFile() const
{
    const QString currentDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    QDir().mkpath(currentDir);
    return currentDir + QLatin1Char('/') + identifier();
}

QByteArray SingleFileResourceBase::calculateHash(const QString &fileName) const
{
    QFile file(fileName);
    if (!file.exists()) {
        return {};
    }

    if (!file.open(QIODevice::ReadOnly)) {
        return {};
    }

    QCryptographicHash hash(QCryptographicHash::Md5);
    qint64 blockSize = 512 * 1024; // Read blocks of 512K

    while (!file.atEnd()) {
        hash.addData(file.read(blockSize));
    }

    file.close();

    return hash.result();
}

void SingleFileResourceBase::handleHashChange()
{
    // Default implementation does nothing.
    qDebug() << "The hash has changed.";
}

QByteArray SingleFileResourceBase::loadHash() const
{
    KConfigGroup generalGroup(runtimeConfig(), "General");
    return QByteArray::fromHex(generalGroup.readEntry<QByteArray>("hash", QByteArray()));
}

void SingleFileResourceBase::saveHash(const QByteArray &hash) const
{
    KSharedConfig::Ptr config = runtimeConfig();
    KConfigGroup generalGroup(config, "General");
    generalGroup.writeEntry("hash", hash.toHex());
    config->sync();
}

void SingleFileResourceBase::setSupportedMimetypes(const QStringList &mimeTypes, const QString &icon)
{
    mSupportedMimetypes = mimeTypes;
    mCollectionIcon = icon;
}

void SingleFileResourceBase::collectionChanged(const Akonadi::Collection &collection)
{
    const QString newName = collection.displayName();
    if (collection.hasAttribute<EntityDisplayAttribute>()) {
        const auto attr = collection.attribute<EntityDisplayAttribute>();
        if (!attr->iconName().isEmpty()) {
            mCollectionIcon = attr->iconName();
        }
    }

    if (newName != name()) {
        setName(newName);
    }

    changeCommitted(collection);
}

void SingleFileResourceBase::reloadFile()
{
    // Update the network setting.
    setNeedsNetwork(!mCurrentUrl.isEmpty() && !mCurrentUrl.isLocalFile());

    // if we have something loaded already, make sure we write that back in case
    // the settings changed
    if (!mCurrentUrl.isEmpty() && !readOnly()) {
        writeFile();
    }

    readFile();

    // name or rights could have changed
    synchronizeCollectionTree();
}

void SingleFileResourceBase::handleProgress(KJob *, unsigned long pct)
{
    Q_EMIT percent(pct);
}

void SingleFileResourceBase::fileChanged(const QString &fileName)
{
    if (fileName != mCurrentUrl.toLocalFile()) {
        return;
    }

    const QByteArray newHash = calculateHash(fileName);

    // There is only a need to synchronize when the file was changed by another
    // process. At this point we're sure that it is the file that the resource
    // was configured for because of the check at the beginning of this function.
    if (newHash == mCurrentHash) {
        return;
    }

    if (!mCurrentUrl.isEmpty()) {
        QString lostFoundFileName;
        const QUrl prevUrl = mCurrentUrl;
        int i = 0;
        do {
            lostFoundFileName = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1Char('/') + identifier() + QLatin1Char('/')
                + prevUrl.fileName() + QLatin1Char('-') + QString::number(++i);
        } while (QFileInfo::exists(lostFoundFileName));

        // create the directory if it doesn't exist yet
        QDir dir = QFileInfo(lostFoundFileName).dir();
        if (!dir.exists()) {
            dir.mkpath(dir.path());
        }

        mCurrentUrl = QUrl::fromLocalFile(lostFoundFileName);
        writeFile();
        mCurrentUrl = prevUrl;

        const QString message = i18n(
            "The file '%1' was changed on disk. "
            "As a precaution, a backup of its previous contents has been created at '%2'.",
            prevUrl.toDisplayString(),
            QUrl::fromLocalFile(lostFoundFileName).toDisplayString());
        Q_EMIT warning(message);
    }

    readFile();

    // Notify resources, so that information bound to the file like indexes etc.
    // can be updated.
    handleHashChange();
    invalidateCache(rootCollection());
    synchronize();
}

void SingleFileResourceBase::scheduleWrite()
{
    scheduleCustomTask(this, "writeFile", QVariant(true), ResourceBase::AfterChangeReplay);
}

void SingleFileResourceBase::slotDownloadJobResult(KJob *job)
{
    if (job->error() && job->error() != KIO::ERR_DOES_NOT_EXIST) {
        const QString message = i18n("Could not load file '%1'.", mCurrentUrl.toDisplayString());
        qWarning() << message;
        Q_EMIT status(Broken, message);
    } else {
        readLocalFile(QUrl::fromLocalFile(cacheFile()).toLocalFile());
    }

    mDownloadJob = nullptr;
    auto ref = job->property("QEventLoopLocker").value<QEventLoopLocker *>();
    if (ref) {
        delete ref;
    }

    Q_EMIT status(Idle, i18nc("@info:status", "Ready"));
}

void SingleFileResourceBase::slotUploadJobResult(KJob *job)
{
    if (job->error()) {
        const QString message = i18n("Could not save file '%1'.", mCurrentUrl.toDisplayString());
        qWarning() << message;
        Q_EMIT status(Broken, message);
    }

    mUploadJob = nullptr;
    auto ref = job->property("QEventLoopLocker").value<QEventLoopLocker *>();
    if (ref) {
        delete ref;
    }

    Q_EMIT status(Idle, i18nc("@info:status", "Ready"));
}
