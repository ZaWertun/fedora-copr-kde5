/*
    SPDX-FileCopyrightText: 2010 Grégory Oestreicher <greg@kamago.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "ui_urlconfigurationdialog.h"

#include <KDAV/Enums>

#include <QDialog>
#include <QString>

class KJob;
class QButtonGroup;
class QModelIndex;
class QStandardItemModel;
class QPushButton;
class UrlConfigurationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UrlConfigurationDialog(QWidget *parent = nullptr);
    ~UrlConfigurationDialog() override;

    KDAV::Protocol protocol() const;
    void setProtocol(KDAV::Protocol protocol);

    QString remoteUrl() const;
    void setRemoteUrl(const QString &url);

    bool useDefaultCredentials() const;
    void setUseDefaultCredentials(bool defaultCreds);

    QString username() const;
    void setDefaultUsername(const QString &name);
    void setUsername(const QString &name);

    QString password() const;
    void setDefaultPassword(const QString &password);
    void setPassword(const QString &password);

private:
    void onConfigChanged();
    void checkUserInput();
    void onFetchButtonClicked();
    void onOkButtonClicked();
    void onCollectionsFetchDone(KJob *job);
    void onModelDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    void onChangeDisplayNameFinished(KJob *job);
    void initModel();
    bool checkUserAuthInput();
    void addModelRow(const QString &displayName, const QString &url);
    void writeConfig();
    void readConfig();

    Ui::UrlConfigurationDialog mUi;
    QStandardItemModel *const mModel;
    QString mDefaultUsername;
    QString mDefaultPassword;
    QPushButton *mOkButton = nullptr;
    QButtonGroup *const mRemoteProtocolGroup;
};
