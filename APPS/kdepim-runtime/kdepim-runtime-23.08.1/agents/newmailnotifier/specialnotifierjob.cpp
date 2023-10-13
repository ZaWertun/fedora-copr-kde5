/*
   SPDX-FileCopyrightText: 2013-2023 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "specialnotifierjob.h"
#include "newmailnotifieragentsettings.h"
#include "newmailnotifierreplymessagejob.h"
#include "newmailnotifiershowmessagejob.h"

#include <Akonadi/ContactSearchJob>
#include <Akonadi/ItemDeleteJob>
#include <Akonadi/ItemFetchJob>
#include <Akonadi/ItemFetchScope>
#include <Akonadi/MessageParts>

#include <KEmailAddress>
#include <KNotification>

#include <KMime/Message>

#include "newmailnotifier_debug.h"
#include <KLocalizedString>

SpecialNotifierJob::SpecialNotifierJob(const QStringList &listEmails, const QString &path, Akonadi::Item::Id id, QObject *parent)
    : QObject(parent)
    , mListEmails(listEmails)
    , mPath(path)
    , mItemId(id)
{
    Akonadi::Item item(mItemId);
    auto job = new Akonadi::ItemFetchJob(item, this);
    job->fetchScope().fetchPayloadPart(Akonadi::MessagePart::Envelope, true);

    connect(job, &Akonadi::ItemFetchJob::result, this, &SpecialNotifierJob::slotItemFetchJobDone);
}

SpecialNotifierJob::~SpecialNotifierJob() = default;

void SpecialNotifierJob::setDefaultIconName(const QString &iconName)
{
    mDefaultIconName = iconName;
}

void SpecialNotifierJob::slotItemFetchJobDone(KJob *job)
{
    if (job->error()) {
        qCWarning(NEWMAILNOTIFIER_LOG) << job->errorString();
        deleteLater();
        return;
    }
    const Akonadi::Item::List lst = qobject_cast<Akonadi::ItemFetchJob *>(job)->items();
    if (lst.count() == 1) {
        mItem = lst.first();
        if (!mItem.hasPayload<KMime::Message::Ptr>()) {
            qCDebug(NEWMAILNOTIFIER_LOG) << " message has not payload.";
            deleteLater();
            return;
        }

        const auto mb = mItem.payload<KMime::Message::Ptr>();
        mFrom = mb->from()->asUnicodeString();
        mSubject = mb->subject()->asUnicodeString();
        if (NewMailNotifierAgentSettings::showPhoto()) {
            auto searchJob = new Akonadi::ContactSearchJob(this);
            searchJob->setLimit(1);
            searchJob->setQuery(Akonadi::ContactSearchJob::Email, KEmailAddress::firstEmailAddress(mFrom).toLower(), Akonadi::ContactSearchJob::ExactMatch);
            connect(searchJob, &Akonadi::ItemFetchJob::result, this, &SpecialNotifierJob::slotSearchJobFinished);
        } else {
            emitNotification();
        }
    } else {
        qCWarning(NEWMAILNOTIFIER_LOG) << " Found item different from 1: " << lst.count();
        deleteLater();
        return;
    }
}

void SpecialNotifierJob::slotSearchJobFinished(KJob *job)
{
    const Akonadi::ContactSearchJob *searchJob = qobject_cast<Akonadi::ContactSearchJob *>(job);
    if (searchJob->error()) {
        qCWarning(NEWMAILNOTIFIER_LOG) << "Unable to fetch contact:" << searchJob->errorText();
        emitNotification();
        return;
    }
    if (!searchJob->contacts().isEmpty()) {
        const KContacts::Addressee addressee = searchJob->contacts().at(0);
        const KContacts::Picture photo = addressee.photo();
        const QImage image = photo.data();
        if (image.isNull()) {
            emitNotification();
        } else {
            emitNotification(QPixmap::fromImage(image));
        }
    } else {
        emitNotification();
    }
}

void SpecialNotifierJob::emitNotification(const QPixmap &pixmap)
{
    if (NewMailNotifierAgentSettings::excludeEmailsFromMe()) {
        for (const QString &email : std::as_const(mListEmails)) {
            if (mFrom.contains(email)) {
                // Exclude this notification
                deleteLater();
                return;
            }
        }
    }

    QStringList result;
    if (NewMailNotifierAgentSettings::showFrom()) {
        result << i18n("From: %1", mFrom.toHtmlEscaped());
    }
    if (NewMailNotifierAgentSettings::showSubject()) {
        QString subject = mSubject.simplified();
        if (subject.length() > 80) {
            subject.truncate(80);
            subject += QStringLiteral("...");
        }
        result << i18n("Subject: %1", subject.toHtmlEscaped());
    }
    if (NewMailNotifierAgentSettings::showFolder()) {
        result << i18n("In: %1", mPath);
    }

    if (NewMailNotifierAgentSettings::textToSpeakEnabled()) {
        if (!NewMailNotifierAgentSettings::textToSpeak().isEmpty()) {
            QString message = NewMailNotifierAgentSettings::textToSpeak();
            message.replace(QLatin1String("%s"), mSubject.toHtmlEscaped());
            message.replace(QLatin1String("%f"), mFrom.toHtmlEscaped());
            Q_EMIT say(message);
        }
    }

    if (NewMailNotifierAgentSettings::showButtonToDisplayMail()) {
        auto notification =
            new KNotification(QStringLiteral("new-email"),
                              NewMailNotifierAgentSettings::keepPersistentNotification() ? KNotification::Persistent | KNotification::SkipGrouping
                                                                                         : KNotification::CloseOnTimeout);
        notification->setText(result.join(QLatin1Char('\n')));
        if (pixmap.isNull()) {
            notification->setIconName(mDefaultIconName);
        } else {
            notification->setPixmap(pixmap);
        }
        QStringList lstActions{i18n("Show mail..."), i18n("Mark As Read"), i18n("Delete")};
        if (NewMailNotifierAgentSettings::replyMail()) {
            switch (NewMailNotifierAgentSettings::replyMailType()) {
            case 0:
                lstActions << i18n("Reply to Author");
                break;
            case 1:
                lstActions << i18n("Reply to All");
                break;
            default:
                qCWarning(NEWMAILNOTIFIER_LOG) << " Problem with NewMailNotifierAgentSettings::replyMailType() value: "
                                               << NewMailNotifierAgentSettings::replyMailType();
                break;
            }
        }
        notification->setActions(lstActions);

        connect(notification, &KNotification::activated, this, &SpecialNotifierJob::slotActivateNotificationAction);
        connect(notification, &KNotification::closed, this, &SpecialNotifierJob::deleteLater);

        notification->sendEvent();
    } else {
        Q_EMIT displayNotification(pixmap, result.join(QLatin1Char('\n')));
        deleteLater();
    }
}

void SpecialNotifierJob::slotActivateNotificationAction(unsigned int index)
{
    // Index == 0 => is the default action. We don't have it.
    switch (index) {
    case 1:
        slotOpenMail();
        return;
    case 2:
        slotMarkAsRead();
        return;
    case 3:
        slotDeleteMessage();
        return;
    case 4:
        slotReplyMessage();
        return;
    }
    qCWarning(NEWMAILNOTIFIER_LOG) << " SpecialNotifierJob::slotActivateNotificationAction unknown index " << index;
}

void SpecialNotifierJob::slotReplyMessage()
{
    auto job = new NewMailNotifierReplyMessageJob(mItem.id());
    job->setReplyToAll(NewMailNotifierAgentSettings::replyMailType() == 0 ? false : true);
    job->start();
    deleteLater();
}

void SpecialNotifierJob::slotDeleteMessage()
{
    auto job = new Akonadi::ItemDeleteJob(mItem);
    connect(job, &Akonadi::ItemDeleteJob::result, this, &SpecialNotifierJob::deleteItemDone);
}

void SpecialNotifierJob::deleteItemDone(KJob *job)
{
    if (job->error()) {
        qCWarning(NEWMAILNOTIFIER_LOG) << "SpecialNotifierJob::deleteItemDone error:" << job->errorString();
    }
    deleteLater();
}

void SpecialNotifierJob::slotMarkAsRead()
{
    Akonadi::MessageStatus messageStatus;
    messageStatus.setRead(true);
    auto markAsReadAllJob = new Akonadi::MarkAsCommand(messageStatus, Akonadi::Item::List() << mItem);
    connect(markAsReadAllJob, &Akonadi::MarkAsCommand::result, this, &SpecialNotifierJob::slotMarkAsResult);
    markAsReadAllJob->execute();
}

void SpecialNotifierJob::slotMarkAsResult(Akonadi::MarkAsCommand::Result result)
{
    switch (result) {
    case Akonadi::MarkAsCommand::Undefined:
        qCDebug(NEWMAILNOTIFIER_LOG) << "SpecialNotifierJob undefined result";
        break;
    case Akonadi::MarkAsCommand::OK:
        qCDebug(NEWMAILNOTIFIER_LOG) << "SpecialNotifierJob Done";
        break;
    case Akonadi::MarkAsCommand::Canceled:
        qCDebug(NEWMAILNOTIFIER_LOG) << "SpecialNotifierJob was canceled";
        break;
    case Akonadi::MarkAsCommand::Failed:
        qCDebug(NEWMAILNOTIFIER_LOG) << "SpecialNotifierJob was failed";
        break;
    }
    deleteLater();
}

void SpecialNotifierJob::slotOpenMail()
{
    auto job = new NewMailNotifierShowMessageJob(mItem.id());
    job->start();
    deleteLater();
}
