/*
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
    SPDX-FileContributor: Tobias Koenig <tokoe@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "resourcetask.h"

class RemoveCollectionRecursiveTask : public ResourceTask
{
    Q_OBJECT

public:
    explicit RemoveCollectionRecursiveTask(const ResourceStateInterface::Ptr &resource, QObject *parent = nullptr);
    ~RemoveCollectionRecursiveTask() override;

private Q_SLOTS:
    void onMailBoxesReceived(const QList<KIMAP::MailBoxDescriptor> &descriptors, const QList<QList<QByteArray>> &flags);
    void onCloseJobDone(KJob *job);
    void onDeleteJobDone(KJob *job);
    void onJobDone(KJob *job);

protected:
    void doStart(KIMAP::Session *session) override;

private:
    void deleteNextMailbox();

    KIMAP::Session *mSession = nullptr;
    bool mFolderFound = false;

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QScopedPointer<QMapIterator<int, KIMAP::MailBoxDescriptor>> mFolderIterator;
#else
    QScopedPointer<QMultiMapIterator<int, KIMAP::MailBoxDescriptor>> mFolderIterator;
#endif
};
