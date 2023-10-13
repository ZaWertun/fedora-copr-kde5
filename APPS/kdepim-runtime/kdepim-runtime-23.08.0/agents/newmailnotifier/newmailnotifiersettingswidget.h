/*
    SPDX-FileCopyrightText: 2013-2023 Laurent Montel <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once
#include "config-newmailnotifier.h"
#include <Akonadi/AgentConfigurationBase>
#include <Akonadi/Collection>

class KNotifyConfigWidget;
class QCheckBox;
class QLineEdit;
class QComboBox;
class NewMailNotifierSelectCollectionWidget;
class NewMailNotifierSettingsWidget : public Akonadi::AgentConfigurationBase
{
    Q_OBJECT
public:
    explicit NewMailNotifierSettingsWidget(const KSharedConfigPtr &config, QWidget *parent, const QVariantList &args);
    ~NewMailNotifierSettingsWidget() override;

    void load() override;
    bool save() const override;

private:
    void slotHelpLinkClicked(const QString &);
    void updateReplyMail(bool enabled);
    QCheckBox *mShowPhoto = nullptr;
    QCheckBox *mShowFrom = nullptr;
    QCheckBox *mShowSubject = nullptr;
    QCheckBox *mShowFolders = nullptr;
    QCheckBox *mExcludeMySelf = nullptr;
    QCheckBox *mAllowToShowMail = nullptr;
    QCheckBox *mKeepPersistentNotification = nullptr;
    KNotifyConfigWidget *mNotify = nullptr;
#if HAVE_TEXT_TO_SPEECH_SUPPORT
    QCheckBox *mTextToSpeak = nullptr;
    QLineEdit *mTextToSpeakSetting = nullptr;
#endif
    QCheckBox *mReplyMail = nullptr;
    QComboBox *mReplyMailTypeComboBox = nullptr;
    NewMailNotifierSelectCollectionWidget *const mSelectCollection;
};

AKONADI_AGENTCONFIG_FACTORY(NewMailNotifierSettingsFactory, "newmailnotifierconfig.json", NewMailNotifierSettingsWidget)
