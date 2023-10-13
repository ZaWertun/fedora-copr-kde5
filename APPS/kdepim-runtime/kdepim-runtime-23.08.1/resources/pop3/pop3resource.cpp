/*
   SPDX-FileCopyrightText: 2010 Thomas McGuire <mcguire@kde.org>

   SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "pop3resource.h"
#include "jobs.h"
#include "pop3protocol.h"

#include <Akonadi/AttributeFactory>
#include <Akonadi/CollectionFetchJob>
#include <Akonadi/ItemCreateJob>
#include <Akonadi/MessageFlags>
#include <Akonadi/Pop3ResourceAttribute>
#include <Akonadi/SpecialMailCollections>
#include <Akonadi/SpecialMailCollectionsRequestJob>
#include <MailTransport/PrecommandJob>
#include <MailTransport/Transport>

#include "pop3resource_debug.h"
#include <KAuthorized>
#include <KMessageBox>
#include <KNotification>
#include <KPasswordDialog>

#include <QPointer>
#include <QTimer>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <qt5keychain/keychain.h>
#else
#include <qt6keychain/keychain.h>
#endif
using namespace QKeychain;
using namespace Akonadi;
using namespace MailTransport;

POP3Resource::POP3Resource(const QString &id)
    : ResourceBase(id)
    , mState(Idle)
    , mIntervalTimer(new QTimer(this))
    , mSettings(KSharedConfig::openConfig())
{
    Akonadi::AttributeFactory::registerAttribute<Akonadi::Pop3ResourceAttribute>();
    setNeedsNetwork(true);
    mSettings.setResourceId(identifier());
    if (mSettings.name().isEmpty()) {
        if (name() == identifier()) {
            mSettings.setName(i18n("POP3 Account"));
        } else {
            mSettings.setName(name());
        }
    }
    setName(mSettings.name());
    resetState();

    connect(this, &POP3Resource::abortRequested, this, &POP3Resource::slotAbortRequested);
    connect(mIntervalTimer, &QTimer::timeout, this, &POP3Resource::intervalCheckTriggered);
    connect(this, &POP3Resource::reloadConfiguration, this, &POP3Resource::configurationChanged);
}

POP3Resource::~POP3Resource()
{
    mSettings.save();
}

void POP3Resource::configurationChanged()
{
    mSettings.load();
    updateIntervalTimer();
    mPassword.clear();
}

void POP3Resource::updateIntervalTimer()
{
    if (mSettings.intervalCheckEnabled() && mState == Idle) {
        mIntervalTimer->start(mSettings.intervalCheckInterval() * 1000 * 60);
    } else {
        mIntervalTimer->stop();
    }
}

void POP3Resource::intervalCheckTriggered()
{
    Q_ASSERT(mState == Idle);
    if (isOnline()) {
        qCDebug(POP3RESOURCE_LOG) << "Starting interval mail check.";
        startMailCheck();
        mIntervalCheckInProgress = true;
    } else {
        mIntervalTimer->start();
    }
}

void POP3Resource::aboutToQuit()
{
    if (mState != Idle) {
        cancelSync(i18n("Mail check aborted."));
    }
}

void POP3Resource::slotAbortRequested()
{
    if (mState != Idle) {
        cancelSync(i18n("Mail check was canceled manually."), false /* no error */);
    }
}

void POP3Resource::retrieveItems(const Akonadi::Collection &collection)
{
    Q_UNUSED(collection)
    qCWarning(POP3RESOURCE_LOG) << "This should never be called, we don't have a collection!";
}

bool POP3Resource::retrieveItems(const Akonadi::Item::List &items, const QSet<QByteArray> &parts)
{
    Q_UNUSED(items)
    Q_UNUSED(parts)
    qCWarning(POP3RESOURCE_LOG) << "This should never be called, we don't have any item!";
    return false;
}

bool POP3Resource::retrieveItem(const Akonadi::Item &item, const QSet<QByteArray> &parts)
{
    Q_UNUSED(item)
    Q_UNUSED(parts)
    qCWarning(POP3RESOURCE_LOG) << "This should never be called, we don't have any item!";
    return false;
}

QString POP3Resource::buildLabelForPasswordDialog(const QString &detailedError) const
{
    const QString queryText = i18n("Please enter the username and password for account '%1'.", agentName()) + QLatin1String("<br>") + detailedError;
    return queryText;
}

void POP3Resource::walletOpenedForLoading(QKeychain::Job *baseJob)
{
    auto job = qobject_cast<ReadPasswordJob *>(baseJob);
    bool passwordLoaded = false;
    Q_ASSERT(job);
    if (!job->error()) {
        mPassword = job->textData();
        passwordLoaded = true;
    } else {
        passwordLoaded = false;
        qCWarning(POP3RESOURCE_LOG) << "We have an error during reading password " << job->errorString();
    }
    if (!passwordLoaded) {
        const QString queryText = buildLabelForPasswordDialog(i18n("You are asked here because the password could not be loaded from the wallet."));
        showPasswordDialog(queryText);
    } else {
        advanceState(Connect);
    }
}

