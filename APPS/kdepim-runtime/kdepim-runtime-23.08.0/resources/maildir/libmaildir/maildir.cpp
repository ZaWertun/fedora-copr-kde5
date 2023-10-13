/*
    SPDX-FileCopyrightText: 2007 Till Adam <adam@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "maildir.h"
#include "keycache.h"

#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QHostInfo>
#include <QRandomGenerator>
#include <QRegularExpression>
#include <QUuid>

#include "libmaildir_debug.h"
#include <Akonadi/MessageFlags>
#include <KLocalizedString>
#include <fcntl.h>

// Define it to get more debug output to expense of operating speed
// #define DEBUG_KEYCACHE_CONSITENCY

using namespace KPIM;

static QRegularExpression statusSeparatorRx()
{
    static const QRegularExpression expr(QStringLiteral(":|!"));
    return expr;
}

class Maildir::Private
{
public:
    Private(const QString &p, bool isRoot)
        : path(p)
        , hostName(QHostInfo::localHostName())
        , isRoot(isRoot)
    {
        // Cache object is created the first time this runs.
        // It will live throughout the lifetime of the application
        KeyCache::self()->addKeys(path);
    }

    Private(const Private &rhs)
        : path(rhs.path)
        , hostName(rhs.hostName)
        , isRoot(rhs.isRoot)
    {
    }

    bool operator==(const Private &rhs) const
    {
        return path == rhs.path;
    }

    bool accessIsPossible(bool createMissingFolders = true);
    Q_REQUIRED_RESULT bool canAccess(const QString &path) const;

    Q_REQUIRED_RESULT QStringList subPaths() const
    {
        QStringList paths;
        paths << path + QLatin1String("/cur");
        paths << path + QLatin1String("/new");
        paths << path + QLatin1String("/tmp");
        return paths;
    }

    Q_REQUIRED_RESULT QStringList listNew() const
    {
        QDir d(path + QLatin1String("/new"));
        d.setSorting(QDir::NoSort);
        return d.entryList(QDir::Files);
    }

    Q_REQUIRED_RESULT QStringList listCurrent() const
    {
        QDir d(path + QLatin1String("/cur"));
        d.setSorting(QDir::NoSort);
        return d.entryList(QDir::Files);
    }

    Q_REQUIRED_RESULT QString findRealKey(const QString &key) const
    {
        KeyCache *keyCache = KeyCache::self();
        if (keyCache->isNewKey(path, key)) {
#ifdef DEBUG_KEYCACHE_CONSITENCY
            if (!QFile::exists(path + QString::fromLatin1("/new/") + key)) {
                qCDebug(LIBMAILDIR_LOG) << "WARNING: key is in cache, but the file is gone: " << path + QString::fromLatin1("/new/") + key;
            }
#endif
            return path + QLatin1String("/new/") + key;
        }
        if (keyCache->isCurKey(path, key)) {
#ifdef DEBUG_KEYCACHE_CONSITENCY
            if (!QFile::exists(path + QString::fromLatin1("/cur/") + key)) {
                qCDebug(LIBMAILDIR_LOG) << "WARNING: key is in cache, but the file is gone: " << path + QString::fromLatin1("/cur/") + key;
            }
#endif
            return path + QLatin1String("/cur/") + key;
        }
        QString realKey = path + QLatin1String("/new/") + key;

        if (QFileInfo::exists(realKey)) {
            keyCache->addNewKey(path, key);
        } else { // not in "new", search in "cur"
            realKey = path + QLatin1String("/cur/") + key;
            if (QFileInfo::exists(realKey)) {
                keyCache->addCurKey(path, key);
            } else {
                realKey.clear(); // not in "cur" either
            }
        }

        return realKey;
    }

    static QString subDirNameForFolderName(const QString &folderName)
    {
        return QStringLiteral(".%1.directory").arg(folderName);
    }

    Q_REQUIRED_RESULT QString subDirPath() const
    {
        QDir dir(path);
        return subDirNameForFolderName(dir.dirName());
    }

    bool moveAndRename(QDir &dest, const QString &newName)
    {
        if (!dest.exists()) {
            qCDebug(LIBMAILDIR_LOG) << "Destination does not exist";
            return false;
        }
        if (dest.exists(newName) || dest.exists(subDirNameForFolderName(newName))) {
            qCDebug(LIBMAILDIR_LOG) << "New name already in use";
            return false;
        }

        if (!dest.rename(path, newName)) {
            qCDebug(LIBMAILDIR_LOG) << "Failed to rename maildir";
            return false;
        }
        const QDir subDirs(Maildir::subDirPathForFolderPath(path));
        if (subDirs.exists() && !dest.rename(subDirs.path(), subDirNameForFolderName(newName))) {
            qCDebug(LIBMAILDIR_LOG) << "Failed to rename subfolders";
            return false;
        }

        path = dest.path() + QLatin1Char('/') + newName;
        return true;
    }

    QString path;
    QString hostName;
    QString lastError;
    bool isRoot;
};

Maildir::Maildir(const QString &path, bool isRoot)
    : d(new Private(path, isRoot))
{
}

void Maildir::swap(const Maildir &rhs)
{
    Private *p = d;
    d = new Private(*rhs.d);
    delete p;
}

Maildir::Maildir(const Maildir &rhs)
    : d(new Private(*rhs.d))
{
}

Maildir &Maildir::operator=(const Maildir &rhs)
{
    // copy and swap, exception safe, and handles assignment to self
    Maildir temp(rhs);
    swap(temp);
    return *this;
}

bool Maildir::operator==(const Maildir &rhs) const
{
    return *d == *rhs.d;
}

Maildir::~Maildir()
{
    delete d;
}

bool Maildir::Private::canAccess(const QString &path) const
{
    // return access( QFile::encodeName( path ), R_OK | W_OK | X_OK ) != 0;
    // FIXME X_OK?
    QFileInfo d(path);
    return d.isReadable() && d.isWritable();
}

bool Maildir::Private::accessIsPossible(bool createMissingFolders)
{
    QStringList paths = subPaths();

    paths.prepend(path);

    for (const QString &p : std::as_const(paths)) {
        if (!QFileInfo::exists(p)) {
            if (!createMissingFolders) {
                lastError = i18n("Error opening %1; this folder is missing.", p);
                return false;
            }
            QDir().mkpath(p);
            if (!QFileInfo::exists(p)) {
                lastError = i18n("Error opening %1; this folder is missing.", p);
                return false;
            }
        }
        if (!canAccess(p)) {
            lastError = i18n(
                "Error opening %1; either this is not a valid "
                "maildir folder, or you do not have sufficient access permissions.",
                p);
            return false;
        }
    }
    return true;
}

bool Maildir::isValid(bool createMissingFolders) const
{
    if (path().isEmpty()) {
        return false;
    }
    if (!d->isRoot) {
        if (d->accessIsPossible(createMissingFolders)) {
            return true;
        }
    } else {
        const QStringList lstMaildir = subFolderList();
        for (const QString &sf : lstMaildir) {
            const Maildir subMd = Maildir(path() + QLatin1Char('/') + sf);
            if (!subMd.isValid(createMissingFolders)) {
                d->lastError = subMd.lastError();
                return false;
            }
        }
        return true;
    }
    return false;
}

bool Maildir::isRoot() const
{
    return d->isRoot;
}

bool Maildir::create()
{
    // FIXME: in a failure case, this will leave partially created dirs around
    // we should clean them up, but only if they didn't previously existed...
    const QStringList lstPath = d->subPaths();
    for (const QString &p : lstPath) {
        QDir dir(p);
        if (!dir.exists(p)) {
            if (!dir.mkpath(p)) {
                return false;
            }
        }
    }
    return true;
}

QString Maildir::path() const
{
    return d->path;
}

QString Maildir::name() const
{
    const QDir dir(d->path);
    return dir.dirName();
}

QString Maildir::addSubFolder(const QString &path)
{
    if (!isValid()) {
        return {};
    }

    // make the subdir dir
    QDir dir(d->path);
    if (!d->isRoot) {
        dir.cdUp();
        if (!dir.exists(d->subDirPath())) {
            dir.mkdir(d->subDirPath());
        }
        dir.cd(d->subDirPath());
    }

    const QString fullPath = dir.path() + QLatin1Char('/') + path;
    Maildir subdir(fullPath);
    if (subdir.create()) {
        return fullPath;
    }
    return {};
}

bool Maildir::removeSubFolder(const QString &folderName)
{
    if (!isValid()) {
        return false;
    }
    QDir dir(d->path);
    if (!d->isRoot) {
        dir.cdUp();
        if (!dir.exists(d->subDirPath())) {
            return false;
        }
        dir.cd(d->subDirPath());
    }
    if (!dir.exists(folderName)) {
        return false;
    }

    // remove it recursively
    bool result = QDir(dir.absolutePath() + QLatin1Char('/') + folderName).removeRecursively();
    QString subfolderName = subDirNameForFolderName(folderName);
    if (dir.exists(subfolderName)) {
        result &= QDir(dir.absolutePath() + QLatin1Char('/') + subfolderName).removeRecursively();
    }
    return result;
}

Maildir Maildir::subFolder(const QString &subFolder) const
{
    // make the subdir dir
    QDir dir(d->path);
    if (!d->isRoot) {
        dir.cdUp();
        if (dir.exists(d->subDirPath())) {
            dir.cd(d->subDirPath());
        }
    }
    return Maildir(dir.path() + QLatin1Char('/') + subFolder);
}

Maildir Maildir::parent() const
{
    if (!isValid() || d->isRoot) {
        return Maildir();
    }
    QDir dir(d->path);
    dir.cdUp();
    if (!dir.dirName().startsWith(QLatin1Char('.')) || !dir.dirName().endsWith(QLatin1String(".directory"))) {
        return Maildir();
    }
    const QString parentName = dir.dirName().mid(1, dir.dirName().size() - 11);
    dir.cdUp();
    dir.cd(parentName);
    return Maildir(dir.path());
}

QStringList Maildir::entryList() const
{
    QStringList result;
    if (isValid()) {
        result = d->listNew() + d->listCurrent();
    }
    //  qCDebug(LIBMAILDIR_LOG) <<"Maildir::entryList()" << result;
    return result;
}

QStringList Maildir::listCurrent() const
{
    QStringList result;
    if (isValid()) {
        result = d->listCurrent();
    }
    return result;
}

QString Maildir::findRealKey(const QString &key) const
{
    return d->findRealKey(key);
}

QStringList Maildir::listNew() const
{
    QStringList result;
    if (isValid()) {
        result = d->listNew();
    }
    return result;
}

QString Maildir::pathToNew() const
{
    if (isValid()) {
        return d->path + QLatin1String("/new");
    }
    return {};
}

QString Maildir::pathToCurrent() const
{
    if (isValid()) {
        return d->path + QLatin1String("/cur");
    }
    return {};
}

QString Maildir::subDirPath() const
{
    QDir dir(d->path);
    dir.cdUp();
    return dir.path() + QLatin1Char('/') + d->subDirPath();
}

QStringList Maildir::subFolderList() const
{
    QDir dir(d->path);

    // the root maildir has its subfolders directly beneath it
    if (!d->isRoot) {
        dir.cdUp();
        if (!dir.exists(d->subDirPath())) {
            return {};
        }
        dir.cd(d->subDirPath());
    }
    dir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
    QStringList entries = dir.entryList();
    entries.removeAll(QStringLiteral("cur"));
    entries.removeAll(QStringLiteral("new"));
    entries.removeAll(QStringLiteral("tmp"));
    return entries;
}

QByteArray Maildir::readEntry(const QString &key) const
{
    QByteArray result;

    QString realKey(d->findRealKey(key));
    if (realKey.isEmpty()) {
        // FIXME error handling?
        qCWarning(LIBMAILDIR_LOG) << "Maildir::readEntry unable to find: " << key;
        d->lastError = i18n("Cannot locate mail file %1.", key);
        return result;
    }

    QFile f(realKey);
    if (!f.open(QIODevice::ReadOnly)) {
        d->lastError = i18n("Cannot open mail file %1.", realKey);
        return result;
    }

    // FIXME be safer than this
    result = f.readAll();

    return result;
}

qint64 Maildir::size(const QString &key) const
{
    QString realKey(d->findRealKey(key));
    if (realKey.isEmpty()) {
        // FIXME error handling?
        qCWarning(LIBMAILDIR_LOG) << "Maildir::size unable to find: " << key;
        d->lastError = i18n("Cannot locate mail file %1.", key);
        return -1;
    }

    const QFileInfo info(realKey);
    if (!info.exists()) {
        d->lastError = i18n("Cannot open mail file %1.", realKey);
        return -1;
    }

    return info.size();
}

QDateTime Maildir::lastModified(const QString &key) const
{
    const QString realKey(d->findRealKey(key));
    if (realKey.isEmpty()) {
        qCWarning(LIBMAILDIR_LOG) << "Maildir::lastModified unable to find: " << key;
        d->lastError = i18n("Cannot locate mail file %1.", key);
        return {};
    }

    const QFileInfo info(realKey);
    if (!info.exists()) {
        return {};
    }

    return info.lastModified();
}

QByteArray Maildir::readEntryHeadersFromFile(const QString &file) const
{
    QByteArray result;

    QFile f(file);
    if (!f.open(QIODevice::ReadOnly)) {
        // FIXME error handling?
        qCWarning(LIBMAILDIR_LOG) << "Maildir::readEntryHeaders unable to find: " << file;
        d->lastError = i18n("Cannot locate mail file %1.", file);
        return result;
    }
    f.map(0, qMin((qint64)8000, f.size()));
    for (;;) {
        QByteArray line = f.readLine();
        if (line.isEmpty() || line.startsWith('\n')) {
            break;
        }
        result.append(line);
    }
    return result;
}

QByteArray Maildir::readEntryHeaders(const QString &key) const
{
    const QString realKey(d->findRealKey(key));
    if (realKey.isEmpty()) {
        qCWarning(LIBMAILDIR_LOG) << "Maildir::readEntryHeaders unable to find: " << key;
        d->lastError = i18n("Cannot locate mail file %1.", key);
        return {};
    }

    return readEntryHeadersFromFile(realKey);
}

static QString createUniqueFileName()
{
    const qint64 time = QDateTime::currentMSecsSinceEpoch();
    const int r = QRandomGenerator::global()->bounded(1000);
    const QString identifier = QLatin1String("R") + QString::number(r);

    QString fileName = QString::number(time) + QLatin1Char('.') + identifier + QLatin1Char('.');

    return fileName;
}

bool Maildir::writeEntry(const QString &key, const QByteArray &data)
{
    QString realKey(d->findRealKey(key));
    if (realKey.isEmpty()) {
        // FIXME error handling?
        qCWarning(LIBMAILDIR_LOG) << "Maildir::writeEntry unable to find: " << key;
        d->lastError = i18n("Cannot locate mail file %1.", key);
        return false;
    }
    QFile f(realKey);
    bool result = f.open(QIODevice::WriteOnly);
    result = result & (f.write(data) != -1);
    f.close();
    if (!result) {
        d->lastError = i18n("Cannot write to mail file %1.", realKey);
        return false;
    }
    return true;
}

QString Maildir::addEntry(const QByteArray &data)
{
    QString uniqueKey;
    QString key;
    QString finalKey;
    QString curKey;

    // QUuid doesn't return globally unique identifiers, therefore we query until we
    // get one that doesn't exists yet
    do {
        uniqueKey = createUniqueFileName() + d->hostName;
        key = d->path + QLatin1String("/tmp/") + uniqueKey;
        finalKey = d->path + QLatin1String("/new/") + uniqueKey;
        curKey = d->path + QLatin1String("/cur/") + uniqueKey;
    } while (QFile::exists(key) || QFile::exists(finalKey) || QFile::exists(curKey));

    QFile f(key);
    bool result = f.open(QIODevice::WriteOnly);
    result = result & (f.write(data) != -1);
    f.close();
    if (!result) {
        d->lastError = i18n("Cannot write to mail file %1.", key);
        return {};
    }
    /*
     * FIXME:
     *
     * The whole point of the locking free maildir idea is that the moves between
     * the internal directories are atomic. Afaik QFile::rename does not guarantee
     * that, so this will need to be done properly. - ta
     *
     * For reference: http://trolltech.com/developer/task-tracker/index_html?method=entry&id=211215
     */
    if (!f.rename(finalKey)) {
        qCWarning(LIBMAILDIR_LOG) << "Maildir: Failed to add entry: " << finalKey << "! Error: " << f.errorString();
        d->lastError = i18n("Failed to create mail file %1. The error was: %2", finalKey, f.errorString());
        return {};
    }
    KeyCache *keyCache = KeyCache::self();
    keyCache->removeKey(d->path, key); // remove all keys, be it "cur" or "new" first
    keyCache->addNewKey(d->path, key); // and add a key for "new", as the mail was moved there
    return uniqueKey;
}

