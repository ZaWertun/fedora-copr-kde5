/*
    SPDX-FileCopyrightText: 2013-2023 Laurent Montel <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <Akonadi/Collection>
#include <QIdentityProxyModel>
#include <QModelIndex>
#include <QWidget>
class QSortFilterProxyModel;
namespace Akonadi
{
class EntityTreeModel;
class ChangeRecorder;
}
class QTreeView;
class KJob;

class NewMailNotifierCollectionProxyModel : public QIdentityProxyModel
{
    Q_OBJECT
public:
    explicit NewMailNotifierCollectionProxyModel(QObject *parent = nullptr);

    QVariant data(const QModelIndex &index, int role) const override;

    bool setData(const QModelIndex &index, const QVariant &_data, int role) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    QHash<Akonadi::Collection, bool> notificationCollection() const;

private:
    QHash<Akonadi::Collection, bool> mNotificationCollection;
};

class NewMailNotifierSelectCollectionWidget : public QWidget
{
    Q_OBJECT
public:
    explicit NewMailNotifierSelectCollectionWidget(QWidget *parent = nullptr);
    ~NewMailNotifierSelectCollectionWidget() override;

    void updateCollectionsRecursive();

private:
    void slotSelectAllCollections();
    void slotUnselectAllCollections();
    void slotModifyJobDone(KJob *job);
    void slotSetCollectionFilter(const QString &);

    void slotCollectionTreeFetched();
    void forceStatus(const QModelIndex &parent, bool status);
    QTreeView *const mFolderView;
    Akonadi::EntityTreeModel *mModel = nullptr;
    Akonadi::ChangeRecorder *const mChangeRecorder;
    QSortFilterProxyModel *const mCollectionFilter;
    NewMailNotifierCollectionProxyModel *mNewMailNotifierProxyModel = nullptr;
};
