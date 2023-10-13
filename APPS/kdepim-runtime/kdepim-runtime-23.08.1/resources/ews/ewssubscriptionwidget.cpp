/*
    SPDX-FileCopyrightText: 2015-2017 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ewssubscriptionwidget.h"

#include <QCheckBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLineEdit>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QTreeView>
#include <QVBoxLayout>

#include "ewsclient.h"
#include "ewsfindfolderrequest.h"
#include "ewssettings.h"
#include "ewssubscribedfoldersjob.h"
#include <KLocalizedString>
#include <KMessageWidget>

class EwsSubscriptionFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit EwsSubscriptionFilterModel(QObject *parent = nullptr);
    ~EwsSubscriptionFilterModel() override;
public Q_SLOTS:
    void setFilterSelected(bool enabled);

protected:
    Q_REQUIRED_RESULT bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

private:
    Q_REQUIRED_RESULT bool hasCheckedChildren(const QModelIndex &index) const;
    bool mFilterSelected;
};

class EwsSubscriptionWidgetPrivate : public QObject
{
    Q_OBJECT
public:
    EwsSubscriptionWidgetPrivate(EwsClient &client, EwsSettings *settings, QObject *parent);
    ~EwsSubscriptionWidgetPrivate() override;

    enum TreeModelRoles {
        ItemIdRole = Qt::UserRole + 1,
    };

    void populateFolderTree();

public Q_SLOTS:
    void enableCheckBoxToggled(bool checked);
    void reloadFolderList(bool);
    void resetSelection(bool);
    void readFolderListFinished(KJob *job);
    void subscribedFoldersJobFinished(KJob *job);
    void treeItemChanged(QStandardItem *item);
    void filterTextChanged(const QString &text);

public:
    bool mEnabled = true;
    QCheckBox *mEnableCheckBox = nullptr;
    QTreeView *mFolderTreeView = nullptr;
    QWidget *mSubContainer = nullptr;
    QPushButton *mRefreshButton = nullptr;
    EwsClient &mClient;
    KMessageWidget *mMsgWidget = nullptr;
    QStandardItemModel *mFolderTreeModel = nullptr;
    QHash<QString, QStandardItem *> mFolderItemHash;
    int mFolderListPendingRequests = 0;
    EwsFolder::List mFolders;
    EwsId::List mSubscribedIds;
    EwsId::List mOrigSubscribedIds;
    bool mSubscribedIdsRetrieved = true;
    EwsSubscriptionFilterModel *mFilterModel = nullptr;
    EwsSettings *mSettings = nullptr;

    EwsSubscriptionWidget *q_ptr = nullptr;
    Q_DECLARE_PUBLIC(EwsSubscriptionWidget)
};

EwsSubscriptionFilterModel::EwsSubscriptionFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent)
    , mFilterSelected(false)
{
    setRecursiveFilteringEnabled(true);
}

EwsSubscriptionFilterModel::~EwsSubscriptionFilterModel() = default;

bool EwsSubscriptionFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    bool show = true;
    if (mFilterSelected) {
        QModelIndex sourceIndex = sourceModel()->index(sourceRow, 0, sourceParent);

        show = sourceIndex.data(Qt::CheckStateRole).toInt() == Qt::Checked;
        show |= hasCheckedChildren(sourceIndex);
    }

    if (!show) {
        return false;
    } else {
        return QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent);
    }
}

bool EwsSubscriptionFilterModel::hasCheckedChildren(const QModelIndex &index) const
{
    QModelIndex child;
    int row = 0;
    child = sourceModel()->index(row, 0, index);
    while (child.isValid()) {
        if (child.data(Qt::CheckStateRole).toInt() == Qt::Checked) {
            return true;
        } else if (hasCheckedChildren(child)) {
            return true;
        }
        child = sourceModel()->index(++row, 0, index);
    }

    return false;
}

void EwsSubscriptionFilterModel::setFilterSelected(bool enabled)
{
    mFilterSelected = enabled;
    invalidateFilter();
}

EwsSubscriptionWidgetPrivate::EwsSubscriptionWidgetPrivate(EwsClient &client, EwsSettings *settings, QObject *parent)
    : QObject(parent)
    , mClient(client)
    , mSubscribedIdsRetrieved(false)
    , mSettings(settings)
{
}

EwsSubscriptionWidgetPrivate::~EwsSubscriptionWidgetPrivate() = default;

void EwsSubscriptionWidgetPrivate::enableCheckBoxToggled(bool checked)
{
    mSubContainer->setEnabled(checked);
}

void EwsSubscriptionWidgetPrivate::reloadFolderList(bool)
{
    if (mClient.isConfigured()) {
        auto req = new EwsFindFolderRequest(mClient, this);
        EwsFolderShape shape(EwsShapeIdOnly);
        shape << EwsPropertyField(QStringLiteral("folder:DisplayName"));
        shape << EwsPropertyField(QStringLiteral("folder:ParentFolderId"));
        req->setFolderShape(shape);
        req->setParentFolderId(EwsId(EwsDIdMsgFolderRoot));
        connect(req, &EwsRequest::result, this, &EwsSubscriptionWidgetPrivate::readFolderListFinished);
        req->start();
        mFolderListPendingRequests = 1;
        if (!mSubscribedIdsRetrieved) {
            auto job = new EwsSubscribedFoldersJob(mClient, mSettings, this);
            connect(job, &EwsRequest::result, this, &EwsSubscriptionWidgetPrivate::subscribedFoldersJobFinished);
            job->start();
            mFolderListPendingRequests++;
        }
        mRefreshButton->setEnabled(false);
    } else {
        mMsgWidget->setText(i18nc("@info", "Exchange server not configured."));
        mMsgWidget->setMessageType(KMessageWidget::Error);
        mMsgWidget->animatedShow();
    }
}

void EwsSubscriptionWidgetPrivate::readFolderListFinished(KJob *job)
{
    if (job->error()) {
        mMsgWidget->setText(i18nc("@info", "Failed to retrieve folder list."));
        mMsgWidget->setMessageType(KMessageWidget::Error);
        mMsgWidget->animatedShow();
        mRefreshButton->setEnabled(true);
    } else {
        auto req = qobject_cast<EwsFindFolderRequest *>(job);
        Q_ASSERT(req);

        mFolders = req->folders();

        mFolderListPendingRequests--;
        if (mFolderListPendingRequests == 0) {
            mRefreshButton->setEnabled(true);
            populateFolderTree();
        }
    }
}

void EwsSubscriptionWidgetPrivate::subscribedFoldersJobFinished(KJob *job)
{
    if (job->error()) {
        mMsgWidget->setText(i18nc("@info", "Failed to retrieve folder list."));
        mMsgWidget->setMessageType(KMessageWidget::Error);
        mMsgWidget->animatedShow();
        mRefreshButton->setEnabled(true);
    } else {
        auto req = qobject_cast<EwsSubscribedFoldersJob *>(job);
        Q_ASSERT(req);

        mSubscribedIds = req->folders();

        mFolderListPendingRequests--;
        mOrigSubscribedIds = mSubscribedIds;
        mSubscribedIdsRetrieved = true;
        if (mFolderListPendingRequests == 0) {
            mRefreshButton->setEnabled(true);
            populateFolderTree();
        }
    }
}

void EwsSubscriptionWidgetPrivate::populateFolderTree()
{
    mFolderTreeModel->clear();
    mFolderItemHash.clear();

    for (const EwsFolder &folder : std::as_const(mFolders)) {
        auto item = new QStandardItem(folder[EwsFolderFieldDisplayName].toString());
        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        item->setCheckable(true);
        auto id = folder[EwsFolderFieldFolderId].value<EwsId>();
        item->setData(id.id(), ItemIdRole);
        if (mSubscribedIds.contains(EwsId(id.id()))) {
            item->setCheckState(Qt::Checked);
        }
        auto parentId = folder[EwsFolderFieldParentFolderId].value<EwsId>();
        if (parentId.type() != EwsId::Unspecified) {
            QStandardItem *parentItem = mFolderItemHash.value(parentId.id());
            if (parentItem) {
                parentItem->appendRow(item);
            }
        }
        mFolderItemHash.insert(id.id(), item);
    }

    for (QStandardItem *item : std::as_const(mFolderItemHash)) {
        if (!item->parent()) {
            mFolderTreeModel->appendRow(item);
        }
    }
}

void EwsSubscriptionWidgetPrivate::treeItemChanged(QStandardItem *item)
{
    EwsId id = EwsId(item->data(ItemIdRole).toString());
    if (item->checkState() == Qt::Checked) {
        mSubscribedIds += id;
    } else {
        mSubscribedIds.removeOne(id);
    }
}

void EwsSubscriptionWidgetPrivate::filterTextChanged(const QString &text)
{
    mFilterModel->setFilterFixedString(text);
}

void EwsSubscriptionWidgetPrivate::resetSelection(bool)
{
    mSubscribedIds = mOrigSubscribedIds;
    populateFolderTree();
}

EwsSubscriptionWidget::EwsSubscriptionWidget(EwsClient &client, EwsSettings *settings, QWidget *parent)
    : QWidget(parent)
    , d_ptr(new EwsSubscriptionWidgetPrivate(client, settings, this))
{
    Q_D(EwsSubscriptionWidget);

    d->mEnabled = d->mSettings->serverSubscription();

    auto topLayout = new QVBoxLayout(this);

    d->mMsgWidget = new KMessageWidget(this);
    d->mMsgWidget->setVisible(false);

    d->mEnableCheckBox = new QCheckBox(i18nc("@option:check", "Enable server-side subscriptions"), this);
    d->mEnableCheckBox->setChecked(d->mEnabled);

    d->mSubContainer = new QWidget(this);
    auto subContainerLayout = new QVBoxLayout(d->mSubContainer);
    subContainerLayout->setContentsMargins({});

    auto filterLineEdit = new QLineEdit(this);
    filterLineEdit->setPlaceholderText(i18nc("@label:textbox", "Filter folders"));

    auto treeContainer = new QWidget(this);
    auto treeContainerLayout = new QHBoxLayout(treeContainer);
    treeContainerLayout->setContentsMargins({});

    d->mFolderTreeView = new QTreeView(this);
    d->mFolderTreeModel = new QStandardItemModel(this);
    d->mFilterModel = new EwsSubscriptionFilterModel(this);
    d->mFilterModel->setSourceModel(d->mFolderTreeModel);
    d->mFilterModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    d->mFilterModel->sort(0, Qt::AscendingOrder);
    d->mFolderTreeView->setModel(d->mFilterModel);
    d->mFolderTreeView->header()->hide();

    auto buttonContainer = new QWidget(this);
    auto buttonContainerLayout = new QVBoxLayout(buttonContainer);
    buttonContainerLayout->setContentsMargins({});

    d->mRefreshButton = new QPushButton(this);
    d->mRefreshButton->setText(i18nc("@action:button", "Reload &List"));

    auto resetButton = new QPushButton(this);
    resetButton->setText(i18nc("@action:button", "&Reset"));

    buttonContainerLayout->addWidget(d->mRefreshButton);
    buttonContainerLayout->addWidget(resetButton);
    buttonContainerLayout->addStretch();

    treeContainerLayout->addWidget(d->mFolderTreeView);
    treeContainerLayout->addWidget(buttonContainer);

    auto subOnlyCheckBox = new QCheckBox(i18nc("@option:check", "Subscribed only"), this);

    subContainerLayout->addWidget(filterLineEdit);
    subContainerLayout->addWidget(treeContainer);
    subContainerLayout->addWidget(subOnlyCheckBox);

    topLayout->addWidget(d->mMsgWidget);
    topLayout->addWidget(d->mEnableCheckBox);
    topLayout->addWidget(d->mSubContainer);

    connect(d->mEnableCheckBox, &QCheckBox::toggled, d, &EwsSubscriptionWidgetPrivate::enableCheckBoxToggled);
    connect(d->mRefreshButton, &QPushButton::clicked, d, &EwsSubscriptionWidgetPrivate::reloadFolderList);
    connect(resetButton, &QPushButton::clicked, d, &EwsSubscriptionWidgetPrivate::resetSelection);
    connect(d->mFolderTreeModel, &QStandardItemModel::itemChanged, d, &EwsSubscriptionWidgetPrivate::treeItemChanged);
    connect(filterLineEdit, &QLineEdit::textChanged, d, &EwsSubscriptionWidgetPrivate::filterTextChanged);
    connect(subOnlyCheckBox, &QCheckBox::toggled, d->mFilterModel, &EwsSubscriptionFilterModel::setFilterSelected);

    d->enableCheckBoxToggled(d->mEnabled);
    d->reloadFolderList(false);
}

EwsSubscriptionWidget::~EwsSubscriptionWidget() = default;

QStringList EwsSubscriptionWidget::subscribedList() const
{
    Q_D(const EwsSubscriptionWidget);

    QStringList list;
    list.reserve(d->mSubscribedIds.count());
    for (const EwsId &id : std::as_const(d->mSubscribedIds)) {
        list.append(id.id());
    }

    return list;
}

bool EwsSubscriptionWidget::subscriptionEnabled() const
{
    Q_D(const EwsSubscriptionWidget);

    return d->mEnableCheckBox->isChecked();
}

bool EwsSubscriptionWidget::subscribedListValid() const
{
    Q_D(const EwsSubscriptionWidget);

    return d->mSubscribedIdsRetrieved;
}

#include "ewssubscriptionwidget.moc"
