/*
 * SPDX-FileCopyrightText: 2012 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "xmlobject.h"
#include "conversion/commonconversion.h"
#include "conversion/kabcconversion.h"
#include "conversion/kcalconversion.h"
#include "conversion/kolabconversion.h"
#include "kolabformatV2/event.h"
#include "pimkolab_debug.h"
#include "v2helpers.h"
#include <QUuid>

namespace Kolab
{
static QString createUuid()
{
    const QString uuid = QUuid::createUuid().toString();
    return uuid.mid(1, uuid.size() - 2);
}

XMLObject::XMLObject() = default;

std::string XMLObject::getSerializedUID() const
{
    return mWrittenUID;
}

std::vector<std::string> XMLObject::getAttachments() const
{
    return mAttachments;
}

std::string XMLObject::writeEvent(const Event &event, Version version, const std::string &productId)
{
    mWrittenUID.clear();
    if (version == KolabV2) {
        const KCalendarCore::Event::Ptr i = Conversion::toKCalendarCore(event);
        if (!i) {
            qCCritical(PIMKOLAB_LOG) << "invalid incidence";
            return {};
        }
        if (i->uid().isEmpty()) {
            i->setUid(createUuid());
        }
        mWrittenUID = Conversion::toStdString(i->uid());
        // The timezone is used for created and last modified dates
        const QString &xml = KolabV2::Event::eventToXML(i, QStringLiteral("UTC"));
        return Conversion::toStdString(xml);
    }
    const std::string result = Kolab::writeEvent(event, productId);
    mWrittenUID = Kolab::getSerializedUID();
    ErrorHandler::handleLibkolabxmlErrors();
    return result;
}

Event XMLObject::readEvent(const std::string &s, Version version)
{
    if (version == KolabV2) {
        QStringList attachments;
        const auto event = Kolab::fromXML<KCalendarCore::Event::Ptr, KolabV2::Event>(QString::fromUtf8(s.c_str()).toUtf8(), attachments);
        if (!event || Kolab::ErrorHandler::errorOccured()) {
            qCCritical(PIMKOLAB_LOG) << "failed to read xml";
            return {};
        }
        mAttachments.clear();
        mAttachments.reserve(attachments.count());
        for (const QString &attachment : std::as_const(attachments)) {
            mAttachments.push_back(Conversion::toStdString(attachment));
        }
        return Conversion::fromKCalendarCore(*event);
    }
    const Kolab::Event event = Kolab::readEvent(s, false);
    ErrorHandler::handleLibkolabxmlErrors();
    return event;
}

std::string XMLObject::writeTodo(const Todo &event, Version version, const std::string &productId)
{
    mWrittenUID.clear();
    if (version == KolabV2) {
        const KCalendarCore::Todo::Ptr i = Conversion::toKCalendarCore(event);
        if (!i) {
            qCCritical(PIMKOLAB_LOG) << "invalid incidence";
            return {};
        }
        if (i->uid().isEmpty()) {
            i->setUid(createUuid());
        }
        mWrittenUID = Conversion::toStdString(i->uid());
        // The timezone is used for created and last modified dates
        const QString &xml = KolabV2::Task::taskToXML(i, QStringLiteral("UTC"));
        return Conversion::toStdString(xml);
    }
    const std::string result = Kolab::writeTodo(event, productId);
    mWrittenUID = Kolab::getSerializedUID();
    ErrorHandler::handleLibkolabxmlErrors();
    return result;
}

Todo XMLObject::readTodo(const std::string &s, Version version)
{
    if (version == KolabV2) {
        QStringList attachments;
        const auto event = Kolab::fromXML<KCalendarCore::Todo::Ptr, KolabV2::Task>(QString::fromUtf8(s.c_str()).toUtf8(), attachments);
        if (!event || Kolab::ErrorHandler::errorOccured()) {
            qCCritical(PIMKOLAB_LOG) << "failed to read xml";
            return {};
        }
        mAttachments.clear();
        mAttachments.reserve(attachments.count());
        for (const QString &attachment : std::as_const(attachments)) {
            mAttachments.push_back(Conversion::toStdString(attachment));
        }
        return Conversion::fromKCalendarCore(*event);
    }
    const Kolab::Todo todo = Kolab::readTodo(s, false);
    ErrorHandler::handleLibkolabxmlErrors();
    return todo;
}

std::string XMLObject::writeJournal(const Journal &event, Version version, const std::string &productId)
{
    mWrittenUID.clear();
    if (version == KolabV2) {
        const KCalendarCore::Journal::Ptr i = Conversion::toKCalendarCore(event);
        if (!i) {
            qCCritical(PIMKOLAB_LOG) << "invalid journal";
            return {};
        }
        if (i->uid().isEmpty()) {
            i->setUid(createUuid());
        }
        mWrittenUID = Conversion::toStdString(i->uid());
        // The timezone is used for created and last modified dates
        const QString &xml = KolabV2::Journal::journalToXML(i, QStringLiteral("UTC"));
        return Conversion::toStdString(xml);
    }
    const std::string result = Kolab::writeJournal(event, productId);
    mWrittenUID = Kolab::getSerializedUID();
    return result;
}

Journal XMLObject::readJournal(const std::string &s, Version version)
{
    if (version == KolabV2) {
        QStringList attachments;
        const auto event = Kolab::fromXML<KCalendarCore::Journal::Ptr, KolabV2::Journal>(QString::fromUtf8(s.c_str()).toUtf8(), attachments);
        if (!event || Kolab::ErrorHandler::errorOccured()) {
            qCCritical(PIMKOLAB_LOG) << "failed to read xml";
            return {};
        }
        mAttachments.clear();
        mAttachments.reserve(attachments.count());
        for (const QString &attachment : std::as_const(attachments)) {
            mAttachments.push_back(Conversion::toStdString(attachment));
        }
        return Conversion::fromKCalendarCore(*event);
    }
    const Kolab::Journal journal = Kolab::readJournal(s, false);
    ErrorHandler::handleLibkolabxmlErrors();
    return journal;
}

std::string XMLObject::writeFreebusy(const Freebusy &event, Version version, const std::string &productId)
{
    mWrittenUID.clear();
    if (version != KolabV3) {
        qCCritical(PIMKOLAB_LOG) << "only v3 implementation available";
        return {};
    }
    const std::string result = Kolab::writeFreebusy(event, productId);
    mWrittenUID = Kolab::getSerializedUID();
    return result;
}

Freebusy XMLObject::readFreebusy(const std::string &s, Version version)
{
    if (version != KolabV3) {
        qCCritical(PIMKOLAB_LOG) << "only v3 implementation available";
        return {};
    }
    return Kolab::readFreebusy(s, false);
}

std::string XMLObject::logoAttachmentName() const
{
    return mLogoAttachmentName;
}

std::string XMLObject::pictureAttachmentName() const
{
    return mPictureAttachmentName;
}

std::string XMLObject::soundAttachmentName() const
{
    return mSoundAttachmentName;
}

Contact XMLObject::readContact(const std::string &s, Version version)
{
    if (version == KolabV2) {
        const QByteArray xmlData(s.c_str(), s.size());
        QString pictureAttachmentName;
        QString logoAttachmentName;
        QString soundAttachmentName;
        const KContacts::Addressee addressee = addresseeFromKolab(xmlData, pictureAttachmentName, logoAttachmentName, soundAttachmentName);
        mPictureAttachmentName = Conversion::toStdString(pictureAttachmentName);
        mLogoAttachmentName = Conversion::toStdString(logoAttachmentName);
        mSoundAttachmentName = Conversion::toStdString(soundAttachmentName);
        return Conversion::fromKABC(addressee);
    }
    const Kolab::Contact contact = Kolab::readContact(s, false);
    ErrorHandler::handleLibkolabxmlErrors();
    return contact;
}

std::string XMLObject::writeContact(const Contact &contact, Version version, const std::string &productId)
{
    mWrittenUID.clear();
    if (version == KolabV2) {
        // FIXME attachment names are hardcoded for now
        KContacts::Addressee addressee = Conversion::toKABC(contact);
        if (addressee.uid().isEmpty()) {
            addressee.setUid(createUuid());
        }
        mWrittenUID = Conversion::toStdString(addressee.uid());
        const KolabV2::Contact contact(&addressee);
        return Conversion::toStdString(contact.saveXML());
    }
    const std::string result = Kolab::writeContact(contact, productId);
    mWrittenUID = Kolab::getSerializedUID();
    ErrorHandler::handleLibkolabxmlErrors();
    return result;
}

DistList XMLObject::readDistlist(const std::string &s, Version version)
{
    if (version == KolabV2) {
        const QByteArray xmlData(s.c_str(), s.size());
        const KContacts::ContactGroup contactGroup = contactGroupFromKolab(xmlData);
        return Conversion::fromKABC(contactGroup);
    }
    const Kolab::DistList distlist = Kolab::readDistlist(s, false);
    ErrorHandler::handleLibkolabxmlErrors();
    return distlist;
}

std::string XMLObject::writeDistlist(const DistList &distlist, Version version, const std::string &productId)
{
    mWrittenUID.clear();
    if (version == KolabV2) {
        KContacts::ContactGroup contactGroup = Conversion::toKABC(distlist);
        if (contactGroup.id().isEmpty()) {
            contactGroup.setId(createUuid());
        }
        mWrittenUID = Conversion::toStdString(contactGroup.id());
        const KolabV2::DistributionList d(&contactGroup);
        return Conversion::toStdString(d.saveXML());
    }
    const std::string result = Kolab::writeDistlist(distlist, productId);
    mWrittenUID = Kolab::getSerializedUID();
    ErrorHandler::handleLibkolabxmlErrors();
    return result;
}

Note XMLObject::readNote(const std::string &s, Version version)
{
    if (version == KolabV2) {
        const KMime::Message::Ptr msg = noteFromKolab(QByteArray(s.c_str(), s.length()), QDateTime());
        if (!msg || Kolab::ErrorHandler::errorOccured()) {
            qCCritical(PIMKOLAB_LOG) << "failed to read xml";
            return {};
        }
        return Conversion::fromNote(msg);
    }
    const Kolab::Note note = Kolab::readNote(s, false);
    ErrorHandler::handleLibkolabxmlErrors();
    return note;
}

std::string XMLObject::writeNote(const Note &note, Version version, const std::string &productId)
{
    mWrittenUID.clear();
    if (version == KolabV2) {
        Note noteWithUID = note;
        if (noteWithUID.uid().empty()) {
            noteWithUID.setUid(Conversion::toStdString(createUuid()));
        }
        mWrittenUID = noteWithUID.uid();
        const KMime::Message::Ptr n = Conversion::toNote(noteWithUID);
        const QByteArray &xml = noteToKolabXML(n);
        return std::string(xml.constData());
    }
    const std::string result = Kolab::writeNote(note, productId);
    mWrittenUID = Kolab::getSerializedUID();
    ErrorHandler::handleLibkolabxmlErrors();
    return result;
}

Configuration XMLObject::readConfiguration(const std::string &s, Version version)
{
    if (version == KolabV2) {
        QString lang;
        const QStringList dict = readLegacyDictionaryConfiguration(QByteArray(s.c_str(), s.length()), lang);
        if (lang.isEmpty()) {
            qCCritical(PIMKOLAB_LOG) << "not a dictionary or not a v2 configuration object";
            return {};
        }
        std::vector<std::string> entries;
        entries.reserve(dict.size());
        for (const QString &e : std::as_const(dict)) {
            entries.push_back(Conversion::toStdString(e));
        }
        Kolab::Dictionary dictionary(Conversion::toStdString(lang));
        dictionary.setEntries(entries);
        return Configuration(dictionary);
    }
    const Kolab::Configuration configuration = Kolab::readConfiguration(s, false);
    ErrorHandler::handleLibkolabxmlErrors();
    return configuration;
}

std::string XMLObject::writeConfiguration(const Configuration &configuration, Version version, const std::string &productId)
{
    mWrittenUID.clear();
    if (version != KolabV3) {
        qCCritical(PIMKOLAB_LOG) << "only v3 implementation available";
        return {};
    }
    const std::string result = Kolab::writeConfiguration(configuration, productId);
    mWrittenUID = Kolab::getSerializedUID();
    ErrorHandler::handleLibkolabxmlErrors();
    return result;
}

File XMLObject::readFile(const std::string &s, Version version)
{
    if (version == KolabV2) {
        qCCritical(PIMKOLAB_LOG) << "only v3 implementation available";
        return {};
    }
    const Kolab::File file = Kolab::readFile(s, false);
    ErrorHandler::handleLibkolabxmlErrors();
    return file;
}

std::string XMLObject::writeFile(const File &file, Version version, const std::string &productId)
{
    mWrittenUID.clear();
    if (version != KolabV3) {
        qCCritical(PIMKOLAB_LOG) << "only v3 implementation available";
        return {};
    }
    const std::string result = Kolab::writeFile(file, productId);
    mWrittenUID = Kolab::getSerializedUID();
    ErrorHandler::handleLibkolabxmlErrors();
    return result;
}
}
