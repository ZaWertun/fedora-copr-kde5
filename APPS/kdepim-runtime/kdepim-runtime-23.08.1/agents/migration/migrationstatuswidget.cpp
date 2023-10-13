/*
 * SPDX-FileCopyrightText: 2013 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 *
 */

#include "migrationstatuswidget.h"
#include <KLocalizedString>
#include <QAction>
#include <QDialog>
#include <QDialogButtonBox>
#include <QIcon>
#include <QLabel>
#include <QListView>
#include <QToolBar>
#include <QTreeView>
#include <QVBoxLayout>

MigrationStatusWidget::MigrationStatusWidget(MigrationScheduler &scheduler, QWidget *parent)
    : QWidget(parent)
    , mScheduler(scheduler)
{
    auto vboxLayout = new QVBoxLayout(this);
    auto toolbar = new QToolBar(QStringLiteral("MigrationControlToolbar"), this);

    QAction *start = toolbar->addAction(QStringLiteral("Start"));
    start->setIcon(QIcon::fromTheme(QStringLiteral("media-playback-start")));
    connect(start, &QAction::triggered, this, &MigrationStatusWidget::startSelected);

    QAction *pause = toolbar->addAction(QStringLiteral("Pause"));
    pause->setIcon(QIcon::fromTheme(QStringLiteral("media-playback-pause")));
    connect(pause, &QAction::triggered, this, &MigrationStatusWidget::pauseSelected);

    QAction *abort = toolbar->addAction(QStringLiteral("Abort"));
    abort->setIcon(QIcon::fromTheme(QStringLiteral("media-playback-stop")));
    connect(abort, &QAction::triggered, this, &MigrationStatusWidget::abortSelected);

    vboxLayout->addWidget(toolbar);
    auto treeView = new QTreeView(this);
    treeView->setModel(&mScheduler.model());
    mSelectionModel = treeView->selectionModel();
    Q_ASSERT(mSelectionModel);
    // Not sure why this is required, but otherwise the view doesn't load anything from the model
    treeView->update(QModelIndex());
    connect(treeView, &QTreeView::doubleClicked, this, &MigrationStatusWidget::onItemActivated);

    vboxLayout->addWidget(treeView);
}

void MigrationStatusWidget::startSelected()
{
    const QModelIndexList lst = mSelectionModel->selectedRows();
    for (const QModelIndex &index : lst) {
        mScheduler.start(index.data(MigratorModel::IdentifierRole).toString());
    }
}

void MigrationStatusWidget::pauseSelected()
{
    const QModelIndexList lst = mSelectionModel->selectedRows();
    for (const QModelIndex &index : lst) {
        mScheduler.pause(index.data(MigratorModel::IdentifierRole).toString());
    }
}

void MigrationStatusWidget::abortSelected()
{
    const QModelIndexList lst = mSelectionModel->selectedRows();
    for (const QModelIndex &index : lst) {
        mScheduler.abort(index.data(MigratorModel::IdentifierRole).toString());
    }
}

void MigrationStatusWidget::onItemActivated(const QModelIndex &index)
{
    auto dlg = new QDialog(this);
    auto topLayout = new QVBoxLayout(dlg);
    dlg->setLayout(topLayout);
    auto widget = new QWidget(dlg);
    topLayout->addWidget(widget);
    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Close, dlg);
    connect(buttonBox, &QDialogButtonBox::accepted, dlg, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, dlg, &QDialog::reject);
    topLayout->addWidget(buttonBox);

    auto vboxLayout = new QVBoxLayout;
    {
        auto listView = new QListView(widget);
        listView->setModel(&mScheduler.logModel(index.data(MigratorModel::IdentifierRole).toString()));
        listView->setAutoScroll(true);
        listView->scrollToBottom();
        vboxLayout->addWidget(listView);
    }
    {
        auto hboxLayout = new QHBoxLayout;
        auto label =
            new QLabel(QStringLiteral("<a href=\"%1\">%2</a>").arg(index.data(MigratorModel::LogfileRole).toString(), ki18n("Logfile").toString()), widget);
        label->setOpenExternalLinks(true);
        hboxLayout->addWidget(label);
        hboxLayout->addStretch();
        vboxLayout->addLayout(hboxLayout);
    }
    widget->setLayout(vboxLayout);

    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setWindowTitle(i18nc("Title of the window displaying the log of a single migration job.", "Migration Info"));
    dlg->resize(600, 300);
    dlg->show();
}