void POP3Resource::showPasswordDialog(const QString &queryText)
{
    QPointer<KPasswordDialog> dlg = new KPasswordDialog(nullptr, KPasswordDialog::ShowUsernameLine);
    dlg->setRevealPasswordAvailable(KAuthorized::authorize(QStringLiteral("lineedit_reveal_password")));
    dlg->setModal(true);
    dlg->setUsername(mSettings.login());
    dlg->setPassword(mPassword);
    dlg->setPrompt(queryText);
    dlg->setWindowTitle(name());
    dlg->addCommentLine(i18n("Account:"), name());

    bool gotIt = false;
    if (dlg->exec()) {
        mPassword = dlg->password();
        mSettings.setLogin(dlg->username());
        mSettings.save();
        if (!dlg->password().isEmpty()) {
            mSavePassword = true;
        }

        mAskAgain = false;
        advanceState(Connect);
        gotIt = true;
    }
    delete dlg;
    if (!gotIt) {
        cancelSync(i18n("No username and password supplied."));
    }
}

void POP3Resource::advanceState(State nextState)
{
    mState = nextState;
    doStateStep();
}

void POP3Resource::doStateStep()
{
    switch (mState) {
    case Idle:
        Q_ASSERT(false);
        qCWarning(POP3RESOURCE_LOG) << "State machine should not be called in idle state!";
        break;
    case FetchTargetCollection: {
        qCDebug(POP3RESOURCE_LOG) << "================ Starting state FetchTargetCollection ==========";
        Q_EMIT status(Running, i18n("Preparing transmission from \"%1\".", name()));
        Collection targetCollection(mSettings.targetCollection());
        if (targetCollection.isValid()) {
            auto fetchJob = new CollectionFetchJob(targetCollection, CollectionFetchJob::Base);
            fetchJob->start();
            connect(fetchJob, &CollectionFetchJob::result, this, &POP3Resource::targetCollectionFetchJobFinished);
        } else {
            // No target collection set in the config? Try requesting a default inbox
            auto requestJob = new SpecialMailCollectionsRequestJob(this);
            requestJob->requestDefaultCollection(SpecialMailCollections::Inbox);
            requestJob->start();
            connect(requestJob, &SpecialMailCollectionsRequestJob::result, this, &POP3Resource::localFolderRequestJobFinished);
        }
        break;
    }
    case Precommand:
        qCDebug(POP3RESOURCE_LOG) << "================ Starting state Precommand =====================";
        if (!mSettings.precommand().isEmpty()) {
            auto precommandJob = new PrecommandJob(mSettings.precommand(), this);
            connect(precommandJob, &PrecommandJob::result, this, &POP3Resource::precommandResult);
            precommandJob->start();
            Q_EMIT status(Running, i18n("Executing precommand."));
        } else {
            advanceState(RequestPassword);
        }
        break;
    case RequestPassword: {
        qCDebug(POP3RESOURCE_LOG) << "================ Starting state RequestPassword ================";

        // Don't show any wallet or password prompts when we are unit-testing
        if (!mSettings.unitTestPassword().isEmpty()) {
            mPassword = mSettings.unitTestPassword();
            advanceState(Connect);
            break;
        }

        const bool passwordNeeded = mSettings.authenticationMethod() != MailTransport::Transport::EnumAuthenticationType::GSSAPI;
        const bool loadPasswordFromWallet = !mAskAgain && passwordNeeded && !mSettings.login().isEmpty() && mPassword.isEmpty();
        if (loadPasswordFromWallet) {
            auto readJob = new ReadPasswordJob(QStringLiteral("pop3"), this);
            connect(readJob, &QKeychain::Job::finished, this, &POP3Resource::walletOpenedForLoading);
            readJob->setKey(identifier());
            readJob->start();
        } else if (passwordNeeded && (mPassword.isEmpty() || mAskAgain)) {
            QString detail;
            if (mAskAgain) {
                detail = i18n("You are asked here because the previous login was not successful.");
            } else if (mSettings.login().isEmpty()) {
                detail = i18n("You are asked here because the username you supplied is empty.");
            }

            showPasswordDialog(buildLabelForPasswordDialog(detail));
        } else {
            // No password needed or using previous password, go on with Connect
            advanceState(Connect);
        }

        break;
    }
    case Connect:
        qCDebug(POP3RESOURCE_LOG) << "================ Starting state Connect ========================";
        Q_ASSERT(!mPopSession);
        mPopSession = new POPSession(mSettings, mPassword);
        advanceState(Login);
        break;
    case Login: {
        qCDebug(POP3RESOURCE_LOG) << "================ Starting state Login ==========================";

        auto loginJob = new LoginJob(mPopSession);
        connect(loginJob, &LoginJob::result, this, &POP3Resource::loginJobResult);
        loginJob->start();
        break;
    }
    case List: {
        qCDebug(POP3RESOURCE_LOG) << "================ Starting state List ===========================";
        Q_EMIT status(Running, i18n("Fetching mail listing."));
        auto listJob = new ListJob(mPopSession);
        connect(listJob, &ListJob::result, this, &POP3Resource::listJobResult);
        listJob->start();
        break;
    }
    case UIDList: {
        qCDebug(POP3RESOURCE_LOG) << "================ Starting state UIDList ========================";
        auto uidListJob = new UIDListJob(mPopSession);
        connect(uidListJob, &UIDListJob::result, this, &POP3Resource::uidListJobResult);
        uidListJob->start();
        break;
    }
    case Download: {
        qCDebug(POP3RESOURCE_LOG) << "================ Starting state Download =======================";

        // Determine which mails we want to download. Those are all mails which are
        // currently on their server, minus the ones we have already downloaded (we
        // remember which UIDs we have downloaded in the settings)
        QList<int> idsToDownload = mIdsToSizeMap.keys();
        const QStringList alreadyDownloadedUIDs = mSettings.seenUidList();
        for (const QString &uidOnServer : std::as_const(mIdsToUidsMap)) {
            if (alreadyDownloadedUIDs.contains(uidOnServer)) {
                const int idOfUIDOnServer = mUidsToIdsMap.value(uidOnServer, -1);
                Q_ASSERT(idOfUIDOnServer != -1);
                idsToDownload.removeAll(idOfUIDOnServer);
            }
        }
        mIdsToDownload = idsToDownload;
        qCDebug(POP3RESOURCE_LOG) << "We are going to download" << mIdsToDownload.size() << "messages";

        // For proper progress, the job needs to know the sizes of the messages, so
        // put them into a list here
        QList<int> sizesOfMessagesToDownload;
        sizesOfMessagesToDownload.reserve(idsToDownload.count());
        for (int id : std::as_const(idsToDownload)) {
            sizesOfMessagesToDownload.append(mIdsToSizeMap.value(id));
        }

        if (mIdsToDownload.empty()) {
            advanceState(CheckRemovingMessage);
        } else {
            auto fetchJob = new FetchJob(mPopSession);
            fetchJob->setFetchIds(idsToDownload, sizesOfMessagesToDownload);
            connect(fetchJob, &FetchJob::result, this, &POP3Resource::fetchJobResult);
            connect(fetchJob, &FetchJob::messageFinished, this, &POP3Resource::messageFinished);
            // TODO wait kf6. For the moment we can't convert to new connect api.
            connect(fetchJob, SIGNAL(processedAmount(KJob *, KJob::Unit, qulonglong)), this, SLOT(messageDownloadProgress(KJob *, KJob::Unit, qulonglong)));

            fetchJob->start();
        }
        break;
    }
    case Save:
        qCDebug(POP3RESOURCE_LOG) << "================ Starting state Save ===========================";
        qCDebug(POP3RESOURCE_LOG) << mPendingCreateJobs.size() << "item create jobs are pending";
        if (!mPendingCreateJobs.isEmpty()) {
            Q_EMIT status(Running, i18n("Saving downloaded messages."));
        }

        if (shouldAdvanceToQuitState()) {
            advanceState(Quit);
        }
        break;
    case Quit: {
        qCDebug(POP3RESOURCE_LOG) << "================ Starting state Quit ===========================";
        auto quitJob = new QuitJob(mPopSession);
        connect(quitJob, &QuitJob::result, this, &POP3Resource::quitJobResult);
        quitJob->start();
        break;
    }
    case SavePassword:
        qCDebug(POP3RESOURCE_LOG) << "================ Starting state SavePassword ===================";
        if (mSavePassword) {
            qCDebug(POP3RESOURCE_LOG) << "Writing password back to the wallet.";
            Q_EMIT status(Running, i18n("Saving password to the wallet."));
            auto writeJob = new WritePasswordJob(QStringLiteral("pop3"), this);
            connect(writeJob, &QKeychain::Job::finished, this, [this](QKeychain::Job *baseJob) {
                if (baseJob->error()) {
                    qCWarning(POP3RESOURCE_LOG) << "Error writing password using QKeychain:" << baseJob->errorString();
                }
                finish();
            });
            writeJob->setKey(identifier());
            writeJob->setTextData(mPassword);
            writeJob->start();
        } else {
            finish();
        }
        break;
    case CheckRemovingMessage:
        qCDebug(POP3RESOURCE_LOG) << "================ Starting state CheckRemovingMessage ===================";
        checkRemovingMessageFromServer();
        break;
    }
}

