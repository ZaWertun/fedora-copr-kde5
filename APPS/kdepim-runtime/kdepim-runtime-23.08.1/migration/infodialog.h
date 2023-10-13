/*
    SPDX-FileCopyrightText: 2008 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "kmigratorbase.h"
#include "migratorbase.h"
#include <QDialog>
#include <QEventLoopLocker>

class QLabel;
class QListWidget;
class QProgressBar;
class QDialogButtonBox;

class InfoDialog : public QDialog
{
    Q_OBJECT
public:
    InfoDialog(bool closeWhenDone = true);
    ~InfoDialog() override;

    static bool hasError()
    {
        return mError;
    }

public Q_SLOTS:
    void message(KMigratorBase::MessageType type, const QString &msg);
    void message(MigratorBase::MessageType type, const QString &msg);

    void migratorAdded();
    void migratorDone();

    void status(const QString &msg);

    void progress(int value);
    void progress(int min, int max, int value);

private:
    bool hasChange() const
    {
        return mChange;
    }

    void scrollBarMoved(int value);
    QEventLoopLocker eventLoopLocker;
    QDialogButtonBox *const mButtonBox;
    QListWidget *const mList;
    QLabel *const mStatusLabel;
    QProgressBar *const mProgressBar;
    int mMigratorCount = 0;
    static bool mError;
    bool mChange = false;
    const bool mCloseWhenDone;
    bool mAutoScrollList = true;
};
