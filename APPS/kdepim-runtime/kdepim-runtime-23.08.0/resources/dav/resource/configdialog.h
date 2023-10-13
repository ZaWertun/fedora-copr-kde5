/*
    SPDX-FileCopyrightText: 2009 Grégory Oestreicher <greg@kamago.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "ui_configdialog.h"

#include <KDAV/Enums>

#include <QDialog>

#include <QList>
#include <QPair>
#include <QString>
class QPushButton;
class KConfigDialogManager;
class QStandardItemModel;

class ConfigDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConfigDialog(QWidget *parent = nullptr);
    ~ConfigDialog() override;

    void setPassword(const QString &password);

private Q_SLOTS:
    void onSyncRangeStartTypeChanged();
    void checkUserInput();
    void onAddButtonClicked();
    void onSearchButtonClicked();
    void onRemoveButtonClicked();
    void onEditButtonClicked();
    void checkConfiguredUrlsButtonsState();
    void onOkClicked();
    void onCancelClicked();

private:
    void readConfig();
    void writeConfig();
    void addModelRow(const QString &protocol, const QString &url);
    void insertModelRow(int index, const QString &protocol, const QString &url);

    Ui::ConfigDialog mUi;
    KConfigDialogManager *mManager = nullptr;
    QList<QPair<QString, KDAV::Protocol>> mAddedUrls;
    QList<QPair<QString, KDAV::Protocol>> mRemovedUrls;
    QStandardItemModel *const mModel;
    QPushButton *mOkButton = nullptr;
};