void POP3Resource::checkRemovingMessageFromServer()
{
    const QList<int> idToDeleteMessage = shouldDeleteId(-1);
    if (!idToDeleteMessage.isEmpty()) {
        mIdsWaitingToDelete << idToDeleteMessage;
        if (!mDeleteJob) {
            mDeleteJob = new DeleteJob(mPopSession);
            mDeleteJob->setDeleteIds(mIdsWaitingToDelete);
            mIdsWaitingToDelete.clear();
            connect(mDeleteJob, &DeleteJob::result, this, &POP3Resource::deleteJobResult);
            mDeleteJob->start();
        }
    } else {
        advanceState(Save);
    }
}

void POP3Resource::localFolderRequestJobFinished(KJob *job)
{
    if (job->error()) {
        cancelSync(i18n("Error while trying to get the local inbox folder, "
                        "aborting mail check.")
                   + QLatin1Char('\n') + job->errorString());
        return;
    }
    if (mTestLocalInbox) {
        KMessageBox::information(nullptr,
                                 i18n("<qt>The folder you deleted was associated with the account "
                                      "<b>%1</b> which delivered mail into it. The folder the account "
                                      "delivers new mail into was reset to the main Inbox folder.</qt>",
                                      name()));
    }
    mTestLocalInbox = false;

    mTargetCollection = SpecialMailCollections::self()->defaultCollection(SpecialMailCollections::Inbox);
    Q_ASSERT(mTargetCollection.isValid());
    advanceState(Precommand);
}

