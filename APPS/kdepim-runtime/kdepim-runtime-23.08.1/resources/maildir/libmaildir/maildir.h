/*
    SPDX-FileCopyrightText: 2007 Till Adam <adam@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "maildir_export.h"

#include <Akonadi/Item>
#include <QString>
#include <QStringList>

class QDateTime;

namespace KPIM
{
class MAILDIR_EXPORT Maildir
{
public:
    /**
      Create a new Maildir object.
      @param path The path to the maildir, if @p isRoot is @c false, that's the path
      to the folder containing the cur/new/tmp folders, if @p isRoot is @c true this
      is the path to a folder containing a number of maildirs.
      @param isRoot Indicate whether this is a maildir containing mails and various
      sub-folders or a container only containing maildirs.
    */
    explicit Maildir(const QString &path = QString(), bool isRoot = false);
    /* Copy constructor */
    Maildir(const Maildir &rhs);
    /* Copy operator */
    Maildir &operator=(const Maildir &rhs);
    /** Equality comparison */
    bool operator==(const Maildir &rhs) const;
    /* Destructor */
    ~Maildir();

    /** Returns whether the maildir has all the necessary subdirectories,
     * that they are readable, etc.
     * @param createMissingFolders if true (the default), the cur/new/tmp folders are created if they are missing
     */
    bool isValid(bool createMissingFolders = true) const;

    /**
     * Returns whether this is a normal maildir or a container containing maildirs.
     */
    bool isRoot() const;

    /**
     * Make a valid maildir at the path of this Maildir object. This involves
     * creating the necessary subdirs, etc. Note that an empty Maildir is
     * not valid, unless it is given  valid path, or until create( ) is
     * called on it.
     */
    bool create();

    /**
     * Returns the path of this maildir.
     */
    QString path() const;

    /**
     * Returns the name of this maildir.
     */
    QString name() const;

    /**
     * Returns the list of items (mails) in the maildir. These are keys, which
     * map to filenames, internally, but that's an implementation detail, which
     * should not be relied on.
     */
    QStringList entryList() const;

    /** Returns the list of items (mails) in the maildirs "new" folder. These are keys, which
     * map to filenames, internally, but that's an implementation detail, which
     * should not be relied on.
     */
    QStringList listNew() const;

    /** Returns the list of items (mails) in the maildirs "cur" folder. These are keys, which
     * map to filenames, internally, but that's an implementation detail, which
     * should not be relied on.
     */
    QStringList listCurrent() const;

    /** Return the path to the "new" directory */
    QString pathToNew() const;

    /** Return the path to the "cur" directory */
    QString pathToCurrent() const;

    /**
     * Returns the full path to the subdir (the NAME.directory folder ).
     **/
    QString subDirPath() const;

    /**
     * Return the full path to the file identified by key (it can be either in the "new" or "cur" folder
     **/
    QString findRealKey(const QString &key) const;

    /**
     * Returns the list of subfolders, as names (relative paths). Use the
     * subFolder method to get Maildir objects representing them.
     */
    QStringList subFolderList() const;

    /**
     * Adds subfolder with the given @p folderName.
     * @return an empty string on failure or the full path of the new subfolder
     *         on success
     */
    QString addSubFolder(const QString &folderName);

    /**
     * Removes subfolder with the given @p folderName. Returns success or failure.
     */
    bool removeSubFolder(const QString &folderName);

    /**
     * Returns a Maildir object for the given @p folderName. If such a folder
     * exists, the Maildir object will be valid, otherwise you can call create()
     * on it, to make a subfolder with that name.
     */
    Maildir subFolder(const QString &folderName) const;

    /**
     * Returns the parent Maildir object for this Maildir, if there is one (ie. this is not the root).
     */
    Maildir parent() const;

    /**
     * Returns the size of the file in the maildir with the given @p key or \c -1 if key is not valid.
     * @since 4.2
     */
    qint64 size(const QString &key) const;

    /**
     * Returns the modification time of the file in the maildir with the given @p key.
     * @since 4.7
     */
    QDateTime lastModified(const QString &key) const;

    /**
     * Return the contents of the file in the maildir with the given @p key.
     */
    QByteArray readEntry(const QString &key) const;

    /**
     * Return the flags encoded in the maildir file name for an entry
     **/
    Akonadi::Item::Flags readEntryFlags(const QString &key) const;

    /**
     * Return the contents of the headers section of the file the maildir with the given @p file, that
     * is a full path to the file. You can get it by using findRealKey(key) .
     */
    QByteArray readEntryHeadersFromFile(const QString &file) const;

    /**
     * Return the contents of the headers section of the file the maildir with the given @p key.
     */
    QByteArray readEntryHeaders(const QString &key) const;

    /**
     * Write the given @p data to a file in the maildir with the given  @p key.
     * Returns true in case of success, false in case of any error.
     */
    bool writeEntry(const QString &key, const QByteArray &data);

    /**
     * Adds the given @p data to the maildir. Returns the key of the entry.
     */
    QString addEntry(const QByteArray &data);

    /**
     * Removes the entry with the given @p key. Returns success or failure.
     */
    bool removeEntry(const QString &key);

    /**
     * Change the flags for an entry specified by @p key. Returns the new key of the entry (the key might change because
     * flags are stored in the unique filename).
     */
    QString changeEntryFlags(const QString &key, const Akonadi::Item::Flags &flags);

    /**
     * Moves this maildir into @p destination.
     */
    bool moveTo(const Maildir &destination);

    /**
     * Renames this maildir to @p newName.
     */
    bool rename(const QString &newName);

    /**
     * Moves the file with the given @p key into the Maildir @p destination.
     * @returns The new file name inside @p destination.
     */
    QString moveEntryTo(const QString &key, const KPIM::Maildir &destination);

    /**
     * Creates the maildir tree structure specific directory path that the
     * given @p folderPath folder would have for its sub folders
     * @param folderPath a maildir folder path
     * @return the relative subDirPath for the given @p folderPath
     *
     * @see subDirNameForFolderName()
     */
    static QString subDirPathForFolderPath(const QString &folderPath);

    /**
     * Creates the maildir tree structure specific directory name that the
     * given @p folderName folder would have for its sub folders
     * @param folderName a maildir folder name
     * @return the relative subDirName for the given @p folderMame
     *
     * @see subDirPathForFolderPath()
     */
    static QString subDirNameForFolderName(const QString &folderName);

    /** Removes the listed keys from the key cache */
    void removeCachedKeys(const QStringList &keys);

    /** Reloads the keys associated with the maildir in the key cache*/
    void refreshKeyCache();

    /** Return the last error message string. The error might not come from the last performed operation,
     if that was successful. The caller should always check the return value of the methods before
     querying the last error string. */
    QString lastError() const;

private:
    void swap(const Maildir &);
    class Private;
    Private *d;
};
}
