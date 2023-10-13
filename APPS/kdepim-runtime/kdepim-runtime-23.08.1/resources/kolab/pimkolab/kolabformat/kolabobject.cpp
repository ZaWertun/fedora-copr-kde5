/*
 * SPDX-FileCopyrightText: 2012 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "kolabobject.h"
#include "libkolab-version.h"
#include "pimkolab_debug.h"
#include "v2helpers.h"

#include <Akonadi/NoteUtils>

#include <conversion/commonconversion.h>
#include <conversion/kabcconversion.h>
#include <conversion/kcalconversion.h>
#include <conversion/kolabconversion.h>
#include <kolabformat.h>
#include <kolabformat/mimeobject.h>
#include <kolabformatV2/contact.h>
#include <kolabformatV2/distributionlist.h>
#include <kolabformatV2/event.h>
#include <kolabformatV2/journal.h>
#include <kolabformatV2/note.h>
#include <kolabformatV2/task.h>
#include <mime/mimeutils.h>

#include <QUrlQuery>

namespace Kolab
{
static inline QString eventKolabType()
{
    return QStringLiteral(KOLAB_TYPE_EVENT);
}

static inline QString todoKolabType()
{
    return QStringLiteral(KOLAB_TYPE_TASK);
}

static inline QString journalKolabType()
{
    return QStringLiteral(KOLAB_TYPE_JOURNAL);
}

static inline QString contactKolabType()
{
    return QStringLiteral(KOLAB_TYPE_CONTACT);
}

static inline QString distlistKolabType()
{
    return QStringLiteral(KOLAB_TYPE_DISTLIST);
}

static inline QString distlistKolabTypeCompat()
{
    return QStringLiteral(KOLAB_TYPE_DISTLIST_V2);
}

static inline QString noteKolabType()
{
    return QStringLiteral(KOLAB_TYPE_NOTE);
}

static inline QString configurationKolabType()
{
    return QStringLiteral(KOLAB_TYPE_CONFIGURATION);
}

static inline QString dictKolabType()
{
    return QStringLiteral(KOLAB_TYPE_DICT);
}

static inline QString freebusyKolabType()
{
    return QStringLiteral(KOLAB_TYPE_FREEBUSY);
}

static inline QString relationKolabType()
{
    return QStringLiteral(KOLAB_TYPE_RELATION);
}

static inline QString xCalMimeType()
{
    return QStringLiteral(MIME_TYPE_XCAL);
}

static inline QString xCardMimeType()
{
    return QStringLiteral(MIME_TYPE_XCARD);
}

static inline QString kolabMimeType()
{
    return QStringLiteral(MIME_TYPE_KOLAB);
}

KCalendarCore::Event::Ptr readV2EventXML(const QByteArray &xmlData, QStringList &attachments)
{
    return fromXML<KCalendarCore::Event::Ptr, KolabV2::Event>(xmlData, attachments);
}

QString ownUrlDecode(QByteArray encodedParam)
{
    encodedParam.replace('+', ' ');
    return QUrl::fromPercentEncoding(encodedParam);
}

RelationMember parseMemberUrl(const QString &string)
{
    if (string.startsWith(QLatin1String("urn:uuid:"))) {
        RelationMember member;
        member.gid = string.mid(9);
        return member;
    }
    QUrl url(QUrl::fromEncoded(string.toLatin1()));
    QList<QByteArray> path;
    const QList<QByteArray> fragments = url.path(QUrl::FullyEncoded).toLatin1().split('/');
    path.reserve(fragments.count());
    for (const QByteArray &fragment : fragments) {
        path.append(ownUrlDecode(fragment).toUtf8());
    }
    // qCDebug(PIMKOLAB_LOG) << path;
    bool isShared = false;
    int start = path.indexOf("user");
    if (start < 0) {
        start = path.indexOf("shared");
        isShared = true;
    }
    if (start < 0) {
        qCWarning(PIMKOLAB_LOG) << R"(Couldn't find "user" or "shared" in path: )" << path;
        return {};
    }
    path = path.mid(start + 1);
    if (path.size() < 2) {
        qCWarning(PIMKOLAB_LOG) << "Incomplete path: " << path;
        return {};
    }
    RelationMember member;
    if (!isShared) {
        member.user = QString::fromUtf8(path.takeFirst());
    }
    member.uid = path.takeLast().toLong();
    member.mailbox = path;
    QUrlQuery query(url);
    member.messageId = ownUrlDecode(query.queryItemValue(QStringLiteral("message-id"), QUrl::FullyEncoded).toUtf8());
    member.subject = ownUrlDecode(query.queryItemValue(QStringLiteral("subject"), QUrl::FullyEncoded).toUtf8());
    member.date = ownUrlDecode(query.queryItemValue(QStringLiteral("date"), QUrl::FullyEncoded).toUtf8());
    // qCDebug(PIMKOLAB_LOG) << member.uid << member.mailbox;
    return member;
}

static QByteArray join(const QList<QByteArray> &list, const QByteArray &c)
{
    QByteArray result;
    for (const QByteArray &a : list) {
        result += a + c;
    }
    result.chop(c.size());
    return result;
}

KOLAB_EXPORT QString generateMemberUrl(const RelationMember &member)
{
    if (!member.gid.isEmpty()) {
        return QStringLiteral("urn:uuid:%1").arg(member.gid);
    }
    QUrl url;
    url.setScheme(QStringLiteral("imap"));
    QList<QByteArray> path;
    path << "/";
    if (!member.user.isEmpty()) {
        path << "user";
        path << QUrl::toPercentEncoding(member.user);
    } else {
        path << "shared";
    }
    const auto memberMailbox{member.mailbox};
    for (const QByteArray &mb : memberMailbox) {
        path << QUrl::toPercentEncoding(QString::fromUtf8(mb));
    }
    path << QByteArray::number(member.uid);
    url.setPath(QString::fromUtf8('/' + join(path, "/")), QUrl::TolerantMode);

    QUrlQuery query;
    query.addQueryItem(QStringLiteral("message-id"), member.messageId);
    query.addQueryItem(QStringLiteral("subject"), member.subject);
    query.addQueryItem(QStringLiteral("date"), member.date);
    url.setQuery(query);

    return QString::fromLatin1(url.toEncoded());
}

//@cond PRIVATE
class KolabObjectReaderPrivate
{
public:
    KolabObjectReaderPrivate()
        : mAddressee(KContacts::Addressee())
        , mObjectType(InvalidObject)
        , mVersion(KolabV3)
        , mOverrideObjectType(InvalidObject)
    {
    }

    KCalendarCore::Incidence::Ptr mIncidence;
    KContacts::Addressee mAddressee;
    KContacts::ContactGroup mContactGroup;
    KMime::Message::Ptr mNote;
    QStringList mDictionary;
    QString mDictionaryLanguage;
    ObjectType mObjectType;
    Version mVersion;
    Kolab::Freebusy mFreebusy;
    ObjectType mOverrideObjectType;
    Version mOverrideVersion;
    bool mDoOverrideVersion = false;
    Akonadi::Relation mRelation;
    Akonadi::Tag mTag;
    QStringList mTagMembers;
};
//@endcond

KolabObjectReader::KolabObjectReader()
    : d(new KolabObjectReaderPrivate)
{
}

KolabObjectReader::KolabObjectReader(const KMime::Message::Ptr &msg)
    : d(new KolabObjectReaderPrivate)
{
    parseMimeMessage(msg);
}

KolabObjectReader::~KolabObjectReader() = default;

void KolabObjectReader::setObjectType(ObjectType type)
{
    d->mOverrideObjectType = type;
}

void KolabObjectReader::setVersion(Version version)
{
    d->mOverrideVersion = version;
    d->mDoOverrideVersion = true;
}

void printMessageDebugInfo(const KMime::Message::Ptr &msg)
{
    // TODO replace by Debug stream for Mimemessage
    qCDebug(PIMKOLAB_LOG) << "MessageId: " << msg->messageID()->asUnicodeString();
    qCDebug(PIMKOLAB_LOG) << "Subject: " << msg->subject()->asUnicodeString();
    //     Debug() << msg->encodedContent();
}

ObjectType KolabObjectReader::parseMimeMessage(const KMime::Message::Ptr &msg)
{
    ErrorHandler::clearErrors();
    d->mObjectType = InvalidObject;
    if (msg->contents().isEmpty()) {
        qCCritical(PIMKOLAB_LOG) << "message has no contents (we likely failed to parse it correctly)";
        printMessageDebugInfo(msg);
        return InvalidObject;
    }
    const std::string message = msg->encodedContent().toStdString();
    Kolab::MIMEObject mimeObject;
    mimeObject.setObjectType(d->mOverrideObjectType);
    if (d->mDoOverrideVersion) {
        mimeObject.setVersion(d->mOverrideVersion);
    }
    d->mObjectType = mimeObject.parseMessage(message);
    d->mVersion = mimeObject.getVersion();
    switch (mimeObject.getType()) {
    case EventObject: {
        const Kolab::Event &event = mimeObject.getEvent();
        d->mIncidence = Kolab::Conversion::toKCalendarCore(event);
        break;
    }
    case TodoObject: {
        const Kolab::Todo &event = mimeObject.getTodo();
        d->mIncidence = Kolab::Conversion::toKCalendarCore(event);
        break;
    }
    case JournalObject: {
        const Kolab::Journal &event = mimeObject.getJournal();
        d->mIncidence = Kolab::Conversion::toKCalendarCore(event);
        break;
    }
    case ContactObject: {
        const Kolab::Contact &contact = mimeObject.getContact();
        d->mAddressee = Kolab::Conversion::toKABC(contact); // TODO extract attachments
        break;
    }
    case DistlistObject: {
        const Kolab::DistList &distlist = mimeObject.getDistlist();
        d->mContactGroup = Kolab::Conversion::toKABC(distlist);
        break;
    }
    case NoteObject: {
        const Kolab::Note &note = mimeObject.getNote();
        d->mNote = Kolab::Conversion::toNote(note);
        break;
    }
    case DictionaryConfigurationObject: {
        const Kolab::Configuration &configuration = mimeObject.getConfiguration();
        const Kolab::Dictionary &dictionary = configuration.dictionary();
        d->mDictionary.clear();
        const std::vector<std::string> entries = dictionary.entries();
        d->mDictionary.reserve(entries.size());
        for (const std::string &entry : entries) {
            d->mDictionary.append(Conversion::fromStdString(entry));
        }
        d->mDictionaryLanguage = Conversion::fromStdString(dictionary.language());
        break;
    }
    case FreebusyObject: {
        const Kolab::Freebusy &fb = mimeObject.getFreebusy();
        d->mFreebusy = fb;
        break;
    }
    case RelationConfigurationObject: {
        const Kolab::Configuration &configuration = mimeObject.getConfiguration();
        const Kolab::Relation &relation = configuration.relation();

        if (relation.type() == "tag") {
            d->mTag = Akonadi::Tag();
            d->mTag.setName(Conversion::fromStdString(relation.name()));
            d->mTag.setGid(Conversion::fromStdString(configuration.uid()).toLatin1());
            d->mTag.setType(Akonadi::Tag::GENERIC);

            d->mTagMembers.reserve(relation.members().size());
            const auto members{relation.members()};
            for (const std::string &member : members) {
                d->mTagMembers << Conversion::fromStdString(member);
            }
        } else if (relation.type() == "generic") {
            if (relation.members().size() == 2) {
                d->mRelation = Akonadi::Relation();
                d->mRelation.setRemoteId(Conversion::fromStdString(configuration.uid()).toLatin1());
                d->mRelation.setType(Akonadi::Relation::GENERIC);

                d->mTagMembers.reserve(relation.members().size());
                const auto members{relation.members()};
                for (const std::string &member : members) {
                    d->mTagMembers << Conversion::fromStdString(member);
                }
            } else {
                qCCritical(PIMKOLAB_LOG) << "generic relation had wrong number of members:" << relation.members().size();
                printMessageDebugInfo(msg);
            }
        } else {
            qCCritical(PIMKOLAB_LOG) << "unknown configuration object type" << relation.type();
            printMessageDebugInfo(msg);
        }
        break;
    }
    default:
        qCCritical(PIMKOLAB_LOG) << "no kolab object found ";
        printMessageDebugInfo(msg);
        break;
    }
    return d->mObjectType;
}

Version KolabObjectReader::getVersion() const
{
    return d->mVersion;
}

ObjectType KolabObjectReader::getType() const
{
    return d->mObjectType;
}

KCalendarCore::Event::Ptr KolabObjectReader::getEvent() const
{
    return d->mIncidence.dynamicCast<KCalendarCore::Event>();
}

KCalendarCore::Todo::Ptr KolabObjectReader::getTodo() const
{
    return d->mIncidence.dynamicCast<KCalendarCore::Todo>();
}

KCalendarCore::Journal::Ptr KolabObjectReader::getJournal() const
{
    return d->mIncidence.dynamicCast<KCalendarCore::Journal>();
}

KCalendarCore::Incidence::Ptr KolabObjectReader::getIncidence() const
{
    return d->mIncidence;
}

KContacts::Addressee KolabObjectReader::getContact() const
{
    return d->mAddressee;
}

KContacts::ContactGroup KolabObjectReader::getDistlist() const
{
    return d->mContactGroup;
}

KMime::Message::Ptr KolabObjectReader::getNote() const
{
    return d->mNote;
}

QStringList KolabObjectReader::getDictionary(QString &lang) const
{
    lang = d->mDictionaryLanguage;
    return d->mDictionary;
}

Freebusy KolabObjectReader::getFreebusy() const
{
    return d->mFreebusy;
}

bool KolabObjectReader::isTag() const
{
    return !d->mTag.gid().isEmpty();
}

Akonadi::Tag KolabObjectReader::getTag() const
{
    return d->mTag;
}

QStringList KolabObjectReader::getTagMembers() const
{
    return d->mTagMembers;
}

bool KolabObjectReader::isRelation() const
{
    return d->mRelation.isValid();
}

Akonadi::Relation KolabObjectReader::getRelation() const
{
    return d->mRelation;
}

static KMime::Message::Ptr createMimeMessage(const std::string &mimeMessage)
{
    KMime::Message::Ptr msg(new KMime::Message);
    msg->setContent(QByteArray(mimeMessage.c_str()));
    msg->parse();
    return msg;
}

KMime::Message::Ptr KolabObjectWriter::writeEvent(const KCalendarCore::Event::Ptr &i, Version v, const QString &productId, const QString &)
{
    ErrorHandler::clearErrors();
    if (!i) {
        qCCritical(PIMKOLAB_LOG) << "passed a null pointer";
        return {};
    }
    const Kolab::Event &event = Kolab::Conversion::fromKCalendarCore(*i);
    Kolab::MIMEObject mimeObject;
    const std::string mimeMessage = mimeObject.writeEvent(event, v, productId.toStdString());
    return createMimeMessage(mimeMessage);
}

KMime::Message::Ptr KolabObjectWriter::writeTodo(const KCalendarCore::Todo::Ptr &i, Version v, const QString &productId, const QString &)
{
    ErrorHandler::clearErrors();
    if (!i) {
        qCCritical(PIMKOLAB_LOG) << "passed a null pointer";
        return {};
    }
    const Kolab::Todo &todo = Kolab::Conversion::fromKCalendarCore(*i);
    Kolab::MIMEObject mimeObject;
    const std::string mimeMessage = mimeObject.writeTodo(todo, v, productId.toStdString());
    return createMimeMessage(mimeMessage);
}

KMime::Message::Ptr KolabObjectWriter::writeJournal(const KCalendarCore::Journal::Ptr &i, Version v, const QString &productId, const QString &)
{
    ErrorHandler::clearErrors();
    if (!i) {
        qCCritical(PIMKOLAB_LOG) << "passed a null pointer";
        return {};
    }
    const Kolab::Journal &journal = Kolab::Conversion::fromKCalendarCore(*i);
    Kolab::MIMEObject mimeObject;
    const std::string mimeMessage = mimeObject.writeJournal(journal, v, productId.toStdString());
    return createMimeMessage(mimeMessage);
}

KMime::Message::Ptr KolabObjectWriter::writeIncidence(const KCalendarCore::Incidence::Ptr &i, Version v, const QString &productId, const QString &tz)
{
    if (!i) {
        qCCritical(PIMKOLAB_LOG) << "passed a null pointer";
        return {};
    }
    switch (i->type()) {
    case KCalendarCore::IncidenceBase::TypeEvent:
        return writeEvent(i.dynamicCast<KCalendarCore::Event>(), v, productId, tz);
    case KCalendarCore::IncidenceBase::TypeTodo:
        return writeTodo(i.dynamicCast<KCalendarCore::Todo>(), v, productId, tz);
    case KCalendarCore::IncidenceBase::TypeJournal:
        return writeJournal(i.dynamicCast<KCalendarCore::Journal>(), v, productId, tz);
    default:
        qCCritical(PIMKOLAB_LOG) << "unknown incidence type";
    }
    return {};
}

KMime::Message::Ptr KolabObjectWriter::writeContact(const KContacts::Addressee &addressee, Version v, const QString &productId)
{
    ErrorHandler::clearErrors();
    const Kolab::Contact &contact = Kolab::Conversion::fromKABC(addressee);
    Kolab::MIMEObject mimeObject;
    const std::string mimeMessage = mimeObject.writeContact(contact, v, productId.toStdString());
    return createMimeMessage(mimeMessage);
}

KMime::Message::Ptr KolabObjectWriter::writeDistlist(const KContacts::ContactGroup &kDistList, Version v, const QString &productId)
{
    ErrorHandler::clearErrors();
    const Kolab::DistList &distlist = Kolab::Conversion::fromKABC(kDistList);
    Kolab::MIMEObject mimeObject;
    const std::string mimeMessage = mimeObject.writeDistlist(distlist, v, productId.toStdString());
    return createMimeMessage(mimeMessage);
}

KMime::Message::Ptr KolabObjectWriter::writeNote(const KMime::Message::Ptr &n, Version v, const QString &productId)
{
    ErrorHandler::clearErrors();
    if (!n) {
        qCCritical(PIMKOLAB_LOG) << "passed a null pointer";
        return {};
    }
    const Kolab::Note &note = Kolab::Conversion::fromNote(n);
    Kolab::MIMEObject mimeObject;
    const std::string mimeMessage = mimeObject.writeNote(note, v, productId.toStdString());
    return createMimeMessage(mimeMessage);
}

KMime::Message::Ptr KolabObjectWriter::writeDictionary(const QStringList &entries, const QString &lang, Version v, const QString &productId)
{
    ErrorHandler::clearErrors();

    Kolab::Dictionary dictionary(Conversion::toStdString(lang));
    std::vector<std::string> ent;
    ent.reserve(entries.count());
    for (const QString &e : entries) {
        ent.push_back(Conversion::toStdString(e));
    }
    dictionary.setEntries(ent);
    Kolab::Configuration configuration(dictionary); // TODO preserve creation/lastModified date
    Kolab::MIMEObject mimeObject;
    const std::string mimeMessage = mimeObject.writeConfiguration(configuration, v, productId.toStdString());
    return createMimeMessage(mimeMessage);
}

KMime::Message::Ptr KolabObjectWriter::writeFreebusy(const Freebusy &freebusy, Version v, const QString &productId)
{
    ErrorHandler::clearErrors();
    Kolab::MIMEObject mimeObject;
    const std::string mimeMessage = mimeObject.writeFreebusy(freebusy, v, productId.toStdString());
    return createMimeMessage(mimeMessage);
}

KMime::Message::Ptr writeRelationHelper(const Kolab::Relation &relation, const QByteArray &uid, const QString &productId)
{
    ErrorHandler::clearErrors();
    Kolab::MIMEObject mimeObject;

    Kolab::Configuration configuration(relation); // TODO preserve creation/lastModified date
    configuration.setUid(uid.constData());
    const std::string mimeMessage = mimeObject.writeConfiguration(configuration, Kolab::KolabV3, Conversion::toStdString(productId));
    return createMimeMessage(mimeMessage);
}

KMime::Message::Ptr KolabObjectWriter::writeTag(const Akonadi::Tag &tag, const QStringList &members, Version v, const QString &productId)
{
    ErrorHandler::clearErrors();
    if (v != KolabV3) {
        qCCritical(PIMKOLAB_LOG) << "only v3 implementation available";
    }

    Kolab::Relation relation(Conversion::toStdString(tag.name()), "tag");
    std::vector<std::string> m;
    m.reserve(members.count());
    for (const QString &member : members) {
        m.push_back(Conversion::toStdString(member));
    }
    relation.setMembers(m);

    return writeRelationHelper(relation, tag.gid(), productId);
}

KMime::Message::Ptr KolabObjectWriter::writeRelation(const Akonadi::Relation &relation, const QStringList &items, Version v, const QString &productId)
{
    ErrorHandler::clearErrors();
    if (v != KolabV3) {
        qCCritical(PIMKOLAB_LOG) << "only v3 implementation available";
    }

    if (items.size() != 2) {
        qCCritical(PIMKOLAB_LOG) << "Wrong number of members for generic relation.";
        return {};
    }

    Kolab::Relation kolabRelation(std::string(), "generic");
    std::vector<std::string> m;
    m.reserve(2);
    m.push_back(Conversion::toStdString(items.at(0)));
    m.push_back(Conversion::toStdString(items.at(1)));
    kolabRelation.setMembers(m);

    return writeRelationHelper(kolabRelation, relation.remoteId(), productId);
}
} // Namespace