bool Maildir::removeEntry(const QString &key)
{
    QString realKey(d->findRealKey(key));
    if (realKey.isEmpty()) {
        qCWarning(LIBMAILDIR_LOG) << "Maildir::removeEntry unable to find: " << key;
        return false;
    }
    KeyCache *keyCache = KeyCache::self();
    keyCache->removeKey(d->path, key);
    return QFile::remove(realKey);
}

QString Maildir::changeEntryFlags(const QString &key, const Akonadi::Item::Flags &flags)
{
    QString realKey(d->findRealKey(key));
    if (realKey.isEmpty()) {
        qCWarning(LIBMAILDIR_LOG) << "Maildir::changeEntryFlags unable to find: " << key;
        d->lastError = i18n("Cannot locate mail file %1.", key);
        return {};
    }

    const QRegularExpression rx = statusSeparatorRx();
    QString finalKey = key.left(key.indexOf(rx));

    QStringList mailDirFlags;
    for (const Akonadi::Item::Flag &flag : flags) {
        if (flag == Akonadi::MessageFlags::Forwarded) {
            mailDirFlags << QStringLiteral("P");
        } else if (flag == Akonadi::MessageFlags::Replied) {
            mailDirFlags << QStringLiteral("R");
        } else if (flag == Akonadi::MessageFlags::Seen) {
            mailDirFlags << QStringLiteral("S");
        } else if (flag == Akonadi::MessageFlags::Deleted) {
            mailDirFlags << QStringLiteral("T");
        } else if (flag == Akonadi::MessageFlags::Flagged) {
            mailDirFlags << QStringLiteral("F");
        }
    }
    mailDirFlags.sort();
    if (!mailDirFlags.isEmpty()) {
#ifdef Q_OS_WIN
        finalKey.append(QLatin1String("!2,") + mailDirFlags.join(QString()));
#else
        finalKey.append(QLatin1String(":2,") + mailDirFlags.join(QString()));
#endif
    }

    const QString newUniqueKey = finalKey; // key without path
    finalKey.prepend(d->path + QLatin1String("/cur/"));

    if (realKey == finalKey) {
        // Somehow it already is named this way (e.g. migration bug -> wrong status in akonadi)
        return newUniqueKey;
    }

    QFile f(realKey);
    if (QFile::exists(finalKey)) {
        QFile destFile(finalKey);
        QByteArray destContent;
        if (destFile.open(QIODevice::ReadOnly)) {
            destContent = destFile.readAll();
            destFile.close();
        }
        QByteArray sourceContent;
        if (f.open(QIODevice::ReadOnly)) {
            sourceContent = f.readAll();
            f.close();
        }

        if (destContent != sourceContent) {
            QString newFinalKey = QLatin1String("1-") + newUniqueKey;
            int i = 1;
            const QString currentPath = d->path + QLatin1String("/cur/");
            while (QFile::exists(currentPath + newFinalKey)) {
                i++;
                newFinalKey = QString::number(i) + QLatin1Char('-') + newUniqueKey;
            }
            finalKey = currentPath + newFinalKey;
        } else {
            QFile::remove(finalKey); // they are the same
        }
    }

    if (!f.rename(finalKey)) {
        qCWarning(LIBMAILDIR_LOG) << "Maildir: Failed to rename entry: " << f.fileName() << " to " << finalKey << "! Error: " << f.errorString();
        d->lastError = i18n("Failed to update the file name %1 to %2 on the disk. The error was: %3.", f.fileName(), finalKey, f.errorString());
        return {};
    }

    KeyCache *keyCache = KeyCache::self();
    keyCache->removeKey(d->path, key);
    keyCache->addCurKey(d->path, newUniqueKey);

    return newUniqueKey;
}

