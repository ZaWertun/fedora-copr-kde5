/*
    SPDX-FileCopyrightText: 2013-2023 Laurent Montel <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "newmailnotifierselectcollectionwidget.h"

#include <Akonadi/AttributeFactory>
#include <Akonadi/CollectionFilterProxyModel>
#include <Akonadi/CollectionModifyJob>
#include <Akonadi/NewMailNotifierAttribute>
#include <QSortFilterProxyModel>

#include <Akonadi/ChangeRecorder>
#include <Akonadi/EntityTreeModel>
#include <KMime/Message>

#include "newmailnotifier_debug.h"
#include <KLineEdit>
#include <KLocalizedString>
#include <QPushButton>

#include <QHBoxLayout>
#include <QLabel>
#include <QTreeView>
#include <QVBoxLayout>

NewMailNotifierCollectionProxyModel::NewMailNotifierCollectionProxyModel(QObject *parent)
    : QIdentityProxyModel(parent)
{
}

QVariant NewMailNotifierCollectionProxyModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::CheckStateRole) {
        if (index.isValid()) {
            const auto collection = data(index, Akonadi::EntityTreeModel::CollectionRole).value<Akonadi::Collection>();
            // Make top-level collections uncheckable
            if (collection.parentCollection() == Akonadi::Collection::root()) {
                return {};
            }
            if (mNotificationCollection.contains(collection)) {
                return mNotificationCollection.value(collection) ? Qt::Checked : Qt::Unchecked;
            } else {
                const auto attr = collection.attribute<Akonadi::NewMailNotifierAttribute>();
                if (!attr || !attr->ignoreNewMail()) {
                    return Qt::Checked;
                }
                return Qt::Unchecked;
            }
        }
    }
    return QIdentityProxyModel::data(index, role);
}

bool NewMailNotifierCollectionProxyModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role == Qt::CheckStateRole) {
        if (index.isValid()) {
            const auto collection = data(index, Akonadi::EntityTreeModel::CollectionRole).value<Akonadi::Collection>();
            mNotificationCollection[collection] = (value == Qt::Checked);
            Q_EMIT dataChanged(index, index);
            return true;
        }
    }

    return QIdentityProxyModel::setData(index, value, role);
}

Qt::ItemFlags NewMailNotifierCollectionProxyModel::flags(const QModelIndex &index) const
{
    if (index.isValid()) {
        return QIdentityProxyModel::flags(index) | Qt::ItemIsUserCheckable;
    } else {
        return QIdentityProxyModel::flags(index);
    }
}

QHash<Akonadi::Collection, bool> NewMailNotifierCollectionProxyModel::notificationCollection() const
{
    return mNotificationCollection;
}

NewMailNotifierSelectCollectionWidget::NewMailNotifierSelectCollectionWidget(QWidget *parent)
    : QWidget(parent)
    , mFolderView(new QTreeView(this))
    , mChangeRecorder(new Akonadi::ChangeRecorder(this))
    , mCollectionFilter(new QSortFilterProxyModel(this))
{
    Akonadi::AttributeFactory::registerAttribute<Akonadi::NewMailNotifierAttribute>();
    auto vbox = new QVBoxLayout(this);

    auto label = new QLabel(i18n("Select which folders to monitor for new message notifications:"));
    vbox->addWidget(label);

    // Create a new change recorder.
    mChangeRecorder->setMimeTypeMonitored(KMime::Message::mimeType());
    mChangeRecorder->fetchCollection(true);
    mChangeRecorder->setAllMonitored(true);

    mModel = new Akonadi::EntityTreeModel(mChangeRecorder, this);
    // Set the model to show only collections, not items.
    mModel->setItemPopulationStrategy(Akonadi::EntityTreeModel::NoItemPopulation);
    connect(mModel, &Akonadi::EntityTreeModel::collectionTreeFetched, this, &NewMailNotifierSelectCollectionWidget::slotCollectionTreeFetched);

    auto mimeTypeProxy = new Akonadi::CollectionFilterProxyModel(this);
    mimeTypeProxy->setExcludeVirtualCollections(true);
    mimeTypeProxy->setDynamicSortFilter(true);
    mimeTypeProxy->addMimeTypeFilters(QStringList() << KMime::Message::mimeType());
    mimeTypeProxy->setSourceModel(mModel);

    mNewMailNotifierProxyModel = new NewMailNotifierCollectionProxyModel(this);
    mNewMailNotifierProxyModel->setSourceModel(mimeTypeProxy);

    mCollectionFilter->setRecursiveFilteringEnabled(true);
    mCollectionFilter->setSourceModel(mNewMailNotifierProxyModel);
    mCollectionFilter->setDynamicSortFilter(true);
    mCollectionFilter->setFilterCaseSensitivity(Qt::CaseInsensitive);
    mCollectionFilter->setSortRole(Qt::DisplayRole);
    mCollectionFilter->setSortCaseSensitivity(Qt::CaseSensitive);
    mCollectionFilter->setSortLocaleAware(true);

    auto searchLine = new KLineEdit(this);
    searchLine->setPlaceholderText(i18n("Search..."));
    searchLine->setClearButtonEnabled(true);
    connect(searchLine, &QLineEdit::textChanged, this, &NewMailNotifierSelectCollectionWidget::slotSetCollectionFilter);

    vbox->addWidget(searchLine);

    mFolderView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    mFolderView->setAlternatingRowColors(true);
    vbox->addWidget(mFolderView);

    mFolderView->setModel(mCollectionFilter);

    auto hbox = new QHBoxLayout;
    vbox->addLayout(hbox);

    auto button = new QPushButton(i18n("&Select All"), this);
    connect(button, &QPushButton::clicked, this, &NewMailNotifierSelectCollectionWidget::slotSelectAllCollections);
    hbox->addWidget(button);

    button = new QPushButton(i18n("&Unselect All"), this);
    connect(button, &QPushButton::clicked, this, &NewMailNotifierSelectCollectionWidget::slotUnselectAllCollections);
    hbox->addWidget(button);
    hbox->addStretch(1);
}

NewMailNotifierSelectCollectionWidget::~NewMailNotifierSelectCollectionWidget() = default;

void NewMailNotifierSelectCollectionWidget::slotCollectionTreeFetched()
{
    mCollectionFilter->sort(0, Qt::AscendingOrder);
    mFolderView->expandAll();
}

void NewMailNotifierSelectCollectionWidget::slotSetCollectionFilter(const QString &filter)
{
    mCollectionFilter->setFilterWildcard(filter);
    mFolderView->expandAll();
}

void NewMailNotifierSelectCollectionWidget::slotSelectAllCollections()
{
    forceStatus(QModelIndex(), true);
}

void NewMailNotifierSelectCollectionWidget::slotUnselectAllCollections()
{
    forceStatus(QModelIndex(), false);
}

void NewMailNotifierSelectCollectionWidget::forceStatus(const QModelIndex &parent, bool status)
{
    const int nbCol = mNewMailNotifierProxyModel->rowCount(parent);
    for (int i = 0; i < nbCol; ++i) {
        const QModelIndex child = mNewMailNotifierProxyModel->index(i, 0, parent);
        mNewMailNotifierProxyModel->setData(child, status ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole);
        forceStatus(child, status);
    }
}

void NewMailNotifierSelectCollectionWidget::updateCollectionsRecursive()
{
    QHashIterator<Akonadi::Collection, bool> i(mNewMailNotifierProxyModel->notificationCollection());
    while (i.hasNext()) {
        i.next();
        Akonadi::Collection collection = i.key();
        auto attr = collection.attribute<Akonadi::NewMailNotifierAttribute>();
        Akonadi::CollectionModifyJob *modifyJob = nullptr;
        const bool selected = i.value();
        if (selected && attr && attr->ignoreNewMail()) {
            collection.removeAttribute<Akonadi::NewMailNotifierAttribute>();
            modifyJob = new Akonadi::CollectionModifyJob(collection);
            modifyJob->setProperty("AttributeAdded", true);
        } else if (!selected && (!attr || !attr->ignoreNewMail())) {
            attr = collection.attribute<Akonadi::NewMailNotifierAttribute>(Akonadi::Collection::AddIfMissing);
            attr->setIgnoreNewMail(true);
            modifyJob = new Akonadi::CollectionModifyJob(collection);
            modifyJob->setProperty("AttributeAdded", false);
        }

        if (modifyJob) {
            connect(modifyJob, &Akonadi::CollectionModifyJob::finished, this, &NewMailNotifierSelectCollectionWidget::slotModifyJobDone);
        }
    }
}

void NewMailNotifierSelectCollectionWidget::slotModifyJobDone(KJob *job)
{
    auto modifyJob = qobject_cast<Akonadi::CollectionModifyJob *>(job);
    if (modifyJob && job->error()) {
        if (job->property("AttributeAdded").toBool()) {
            qCWarning(NEWMAILNOTIFIER_LOG) << "Failed to append NewMailNotifierAttribute to collection" << modifyJob->collection().id() << ":"
                                           << job->errorString();
        } else {
            qCWarning(NEWMAILNOTIFIER_LOG) << "Failed to remove NewMailNotifierAttribute from collection" << modifyJob->collection().id() << ":"
                                           << job->errorString();
        }
    }
}