void POP3Resource::targetCollectionFetchJobFinished(KJob *job)
{
    if (job->error()) {
        if (!mTestLocalInbox) {
            mTestLocalInbox = true;
            mSettings.setTargetCollection(-1);
            advanceState(FetchTargetCollection);
            return;
        } else {
            cancelSync(i18n("Error while trying to get the folder for incoming mail, "
                            "aborting mail check.")
                       + QLatin1Char('\n') + job->errorString());
            mTestLocalInbox = false;
            return;
        }
    }
    mTestLocalInbox = false;
    auto fetchJob = qobject_cast<Akonadi::CollectionFetchJob *>(job);
    Q_ASSERT(fetchJob);
    Q_ASSERT(fetchJob->collections().size() <= 1);

    if (fetchJob->collections().isEmpty()) {
        cancelSync(i18n("Could not find folder for incoming mail, aborting mail check."));
        return;
    } else {
        mTargetCollection = fetchJob->collections().at(0);
        advanceState(Precommand);
    }
}

void POP3Resource::precommandResult(KJob *job)
{
    if (job->error()) {
        cancelSync(i18n("Error while executing precommand.") + QLatin1Char('\n') + job->errorString());
        return;
    } else {
        advanceState(RequestPassword);
    }
}

void POP3Resource::loginJobResult(KJob *job)
{
    if (job->error()) {
        qCDebug(POP3RESOURCE_LOG) << job->error() << job->errorText();
        if (job->error() == POP3Protocol::ERR_CANNOT_LOGIN) {
            mAskAgain = true;
        }
        cancelSync(i18n("Unable to login to the server \"%1\".", mSettings.host()) + QLatin1Char('\n') + job->errorString());
    } else {
        advanceState(List);
    }
}

void POP3Resource::listJobResult(KJob *job)
{
    if (job->error()) {
        cancelSync(i18n("Error while getting the list of messages on the server.") + QLatin1Char('\n') + job->errorString());
    } else {
        auto listJob = qobject_cast<ListJob *>(job);
        Q_ASSERT(listJob);
        mIdsToSizeMap = listJob->idList();
        mIdsToSaveValid = false;
        qCDebug(POP3RESOURCE_LOG) << "IdsToSizeMap size" << mIdsToSizeMap.size();
        advanceState(UIDList);
    }
}

void POP3Resource::uidListJobResult(KJob *job)
{
    if (job->error()) {
        cancelSync(i18n("Error while getting list of unique mail identifiers from the server.") + QLatin1Char('\n') + job->errorString());
    } else {
        auto listJob = qobject_cast<UIDListJob *>(job);
        Q_ASSERT(listJob);
        mIdsToUidsMap = listJob->uidList();
        mUidsToIdsMap = listJob->idList();
        qCDebug(POP3RESOURCE_LOG) << "IdsToUidsMap size" << mIdsToUidsMap.size();
        qCDebug(POP3RESOURCE_LOG) << "UidsToIdsMap size" << mUidsToIdsMap.size();

        mUidListValid = !mIdsToUidsMap.isEmpty() || mIdsToSizeMap.isEmpty();
        if (mSettings.leaveOnServer() && !mUidListValid) {
            // FIXME: this needs a proper parent window
            KMessageBox::error(nullptr,
                               i18n("Your POP3 server (Account: %1) does not support "
                                    "the UIDL command: this command is required to determine, in a reliable way, "
                                    "which of the mails on the server KMail has already seen before;\n"
                                    "the feature to leave the mails on the server will therefore not "
                                    "work properly.",
                                    name()));
        }

        advanceState(Download);
    }
}