Akonadi::Item::Flags Maildir::readEntryFlags(const QString &key) const
{
    Akonadi::Item::Flags flags;

    const QRegularExpression rx = statusSeparatorRx();
    const int index = key.indexOf(rx);
    if (index != -1) {
        const QStringView mailDirFlags = QStringView(key).mid(index + 3); // after "(:|!)2,"
        const int flagSize(mailDirFlags.size());
        for (int i = 0; i < flagSize; ++i) {
            const QChar flag = mailDirFlags.at(i);
            if (flag == QLatin1Char('P')) {
                flags << Akonadi::MessageFlags::Forwarded;
            } else if (flag == QLatin1Char('R')) {
                flags << Akonadi::MessageFlags::Replied;
            } else if (flag == QLatin1Char('S')) {
                flags << Akonadi::MessageFlags::Seen;
            } else if (flag == QLatin1Char('F')) {
                flags << Akonadi::MessageFlags::Flagged;
            }
        }
    }

    return flags;
}

bool Maildir::moveTo(const Maildir &newParent)
{
    if (d->isRoot) {
        return false; // not supported
    }

    QDir newDir(newParent.path());
    if (!newParent.d->isRoot) {
        newDir.cdUp();
        if (!newDir.exists(newParent.d->subDirPath())) {
            newDir.mkdir(newParent.d->subDirPath());
        }
        newDir.cd(newParent.d->subDirPath());
    }

    QDir currentDir(d->path);
    currentDir.cdUp();

    if (newDir == currentDir) {
        return true;
    }

    return d->moveAndRename(newDir, name());
}

