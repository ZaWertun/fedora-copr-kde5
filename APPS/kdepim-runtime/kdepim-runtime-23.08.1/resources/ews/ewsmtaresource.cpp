/*
    SPDX-FileCopyrightText: 2015-2017 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ewsmtaresource.h"

#include <QDBusConnection>

#include "ewsmtaconfigdialog.h"
#include "ewsmtasettings.h"
#include "ewsresourceinterface.h"
#include <KLocalizedString>
#include <KMime/Message>

using namespace Akonadi;

EwsMtaResource::EwsMtaResource(const QString &id)
    : Akonadi::ResourceBase(id)
{
}

EwsMtaResource::~EwsMtaResource() = default;

bool EwsMtaResource::connectEws()
{
    if (mEwsResource) {
        return true;
    }
    mEwsResource = new OrgKdeAkonadiEwsResourceInterface(QStringLiteral("org.freedesktop.Akonadi.Resource.") + EwsMtaSettings::ewsResource(),
                                                         QStringLiteral("/"),
                                                         QDBusConnection::sessionBus(),
                                                         this);
    if (!mEwsResource->isValid()) {
        delete mEwsResource;
        mEwsResource = nullptr;
    } else {
        connect(mEwsResource, &OrgKdeAkonadiEwsResourceInterface::messageSent, this, &EwsMtaResource::messageSent);
    }

    return mEwsResource != nullptr;
}

void EwsMtaResource::configure(WId windowId)
{
    QPointer<EwsMtaConfigDialog> dlg = new EwsMtaConfigDialog(this, windowId);
    if (dlg->exec()) {
        EwsMtaSettings::self()->save();
    }
    delete dlg;
}

void EwsMtaResource::sendItem(const Akonadi::Item &item)
{
    if (!connectEws()) {
        itemSent(item, TransportFailed, i18n("Unable to connect to master EWS resource"));
        return;
    }

    mItemHash.insert(item.remoteId(), item);

    auto msg = item.payload<KMime::Message::Ptr>();
    /* Exchange doesn't just store whatever MIME content that was sent to it - it will parse it and send
     * further the version assembled back from the parsed parts. It seems that this parsing doesn't work well
     * with the quoted-printable encoding, which KMail prefers. This results in malformed encoding, which the
     * sender doesn't even see.
     * As a workaround force encoding of the body (or in case of multipart - all parts) to Base64. */
    if (msg->contents().isEmpty()) {
        msg->changeEncoding(KMime::Headers::CEbase64);
        msg->contentTransferEncoding(true)->setEncoding(KMime::Headers::CEbase64);
    } else {
        const auto contents = msg->contents();
        for (KMime::Content *content : contents) {
            content->changeEncoding(KMime::Headers::CEbase64);
            content->contentTransferEncoding(true)->setEncoding(KMime::Headers::CEbase64);
        }
    }
    msg->assemble();
    QByteArray mimeContent = msg->encodedContent(true);
    mEwsResource->sendMessage(item.remoteId(), mimeContent);
}

void EwsMtaResource::messageSent(const QString &id, const QString &error)
{
    QHash<QString, Item>::iterator it = mItemHash.find(id);
    if (it != mItemHash.end()) {
        if (error.isEmpty()) {
            itemSent(*it, TransportSucceeded, QString());
        } else {
            itemSent(*it, TransportFailed, error);
        }
        mItemHash.erase(it);
    }
}

void EwsMtaResource::retrieveCollections()
{
    collectionsRetrieved(Collection::List());
}

void EwsMtaResource::retrieveItems(const Akonadi::Collection &collection)
{
    Q_UNUSED(collection)

    itemsRetrieved(Item::List());
}

bool EwsMtaResource::retrieveItems(const Akonadi::Item::List &items, const QSet<QByteArray> &parts)
{
    Q_UNUSED(parts)

    itemsRetrieved(items);

    return true;
}

AKONADI_RESOURCE_MAIN(EwsMtaResource)
