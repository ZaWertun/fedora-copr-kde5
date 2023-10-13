/*
    SPDX-FileCopyrightText: 2015-2016 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ewsfetchmaildetailjob.h"

#include <Akonadi/MessageFlags>
#include <KMime/Message>

#include "ewsgetitemrequest.h"
#include "ewsitemshape.h"
#include "ewsmailbox.h"
#include "ewsmailhandler.h"
#include "ewsresource_debug.h"

using namespace Akonadi;

EwsFetchMailDetailJob::EwsFetchMailDetailJob(EwsClient &client, QObject *parent, const Akonadi::Collection &collection)
    : EwsFetchItemDetailJob(client, parent, collection)
{
    EwsItemShape shape(EwsShapeIdOnly);
    shape << EwsPropertyField(QStringLiteral("item:Subject"));
    shape << EwsPropertyField(QStringLiteral("item:Importance"));
    shape << EwsPropertyField(QStringLiteral("message:From"));
    shape << EwsPropertyField(QStringLiteral("message:ToRecipients"));
    shape << EwsPropertyField(QStringLiteral("message:CcRecipients"));
    shape << EwsPropertyField(QStringLiteral("message:BccRecipients"));
    shape << EwsPropertyField(QStringLiteral("item:Categories"));
    shape << EwsPropertyField(QStringLiteral("item:DateTimeReceived"));
    shape << EwsPropertyField(QStringLiteral("item:InReplyTo"));
    shape << EwsPropertyField(QStringLiteral("message:References"));
    shape << EwsPropertyField(QStringLiteral("message:ReplyTo"));
    shape << EwsPropertyField(QStringLiteral("message:InternetMessageId"));
    shape << EwsPropertyField(QStringLiteral("item:Size"));
    const auto flagsProperties = EwsMailHandler::flagsProperties();
    for (const EwsPropertyField &field : flagsProperties) {
        shape << field;
    }
    const auto tagsProperties = EwsItemHandler::tagsProperties();
    for (const EwsPropertyField &field : tagsProperties) {
        shape << field;
    }
    mRequest->setItemShape(shape);
}

EwsFetchMailDetailJob::~EwsFetchMailDetailJob() = default;

void EwsFetchMailDetailJob::processItems(const EwsGetItemRequest::Response::List &responses)
{
    Item::List::iterator it = mChangedItems.begin();

    for (const EwsGetItemRequest::Response &resp : responses) {
        Item &item = *it;

        if (!resp.isSuccess()) {
            qCWarningNC(EWSRES_LOG) << QStringLiteral("Failed to fetch item %1").arg(item.remoteId());
            continue;
        }

        const EwsItem &ewsItem = resp.item();
        KMime::Message::Ptr msg(new KMime::Message);

        // Rebuild the message headers
        QVariant v = ewsItem[EwsItemFieldSubject];
        if (Q_LIKELY(v.isValid())) {
            msg->subject()->fromUnicodeString(v.toString(), "utf-8");
        }

        v = ewsItem[EwsItemFieldFrom];
        if (Q_LIKELY(v.isValid())) {
            const auto mbox = v.value<EwsMailbox>();
            msg->from()->addAddress(mbox);
        }

        v = ewsItem[EwsItemFieldToRecipients];
        if (Q_LIKELY(v.isValid())) {
            const auto mboxList = v.value<EwsMailbox::List>();
            for (const EwsMailbox &mbox : mboxList) {
                msg->to()->addAddress(mbox);
            }
        }

        v = ewsItem[EwsItemFieldCcRecipients];
        if (Q_LIKELY(v.isValid())) {
            const auto mboxList = v.value<EwsMailbox::List>();
            for (const EwsMailbox &mbox : mboxList) {
                msg->cc()->addAddress(mbox);
            }
        }

        v = ewsItem[EwsItemFieldBccRecipients];
        if (v.isValid()) {
            const auto mboxList = v.value<EwsMailbox::List>();
            for (const EwsMailbox &mbox : mboxList) {
                msg->bcc()->addAddress(mbox);
            }
        }

        v = ewsItem[EwsItemFieldInternetMessageId];
        if (v.isValid()) {
            msg->messageID()->from7BitString(v.toString().toLatin1());
        }

        v = ewsItem[EwsItemFieldInReplyTo];
        if (v.isValid()) {
            msg->inReplyTo()->from7BitString(v.toString().toLatin1());
        }

        v = ewsItem[EwsItemFieldDateTimeReceived];
        if (v.isValid()) {
            msg->date()->setDateTime(v.toDateTime());
        }

        v = ewsItem[EwsItemFieldReferences];
        if (v.isValid()) {
            msg->references()->from7BitString(v.toString().toLatin1());
        }

        v = ewsItem[EwsItemFieldReplyTo];
        if (v.isValid()) {
            const auto mbox = v.value<EwsMailbox>();
            msg->replyTo()->addAddress(mbox);
        }

        msg->assemble();
        item.setPayload(KMime::Message::Ptr(msg));

        v = ewsItem[EwsItemFieldSize];
        if (v.isValid()) {
            item.setSize(v.toUInt());
        }

        // Workaround for Akonadi bug
        // When setting flags, adding each of them separately vs. setting a list in one go makes
        // a difference. In the former case the item treats this as an incremental change and
        // records flags added and removed. In the latter it sets a flag indicating that flags were
        // reset.
        // For some strange reason Akonadi is not seeing the flags in the latter case.
        const auto flags{EwsMailHandler::readFlags(ewsItem)};
        for (const QByteArray &flag : flags) {
            item.setFlag(flag);
        }
        qCDebugNC(EWSRES_LOG) << "EwsFetchMailDetailJob::processItems:" << ewsHash(item.remoteId()) << item.flags();

        ++it;
    }

    qCDebugNC(EWSRES_LOG) << "EwsFetchMailDetailJob::processItems: done";

    emitResult();
}