void POP3Resource::fetchJobResult(KJob *job)
{
    if (job->error()) {
        cancelSync(i18n("Error while fetching mails from the server.") + QLatin1Char('\n') + job->errorString());
        return;
    } else {
        qCDebug(POP3RESOURCE_LOG) << "Downloaded" << mDownloadedIDs.size() << "mails";

        if (!mIdsToDownload.isEmpty()) {
            qCWarning(POP3RESOURCE_LOG) << "We did not download all messages, there are still some remaining "
                                           "IDs, even though we requested their download:"
                                        << mIdsToDownload;
        }

        advanceState(Save);
    }
}

void POP3Resource::messageFinished(int messageId, KMime::Message::Ptr message)
{
    if (mState != Download) {
        // This can happen if the slave does not get notified in time about the fact
        // that the job was killed
        return;
    }

    // qCDebug(POP3RESOURCE_LOG) << "Got message" << messageId
    //         << "with subject" << message->subject()->asUnicodeString();

    Akonadi::Item item;
    item.setMimeType(QStringLiteral("message/rfc822"));
    item.setPayload<KMime::Message::Ptr>(message);

    auto attr = item.attribute<Akonadi::Pop3ResourceAttribute>(Akonadi::Item::AddIfMissing);
    attr->setPop3AccountName(identifier());
    Akonadi::MessageFlags::copyMessageFlags(*message, item);
    auto itemCreateJob = new ItemCreateJob(item, mTargetCollection);

    mPendingCreateJobs.insert(itemCreateJob, messageId);
    connect(itemCreateJob, &ItemCreateJob::result, this, &POP3Resource::itemCreateJobResult);

    mDownloadedIDs.append(messageId);
    mIdsToDownload.removeAll(messageId);
}

void POP3Resource::messageDownloadProgress(KJob *job, KJob::Unit unit, qulonglong totalBytes)
{
    Q_UNUSED(totalBytes)
    Q_UNUSED(unit)
    Q_ASSERT(unit == KJob::Bytes);
    QString statusMessage;
    const int totalMessages = mIdsToDownload.size() + mDownloadedIDs.size();
    int bytesRemainingOnServer = 0;
    const auto seenUidList{mSettings.seenUidList()};
    for (const QString &alreadyDownloadedUID : seenUidList) {
        const int alreadyDownloadedID = mUidsToIdsMap.value(alreadyDownloadedUID, -1);
        if (alreadyDownloadedID != -1) {
            bytesRemainingOnServer += mIdsToSizeMap.value(alreadyDownloadedID);
        }
    }

    if (mSettings.leaveOnServer() && bytesRemainingOnServer > 0) {
        statusMessage = i18n(
            "Fetching message %1 of %2 (%3 of %4 KB) for %5 "
            "(%6 KB remain on the server).",
            mDownloadedIDs.size() + 1,
            totalMessages,
            job->processedAmount(KJob::Bytes) / 1024,
            job->totalAmount(KJob::Bytes) / 1024,
            name(),
            bytesRemainingOnServer / 1024);
    } else {
        statusMessage = i18n("Fetching message %1 of %2 (%3 of %4 KB) for %5",
                             mDownloadedIDs.size() + 1,
                             totalMessages,
                             job->processedAmount(KJob::Bytes) / 1024,
                             job->totalAmount(KJob::Bytes) / 1024,
                             name());
    }
    Q_EMIT status(Running, statusMessage);
    Q_EMIT percent(job->percent());
}

