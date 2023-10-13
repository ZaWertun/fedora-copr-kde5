/*
 * SPDX-FileCopyrightText: 2012 Sofia Balicka <balicka@kolabsys.com>
 * SPDX-FileCopyrightText: 2014 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "mimeobject.h"
#include "conversion/commonconversion.h"
#include "conversion/kabcconversion.h"
#include "conversion/kcalconversion.h"
#include "conversion/kolabconversion.h"
#include "kolabformat/xmlobject.h"

#include "kolabformat/v2helpers.h"
#include "libkolab-version.h"
#include "mime/mimeutils.h"
#include "pimkolab_debug.h"

#include <KRandom>
#include <QString>
#include <cstring>

Q_DECLARE_METATYPE(Kolab::Event)
Q_DECLARE_METATYPE(Kolab::Todo)
Q_DECLARE_METATYPE(Kolab::Journal)
Q_DECLARE_METATYPE(Kolab::Contact)
Q_DECLARE_METATYPE(Kolab::DistList)
Q_DECLARE_METATYPE(Kolab::Note)
Q_DECLARE_METATYPE(Kolab::Freebusy)
Q_DECLARE_METATYPE(Kolab::Configuration)

static inline std::string eventKolabType()
{
    return std::string(KOLAB_TYPE_EVENT);
}

static inline std::string todoKolabType()
{
    return std::string(KOLAB_TYPE_TASK);
}

static inline std::string journalKolabType()
{
    return std::string(KOLAB_TYPE_JOURNAL);
}

static inline std::string contactKolabType()
{
    return std::string(KOLAB_TYPE_CONTACT);
}

static inline std::string distlistKolabType()
{
    return std::string(KOLAB_TYPE_DISTLIST);
}

static inline std::string distlistKolabTypeCompat()
{
    return std::string(KOLAB_TYPE_DISTLIST_V2);
}

static inline std::string noteKolabType()
{
    return std::string(KOLAB_TYPE_NOTE);
}

static inline std::string configurationKolabType()
{
    return std::string(KOLAB_TYPE_CONFIGURATION);
}

static inline std::string dictKolabType()
{
    return std::string(KOLAB_TYPE_DICT);
}

static inline std::string freebusyKolabType()
{
    return std::string(KOLAB_TYPE_FREEBUSY);
}

static inline std::string relationKolabType()
{
    return std::string(KOLAB_TYPE_RELATION);
}

static inline std::string xCalMimeType()
{
    return std::string(MIME_TYPE_XCAL);
}

static inline std::string xCardMimeType()
{
    return std::string(MIME_TYPE_XCARD);
}

static inline std::string kolabMimeType()
{
    return std::string(MIME_TYPE_KOLAB);
}

static std::string getProductId(const std::string &pId)
{
    if (pId.empty()) {
        return LIBKOLAB_LIB_VERSION_STRING;
    }
    return pId + ' ' + LIBKOLAB_LIB_VERSION_STRING;
}

namespace Kolab
{
static Kolab::ObjectType getObjectType(const std::string &type)
{
    if (type == eventKolabType()) {
        return EventObject;
    } else if (type == todoKolabType()) {
        return TodoObject;
    } else if (type == journalKolabType()) {
        return JournalObject;
    } else if (type == contactKolabType()) {
        return ContactObject;
    } else if (type == distlistKolabType() || type == distlistKolabTypeCompat()) {
        return DistlistObject;
    } else if (type == noteKolabType()) {
        return NoteObject;
    } else if (type == freebusyKolabType()) {
        return FreebusyObject;
    } else if (strstr(type.c_str(), KOLAB_TYPE_DICT)) { // Previous versions appended the language to the type
        return DictionaryConfigurationObject;
    } else if (type == relationKolabType()) {
        return RelationConfigurationObject;
    }
    qCWarning(PIMKOLAB_LOG) << "Unknown object type: " << type;
    return Kolab::InvalidObject;
}

static QByteArray getTypeString(Kolab::ObjectType type)
{
    switch (type) {
    case EventObject:
        return KOLAB_TYPE_EVENT;
    case TodoObject:
        return KOLAB_TYPE_TASK;
    case JournalObject:
        return KOLAB_TYPE_JOURNAL;
    case FreebusyObject:
        return KOLAB_TYPE_FREEBUSY;
    case ContactObject:
        return KOLAB_TYPE_CONTACT;
    case DistlistObject:
        return KOLAB_TYPE_DISTLIST;
    case NoteObject:
        return KOLAB_TYPE_NOTE;
    case DictionaryConfigurationObject:
        return KOLAB_TYPE_CONFIGURATION;
    case RelationConfigurationObject:
        return KOLAB_TYPE_RELATION;
    default:
        qCCritical(PIMKOLAB_LOG) << "unknown type " << type;
    }
    return {};
}

static QByteArray getMimeType(Kolab::ObjectType type)
{
    switch (type) {
    case EventObject:
    case TodoObject:
    case JournalObject:
    case FreebusyObject:
        return MIME_TYPE_XCAL;
    case ContactObject:
    case DistlistObject:
        return MIME_TYPE_XCARD;
    case NoteObject:
    case DictionaryConfigurationObject:
    case RelationConfigurationObject:
        return MIME_TYPE_KOLAB;
    default:
        qCCritical(PIMKOLAB_LOG) << "unknown type " << type;
    }
    return {};
}

static Kolab::ObjectType detectType(const KMime::Message::Ptr &msg)
{
    const auto mimetypes{Mime::getContentMimeTypeList(msg)};
    for (const QByteArray &type : mimetypes) {
        Kolab::ObjectType t = getObjectType(type.toStdString()); // works for v2 types
        if (t != InvalidObject) {
            return t;
        }
    }
    return InvalidObject;
}

static void printMessageDebugInfo(const KMime::Message::Ptr &msg)
{
    // TODO replace by Debug stream for Mimemessage
    qCDebug(PIMKOLAB_LOG) << "MessageId: " << msg->messageID()->asUnicodeString();
    qCDebug(PIMKOLAB_LOG) << "Subject: " << msg->subject()->asUnicodeString();
    //     Debug() << msg->encodedContent();
}

//@cond PRIVATE
class MIMEObjectPrivate
{
public:
    MIMEObjectPrivate() = default;

    QVariant readKolabV2(const KMime::Message::Ptr &msg, Kolab::ObjectType objectType);
    QVariant readKolabV3(const KMime::Message::Ptr &msg, Kolab::ObjectType objectType);
    QVariant parseMimeMessage(const KMime::Message::Ptr &msg);
    QVariant parseMimeMessage(const std::string &s);

    ObjectType mObjectType = InvalidObject;
    Version mVersion = KolabV3;
    ObjectType mOverrideObjectType = InvalidObject;
    Version mOverrideVersion = KolabV3;
    bool mDoOverrideVersion = false;
    QVariant mObject;
};
//@endcond

static std::vector<Kolab::Attachment> getAttachments(const std::vector<Kolab::Attachment> &attachments, const KMime::Message::Ptr &msg)
{
    std::vector<Kolab::Attachment> allAttachments;
    for (const Kolab::Attachment &attachment : attachments) {
        if (!attachment.uri().empty()) {
            const Kolab::Attachment extracted = Mime::getAttachment(attachment.uri(), msg);
            if (extracted.isValid()) {
                allAttachments.push_back(extracted);
            }
        } else {
            allAttachments.push_back(attachment);
        }
    }
    return allAttachments;
}

static std::vector<Kolab::Attachment> getAttachments(const QStringList &attachmentNames, const KMime::Message::Ptr &msg)
{
    std::vector<Kolab::Attachment> allAttachments;
    for (const QString &name : attachmentNames) {
        const Kolab::Attachment extracted = Mime::getAttachmentByName(name, msg);
        if (extracted.isValid()) {
            allAttachments.push_back(extracted);
        }
    }
    return allAttachments;
}

QVariant MIMEObjectPrivate::readKolabV2(const KMime::Message::Ptr &msg, Kolab::ObjectType objectType)
{
    if (objectType == DictionaryConfigurationObject) {
        KMime::Content *xmlContent = Mime::findContentByType(msg, "application/xml");
        if (!xmlContent) {
            qCCritical(PIMKOLAB_LOG) << "no application/xml part found";
            printMessageDebugInfo(msg);
            return InvalidObject;
        }
        const QByteArray &xmlData = xmlContent->decodedContent();
        QString dictionaryLanguage;
        const QStringList entries = Kolab::readLegacyDictionaryConfiguration(xmlData, dictionaryLanguage);
        mObjectType = objectType;
        Kolab::Dictionary dictionary(Conversion::toStdString(dictionaryLanguage));
        std::vector<std::string> convertedEntries;
        convertedEntries.reserve(entries.count());
        for (const QString &value : entries) {
            convertedEntries.push_back(Conversion::toStdString(value));
        }
        dictionary.setEntries(convertedEntries);
        return QVariant::fromValue(Kolab::Configuration(dictionary));
    }
    KMime::Content *xmlContent = Mime::findContentByType(msg, getTypeString(objectType));
    if (!xmlContent) {
        qCCritical(PIMKOLAB_LOG) << "no part with type" << getTypeString(objectType) << " found";
        printMessageDebugInfo(msg);
        return {};
    }
    const QByteArray &xmlData = xmlContent->decodedContent();
    if (xmlData.isEmpty()) {
        qCCritical(PIMKOLAB_LOG) << "no content in message part with type" << getTypeString(objectType);
        printMessageDebugInfo(msg);
        return {};
    }

    QVariant variant;
    switch (objectType) {
    case EventObject: {
        QStringList attachments;
        auto kEvent = fromXML<KCalendarCore::Event::Ptr, KolabV2::Event>(xmlData, attachments);
        if (kEvent) {
            Kolab::Event event = Kolab::Conversion::fromKCalendarCore(*kEvent);
            event.setAttachments(getAttachments(attachments, msg));
            variant = QVariant::fromValue(event);
        }
        break;
    }
    case TodoObject: {
        QStringList attachments;
        auto kTodo = fromXML<KCalendarCore::Todo::Ptr, KolabV2::Task>(xmlData, attachments);
        if (kTodo) {
            Kolab::Todo todo = Kolab::Conversion::fromKCalendarCore(*kTodo);
            todo.setAttachments(getAttachments(attachments, msg));
            variant = QVariant::fromValue(todo);
        }
        break;
    }
    case JournalObject: {
        QStringList attachments;
        auto kJournal = fromXML<KCalendarCore::Journal::Ptr, KolabV2::Journal>(xmlData, attachments);
        if (kJournal) {
            Kolab::Journal journal = Kolab::Conversion::fromKCalendarCore(*kJournal);
            journal.setAttachments(getAttachments(attachments, msg));
            variant = QVariant::fromValue(journal);
        }
        break;
    }
    case ContactObject: {
        KContacts::Addressee kContact = addresseeFromKolab(xmlData, msg);
        Kolab::Contact contact = Kolab::Conversion::fromKABC(kContact);
        variant = QVariant::fromValue(contact);
        break;
    }
    case DistlistObject: {
        KContacts::ContactGroup kContactGroup = contactGroupFromKolab(xmlData);
        Kolab::DistList distlist = Kolab::Conversion::fromKABC(kContactGroup);
        variant = QVariant::fromValue(distlist);
        break;
    }
    case NoteObject: {
        KMime::Message::Ptr kNote = noteFromKolab(xmlData, msg->date()->dateTime());
        Kolab::Note note = Kolab::Conversion::fromNote(kNote);
        variant = QVariant::fromValue(note);
        break;
    }
    default:
        CRITICAL(QStringLiteral("no kolab object found "));
        break;
    }
    if (ErrorHandler::errorOccured()) {
        printMessageDebugInfo(msg);
        return {};
    }
    mObjectType = objectType;
    return variant;
}

QVariant MIMEObjectPrivate::readKolabV3(const KMime::Message::Ptr &msg, Kolab::ObjectType objectType)
{
    KMime::Content *const xmlContent = Mime::findContentByType(msg, getMimeType(objectType));
    if (!xmlContent) {
        qCCritical(PIMKOLAB_LOG) << "no " << getMimeType(objectType) << " part found";
        printMessageDebugInfo(msg);
        return InvalidObject;
    }
    const QByteArray &content = xmlContent->decodedContent();
    const std::string xml = std::string(content.data(), content.size());
    QVariant variant;
    switch (objectType) {
    case EventObject: {
        Kolab::Event event = Kolab::readEvent(xml, false);
        event.setAttachments(getAttachments(event.attachments(), msg));
        variant = QVariant::fromValue<Kolab::Event>(event);
        break;
    }
    case TodoObject: {
        Kolab::Todo todo = Kolab::readTodo(xml, false);
        todo.setAttachments(getAttachments(todo.attachments(), msg));
        variant = QVariant::fromValue<Kolab::Todo>(todo);
        break;
    }
    case JournalObject: {
        Kolab::Journal journal = Kolab::readJournal(xml, false);
        journal.setAttachments(getAttachments(journal.attachments(), msg));
        variant = QVariant::fromValue<Kolab::Journal>(journal);
        break;
    }
    case ContactObject:
        variant = QVariant::fromValue<Kolab::Contact>(Kolab::readContact(xml, false));
        break;
    case DistlistObject:
        variant = QVariant::fromValue<Kolab::DistList>(Kolab::readDistlist(xml, false));
        break;
    case NoteObject:
        variant = QVariant::fromValue<Kolab::Note>(Kolab::readNote(xml, false));
        break;
    case FreebusyObject:
        variant = QVariant::fromValue<Kolab::Freebusy>(Kolab::readFreebusy(xml, false));
        break;
    case DictionaryConfigurationObject:
    case RelationConfigurationObject:
        variant = QVariant::fromValue<Kolab::Configuration>(Kolab::readConfiguration(xml, false));
        break;
    default:
        qCCritical(PIMKOLAB_LOG) << "no kolab object found ";
        printMessageDebugInfo(msg);
        break;
    }

    if (ErrorHandler::errorOccured()) {
        printMessageDebugInfo(msg);
        return {};
    }
    mObjectType = objectType;
    return variant;
}

QVariant MIMEObjectPrivate::parseMimeMessage(const KMime::Message::Ptr &msg)
{
    ErrorHandler::clearErrors();
    mObjectType = InvalidObject;
    if (msg->contents().isEmpty()) {
        qCCritical(PIMKOLAB_LOG) << "message has no contents (we likely failed to parse it correctly)";
        printMessageDebugInfo(msg);
        return {};
    }
    Kolab::ObjectType objectType = InvalidObject;
    if (mOverrideObjectType == InvalidObject) {
        if (KMime::Headers::Base *xKolabHeader = msg->headerByType(X_KOLAB_TYPE_HEADER)) {
            objectType = getObjectType(xKolabHeader->asUnicodeString().trimmed().toStdString());
        } else {
            qCWarning(PIMKOLAB_LOG) << "could not find the X-Kolab-Type Header, trying autodetection";
            // This works only for v2 messages atm.
            objectType = detectType(msg);
        }
    } else {
        objectType = mOverrideObjectType;
    }
    if (objectType == InvalidObject) {
        qCCritical(PIMKOLAB_LOG) << "unable to detect object type";
        printMessageDebugInfo(msg);
        return {};
    }

    if (!mDoOverrideVersion) {
        KMime::Headers::Base *xKolabVersion = msg->headerByType(X_KOLAB_MIME_VERSION_HEADER);
        if (!xKolabVersion) {
            // For backwards compatibility to development versions, can be removed in future versions
            xKolabVersion = msg->headerByType(X_KOLAB_MIME_VERSION_HEADER_COMPAT);
        }
        if (!xKolabVersion || xKolabVersion->asUnicodeString() == KOLAB_VERSION_V2) {
            mVersion = KolabV2;
        } else {
            if (xKolabVersion->asUnicodeString() != KOLAB_VERSION_V3) { // TODO version compatibility check?
                qCWarning(PIMKOLAB_LOG) << "Kolab Version Header available but not on the same version as the implementation: "
                                        << xKolabVersion->asUnicodeString();
            }
            mVersion = KolabV3;
        }
    } else {
        mVersion = mOverrideVersion;
    }

    if (mVersion == KolabV2) {
        return readKolabV2(msg, objectType);
    }
    return readKolabV3(msg, objectType);
}

QVariant MIMEObjectPrivate::parseMimeMessage(const std::string &s)
{
    KMime::Message::Ptr msg(new KMime::Message);
    msg->setContent(QByteArray(s.c_str()));
    msg->parse();
    return parseMimeMessage(msg);
}

MIMEObject::MIMEObject()
    : d(new MIMEObjectPrivate)
{
}

MIMEObject::~MIMEObject() = default;

void MIMEObject::setObjectType(ObjectType type)
{
    d->mOverrideObjectType = type;
}

void MIMEObject::setVersion(Version version)
{
    d->mOverrideVersion = version;
    d->mDoOverrideVersion = true;
}

static std::string createCid()
{
    return QStringLiteral("cid:%1@%2").arg(KRandom::randomString(16), QStringLiteral("kolab.resource.akonadi")).toStdString();
}

std::vector<Kolab::Attachment> convertToReferences(const std::vector<Kolab::Attachment> &attachments, std::vector<std::string> &attachmentCids)
{
    std::vector<Kolab::Attachment> attachmentsWithReferences;
    for (const Kolab::Attachment &a : attachments) {
        Kolab::Attachment attachment;
        attachment.setLabel(a.label());
        const std::string cid = a.uri().empty() ? createCid() : a.uri();
        attachmentCids.push_back(cid);
        attachment.setUri(cid, a.mimetype()); // Serialize the attachment as attachment with uri, referencing the created mime-part
        attachmentsWithReferences.push_back(attachment);
    }
    return attachmentsWithReferences;
}

template<class T>
static T convertAttachmentsToReferences(const T &incidence, std::vector<std::string> &attachmentCids)
{
    T removedAttachments = incidence;
    removedAttachments.setAttachments(convertToReferences(incidence.attachments(), attachmentCids));
    return removedAttachments;
}

static void addAttachments(KMime::Message::Ptr msg, const std::vector<Attachment> &attachments, std::vector<std::string> &attachmentCids)
{
    int index = 0;
    for (const Attachment &attachment : attachments) {
        const std::string data = attachment.data();
        const std::string cid = attachmentCids.empty() ? attachment.uri() : attachmentCids.at(index);
        msg->addContent(Mime::createAttachmentPart(Mime::fromCid(QString::fromStdString(cid.c_str())).toLatin1(),
                                                   QByteArray(attachment.mimetype().c_str()),
                                                   QString::fromStdString(attachment.label()),
                                                   QByteArray(data.c_str(), data.size())));
        index++;
    }
}

ObjectType MIMEObject::parseMessage(const std::string &msg)
{
    d->mObject = d->parseMimeMessage(msg);
    return d->mObjectType;
}

ObjectType MIMEObject::getType() const
{
    return d->mObjectType;
}

Version MIMEObject::getVersion() const
{
    return d->mVersion;
}

Kolab::Event MIMEObject::getEvent() const
{
    return d->mObject.value<Kolab::Event>();
}

Kolab::Todo MIMEObject::getTodo() const
{
    return d->mObject.value<Kolab::Todo>();
}

Kolab::Journal MIMEObject::getJournal() const
{
    return d->mObject.value<Kolab::Journal>();
}

Kolab::Note MIMEObject::getNote() const
{
    return d->mObject.value<Kolab::Note>();
}

Kolab::Contact MIMEObject::getContact() const
{
    return d->mObject.value<Kolab::Contact>();
}

Kolab::DistList MIMEObject::getDistlist() const
{
    return d->mObject.value<Kolab::DistList>();
}

Kolab::Freebusy MIMEObject::getFreebusy() const
{
    return d->mObject.value<Kolab::Freebusy>();
}

Kolab::Configuration MIMEObject::getConfiguration() const
{
    return d->mObject.value<Kolab::Configuration>();
}

std::string MIMEObject::writeEvent(const Event &event, Version version, const std::string &pId)
{
    ErrorHandler::clearErrors();
    const std::string productId = getProductId(pId);

    KMime::Message::Ptr msg;
    Kolab::XMLObject xmlObject;
    std::vector<std::string> attachmentCids;
    if (version == KolabV3) {
        const std::string xml = xmlObject.writeEvent(convertAttachmentsToReferences(event, attachmentCids), version, productId);
        msg = Mime::createMessage(xCalMimeType(), eventKolabType(), xml, true, productId, event.organizer().email(), event.organizer().name(), event.uid());
    } else if (version == KolabV2) {
        const std::string xml = xmlObject.writeEvent(event, version, productId);
        msg = Mime::createMessage(eventKolabType(), eventKolabType(), xml, false, productId, event.organizer().email(), event.organizer().name(), event.uid());
    }
    addAttachments(msg, event.attachments(), attachmentCids);
    msg->assemble();
    return msg->encodedContent().data();
}

Event MIMEObject::readEvent(const std::string &s)
{
    return d->parseMimeMessage(s).value<Kolab::Event>();
}

std::string MIMEObject::writeTodo(const Todo &todo, Version version, const std::string &pId)
{
    ErrorHandler::clearErrors();
    const std::string productId = getProductId(pId);

    KMime::Message::Ptr msg;
    Kolab::XMLObject xmlObject;
    std::vector<std::string> attachmentCids;
    if (version == KolabV3) {
        const std::string xml = xmlObject.writeTodo(convertAttachmentsToReferences(todo, attachmentCids), version, productId);
        msg = Mime::createMessage(xCalMimeType(), todoKolabType(), xml, true, productId, todo.organizer().email(), todo.organizer().name(), todo.uid());
    } else if (version == KolabV2) {
        const std::string xml = xmlObject.writeTodo(todo, version, productId);
        msg = Mime::createMessage(todoKolabType(), todoKolabType(), xml, false, productId, todo.organizer().email(), todo.organizer().name(), todo.uid());
    }
    addAttachments(msg, todo.attachments(), attachmentCids);
    msg->assemble();
    return msg->encodedContent().data();
}

Todo MIMEObject::readTodo(const std::string &s)
{
    return d->parseMimeMessage(s).value<Kolab::Todo>();
}

std::string MIMEObject::writeJournal(const Journal &journal, Version version, const std::string &pId)
{
    ErrorHandler::clearErrors();
    const std::string productId = getProductId(pId);

    KMime::Message::Ptr msg;
    Kolab::XMLObject xmlObject;
    std::vector<std::string> attachmentCids;
    if (version == KolabV3) {
        const std::string xml = xmlObject.writeJournal(convertAttachmentsToReferences(journal, attachmentCids), version, productId);
        msg = Mime::createMessage(xCalMimeType(), journalKolabType(), xml, true, productId, std::string(), std::string(), journal.uid());
    } else if (version == KolabV2) {
        const std::string xml = xmlObject.writeJournal(journal, version, productId);
        msg = Mime::createMessage(journalKolabType(), journalKolabType(), xml, false, productId, std::string(), std::string(), journal.uid());
    }
    addAttachments(msg, journal.attachments(), attachmentCids);
    msg->assemble();
    return msg->encodedContent().data();
}

Journal MIMEObject::readJournal(const std::string &s)
{
    return d->parseMimeMessage(s).value<Kolab::Journal>();
}

std::string MIMEObject::writeNote(const Note &note, Version version, const std::string &pId)
{
    ErrorHandler::clearErrors();
    const std::string productId = getProductId(pId);

    KMime::Message::Ptr msg;
    Kolab::XMLObject xmlObject;
    std::vector<std::string> attachmentCids;
    if (version == KolabV3) {
        const std::string xml = xmlObject.writeNote(convertAttachmentsToReferences(note, attachmentCids), version, productId);
        msg = Mime::createMessage(kolabMimeType(), noteKolabType(), xml, true, productId, std::string(), std::string(), note.uid());
    } else if (version == KolabV2) {
        const std::string xml = xmlObject.writeNote(note, version, productId);
        msg = Mime::createMessage(noteKolabType(), noteKolabType(), xml, false, productId, std::string(), std::string(), note.uid());
    }
    addAttachments(msg, note.attachments(), attachmentCids);
    msg->assemble();
    return msg->encodedContent().data();
}

Note MIMEObject::readNote(const std::string &s)
{
    return d->parseMimeMessage(s).value<Kolab::Note>();
}

std::string MIMEObject::writeContact(const Contact &contact, Version version, const std::string &pId)
{
    ErrorHandler::clearErrors();
    const std::string productId = getProductId(pId);

    KMime::Message::Ptr msg;
    Kolab::XMLObject xmlObject;
    const std::string xml = xmlObject.writeContact(contact, version, productId);

    Email preferredEmail = !contact.emailAddresses().empty() ? contact.emailAddresses().at(contact.emailAddressPreferredIndex()) : Email();
    QPair<std::string, std::string> pair = Conversion::fromMailto(preferredEmail.address());
    std::string name = pair.second;
    std::string email = pair.first;
    if (name.empty()) {
        name = contact.name();
    }

    if (version == KolabV3) {
        msg = Mime::createMessage(xCardMimeType(), contactKolabType(), xml, true, productId, email, name, contact.uid());
    } else if (version == KolabV2) {
        msg = Mime::createMessage(contactKolabType(), contactKolabType(), xml, false, productId, email, name, contact.uid());
    }
    msg->assemble();
    return msg->encodedContent().data();
}

Contact MIMEObject::readContact(const std::string &s)
{
    return d->parseMimeMessage(s).value<Kolab::Contact>();
}

std::string MIMEObject::writeDistlist(const DistList &distlist, Version version, const std::string &pId)
{
    ErrorHandler::clearErrors();
    const std::string productId = getProductId(pId);

    KMime::Message::Ptr msg;
    Kolab::XMLObject xmlObject;
    const std::string xml = xmlObject.writeDistlist(distlist, version, productId);
    if (version == KolabV3) {
        msg = Mime::createMessage(xCardMimeType(), distlistKolabType(), xml, true, productId, std::string(), std::string(), distlist.uid());
    } else if (version == KolabV2) {
        msg = Mime::createMessage(distlistKolabType(), distlistKolabType(), xml, false, productId, std::string(), std::string(), distlist.uid());
    }
    msg->assemble();
    return msg->encodedContent().data();
}

DistList MIMEObject::readDistlist(const std::string &s)
{
    return d->parseMimeMessage(s).value<Kolab::DistList>();
}

std::string MIMEObject::writeConfiguration(const Configuration &configuration, Version version, const std::string &pId)
{
    ErrorHandler::clearErrors();
    const std::string productId = getProductId(pId);

    KMime::Message::Ptr msg;
    Kolab::XMLObject xmlObject;
    const std::string xml = xmlObject.writeConfiguration(configuration, version, productId);
    std::string kolabType;
    switch (configuration.type()) {
    case Kolab::Configuration::TypeDictionary:
        kolabType = dictKolabType();
        break;
    case Kolab::Configuration::TypeRelation:
        kolabType = relationKolabType();
        break;
    case Kolab::Configuration::TypeSnippet:
        kolabType = configurationKolabType();
        break;
    case Kolab::Configuration::TypeFileDriver:
        kolabType = configurationKolabType();
        break;
    case Kolab::Configuration::TypeCategoryColor:
        kolabType = configurationKolabType();
        break;
    default:
        break;
    }
    if (version == KolabV3) {
        msg = Mime::createMessage(kolabMimeType(), kolabType, xml, true, productId, std::string(), std::string(), configuration.uid());
    } else if (version == KolabV2) {
        qCCritical(PIMKOLAB_LOG) << "Not available in KolabV2";
    }
    msg->assemble();
    return msg->encodedContent().data();
}

Configuration MIMEObject::readConfiguration(const std::string &s)
{
    return d->parseMimeMessage(s).value<Kolab::Configuration>();
}

std::string MIMEObject::writeFreebusy(const Freebusy &freebusy, Version version, const std::string &pId)
{
    ErrorHandler::clearErrors();
    const std::string productId = getProductId(pId);

    KMime::Message::Ptr msg;
    Kolab::XMLObject xmlObject;
    const std::string xml = xmlObject.writeFreebusy(freebusy, version, productId);
    if (version == KolabV3) {
        msg = Mime::createMessage(xCalMimeType(), freebusyKolabType(), xml, true, productId, std::string(), std::string(), freebusy.uid());
    } else if (version == KolabV2) {
        qCCritical(PIMKOLAB_LOG) << "Not available in KolabV2";
    }
    msg->assemble();
    return msg->encodedContent().data();
}

Freebusy MIMEObject::readFreebusy(const std::string &s)
{
    return d->parseMimeMessage(s).value<Kolab::Freebusy>();
}
}
