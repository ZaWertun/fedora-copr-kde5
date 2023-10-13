/*
    SPDX-FileCopyrightText: 2008 Tobias Koenig <tokoe@kde.org>
    SPDX-FileCopyrightText: 2008 Bertjan Broeksema <broeksema@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "vcarddirresource.h"

#include "settingsadaptor.h"
#include "vcarddirresource_debug.h"

#include <QDir>
#include <QDirIterator>
#include <QFile>

#include <Akonadi/ChangeRecorder>
#include <Akonadi/EntityDisplayAttribute>
#include <Akonadi/ItemFetchScope>

#include <KLocalizedString>

using namespace Akonadi;

VCardDirResource::VCardDirResource(const QString &id)
    : ResourceBase(id)
{
    VcardDirResourceSettings::instance(KSharedConfig::openConfig());
    // setup the resource
    new SettingsAdaptor(VcardDirResourceSettings::self());
    QDBusConnection::sessionBus().registerObject(QStringLiteral("/Settings"), VcardDirResourceSettings::self(), QDBusConnection::ExportAdaptors);

    changeRecorder()->itemFetchScope().fetchFullPayload();
    connect(this, &VCardDirResource::reloadConfiguration, this, &VCardDirResource::slotReloadConfig);
}

VCardDirResource::~VCardDirResource()
{
    // clear cache
    mAddressees.clear();
}

void VCardDirResource::slotReloadConfig()
{
    initializeVCardDirectory();
    loadAddressees();

    synchronize();
}

void VCardDirResource::aboutToQuit()
{
    VcardDirResourceSettings::self()->save();
}

void VCardDirResource::loadAddressees()
{
    mAddressees.clear();

    QDirIterator it(vCardDirectoryName());
    while (it.hasNext()) {
        it.next();
        const QString filename = it.fileName();
        if (filename != QLatin1String(".") && filename != QLatin1String("..") && filename != QLatin1String("WARNING_README.txt")) {
            QFile file(it.filePath());
            if (file.open(QIODevice::ReadOnly)) {
                const QByteArray data = file.readAll();
                file.close();

                const KContacts::Addressee addr = mConverter.parseVCard(data);
                if (!addr.isEmpty()) {
                    mAddressees.insert(addr.uid(), addr);
                }
            } else {
                qCCritical(VCARDDIRRESOURCE_LOG) << " file can't be load " << it.filePath();
            }
        }
    }

    Q_EMIT status(Idle);
}

bool VCardDirResource::retrieveItems(const Akonadi::Item::List &items, const QSet<QByteArray> &parts)
{
    Q_UNUSED(parts);

    Akonadi::Item::List resultItems;
    resultItems.reserve(items.size());

    for (const Akonadi::Item &item : items) {
        Item newItem(item);
        if (!doRetrieveItem(newItem)) {
            return false;
        }
        resultItems.append(newItem);
    }

    itemsRetrieved(resultItems);

    return true;
}

bool VCardDirResource::retrieveItem(const Akonadi::Item &item, const QSet<QByteArray> &)
{
    Item newItem(item);

    if (!doRetrieveItem(newItem)) {
        return false;
    }

    itemRetrieved(newItem);

    return true;
}

bool VCardDirResource::doRetrieveItem(Akonadi::Item &item)
{
    const QString remoteId = item.remoteId();
    if (!mAddressees.contains(remoteId)) {
        Q_EMIT error(i18n("Contact with uid '%1' not found.", remoteId));
        return false;
    }

    item.setPayload<KContacts::Addressee>(mAddressees.value(remoteId));

    return true;
}

void VCardDirResource::itemAdded(const Akonadi::Item &item, const Akonadi::Collection &)
{
    if (VcardDirResourceSettings::self()->readOnly()) {
        Q_EMIT error(i18n("Trying to write to a read-only directory: '%1'", vCardDirectoryName()));
        cancelTask();
        return;
    }

    KContacts::Addressee addressee;
    if (item.hasPayload<KContacts::Addressee>()) {
        addressee = item.payload<KContacts::Addressee>();
    }

    if (!addressee.isEmpty()) {
        // add it to the cache...
        mAddressees.insert(addressee.uid(), addressee);

        // ... and write it through to the file system
        const QByteArray data = mConverter.createVCard(addressee);

        QFile file(vCardDirectoryFileName(addressee.uid()));
        file.open(QIODevice::WriteOnly);
        file.write(data);
        file.close();

        // report everything ok
        Item newItem(item);
        newItem.setRemoteId(addressee.uid());
        changeCommitted(newItem);
    } else {
        changeProcessed();
    }
}

void VCardDirResource::itemChanged(const Akonadi::Item &item, const QSet<QByteArray> &)
{
    if (VcardDirResourceSettings::self()->readOnly()) {
        Q_EMIT error(i18n("Trying to write to a read-only directory: '%1'", vCardDirectoryName()));
        cancelTask();
        return;
    }

    KContacts::Addressee addressee;
    if (item.hasPayload<KContacts::Addressee>()) {
        addressee = item.payload<KContacts::Addressee>();
    }

    if (!addressee.isEmpty()) {
        // change it in the cache...
        mAddressees.insert(addressee.uid(), addressee);

        // ... and write it through to the file system
        const QByteArray data = mConverter.createVCard(addressee);

        QFile file(vCardDirectoryFileName(addressee.uid()));
        if (file.open(QIODevice::WriteOnly)) {
            file.write(data);
            file.close();

            Item newItem(item);
            newItem.setRemoteId(addressee.uid());
            changeCommitted(newItem);
        } else {
            qCritical() << " We can't write in file " << file.fileName();
        }
    } else {
        changeProcessed();
    }
}

void VCardDirResource::itemRemoved(const Akonadi::Item &item)
{
    if (VcardDirResourceSettings::self()->readOnly()) {
        Q_EMIT error(i18n("Trying to write to a read-only directory: '%1'", vCardDirectoryName()));
        cancelTask();
        return;
    }

    // remove it from the cache...
    mAddressees.remove(item.remoteId());

    // ... and remove it from the file system
    QFile::remove(vCardDirectoryFileName(item.remoteId()));

    changeProcessed();
}

void VCardDirResource::retrieveCollections()
{
    Collection c;
    c.setParentCollection(Collection::root());
    c.setRemoteId(vCardDirectoryName());
    c.setName(name());
    QStringList mimeTypes;
    mimeTypes << KContacts::Addressee::mimeType();
    c.setContentMimeTypes(mimeTypes);
    if (VcardDirResourceSettings::self()->readOnly()) {
        c.setRights(Collection::ReadOnly);
    } else {
        Collection::Rights rights;
        rights |= Collection::CanChangeItem;
        rights |= Collection::CanCreateItem;
        rights |= Collection::CanDeleteItem;
        rights |= Collection::CanChangeCollection;
        c.setRights(rights);
    }

    auto attr = c.attribute<EntityDisplayAttribute>(Collection::AddIfMissing);
    attr->setDisplayName(i18n("Contacts Folder"));
    attr->setIconName(QStringLiteral("x-office-address-book"));

    Collection::List list;
    list << c;
    collectionsRetrieved(list);
}

void VCardDirResource::retrieveItems(const Akonadi::Collection &)
{
    Item::List items;
    items.reserve(mAddressees.count());

    for (const KContacts::Addressee &addressee : std::as_const(mAddressees)) {
        Item item;
        item.setRemoteId(addressee.uid());
        item.setMimeType(KContacts::Addressee::mimeType());
        items.append(item);
    }

    itemsRetrieved(items);
}

QString VCardDirResource::vCardDirectoryName() const
{
    return VcardDirResourceSettings::self()->path();
}

QString VCardDirResource::vCardDirectoryFileName(const QString &file) const
{
    return VcardDirResourceSettings::self()->path() + QLatin1Char('/') + file;
}

void VCardDirResource::initializeVCardDirectory() const
{
    QDir dir(vCardDirectoryName());

    // if folder does not exists, create it
    if (!dir.exists()) {
        QDir::root().mkpath(dir.absolutePath());
    }

    // check whether warning file is in place...
    QFile file(dir.absolutePath() + QStringLiteral("/WARNING_README.txt"));
    if (!file.exists()) {
        // ... if not, create it
        file.open(QIODevice::WriteOnly);
        file.write(
            "Important Warning!!!\n\n"
            "Don't create or copy vCards inside this folder manually, they are managed by the Akonadi framework!\n");
        file.close();
    }
}

AKONADI_RESOURCE_MAIN(VCardDirResource)