void POP3Resource::itemCreateJobResult(KJob *job)
{
    if (mState != Download && mState != Save) {
        // This can happen if the slave does not get notified in time about the fact
        // that the job was killed
        return;
    }

    auto createJob = qobject_cast<ItemCreateJob *>(job);
    Q_ASSERT(createJob);

    if (job->error()) {
        cancelSync(i18n("Unable to store downloaded mails.") + QLatin1Char('\n') + job->errorString());
        return;
    }

    const int idOfMessageJustCreated = mPendingCreateJobs.value(createJob, -1);
    Q_ASSERT(idOfMessageJustCreated != -1);
    mPendingCreateJobs.remove(createJob);
    mIDsStored.append(idOfMessageJustCreated);
    // qCDebug(POP3RESOURCE_LOG) << "Just stored message with ID" << idOfMessageJustCreated
    //         << "on the Akonadi server";

    const QList<int> idToDeleteMessage = shouldDeleteId(idOfMessageJustCreated);
    if (!idToDeleteMessage.isEmpty()) {
        mIdsWaitingToDelete << idToDeleteMessage;
        if (!mDeleteJob) {
            mDeleteJob = new DeleteJob(mPopSession);
            mDeleteJob->setDeleteIds(mIdsWaitingToDelete);
            mIdsWaitingToDelete.clear();
            connect(mDeleteJob, &DeleteJob::result, this, &POP3Resource::deleteJobResult);
            mDeleteJob->start();
        }
    }

    // Have all create jobs finished? Go to the next state, then
    if (shouldAdvanceToQuitState()) {
        advanceState(Quit);
    }
}

int POP3Resource::idToTime(int id) const
{
    const QString uid = mIdsToUidsMap.value(id);
    if (!uid.isEmpty()) {
        const QList<QString> seenUIDs = mSettings.seenUidList();
        const QList<int> timeOfSeenUids = mSettings.seenUidTimeList();
        Q_ASSERT(seenUIDs.size() == timeOfSeenUids.size());
        const int index = seenUIDs.indexOf(uid);
        if (index != -1 && (index < timeOfSeenUids.size())) {
            return timeOfSeenUids.at(index);
        }
    }

    // If we don't find any mail, either we have no UID, or it is not in the seen UID
    // list. In that case, we assume that the mail is new, i.e. from now
    return time(nullptr);
}

int POP3Resource::idOfOldestMessage(const QSet<int> &idList) const
{
    int timeOfOldestMessage = time(nullptr) + 999;
    int idOfOldestMessage = -1;
    for (int id : idList) {
        const int idTime = idToTime(id);
        if (idTime < timeOfOldestMessage) {
            timeOfOldestMessage = idTime;
            idOfOldestMessage = id;
        }
    }
    Q_ASSERT(idList.isEmpty() || idOfOldestMessage != -1);
    return idOfOldestMessage;
}

QList<int> POP3Resource::shouldDeleteId(int downloadedId) const
{
    QList<int> idsToDeleteFromServer;
    // By default, we delete all messages. But if we have "leave on server"
    // rules, we can save some messages.
    if (mSettings.leaveOnServer()) {
        idsToDeleteFromServer = mIdsToSizeMap.keys();
        if (!mIdsToSaveValid) {
            mIdsToSaveValid = true;
            mIdsToSave.clear();

            const QSet<int> idsOnServer(idsToDeleteFromServer.constBegin(), idsToDeleteFromServer.constEnd());

            // If the time-limited leave rule is checked, add the newer messages to
            // the list of messages to keep
            if (mSettings.leaveOnServerDays() > 0) {
                const int secondsPerDay = 86400;
                time_t timeLimit = time(nullptr) - (secondsPerDay * mSettings.leaveOnServerDays());
                for (int idToDelete : idsOnServer) {
                    const int msgTime = idToTime(idToDelete);
                    if (msgTime >= timeLimit) {
                        mIdsToSave << idToDelete;
                    } else {
                        qCDebug(POP3RESOURCE_LOG) << "Message" << idToDelete << "is too old and will be deleted.";
                    }
                }
            }
            // Otherwise, add all messages to the list of messages to keep - this may
            // be reduced in the following number-limited leave rule and size-limited
            // leave rule checks
            else {
                mIdsToSave = idsOnServer;
            }

            //
            // Delete more old messages if there are more than mLeaveOnServerCount
            //
            if (mSettings.leaveOnServerCount() > 0) {
                const int numToDelete = mIdsToSave.count() - mSettings.leaveOnServerCount();
                if (numToDelete > 0 && numToDelete < mIdsToSave.count()) {
                    // Get rid of the first numToDelete messages
                    for (int i = 0; i < numToDelete; i++) {
                        mIdsToSave.remove(idOfOldestMessage(mIdsToSave));
                    }
                } else if (numToDelete >= mIdsToSave.count()) {
                    mIdsToSave.clear();
                }
            }

            //
            // Delete more old messages until we're under mLeaveOnServerSize MBs
            //
            if (mSettings.leaveOnServerSize() > 0) {
                const qint64 limitInBytes = mSettings.leaveOnServerSize() * (1024 * 1024);
                qint64 sizeOnServerAfterDeletion = 0;
                for (int id : std::as_const(mIdsToSave)) {
                    sizeOnServerAfterDeletion += mIdsToSizeMap.value(id);
                }
                while (sizeOnServerAfterDeletion > limitInBytes) {
                    int oldestId = idOfOldestMessage(mIdsToSave);
                    mIdsToSave.remove(oldestId);
                    sizeOnServerAfterDeletion -= mIdsToSizeMap.value(oldestId);
                }
            }
        }
        // Now save the messages from deletion
        //
        for (int idToSave : std::as_const(mIdsToSave)) {
            idsToDeleteFromServer.removeAll(idToSave);
        }
        if (downloadedId != -1 && !mIdsToSave.contains(downloadedId)) {
            idsToDeleteFromServer << downloadedId;
        }
    } else {
        if (downloadedId != -1) {
            idsToDeleteFromServer << downloadedId;
        } else {
            idsToDeleteFromServer << mIdsToSizeMap.keys();
        }
    }
    return idsToDeleteFromServer;
}

