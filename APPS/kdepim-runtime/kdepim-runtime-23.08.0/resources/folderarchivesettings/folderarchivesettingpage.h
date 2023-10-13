/*
   SPDX-FileCopyrightText: 2013-2023 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "folderarchiveaccountinfo.h"
#include "folderarchivesettings_export.h"
#include <QComboBox>
#include <QWidget>

class QCheckBox;
namespace Akonadi
{
class CollectionRequester;
}

class FolderArchiveComboBox : public QComboBox
{
    Q_OBJECT
public:
    explicit FolderArchiveComboBox(QWidget *parent = nullptr);
    ~FolderArchiveComboBox() override;

    void setType(FolderArchiveAccountInfo::FolderArchiveType type);
    FolderArchiveAccountInfo::FolderArchiveType type() const;

private:
    void initialize();
};

class FolderArchiveAccountInfo;
class FOLDERARCHIVESETTINGS_EXPORT FolderArchiveSettingPage : public QWidget
{
    Q_OBJECT
public:
    explicit FolderArchiveSettingPage(const QString &instanceName, QWidget *parent = nullptr);
    ~FolderArchiveSettingPage() override;

    void loadSettings();
    void writeSettings();

private:
    void slotEnableChanged(bool enabled);
    const QString mInstanceName;
    QCheckBox *const mEnabled;
    FolderArchiveComboBox *mArchiveNamed = nullptr;
    Akonadi::CollectionRequester *const mArchiveFolder;
    FolderArchiveAccountInfo *mInfo = nullptr;
};
