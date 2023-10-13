/*
    SPDX-FileCopyrightText: 2009 Constantin Berzan <exit3219@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "sendjob.h"

#include "storeresultjob.h"

#include "maildispatcher_debug.h"
#include <Akonadi/AddressAttribute>
#include <Akonadi/AgentInstance>
#include <Akonadi/AgentManager>
#include <Akonadi/Collection>
#include <Akonadi/CollectionFetchJob>
#include <Akonadi/ItemDeleteJob>
#include <Akonadi/ItemModifyJob>
#include <Akonadi/ItemMoveJob>
#include <Akonadi/MessageParts>
#include <Akonadi/SentBehaviourAttribute>
#include <Akonadi/ServerManager>
#include <Akonadi/SpecialMailCollections>
#include <Akonadi/TransportAttribute>
#include <Akonadi/TransportResourceBase>
#include <KLocalizedString>
#include <MailTransport/Transport>
#include <MailTransport/TransportJob>
#include <MailTransport/TransportManager>

#include <KMime/Message>

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QTimer>

using namespace Akonadi;
using namespace KMime;
using namespace MailTransport;

void SendJob::doTransport()
{
    qCDebug(MAILDISPATCHER_LOG) << "Transporting message.";

    if (mAborting) {
        qCDebug(MAILDISPATCHER_LOG) << "Marking message as aborted.";
        setError(UserDefinedError);
        setErrorText(i18n("Message sending aborted."));
        storeResult(false, i18n("Message sending aborted."));
        return;
    }

    // Is it an Akonadi transport or a traditional one?
    const TransportAttribute *transportAttribute = mItem.attribute<TransportAttribute>();
    const auto transport = TransportManager::self()->transportById(transportAttribute->transportId(), false);
    Q_ASSERT(transportAttribute);
    if (!transport) {
        storeResult(false, i18n("Could not initiate message transport. Possibly invalid transport."));
        return;
    }

    const TransportType type = transport->transportType();
    if (!type.isValid()) {
        storeResult(false, i18n("Could not send message. Invalid transport."));
        return;
    }

    if (!filterItem(8)) { // BeforeOutbound
        return;
    }

    if (type.isAkonadiResource()) {
        // Send the item directly to the resource that will send it.
        mResourceId = transport->host();
        doAkonadiTransport();
    } else {
        // Use a traditional transport job.
        doTraditionalTransport();
    }
}

void SendJob::doAkonadiTransport()
{
    Q_ASSERT(!mResourceId.isEmpty());
    Q_ASSERT(mInterface == nullptr);

    const auto service = Akonadi::ServerManager::agentServiceName(Akonadi::ServerManager::Resource, mResourceId);

    mInterface = new QDBusInterface(service,
                                    QStringLiteral("/Transport"),
                                    QStringLiteral("org.freedesktop.Akonadi.Resource.Transport"),
                                    QDBusConnection::sessionBus(),
                                    this);

    if (!mInterface->isValid()) {
        storeResult(false, i18n("Failed to get D-Bus interface of resource %1.", mResourceId));
        delete mInterface;
        mInterface = nullptr;
        return;
    }

    // Signals.
    QObject::connect(AgentManager::self(), &AgentManager::instanceProgressChanged, this, &SendJob::resourceProgress);
    QObject::connect(mInterface, SIGNAL(transportResult(qlonglong, int, QString)), this, SLOT(resourceResult(qlonglong, int, QString)));

    // Start sending.
    const QDBusReply<void> reply = mInterface->call(QStringLiteral("send"), mItem.id());
    if (!reply.isValid()) {
        storeResult(false, i18n("Invalid D-Bus reply from resource %1.", mResourceId));
        return;
    }
}

void SendJob::doTraditionalTransport()
{
    const TransportAttribute *transportAttribute = mItem.attribute<TransportAttribute>();
    TransportJob *job = TransportManager::self()->createTransportJob(transportAttribute->transportId());

    Q_ASSERT(job);
    Q_ASSERT(mCurrentJob == nullptr);

    mCurrentJob = job;

    // Message.
    Q_ASSERT(mItem.hasPayload<Message::Ptr>());
    const auto message = mItem.payload<Message::Ptr>();
    bool needAssemble = false;
    if (message->removeHeader("Bcc")) {
        needAssemble = true;
    }
    if (message->removeHeader("X-KMail-Identity")) {
        needAssemble = true;
    }
    if (message->removeHeader("X-KMail-Dictionary")) {
        needAssemble = true;
    }
    if (message->removeHeader("X-KMail-Transport")) {
        needAssemble = true;
    }
    if (message->removeHeader("X-KMail-Fcc")) {
        needAssemble = true;
    }
    if (message->removeHeader("X-KMail-Identity-Name")) {
        needAssemble = true;
    }
    if (message->removeHeader("X-KMail-Transport-Name")) {
        needAssemble = true;
    }

    if (needAssemble) {
        message->assemble();
    }
    const QByteArray content = message->encodedContent(true) + "\r\n";
    Q_ASSERT(!content.isEmpty());

    // Addresses.
    const AddressAttribute *addressAttribute = mItem.attribute<AddressAttribute>();
    Q_ASSERT(addressAttribute);

    job->setData(content);
    job->setSender(addressAttribute->from());
    job->setTo(addressAttribute->to());
    job->setCc(addressAttribute->cc());
    job->setBcc(addressAttribute->bcc());
    job->setDeliveryStatusNotification(addressAttribute->deliveryStatusNotification());

    // Signals.
    connect(job, &TransportJob::result, this, &SendJob::transportResult);
    connect(job, &TransportJob::percentChanged, this, [this](KJob *job, ulong val) {
        transportPercent(job, val);
    });
    job->start();
}

void SendJob::transportPercent(KJob *job, unsigned long)
{
    Q_ASSERT(mCurrentJob == job);
    qCDebug(MAILDISPATCHER_LOG) << "Processed amount" << job->processedAmount(KJob::Bytes) << "total amount" << job->totalAmount(KJob::Bytes);

    setTotalAmount(KJob::Bytes, job->totalAmount(KJob::Bytes)); // Is not set at the time of start().
    setProcessedAmount(KJob::Bytes, job->processedAmount(KJob::Bytes));
}

void SendJob::transportResult(KJob *job)
{
    Q_ASSERT(mCurrentJob == job);
    mCurrentJob = nullptr;
    doPostJob(!job->error(), job->errorString());
}

void SendJob::resourceProgress(const AgentInstance &instance)
{
    if (!mInterface) {
        // We might have gotten a very late signal.
        qCWarning(MAILDISPATCHER_LOG) << "called but no resource job running!";
        return;
    }

    if (instance.identifier() == mResourceId) {
        // This relies on the resource's progress representing the progress of
        // sending this item.
        setPercent(instance.progress());
    }
}

void SendJob::resourceResult(qlonglong itemId, int result, const QString &message)
{
    Q_UNUSED(itemId)
    Q_ASSERT(mInterface);
    delete mInterface; // So that abort() knows the transport job is over.
    mInterface = nullptr;

    const auto transportResult = static_cast<TransportResourceBase::TransportResult>(result);

    const bool success = (transportResult == TransportResourceBase::TransportSucceeded);

    Q_ASSERT(itemId == mItem.id());
    doPostJob(success, message);
}

void SendJob::abortPostJob()
{
    // We were unlucky and LocalFolders is recreating its stuff right now.
    // We will not wait for it.
    qCWarning(MAILDISPATCHER_LOG) << "Default sent mail collection unavailable, not moving the mail after sending.";
    setError(UserDefinedError);
    setErrorText(i18n("Default sent-mail folder unavailable. Keeping message in outbox."));
    storeResult(false, errorString());
}

void SendJob::doPostJob(bool transportSuccess, const QString &transportMessage)
{
    qCDebug(MAILDISPATCHER_LOG) << "success" << transportSuccess << "message" << transportMessage;

    if (!transportSuccess) {
        qCDebug(MAILDISPATCHER_LOG) << "Error transporting.";
        setError(UserDefinedError);

        const QString error = mAborting ? i18n("Message transport aborted.") : i18n("Failed to transport message.");

        setErrorText(error + QLatin1Char(' ') + transportMessage);
        storeResult(false, errorString());
    } else {
        qCDebug(MAILDISPATCHER_LOG) << "Success transporting.";

        // Delete or move to sent-mail.
        const SentBehaviourAttribute *attribute = mItem.attribute<SentBehaviourAttribute>();
        Q_ASSERT(attribute);

        if (attribute->sentBehaviour() == SentBehaviourAttribute::Delete) {
            qCDebug(MAILDISPATCHER_LOG) << "Deleting item from outbox.";
            mCurrentJob = new ItemDeleteJob(mItem);
            QObject::connect(mCurrentJob, &ItemDeleteJob::result, this, &SendJob::postJobResult);
        } else {
            if (attribute->sentBehaviour() == SentBehaviourAttribute::MoveToDefaultSentCollection) {
                if (SpecialMailCollections::self()->hasDefaultCollection(SpecialMailCollections::SentMail)) {
                    mCurrentJob = new ItemMoveJob(mItem, SpecialMailCollections::self()->defaultCollection(SpecialMailCollections::SentMail), this);
                    QObject::connect(mCurrentJob, &ItemMoveJob::result, this, &SendJob::postJobResult);
                } else {
                    abortPostJob();
                }
            } else {
                qCDebug(MAILDISPATCHER_LOG) << "sentBehaviour=" << attribute->sentBehaviour() << "using collection from attribute";
                mCurrentJob = new CollectionFetchJob(attribute->moveToCollection(), Akonadi::CollectionFetchJob::Base);
                QObject::connect(mCurrentJob, &CollectionFetchJob::result, this, &SendJob::slotSentMailCollectionFetched);
            }
        }
    }
}

bool SendJob::filterItem(int filterset)
{
    const auto service = Akonadi::ServerManager::agentServiceName(Akonadi::ServerManager::Agent, QStringLiteral("akonadi_mailfilter_agent"));

    QDBusInterface iface(service,
                         QStringLiteral("/MailFilterAgent"),
                         QStringLiteral("org.freedesktop.Akonadi.MailFilterAgent"),
                         QDBusConnection::sessionBus(),
                         this);
    if (!iface.isValid()) {
        storeResult(false, i18n("Failed to get D-Bus interface of mailfilteragent."));
        return false;
    }

    // Outbound = 0x2
    const QDBusReply<void> reply = iface.call(QStringLiteral("filterItem"), mItem.id(), filterset, QString());
    if (!reply.isValid()) {
        storeResult(false, i18n("Invalid D-Bus reply from mailfilteragent"));
        return false;
    }

    return true;
}

void SendJob::slotSentMailCollectionFetched(KJob *job)
{
    Akonadi::Collection fetchCol;
    bool ok = false;
    if (!job->error()) {
        const CollectionFetchJob *const fetchJob = qobject_cast<CollectionFetchJob *>(job);
        if (!fetchJob->collections().isEmpty()) {
            fetchCol = fetchJob->collections().at(0);
            ok = true;
        }
    }
    if (!ok) {
        if (!SpecialMailCollections::self()->hasDefaultCollection(SpecialMailCollections::SentMail)) {
            abortPostJob();
            return;
        }
        fetchCol = SpecialMailCollections::self()->defaultCollection(SpecialMailCollections::SentMail);
    }
    mCurrentJob = new ItemMoveJob(mItem, fetchCol, this);
    QObject::connect(mCurrentJob, &ItemMoveJob::result, this, &SendJob::postJobResult);
}

void SendJob::postJobResult(KJob *job)
{
    Q_ASSERT(mCurrentJob == job);
    mCurrentJob = nullptr;
    const SentBehaviourAttribute *attribute = mItem.attribute<SentBehaviourAttribute>();
    Q_ASSERT(attribute);

    if (job->error()) {
        qCDebug(MAILDISPATCHER_LOG) << "Error deleting or moving to sent-mail.";

        QString errorStr;
        switch (attribute->sentBehaviour()) {
        case SentBehaviourAttribute::Delete:
            errorStr = i18n("Sending succeeded, but failed to remove the message from the outbox.");
            break;
        default:
            errorStr = i18n("Sending succeeded, but failed to move the message to the sent-mail folder.");
            break;
        }
        setError(UserDefinedError);
        setErrorText(errorStr + QLatin1Char(' ') + job->errorString());
        storeResult(false, errorString());
    } else {
        qCDebug(MAILDISPATCHER_LOG) << "Success deleting or moving to sent-mail.";
        if (!filterItem(2)) { // Outbound
            return;
        }
        if (attribute->sentBehaviour() == SentBehaviourAttribute::Delete) {
            emitResult();
        } else {
            storeResult(true);
        }
    }
}

void SendJob::storeResult(bool success, const QString &message)
{
    qCDebug(MAILDISPATCHER_LOG) << "success" << success << "message" << message;

    Q_ASSERT(mCurrentJob == nullptr);
    mCurrentJob = new StoreResultJob(mItem, success, message);
    connect(mCurrentJob, &StoreResultJob::result, this, &SendJob::doEmitResult);
}

void SendJob::doEmitResult(KJob *job)
{
    Q_ASSERT(mCurrentJob == job);
    mCurrentJob = nullptr;

    if (job->error()) {
        qCWarning(MAILDISPATCHER_LOG) << "Error storing result.";
        setError(UserDefinedError);
        setErrorText(errorString() + QLatin1Char(' ') + i18n("Failed to store result in item.") + QLatin1Char(' ') + job->errorString());
    } else {
        qCDebug(MAILDISPATCHER_LOG) << "Success storing result.";
        // It is still possible that the transport failed.
        auto srJob = static_cast<StoreResultJob *>(job);
        if (!srJob->success()) {
            setError(UserDefinedError);
            setErrorText(srJob->message());
        }
    }

    emitResult();
}

SendJob::SendJob(const Item &item, QObject *parent)
    : KJob(parent)
    , mItem(item)
{
}

SendJob::~SendJob() = default;

void SendJob::start()
{
    QTimer::singleShot(0, this, &SendJob::doTransport);
}

void SendJob::setMarkAborted()
{
    Q_ASSERT(!mAborting);
    mAborting = true;
}

void SendJob::abort()
{
    setMarkAborted();

    if (dynamic_cast<TransportJob *>(mCurrentJob)) {
        qCDebug(MAILDISPATCHER_LOG) << "Abort called, active transport job.";
        // Abort transport.
        mCurrentJob->kill(KJob::EmitResult);
    } else if (mInterface != nullptr) {
        qCDebug(MAILDISPATCHER_LOG) << "Abort called, propagating to resource.";
        // Abort resource doing transport.
        AgentInstance instance = AgentManager::self()->instance(mResourceId);
        instance.abortCurrentTask();
    } else {
        qCDebug(MAILDISPATCHER_LOG) << "Abort called, but no transport job is active.";
        // Either transport has not started, in which case doTransport will
        // mark the item as aborted, or the item has already been sent, in which
        // case there is nothing we can do.
    }
}

#include "moc_sendjob.cpp"
