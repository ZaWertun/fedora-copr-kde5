/*
    SPDX-FileCopyrightText: 2023 Laurent Montel <montel@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "newmailnotificationhistorywidget.h"
#include "newmailnotificationhistorymanager.h"
#include "newmailnotificationhistoryplaintextedit.h"
#include "newmailnotificationhistoryplaintexteditor.h"
#include "newmailnotifieragentsettings.h"
#include <KLocalizedString>

#include <KPIMTextEdit/PlainTextEditor>

#include <QCheckBox>
#include <QScrollBar>
#include <QVBoxLayout>

NewMailNotificationHistoryWidget::NewMailNotificationHistoryWidget(QWidget *parent)
    : QWidget{parent}
    , mPlainTextEdit(new NewMailNotificationHistoryPlainTextEdit(new NewMailNotificationHistoryPlainTextEditor(this), this))
    , mEnabledHistory(new QCheckBox(i18n("Enabled"), this))
{
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setObjectName(QStringLiteral("mainLayout"));
    mainLayout->setContentsMargins({});

    mEnabledHistory->setObjectName(QStringLiteral("mEnabledHistory"));

    mPlainTextEdit->setObjectName(QStringLiteral("mPlainTextEdit"));
    mPlainTextEdit->setReadOnly(true);
    mainLayout->addWidget(mPlainTextEdit);

    mainLayout->addWidget(mEnabledHistory);
    connect(mEnabledHistory, &QCheckBox::clicked, this, &NewMailNotificationHistoryWidget::slotEnableChanged);

    connect(NewMailNotificationHistoryManager::self(),
            &NewMailNotificationHistoryManager::historyAdded,
            this,
            &NewMailNotificationHistoryWidget::slotHistoryAdded);

    mPlainTextEdit->setPlainText(NewMailNotificationHistoryManager::self()->history().join(QLatin1Char('\n')));
    connect(mPlainTextEdit, &NewMailNotificationHistoryPlainTextEdit::clear, this, [this]() {
        NewMailNotificationHistoryManager::self()->clear();
        mPlainTextEdit->clear();
    });
    slotEnableChanged(NewMailNotifierAgentSettings::self()->enableNotificationHistory());
    mEnabledHistory->setChecked(NewMailNotifierAgentSettings::self()->enableNotificationHistory());
}

NewMailNotificationHistoryWidget::~NewMailNotificationHistoryWidget() = default;

void NewMailNotificationHistoryWidget::slotHistoryAdded(const QString &str)
{
    mPlainTextEdit->editor()->appendPlainText(str);
    mPlainTextEdit->editor()->verticalScrollBar()->setValue(mPlainTextEdit->editor()->verticalScrollBar()->maximum());
}

void NewMailNotificationHistoryWidget::slotEnableChanged(bool clicked)
{
    mPlainTextEdit->setEnabled(clicked);
    NewMailNotifierAgentSettings::self()->setEnableNotificationHistory(clicked);
    NewMailNotifierAgentSettings::self()->save();
}
