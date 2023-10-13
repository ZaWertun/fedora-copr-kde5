/*
   SPDX-FileCopyrightText: 2013-2023 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <Akonadi/Collection>
#include <KConfigGroup>

class FolderArchiveAccountInfo
{
public:
    FolderArchiveAccountInfo();
    explicit FolderArchiveAccountInfo(const KConfigGroup &config);
    ~FolderArchiveAccountInfo();

    enum FolderArchiveType {
        UniqueFolder,
        FolderByMonths,
        FolderByYears,
    };

    Q_REQUIRED_RESULT bool isValid() const;

    Q_REQUIRED_RESULT QString instanceName() const;
    void setInstanceName(const QString &instance);

    void setArchiveTopLevel(Akonadi::Collection::Id id);
    Q_REQUIRED_RESULT Akonadi::Collection::Id archiveTopLevel() const;

    void setFolderArchiveType(FolderArchiveType type);
    Q_REQUIRED_RESULT FolderArchiveType folderArchiveType() const;

    void setEnabled(bool enabled);
    Q_REQUIRED_RESULT bool enabled() const;

    void setKeepExistingStructure(bool b);
    Q_REQUIRED_RESULT bool keepExistingStructure() const;

    void writeConfig(KConfigGroup &config);
    void readConfig(const KConfigGroup &config);

    Q_REQUIRED_RESULT bool operator==(const FolderArchiveAccountInfo &other) const;

private:
    FolderArchiveAccountInfo::FolderArchiveType mArchiveType = UniqueFolder;
    Akonadi::Collection::Id mArchiveTopLevelCollectionId = -1;
    QString mInstanceName;
    bool mEnabled = false;
    bool mKeepExistingStructure = false;
};
