/*
    SPDX-FileCopyrightText: 2014 Christian Mollekopf <mollekopf@kolabsys.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kolabhelpers.h"
#include "kolabresource_debug.h"
#include "kolabresource_trace.h"
#include "pimkolab/kolabformat/errorhandler.h"
#include "pimkolab/kolabformat/kolabobject.h"

#include <Akonadi/NoteUtils>

#include <Akonadi/Collection>
#include <Akonadi/ItemFetchJob>
#include <Akonadi/ItemFetchScope>

#include <KCalendarCore/Incidence>
#include <KLocalizedString>
#include <QColor>

bool KolabHelpers::checkForErrors(const Akonadi::Item &item)
{
    if (!Kolab::ErrorHandler::instance().errorOccured()) {
        Kolab::ErrorHandler::instance().clear();
        return false;
    }

    QString errorMsg;
    const auto errors{Kolab::ErrorHandler::instance().getErrors()};
    for (const Kolab::ErrorHandler::Err &error : errors) {
        errorMsg.append(error.message);
        errorMsg.append(QLatin1Char('\n'));
    }

    qCWarning(KOLABRESOURCE_LOG) << "Error on item with id: " << item.id() << " remote id: " << item.remoteId() << ":\n" << errorMsg;
    Kolab::ErrorHandler::instance().clear();
    return true;
}

Akonadi::Item getErrorItem(Kolab::FolderType folderType, const QString &remoteId)
{
    // TODO set title, text and icon
    Akonadi::Item item;
    item.setRemoteId(remoteId);
    switch (folderType) {
    case Kolab::EventType: {
        KCalendarCore::Event::Ptr event(new KCalendarCore::Event);
        // FIXME Use message creation date time
        event->setDtStart(QDateTime::currentDateTimeUtc());
        event->setSummary(i18n("Corrupt Event"));
        event->setDescription(
            i18n("Event could not be read. Delete this event to remove it from the server. Technical information: remote identifier %1", remoteId));
        item.setMimeType(KCalendarCore::Event::eventMimeType());
        item.setPayload(event);
        break;
    }
    case Kolab::TaskType: {
        KCalendarCore::Todo::Ptr task(new KCalendarCore::Todo);
        // FIXME Use message creation date time
        task->setDtStart(QDateTime::currentDateTimeUtc());
        task->setSummary(i18n("Corrupt Task"));
        task->setDescription(i18n("Task could not be read. Delete this task to remove it from the server."));
        item.setMimeType(KCalendarCore::Todo::todoMimeType());
        item.setPayload(task);
        break;
    }
    case Kolab::JournalType: {
        KCalendarCore::Journal::Ptr journal(new KCalendarCore::Journal);
        // FIXME Use message creation date time
        journal->setDtStart(QDateTime::currentDateTimeUtc());
        journal->setSummary(i18n("Corrupt journal"));
        journal->setDescription(i18n("Journal could not be read. Delete this journal to remove it from the server."));
        item.setMimeType(KCalendarCore::Journal::journalMimeType());
        item.setPayload(journal);
        break;
    }
    case Kolab::ContactType: {
        KContacts::Addressee addressee;
        addressee.setName(i18n("Corrupt Contact"));
        addressee.setNote(i18n("Contact could not be read. Delete this contact to remove it from the server."));
        item.setMimeType(KContacts::Addressee::mimeType());
        item.setPayload(addressee);
        break;
    }
    case Kolab::NoteType: {
        Akonadi::NoteUtils::NoteMessageWrapper note;
        note.setTitle(i18n("Corrupt Note"));
        note.setText(i18n("Note could not be read. Delete this note to remove it from the server."));
        item.setPayload(Akonadi::NoteUtils::noteMimeType());
        item.setPayload(note.message());
        break;
    }
    case Kolab::MailType:
    // We don't convert mails, so that should never fail.
    default:
        qCWarning(KOLABRESOURCE_LOG) << "unhandled folder type: " << folderType;
    }
    return item;
}

Akonadi::Item KolabHelpers::translateFromImap(Kolab::FolderType folderType, const Akonadi::Item &imapItem, bool &ok)
{
    // Avoid trying to convert imap messages
    if (folderType == Kolab::MailType) {
        return imapItem;
    }

    // No payload, probably a flag change or alike, we just pass it through
    if (!imapItem.hasPayload()) {
        return imapItem;
    }
    if (!imapItem.hasPayload<KMime::Message::Ptr>()) {
        qCWarning(KOLABRESOURCE_LOG) << "Payload is not a MessagePtr!";
        Q_ASSERT(false);
        ok = false;
        return imapItem;
    }

    const auto payload = imapItem.payload<KMime::Message::Ptr>();
    const Kolab::KolabObjectReader reader(payload);
    if (checkForErrors(imapItem)) {
        ok = true;
        // We return an error object so the sync keeps working, and we can clean up the mess by simply deleting the object in the application.
        return getErrorItem(folderType, imapItem.remoteId());
    }
    switch (reader.getType()) {
    case Kolab::EventObject:
    case Kolab::TodoObject:
    case Kolab::JournalObject: {
        const KCalendarCore::Incidence::Ptr incidencePtr = reader.getIncidence();
        if (!incidencePtr) {
            qCWarning(KOLABRESOURCE_LOG) << "Failed to read incidence.";
            ok = false;
            return {};
        }
        Akonadi::Item newItem(incidencePtr->mimeType());
        newItem.setPayload(incidencePtr);
        newItem.setRemoteId(imapItem.remoteId());
        newItem.setGid(incidencePtr->instanceIdentifier());
        return newItem;
    }
    case Kolab::NoteObject: {
        const KMime::Message::Ptr note = reader.getNote();
        if (!note) {
            qCWarning(KOLABRESOURCE_LOG) << "Failed to read note.";
            ok = false;
            return {};
        }
        Akonadi::Item newItem(QStringLiteral("text/x-vnd.akonadi.note"));
        newItem.setPayload(note);
        newItem.setRemoteId(imapItem.remoteId());
        const Akonadi::NoteUtils::NoteMessageWrapper wrapper(note);
        newItem.setGid(wrapper.uid());
        return newItem;
    }
    case Kolab::ContactObject: {
        Akonadi::Item newItem(KContacts::Addressee::mimeType());
        newItem.setPayload(reader.getContact());
        newItem.setRemoteId(imapItem.remoteId());
        newItem.setGid(reader.getContact().uid());
        return newItem;
    }
    case Kolab::DistlistObject: {
        KContacts::ContactGroup contactGroup = reader.getDistlist();

        QList<KContacts::ContactGroup::ContactReference> toAdd;
        for (int index = 0; index < contactGroup.contactReferenceCount(); ++index) {
            const KContacts::ContactGroup::ContactReference &reference = contactGroup.contactReference(index);
            KContacts::ContactGroup::ContactReference ref;
            ref.setGid(reference.uid()); // libkolab set a gid with setUid()
            toAdd << ref;
        }
        contactGroup.removeAllContactReferences();
        for (const KContacts::ContactGroup::ContactReference &ref : std::as_const(toAdd)) {
            contactGroup.append(ref);
        }

        Akonadi::Item newItem(KContacts::ContactGroup::mimeType());
        newItem.setPayload(contactGroup);
        newItem.setRemoteId(imapItem.remoteId());
        newItem.setGid(contactGroup.id());
        return newItem;
    }
    case Kolab::RelationConfigurationObject:
        // Do nothing about tags and relations, this is handled separately in KolabRetrieveTagTask::onMessagesAvailable
        ok = false;
        break;
    default:
        qCWarning(KOLABRESOURCE_LOG) << "Object type not handled";
        ok = false;
        break;
    }
    return {};
}

Akonadi::Item::List KolabHelpers::translateToImap(const Akonadi::Item::List &items, bool &ok)
{
    Akonadi::Item::List imapItems;
    imapItems.reserve(items.count());
    for (const Akonadi::Item &item : items) {
        bool translationOk = true;
        imapItems << translateToImap(item, translationOk);
        if (!translationOk) {
            ok = false;
        }
    }
    return imapItems;
}

static KContacts::ContactGroup convertToGidOnly(const KContacts::ContactGroup &contactGroup)
{
    QList<KContacts::ContactGroup::ContactReference> toAdd;
    for (int index = 0; index < contactGroup.contactReferenceCount(); ++index) {
        const KContacts::ContactGroup::ContactReference &reference = contactGroup.contactReference(index);
        QString gid;
        if (!reference.gid().isEmpty()) {
            gid = reference.gid();
        } else {
            // WARNING: this is an ugly hack for backwards compatibility. Normally this codepath shouldn't be hit.
            // Replace all references with real data-sets
            // Hopefully all resources are available during saving, so we can look up
            // in the addressbook to get name+email from the UID.

            const Akonadi::Item item(reference.uid().toLongLong());
            auto job = new Akonadi::ItemFetchJob(item);
            job->fetchScope().fetchFullPayload();
            if (!job->exec()) {
                continue;
            }

            const Akonadi::Item::List items = job->items();
            if (items.count() != 1) {
                continue;
            }
            const auto addressee = job->items().at(0).payload<KContacts::Addressee>();
            gid = addressee.uid();
        }
        KContacts::ContactGroup::ContactReference ref;
        ref.setUid(gid); // libkolab expects a gid for uid()
        toAdd << ref;
    }
    KContacts::ContactGroup gidOnlyContactGroup = contactGroup;
    gidOnlyContactGroup.removeAllContactReferences();
    for (const KContacts::ContactGroup::ContactReference &ref : std::as_const(toAdd)) {
        gidOnlyContactGroup.append(ref);
    }
    return gidOnlyContactGroup;
}

Akonadi::Item KolabHelpers::translateToImap(const Akonadi::Item &item, bool &ok)
{
    ok = true;
    // imap messages don't need to be translated
    if (item.mimeType() == KMime::Message::mimeType()) {
        Q_ASSERT(item.hasPayload<KMime::Message::Ptr>());
        return item;
    }
    const QLatin1String productId("Akonadi-Kolab-Resource");
    // Everything stays the same, except mime type and payload
    Akonadi::Item imapItem = item;
    imapItem.setMimeType(QStringLiteral("message/rfc822"));
    try {
        switch (getKolabTypeFromMimeType(item.mimeType())) {
        case Kolab::EventObject:
        case Kolab::TodoObject:
        case Kolab::JournalObject: {
            qCDebug(KOLABRESOURCE_LOG) << "converted event";
            const KMime::Message::Ptr message =
                Kolab::KolabObjectWriter::writeIncidence(item.payload<KCalendarCore::Incidence::Ptr>(), Kolab::KolabV3, productId, QStringLiteral("UTC"));
            imapItem.setPayload(message);
            break;
        }
        case Kolab::NoteObject: {
            qCDebug(KOLABRESOURCE_LOG) << "converted note";
            const KMime::Message::Ptr message = Kolab::KolabObjectWriter::writeNote(item.payload<KMime::Message::Ptr>(), Kolab::KolabV3, productId);
            imapItem.setPayload(message);
            break;
        }
        case Kolab::ContactObject: {
            qCDebug(KOLABRESOURCE_LOG) << "converted contact";
            const KMime::Message::Ptr message = Kolab::KolabObjectWriter::writeContact(item.payload<KContacts::Addressee>(), Kolab::KolabV3, productId);
            imapItem.setPayload(message);
            break;
        }
        case Kolab::DistlistObject: {
            const KContacts::ContactGroup contactGroup = convertToGidOnly(item.payload<KContacts::ContactGroup>());
            qCDebug(KOLABRESOURCE_LOG) << "converted distlist";
            const KMime::Message::Ptr message = Kolab::KolabObjectWriter::writeDistlist(contactGroup, Kolab::KolabV3, productId);
            imapItem.setPayload(message);
            break;
        }
        default:
            qCWarning(KOLABRESOURCE_LOG) << "object type not handled: " << item.id() << item.mimeType();
            ok = false;
            return {};
        }
    } catch (const Akonadi::PayloadException &e) {
        qCWarning(KOLABRESOURCE_LOG) << "The item contains the wrong or no payload: " << item.id() << item.mimeType();
        qCWarning(KOLABRESOURCE_LOG) << e.what();
        return {};
    }

    if (checkForErrors(item)) {
        qCWarning(KOLABRESOURCE_LOG) << "an error occurred while trying to translate the item to the kolab format: " << item.id();
        ok = false;
        return {};
    }
    return imapItem;
}

QByteArray KolabHelpers::kolabTypeForMimeType(const QStringList &contentMimeTypes)
{
    if (contentMimeTypes.contains(KContacts::Addressee::mimeType())) {
        return QByteArrayLiteral("contact");
    } else if (contentMimeTypes.contains(KCalendarCore::Event::eventMimeType())) {
        return QByteArrayLiteral("event");
    } else if (contentMimeTypes.contains(KCalendarCore::Todo::todoMimeType())) {
        return QByteArrayLiteral("task");
    } else if (contentMimeTypes.contains(KCalendarCore::Journal::journalMimeType())) {
        return QByteArrayLiteral("journal");
    } else if (contentMimeTypes.contains(QLatin1String("application/x-vnd.akonadi.note"))
               || contentMimeTypes.contains(QLatin1String("text/x-vnd.akonadi.note"))) {
        return QByteArrayLiteral("note");
    }
    return {};
}

Kolab::ObjectType KolabHelpers::getKolabTypeFromMimeType(const QString &type)
{
    if (type == KCalendarCore::Event::eventMimeType()) {
        return Kolab::EventObject;
    } else if (type == KCalendarCore::Todo::todoMimeType()) {
        return Kolab::TodoObject;
    } else if (type == KCalendarCore::Journal::journalMimeType()) {
        return Kolab::JournalObject;
    } else if (type == KContacts::Addressee::mimeType()) {
        return Kolab::ContactObject;
    } else if (type == KContacts::ContactGroup::mimeType()) {
        return Kolab::DistlistObject;
    } else if (type == QLatin1String("text/x-vnd.akonadi.note") || type == QLatin1String("application/x-vnd.akonadi.note")) {
        return Kolab::NoteObject;
    }
    return Kolab::InvalidObject;
}

QString KolabHelpers::getMimeType(Kolab::FolderType type)
{
    switch (type) {
    case Kolab::MailType:
        return KMime::Message::mimeType();
    case Kolab::ConfigurationType:
        return QStringLiteral(KOLAB_TYPE_RELATION);
    default:
        qCDebug(KOLABRESOURCE_LOG) << "unhandled folder type: " << type;
    }
    return {};
}

QStringList KolabHelpers::getContentMimeTypes(Kolab::FolderType type)
{
    QStringList contentTypes;
    contentTypes << Akonadi::Collection::mimeType();
    switch (type) {
    case Kolab::EventType:
        contentTypes << KCalendarCore::Event().mimeType();
        break;
    case Kolab::TaskType:
        contentTypes << KCalendarCore::Todo().mimeType();
        break;
    case Kolab::JournalType:
        contentTypes << KCalendarCore::Journal().mimeType();
        break;
    case Kolab::ContactType:
        contentTypes << KContacts::Addressee::mimeType() << KContacts::ContactGroup::mimeType();
        break;
    case Kolab::NoteType:
        contentTypes << QStringLiteral("text/x-vnd.akonadi.note") << QStringLiteral("application/x-vnd.akonadi.note");
        break;
    case Kolab::MailType:
        contentTypes << KMime::Message::mimeType();
        break;
    case Kolab::ConfigurationType:
        contentTypes << QStringLiteral(KOLAB_TYPE_RELATION);
        break;
    default:
        break;
    }
    return contentTypes;
}

Kolab::FolderType KolabHelpers::folderTypeFromString(const QByteArray &folderTypeName)
{
    const QByteArray stripped = folderTypeName.split('.').first();
    return Kolab::folderTypeFromString(std::string(stripped.data(), stripped.size()));
}

QByteArray KolabHelpers::getFolderTypeAnnotation(const QMap<QByteArray, QByteArray> &annotations)
{
    if (annotations.contains("/shared" KOLAB_FOLDER_TYPE_ANNOTATION) && !annotations.value("/shared" KOLAB_FOLDER_TYPE_ANNOTATION).isEmpty()) {
        return annotations.value("/shared" KOLAB_FOLDER_TYPE_ANNOTATION);
    } else if (annotations.contains("/private" KOLAB_FOLDER_TYPE_ANNOTATION) && !annotations.value("/private" KOLAB_FOLDER_TYPE_ANNOTATION).isEmpty()) {
        return annotations.value("/private" KOLAB_FOLDER_TYPE_ANNOTATION);
    }
    return annotations.value(KOLAB_FOLDER_TYPE_ANNOTATION);
}

void KolabHelpers::setFolderTypeAnnotation(QMap<QByteArray, QByteArray> &annotations, const QByteArray &value)
{
    annotations["/shared" KOLAB_FOLDER_TYPE_ANNOTATION] = value;
}

QColor KolabHelpers::getFolderColor(const QMap<QByteArray, QByteArray> &annotations)
{
    // kolab saves the color without a "#", so we need to add it to the rgb string to have a proper QColor
    if (annotations.contains("/shared" KOLAB_COLOR_ANNOTATION) && !annotations.value("/shared" KOLAB_COLOR_ANNOTATION).isEmpty()) {
        return QColor(QStringLiteral("#").append(QString::fromUtf8(annotations.value("/shared" KOLAB_COLOR_ANNOTATION))));
    } else if (annotations.contains("/private" KOLAB_COLOR_ANNOTATION) && !annotations.value("/private" KOLAB_COLOR_ANNOTATION).isEmpty()) {
        return QColor(QStringLiteral("#").append(QString::fromUtf8(annotations.value("/private" KOLAB_COLOR_ANNOTATION))));
    }
    return {};
}

void KolabHelpers::setFolderColor(QMap<QByteArray, QByteArray> &annotations, const QColor &color)
{
    // kolab saves the color without a "#", so we need to delete the prefix "#" if we save it to the annotations
    annotations["/shared" KOLAB_COLOR_ANNOTATION] = color.name().toLatin1().remove(0, 1);
}

QString KolabHelpers::getIcon(Kolab::FolderType type)
{
    switch (type) {
    case Kolab::EventType:
    case Kolab::TaskType:
    case Kolab::JournalType:
        return QStringLiteral("view-calendar");
    case Kolab::ContactType:
        return QStringLiteral("view-pim-contacts");
    case Kolab::NoteType:
        return QStringLiteral("view-pim-notes");
    case Kolab::MailType:
    case Kolab::ConfigurationType:
    case Kolab::FreebusyType:
    case Kolab::FileType:
    case Kolab::LastType:
        return {};
    }
    return {};
}

bool KolabHelpers::isHandledType(Kolab::FolderType type)
{
    switch (type) {
    case Kolab::EventType:
    case Kolab::TaskType:
    case Kolab::JournalType:
    case Kolab::ContactType:
    case Kolab::NoteType:
    case Kolab::MailType:
        return true;
    case Kolab::ConfigurationType:
    case Kolab::FreebusyType:
    case Kolab::FileType:
    case Kolab::LastType:
        return false;
    }
    return false;
}

QList<QByteArray> KolabHelpers::ancestorChain(const Akonadi::Collection &col)
{
    Q_ASSERT(col.isValid());
    if (col.parentCollection() == Akonadi::Collection::root() || col == Akonadi::Collection::root() || !col.isValid()) {
        return {};
    }
    QList<QByteArray> ancestors = ancestorChain(col.parentCollection());
    Q_ASSERT(!col.remoteId().isEmpty());
    ancestors << col.remoteId().toLatin1().mid(1); // We strip the first character which is always the separator
    return ancestors;
}

QString KolabHelpers::createMemberUrl(const Akonadi::Item &item, const QString &user)
{
    qCDebug(KOLABRESOURCE_TRACE) << item.id() << item.mimeType() << item.gid() << item.hasPayload();
    Kolab::RelationMember member;
    if (item.mimeType() == KMime::Message::mimeType()) {
        if (!item.hasPayload<KMime::Message::Ptr>()) {
            qCWarning(KOLABRESOURCE_LOG) << "Email without payload, failed to add to tag: " << item.id() << item.remoteId();
            return {};
        }
        auto msg = item.payload<KMime::Message::Ptr>();
        member.uid = item.remoteId().toLong();
        member.user = user;
        member.subject = msg->subject()->asUnicodeString();
        member.messageId = msg->messageID()->asUnicodeString();
        member.date = msg->date()->asUnicodeString();
        member.mailbox = ancestorChain(item.parentCollection());
    } else {
        if (item.gid().isEmpty()) {
            qCWarning(KOLABRESOURCE_LOG) << "Groupware object without GID, failed to add to tag: " << item.id() << item.remoteId();
            return {};
        }
        member.gid = item.gid();
    }
    return Kolab::generateMemberUrl(member);
}