void POP3Resource::deleteJobResult(KJob *job)
{
    if (job->error()) {
        cancelSync(i18n("Failed to delete the messages from the server.") + QLatin1Char('\n') + job->errorString());
        return;
    }

    auto finishedDeleteJob = qobject_cast<DeleteJob *>(job);
    Q_ASSERT(finishedDeleteJob);
    Q_ASSERT(finishedDeleteJob == mDeleteJob);
    mDeletedIDs = finishedDeleteJob->deletedIDs();

    // Remove all deleted messages from the list of already downloaded messages,
    // as it is no longer necessary to store them (they just waste space)
    QList<QString> seenUIDs = mSettings.seenUidList();
    QList<int> timeOfSeenUids = mSettings.seenUidTimeList();
    Q_ASSERT(seenUIDs.size() == timeOfSeenUids.size());
    for (int deletedId : std::as_const(mDeletedIDs)) {
        const QString deletedUID = mIdsToUidsMap.value(deletedId);
        if (!deletedUID.isEmpty()) {
            int index = seenUIDs.indexOf(deletedUID);
            if (index != -1) {
                // TEST
                qCDebug(POP3RESOURCE_LOG) << "Removing UID" << deletedUID << "from the seen UID list, as it was deleted.";
                seenUIDs.removeAt(index);
                timeOfSeenUids.removeAt(index);
            }
        }
        mIdsToUidsMap.remove(deletedId);
        mIdsToSizeMap.remove(deletedId);
    }
    mSettings.setSeenUidList(seenUIDs);
    mSettings.setSeenUidTimeList(timeOfSeenUids);
    mSettings.save();

    mDeleteJob = nullptr;
    if (!mIdsWaitingToDelete.isEmpty()) {
        mDeleteJob = new DeleteJob(mPopSession);
        mDeleteJob->setDeleteIds(mIdsWaitingToDelete);
        mIdsWaitingToDelete.clear();
        connect(mDeleteJob, &DeleteJob::result, this, &POP3Resource::deleteJobResult);
        mDeleteJob->start();
    }

    if (shouldAdvanceToQuitState()) {
        advanceState(Quit);
    } else if (mDeleteJob == nullptr) {
        advanceState(Save);
    }
}

void POP3Resource::finish()
{
    qCDebug(POP3RESOURCE_LOG) << "================= Mail check finished. =============================";
    saveSeenUIDList();
    if (!mIntervalCheckInProgress) {
        collectionsRetrieved(Akonadi::Collection::List());
    }
    if (mDownloadedIDs.isEmpty()) {
        Q_EMIT status(Idle, i18n("Finished mail check, no message downloaded."));
    } else {
        Q_EMIT status(Idle, i18np("Finished mail check, 1 message downloaded.", "Finished mail check, %1 messages downloaded.", mDownloadedIDs.size()));
    }

    resetState();
}

bool POP3Resource::shouldAdvanceToQuitState() const
{
    return mState == Save && mPendingCreateJobs.isEmpty() && mIdsWaitingToDelete.isEmpty() && !mDeleteJob;
}

void POP3Resource::quitJobResult(KJob *job)
{
    if (job->error()) {
        cancelSync(i18n("Unable to complete the mail fetch.") + QLatin1Char('\n') + job->errorString());
        return;
    }

    advanceState(SavePassword);
}

