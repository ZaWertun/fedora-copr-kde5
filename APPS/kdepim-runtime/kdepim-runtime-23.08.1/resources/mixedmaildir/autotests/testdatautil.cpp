/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
    SPDX-FileContributor: Kevin Krammer <krake@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "testdatautil.h"

#include "mixedmaildirresource_debug.h"

#include <QDir>
#include <QFile>

using namespace TestDataUtil;

// use this instead of QFile::copy() because we want overwrite in case it exists
static bool copyFile(const QString &sourceFileName, const QString &targetFileName)
{
    QFile sourceFile(sourceFileName);
    QFile targetFile(targetFileName);

    if (!sourceFile.open(QIODevice::ReadOnly)) {
        qCritical() << "Cannot open source file" << sourceFileName;
        return false;
    }

    if (!targetFile.open(QIODevice::WriteOnly)) {
        qCritical() << "Cannot open target file" << targetFileName;
        return false;
    }

    return targetFile.write(sourceFile.readAll()) != -1;
}

static bool copyFiles(const QDir &sourceDir, const QDir &targetDir)
{
    const QStringList files = sourceDir.entryList(QStringList(), QDir::Files);
    for (const QString &file : files) {
        const QFileInfo sourceFileInfo(sourceDir, file);
        const QFileInfo targetFileInfo(targetDir, file);
        if (!copyFile(sourceFileInfo.absoluteFilePath(), targetFileInfo.absoluteFilePath())) {
            qCritical() << "Failed to copy" << sourceFileInfo.absoluteFilePath() << "to" << targetFileInfo.absoluteFilePath();
            return false;
        }
    }

    return true;
}

FolderType TestDataUtil::folderType(const QString &testDataName)
{
    const QDir dir(QStringLiteral(":/data"));
    const QString indexFilePattern = QStringLiteral(".%1.index");

    if (!dir.exists(testDataName) || !dir.exists(indexFilePattern.arg(testDataName))) {
        return InvalidFolder;
    }

    const QFileInfo fileInfo(dir, testDataName);
    return fileInfo.isDir() ? MaildirFolder : MBoxFolder;
}

QStringList TestDataUtil::testDataNames()
{
    const QDir dir(QStringLiteral(":/data"));
    const QFileInfoList dirEntries = dir.entryInfoList();

    const QString indexFilePattern = QStringLiteral(".%1.index");

    QStringList result;
    for (const QFileInfo &fileInfo : dirEntries) {
        if (dir.exists(indexFilePattern.arg(fileInfo.fileName()))) {
            result << fileInfo.fileName();
        }
    }

    result.sort();
    return result;
}

bool TestDataUtil::installFolder(const QString &testDataName, const QString &installPath, const QString &folderName)
{
    const FolderType type = TestDataUtil::folderType(testDataName);
    if (type == InvalidFolder) {
        qCritical() << "testDataName" << testDataName << "is not a valid mail folder type";
        return false;
    }

    if (!QDir::current().mkpath(installPath)) {
        qCritical() << "Couldn't create installPath" << installPath;
        return false;
    }

    const QDir installDir(installPath);
    const QFileInfo installFileInfo(installDir, folderName);
    if (installDir.exists(folderName)) {
        switch (type) {
        case MaildirFolder:
            if (!installFileInfo.isDir()) {
                qCritical() << "Target file name" << folderName << "already exists but is not a directory";
                return false;
            }
            break;

        case MBoxFolder:
            if (!installFileInfo.isFile()) {
                qCritical() << "Target file name" << folderName << "already exists but is not a directory";
                return false;
            }
            break;

        default:
            // already handled at beginning
            Q_ASSERT(false);
            return false;
        }
    }

    const QDir testDataDir(QStringLiteral(":/data"));

    switch (type) {
    case MaildirFolder: {
        const QString subPathPattern = QStringLiteral("%1/%2");
        if (!installDir.mkpath(subPathPattern.arg(folderName, QStringLiteral("new")))
            || !installDir.mkpath(subPathPattern.arg(folderName, QStringLiteral("cur")))
            || !installDir.mkpath(subPathPattern.arg(folderName, QStringLiteral("tmp")))) {
            qCritical() << "Couldn't create maildir directory structure";
            return false;
        }

        QDir sourceDir = testDataDir;
        QDir targetDir = installDir;

        sourceDir.cd(testDataName);
        targetDir.cd(folderName);

        if (sourceDir.cd(QStringLiteral("new"))) {
            targetDir.cd(QStringLiteral("new"));
            if (!copyFiles(sourceDir, targetDir)) {
                return false;
            }
            sourceDir.cdUp();
            targetDir.cdUp();
        }

        if (sourceDir.cd(QStringLiteral("cur"))) {
            targetDir.cd(QStringLiteral("cur"));
            if (!copyFiles(sourceDir, targetDir)) {
                return false;
            }
            sourceDir.cdUp();
            targetDir.cdUp();
        }

        if (sourceDir.cd(QStringLiteral("tmp"))) {
            targetDir.cd(QStringLiteral("tmp"));
            if (!copyFiles(sourceDir, targetDir)) {
                return false;
            }
        }
        break;
    }

    case MBoxFolder: {
        const QFileInfo mboxFileInfo(testDataDir, testDataName);
        if (!copyFile(mboxFileInfo.absoluteFilePath(), installFileInfo.absoluteFilePath())) {
            qCritical() << "Failed to copy" << mboxFileInfo.absoluteFilePath() << "to" << installFileInfo.absoluteFilePath();
            return false;
        }
        break;
    }

    default:
        // already handled at beginning
        Q_ASSERT(false);
        return false;
    }

    const QString indexFilePattern = QStringLiteral(".%1.index");
    const QFileInfo indexFileInfo(testDataDir, indexFilePattern.arg(testDataName));
    const QFileInfo indexInstallFileInfo(installDir, indexFilePattern.arg(folderName));

    return copyFile(indexFileInfo.absoluteFilePath(), indexInstallFileInfo.absoluteFilePath());
}
