/*
    SPDX-FileCopyrightText: 2013-2023 Laurent Montel <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "newmailnotifiersettingswidget.h"
#include "newmailnotifieragentsettings.h"
#include "newmailnotifierselectcollectionwidget.h"

#include "kdepim-runtime-version.h"

#include <KAboutData>
#include <KLocalizedString>
#include <KNotifyConfigWidget>
#include <QLineEdit>
#include <QPushButton>

#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QWhatsThis>

#include <KLazyLocalizedString>
#include <KSharedConfig>

static KLazyLocalizedString textToSpeakMessage = kli18n(
    "<qt>"
    "<p>Here you can define message. "
    "You can use:</p>"
    "<ul>"
    "<li>%s set subject</li>"
    "<li>%f set from</li>"
    "</ul>"
    "</qt>");

NewMailNotifierSettingsWidget::NewMailNotifierSettingsWidget(const KSharedConfigPtr &config, QWidget *parent, const QVariantList &args)
    : Akonadi::AgentConfigurationBase(config, parent, args)
    , mSelectCollection(new NewMailNotifierSelectCollectionWidget(parent))
{
    NewMailNotifierAgentSettings::instance(config);
    setObjectName(QStringLiteral("NewMailNotifierSettingsWidget"));
    auto tab = new QTabWidget(parent);
    parent->layout()->addWidget(tab);

    auto settings = new QWidget;
    auto vbox = new QVBoxLayout(settings);

    auto grp = new QGroupBox(i18n("Choose which fields to show:"), parent);
    vbox->addWidget(grp);
    auto groupboxLayout = new QVBoxLayout;
    grp->setLayout(groupboxLayout);

    mShowPhoto = new QCheckBox(i18n("Show Photo"), parent);
    mShowPhoto->setObjectName(QStringLiteral("mShowPhoto"));
    groupboxLayout->addWidget(mShowPhoto);

    mShowFrom = new QCheckBox(i18n("Show From"), parent);
    mShowFrom->setObjectName(QStringLiteral("mShowFrom"));
    groupboxLayout->addWidget(mShowFrom);

    mShowSubject = new QCheckBox(i18n("Show Subject"), parent);
    mShowSubject->setObjectName(QStringLiteral("mShowSubject"));
    groupboxLayout->addWidget(mShowSubject);

    mShowFolders = new QCheckBox(i18n("Show Folders"), parent);
    mShowFolders->setObjectName(QStringLiteral("mShowFolders"));
    groupboxLayout->addWidget(mShowFolders);

    mExcludeMySelf = new QCheckBox(i18n("Do not notify when email was sent by me"), parent);
    mExcludeMySelf->setObjectName(QStringLiteral("mExcludeMySelf"));
    vbox->addWidget(mExcludeMySelf);

    mKeepPersistentNotification = new QCheckBox(i18n("Keep Persistent Notification"), parent);
    mKeepPersistentNotification->setObjectName(QStringLiteral("mKeepPersistentNotification"));
    vbox->addWidget(mKeepPersistentNotification);

    mAllowToShowMail = new QCheckBox(i18n("Show Action Buttons"), parent);
    mAllowToShowMail->setObjectName(QStringLiteral("mAllowToShowMail"));
    vbox->addWidget(mAllowToShowMail);

    auto hboxLayout = new QHBoxLayout;
    hboxLayout->setObjectName(QStringLiteral("hboxLayout"));
    vbox->addLayout(hboxLayout);

    mReplyMail = new QCheckBox(i18n("Reply Mail"), parent);
    mReplyMail->setObjectName(QStringLiteral("mReplyMail"));
    hboxLayout->addWidget(mReplyMail);
    mReplyMail->setEnabled(false);

    mReplyMailTypeComboBox = new QComboBox(parent);
    mReplyMailTypeComboBox->setObjectName(QStringLiteral("mReplyMailTypeComboBox"));
    mReplyMailTypeComboBox->setEnabled(false);
    mReplyMailTypeComboBox->addItems({i18n("Reply to Author"), i18n("Reply to All")});
    hboxLayout->addWidget(mReplyMailTypeComboBox);
    hboxLayout->addStretch(1);

    connect(mAllowToShowMail, &QCheckBox::clicked, this, [this](bool enabled) {
        updateReplyMail(enabled);
    });

    vbox->addStretch();
    tab->addTab(settings, i18n("Display"));
#if HAVE_TEXT_TO_SPEECH_SUPPORT
    auto textSpeakWidget = new QWidget;
    vbox = new QVBoxLayout;
    textSpeakWidget->setLayout(vbox);
    mTextToSpeak = new QCheckBox(i18n("Enabled"), parent);
    mTextToSpeak->setObjectName(QStringLiteral("mTextToSpeak"));
    vbox->addWidget(mTextToSpeak);

    auto howIsItWork = new QLabel(i18n("<a href=\"whatsthis\">How does this work?</a>"), parent);
    howIsItWork->setObjectName(QStringLiteral("howIsItWork"));
    howIsItWork->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
    howIsItWork->setContextMenuPolicy(Qt::NoContextMenu);
    vbox->addWidget(howIsItWork);
    connect(howIsItWork, &QLabel::linkActivated, this, &NewMailNotifierSettingsWidget::slotHelpLinkClicked);

    auto textToSpeakLayout = new QHBoxLayout;
    textToSpeakLayout->setContentsMargins({});
    auto lab = new QLabel(i18n("Message:"), parent);
    lab->setObjectName(QStringLiteral("labmessage"));
    textToSpeakLayout->addWidget(lab);
    mTextToSpeakSetting = new QLineEdit(parent);
    mTextToSpeakSetting->setObjectName(QStringLiteral("mTextToSpeakSetting"));
    mTextToSpeakSetting->setClearButtonEnabled(true);

    mTextToSpeakSetting->setWhatsThis(textToSpeakMessage.toString());
    textToSpeakLayout->addWidget(mTextToSpeakSetting);
    vbox->addLayout(textToSpeakLayout);
    vbox->addStretch();
    tab->addTab(textSpeakWidget, i18n("Text to Speak"));
    connect(mTextToSpeak, &QCheckBox::toggled, mTextToSpeakSetting, &QLineEdit::setEnabled);
#endif
    mNotify = new KNotifyConfigWidget(parent);
    mNotify->setObjectName(QStringLiteral("mNotify"));
    mNotify->setApplication(QStringLiteral("akonadi_newmailnotifier_agent"));
    tab->addTab(mNotify, i18n("Notify"));

    mSelectCollection->setObjectName(QStringLiteral("mSelectCollection"));
    tab->addTab(mSelectCollection, i18n("Folders"));

    KAboutData aboutData = KAboutData(QStringLiteral("newmailnotifieragent"),
                                      i18n("New Mail Notifier Agent"),
                                      QStringLiteral(KDEPIM_RUNTIME_VERSION),
                                      i18n("Notify about new mails."),
                                      KAboutLicense::GPL_V2,
                                      i18n("Copyright (C) 2013-%1 Laurent Montel", QStringLiteral("2023")));

    aboutData.setProductName(QByteArrayLiteral("Akonadi/New Mail Notifier"));
    aboutData.addAuthor(i18n("Laurent Montel"), i18n("Maintainer"), QStringLiteral("montel@kde.org"));
    aboutData.setTranslator(i18nc("NAME OF TRANSLATORS", "Your names"), i18nc("EMAIL OF TRANSLATORS", "Your emails"));
    setKAboutData(aboutData);
}

NewMailNotifierSettingsWidget::~NewMailNotifierSettingsWidget()
{
    delete NewMailNotifierAgentSettings::self();
}

void NewMailNotifierSettingsWidget::updateReplyMail(bool enabled)
{
    mReplyMail->setEnabled(enabled);
    mReplyMailTypeComboBox->setEnabled(enabled);
}

void NewMailNotifierSettingsWidget::load()
{
    Akonadi::AgentConfigurationBase::load();

    auto settings = NewMailNotifierAgentSettings::self();
    settings->load();

    mShowPhoto->setChecked(settings->showPhoto());
    mShowFrom->setChecked(settings->showFrom());
    mShowSubject->setChecked(settings->showSubject());
    mShowFolders->setChecked(settings->showFolder());
    mExcludeMySelf->setChecked(settings->excludeEmailsFromMe());
    mAllowToShowMail->setChecked(settings->showButtonToDisplayMail());
    mKeepPersistentNotification->setChecked(settings->keepPersistentNotification());
#if HAVE_TEXT_TO_SPEECH_SUPPORT
    mTextToSpeak->setChecked(settings->textToSpeakEnabled());
    mTextToSpeakSetting->setEnabled(mTextToSpeak->isChecked());
    mTextToSpeakSetting->setText(settings->textToSpeak());
#endif
    mReplyMail->setChecked(settings->replyMail());
    mReplyMailTypeComboBox->setCurrentIndex(settings->replyMailType());

    updateReplyMail(mAllowToShowMail->isChecked());
}

bool NewMailNotifierSettingsWidget::save() const
{
    mSelectCollection->updateCollectionsRecursive();
    auto settings = NewMailNotifierAgentSettings::self();
    settings->setShowPhoto(mShowPhoto->isChecked());
    settings->setShowFrom(mShowFrom->isChecked());
    settings->setShowSubject(mShowSubject->isChecked());
    settings->setShowFolder(mShowFolders->isChecked());
    settings->setExcludeEmailsFromMe(mExcludeMySelf->isChecked());
    settings->setShowButtonToDisplayMail(mAllowToShowMail->isChecked());
    settings->setKeepPersistentNotification(mKeepPersistentNotification->isChecked());
#if HAVE_TEXT_TO_SPEECH_SUPPORT
    settings->setTextToSpeakEnabled(mTextToSpeak->isChecked());
    settings->setTextToSpeak(mTextToSpeakSetting->text());
#endif
    settings->setReplyMail(mReplyMail->isChecked());
    settings->setReplyMailType(mReplyMailTypeComboBox->currentIndex());
    settings->save();
    mNotify->save();

    return Akonadi::AgentConfigurationBase::save();
}

void NewMailNotifierSettingsWidget::slotHelpLinkClicked(const QString &)
{
    const QString help = textToSpeakMessage.toString();
    QWhatsThis::showText(QCursor::pos(), help);
}