void POP3Resource::saveSeenUIDList()
{
    QList<QString> seenUIDs = mSettings.seenUidList();
    QList<int> timeOfSeenUIDs = mSettings.seenUidTimeList();

    //
    // Find the messages that we have successfully stored, but did not actually get
    // deleted.
    // Those messages, we have to remember, so we don't download them again.
    //
    QList<int> idsOfMessagesDownloadedButNotDeleted = mIDsStored;
    for (int deletedId : std::as_const(mDeletedIDs)) {
        idsOfMessagesDownloadedButNotDeleted.removeAll(deletedId);
    }
    QList<QString> uidsOfMessagesDownloadedButNotDeleted;
    for (int id : std::as_const(idsOfMessagesDownloadedButNotDeleted)) {
        const QString uid = mIdsToUidsMap.value(id);
        if (!uid.isEmpty()) {
            uidsOfMessagesDownloadedButNotDeleted.append(uid);
        }
    }
    Q_ASSERT(seenUIDs.size() == timeOfSeenUIDs.size());
    for (const QString &uid : std::as_const(uidsOfMessagesDownloadedButNotDeleted)) {
        if (!seenUIDs.contains(uid)) {
            seenUIDs.append(uid);
            timeOfSeenUIDs.append(time(nullptr));
        }
    }

    //
    // If we have a valid UID list from the server, delete those UIDs that are in
    // the seenUidList but are not on the server.
    // This can happen if someone else than this resource deleted the mails from the
    // server which we kept here.
    //
    if (mUidListValid) {
        QList<QString>::iterator uidIt = seenUIDs.begin();
        QList<int>::iterator timeIt = timeOfSeenUIDs.begin();
        while (uidIt != seenUIDs.end()) {
            const QString curSeenUID = *uidIt;
            if (!mUidsToIdsMap.contains(curSeenUID)) {
                // Ok, we have a UID in the seen UID list that is not anymore on the server.
                // Therefore remove it from the seen UID list, it is not needed there anymore,
                // it just wastes space.
                uidIt = seenUIDs.erase(uidIt);
                timeIt = timeOfSeenUIDs.erase(timeIt);
            } else {
                ++uidIt;
                ++timeIt;
            }
        }
    } else {
        qCWarning(POP3RESOURCE_LOG) << "UID list from server is not valid.";
    }

    //
    // Now save it in the settings
    //
    qCDebug(POP3RESOURCE_LOG) << "The seen UID list has" << seenUIDs.size() << "entries";
    mSettings.setSeenUidList(seenUIDs);
    mSettings.setSeenUidTimeList(timeOfSeenUIDs);
    mSettings.save();
}

void POP3Resource::cancelSync(const QString &errorMessage, bool error)
{
    if (error) {
        cancelTask(errorMessage);
        qCWarning(POP3RESOURCE_LOG) << "============== ERROR DURING POP3 SYNC ==========================";
        qCWarning(POP3RESOURCE_LOG) << errorMessage;
        KNotification::event(QStringLiteral("mail-check-error"),
                             name(),
                             errorMessage.toHtmlEscaped(),
                             QString(),
                             nullptr,
                             KNotification::CloseOnTimeout,
                             QStringLiteral("akonadi_pop3_resource"));
    } else {
        qCDebug(POP3RESOURCE_LOG) << "Canceled the sync, but no error.";
        cancelTask();
    }
    saveSeenUIDList();
    resetState();
}

void POP3Resource::resetState()
{
    mState = Idle;
    mTargetCollection = Collection(-1);
    mIdsToSizeMap.clear();
    mIdsToUidsMap.clear();
    mUidsToIdsMap.clear();
    mDownloadedIDs.clear();
    mIdsToDownload.clear();
    mPendingCreateJobs.clear();
    mIDsStored.clear();
    mDeletedIDs.clear();
    mIdsWaitingToDelete.clear();
    if (mDeleteJob) {
        mDeleteJob->deleteLater();
        mDeleteJob = nullptr;
    }
    mUidListValid = false;
    mIntervalCheckInProgress = false;
    mSavePassword = false;
    updateIntervalTimer();

    if (mPopSession) {
        mPopSession->abortCurrentJob(); // no-op since jobs are sync
        // Disconnect after sending the QUIT command.
        mPopSession->deleteLater();
        mPopSession = nullptr;
    }
}

void POP3Resource::startMailCheck()
{
    resetState();
    mIntervalTimer->stop();
    Q_EMIT percent(0); // Otherwise the value from the last sync is taken
    advanceState(FetchTargetCollection);
}

void POP3Resource::retrieveCollections()
{
    if (mState == Idle) {
        startMailCheck();
    } else {
        cancelSync(i18n("Mail check already in progress, unable to start a second check."));
    }
}

void POP3Resource::clearCachedPassword()
{
    mPassword.clear();
}

void POP3Resource::cleanup()
{
    auto deleteJob = new DeletePasswordJob(QStringLiteral("pop3"));
    deleteJob->setKey(identifier());
    deleteJob->start();
    ResourceBase::cleanup();
}

void POP3Resource::doSetOnline(bool online)
{
    ResourceBase::doSetOnline(online);
    if (online) {
        Q_EMIT status(Idle, i18n("Ready"));
    } else {
        if (mState != Idle) {
            cancelSync(i18n("Mail check aborted after going offline."), false /* no error */);
        }
        Q_EMIT status(Idle, i18n("Offline"));
        clearCachedPassword();
    }
}

AKONADI_RESOURCE_MAIN(POP3Resource)