bool Maildir::rename(const QString &newName)
{
    if (name() == newName) {
        return true;
    }
    if (d->isRoot) {
        return false; // not (yet) supported
    }

    QDir dir(d->path);
    dir.cdUp();

    return d->moveAndRename(dir, newName);
}

QString Maildir::moveEntryTo(const QString &key, const Maildir &destination)
{
    const QString realKey(d->findRealKey(key));
    if (realKey.isEmpty()) {
        qCWarning(LIBMAILDIR_LOG) << "Unable to find: " << key;
        d->lastError = i18n("Cannot locate mail file %1.", key);
        return {};
    }
    QFile f(realKey);
    // ### is this safe regarding the maildir locking scheme?
    const QString targetKey = destination.path() + QStringLiteral("/new/") + key;
    if (!f.rename(targetKey)) {
        qCDebug(LIBMAILDIR_LOG) << "Failed to rename" << realKey << "to" << targetKey << "! Error: " << f.errorString();
        d->lastError = f.errorString();
        return {};
    }

    KeyCache *keyCache = KeyCache::self();

    keyCache->addNewKey(destination.path(), key);
    keyCache->removeKey(d->path, key);

    return key;
}

QString Maildir::subDirPathForFolderPath(const QString &folderPath)
{
    QDir dir(folderPath);
    const QString dirName = dir.dirName();
    dir.cdUp();
    return QFileInfo(dir, Private::subDirNameForFolderName(dirName)).filePath();
}

QString Maildir::subDirNameForFolderName(const QString &folderName)
{
    return Private::subDirNameForFolderName(folderName);
}

void Maildir::removeCachedKeys(const QStringList &keys)
{
    KeyCache *keyCache = KeyCache::self();
    for (const QString &key : keys) {
        keyCache->removeKey(d->path, key);
    }
}

void Maildir::refreshKeyCache()
{
    KeyCache::self()->refreshKeys(d->path);
}

QString Maildir::lastError() const
{
    return d->lastError;
}
