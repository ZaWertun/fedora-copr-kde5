/*
   SPDX-FileCopyrightText: 2009 Thomas McGuire <mcguire@kde.org>

   SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "jobs.h"
#include "settings.h"

#include <MailTransport/Transport>

#include "pop3resource_debug.h"
#include <KIO/SslUi>
#include <KLocalizedString>

#include "pop3protocol.h"

#include <QThread>

POPSession::POPSession(Settings &settings, const QString &password)
    : mProtocol(std::make_unique<POP3Protocol>(settings, password))
    , mThread(new QThread)
{
    qRegisterMetaType<Result>();
    connect(mProtocol.get(), &POP3Protocol::sslError, this, &POPSession::handleSslError, Qt::BlockingQueuedConnection);
    mProtocol->moveToThread(mThread.get());
    mThread->start();
}

POPSession::~POPSession()
{
    closeSession();
    mThread->quit();
    mThread->wait();
}

void POPSession::setCurrentJob(BaseJob *job)
{
    mCurrentJob = job;
}

void POPSession::handleSslError(const KSslErrorUiData &errorData)
{
    const bool cont = KIO::SslUi::askIgnoreSslErrors(errorData, KIO::SslUi::RecallAndStoreRules);
    mProtocol->setContinueAfterSslError(cont);
}

POP3Protocol *POPSession::getProtocol() const
{
    return mProtocol.get();
}

void POPSession::abortCurrentJob()
{
    // This is never the case anymore, since all jobs are sync
    if (mCurrentJob) {
        mCurrentJob->kill(KJob::Quietly);
        mCurrentJob = nullptr;
    }
}

void POPSession::closeSession()
{
    QMetaObject::invokeMethod(mProtocol.get(), [=]() {
        Q_ASSERT(QThread::currentThread() != qApp->thread());
        mProtocol->closeConnection();
    });
}

static QByteArray cleanupListResponse(const QByteArray &response)
{
    QByteArray ret = response.simplified(); // Workaround for Maillennium POP3/UNIBOX

    // Get rid of the null terminating character, if it exists
    int retSize = ret.size();
    if (retSize > 0 && ret.at(retSize - 1) == 0) {
        ret.chop(1);
    }
    return ret;
}

static QString intListToString(const QList<int> &intList)
{
    QString idList;
    for (int id : intList) {
        idList += QString::number(id) + QLatin1Char(',');
    }
    idList.chop(1);
    return idList;
}

BaseJob::BaseJob(POPSession *POPSession)
    : mPOPSession(POPSession)
{
    mPOPSession->setCurrentJob(this);
    connect(this, &BaseJob::jobDone, this, &BaseJob::handleJobDone);
}

BaseJob::~BaseJob()
{
    // Don't do that here, the job might be destroyed after another one was started
    // and therefore overwrite the current job
    // mPOPSession->setCurrentJob(nullptr);
}

void BaseJob::startJob(const QString &path)
{
    Q_ASSERT(QThread::currentThread() == qApp->thread());
    POP3Protocol *protocol = mPOPSession->getProtocol();
    connect(protocol, &POP3Protocol::data, this, &BaseJob::slotData);
    // Important: copy the arguments into the lambda, it'll crash if you capture by reference
    QMetaObject::invokeMethod(protocol, [=]() {
        Q_ASSERT(QThread::currentThread() != qApp->thread());
        const Result result = protocol->get(path);
        disconnect(protocol, &POP3Protocol::data, this, &BaseJob::slotData);
        Q_EMIT jobDone(result);
    });
}

void BaseJob::handleJobDone(const Result &result)
{
    Q_ASSERT(QThread::currentThread() == qApp->thread());
    mPOPSession->setCurrentJob(nullptr);
    if (!result.success) {
        setError(result.error);
        setErrorText(result.errorString);
    }
    emitResult();
}

void BaseJob::slotData(const QByteArray &data)
{
    qCWarning(POP3RESOURCE_LOG) << "Got unexpected job data:" << data.data();
}

LoginJob::LoginJob(POPSession *popSession)
    : BaseJob(popSession)
{
}

void LoginJob::start()
{
    if (!POP3Protocol::initSASL()) {
        setError(KJob::UserDefinedError);
        setErrorText(i18n("Unable to initialize SASL, aborting mail check."));
        emitResult();
    }

    Q_ASSERT(QThread::currentThread() == qApp->thread());
    POP3Protocol *protocol = mPOPSession->getProtocol();
    QMetaObject::invokeMethod(protocol, [=]() {
        Q_ASSERT(QThread::currentThread() != qApp->thread());
        const Result result = protocol->openConnection();
        Q_EMIT jobDone(result);
    });
}

ListJob::ListJob(POPSession *popSession)
    : BaseJob(popSession)
{
}

void ListJob::start()
{
    startJob(QStringLiteral("/index"));
}

void ListJob::slotData(const QByteArray &data)
{
    Q_ASSERT(!data.isEmpty());

    const QByteArray cleanData = cleanupListResponse(data);
    const int space = cleanData.indexOf(' ');

    if (space > 0) {
        QByteArray lengthString = cleanData.mid(space + 1);
        const int spaceInLengthPos = lengthString.indexOf(' ');
        if (spaceInLengthPos != -1) {
            lengthString.truncate(spaceInLengthPos);
        }
        const int length = lengthString.toInt();

        QByteArray idString = cleanData.left(space);

        bool idIsNumber;
        int id = QString::fromLatin1(idString).toInt(&idIsNumber);
        if (idIsNumber) {
            mIdList.insert(id, length);
        } else {
            qCWarning(POP3RESOURCE_LOG) << "Got non-integer ID as part of the LIST response, ignoring" << idString.data();
        }
    } else {
        qCWarning(POP3RESOURCE_LOG) << "Got invalid LIST response:" << data.data();
    }
}

QMap<int, int> ListJob::idList() const
{
    return mIdList;
}

UIDListJob::UIDListJob(POPSession *popSession)
    : BaseJob(popSession)
{
}

void UIDListJob::start()
{
    startJob(QStringLiteral("/uidl"));
}

void UIDListJob::slotData(const QByteArray &data)
{
    Q_ASSERT(!data.isEmpty());

    QByteArray cleanData = cleanupListResponse(data);
    const int space = cleanData.indexOf(' ');

    if (space <= 0) {
        qCWarning(POP3RESOURCE_LOG) << "Invalid response to the UIDL command:" << data.data();
        qCWarning(POP3RESOURCE_LOG) << "Ignoring this entry.";
    } else {
        const QByteArray idString = cleanData.left(space);
        const QByteArray uidString = cleanData.mid(space + 1);
        bool idIsNumber;
        int id = QString::fromLatin1(idString).toInt(&idIsNumber);
        if (idIsNumber) {
            const QString uidQString = QString::fromLatin1(uidString);
            if (!uidQString.isEmpty()) {
                mUidList.insert(id, uidQString);
                mIdList.insert(uidQString, id);
            } else {
                qCWarning(POP3RESOURCE_LOG) << "Got invalid/empty UID from the UIDL command:" << uidString.data();
                qCWarning(POP3RESOURCE_LOG) << "The whole response was:" << data.data();
            }
        } else {
            qCWarning(POP3RESOURCE_LOG) << "Got invalid ID from the UIDL command:" << idString.data();
            qCWarning(POP3RESOURCE_LOG) << "The whole response was:" << data.data();
        }
    }
}

QMap<int, QString> UIDListJob::uidList() const
{
    return mUidList;
}

QMap<QString, int> UIDListJob::idList() const
{
    return mIdList;
}

DeleteJob::DeleteJob(POPSession *popSession)
    : BaseJob(popSession)
{
}

void DeleteJob::setDeleteIds(const QList<int> &ids)
{
    mIdsToDelete = ids;
}

void DeleteJob::start()
{
    qCDebug(POP3RESOURCE_LOG) << "================= DeleteJob::start. =============================";
    startJob(QLatin1String("/remove/") + intListToString(mIdsToDelete));
}

QList<int> DeleteJob::deletedIDs() const
{
    // FIXME : The protocol class doesn't tell us which of the IDs were actually deleted, we
    //         just assume all of them here
    return mIdsToDelete;
}

QuitJob::QuitJob(POPSession *popSession)
    : BaseJob(popSession)
{
}

void QuitJob::start()
{
    startJob(QStringLiteral("/quit"));
}

FetchJob::FetchJob(POPSession *session)
    : BaseJob(session)
    , mBytesDownloaded(0)
    , mTotalBytesToDownload(0)
    , mDataCounter(0)
{
}

void FetchJob::setFetchIds(const QList<int> &ids, const QList<int> &sizes)
{
    mIdsPendingDownload = ids;
    for (int size : std::as_const(sizes)) {
        mTotalBytesToDownload += size;
    }
}

void FetchJob::start()
{
    setTotalAmount(KJob::Bytes, mTotalBytesToDownload);
    connect(mPOPSession->getProtocol(), &POP3Protocol::messageComplete, this, &FetchJob::slotMessageComplete);
    startJob(QLatin1String("/download/") + intListToString(mIdsPendingDownload));
}

void FetchJob::slotData(const QByteArray &data)
{
    mCurrentMessage += data;
    mBytesDownloaded += data.size();
    mDataCounter++;
    if (mDataCounter % 5 == 0) {
        setProcessedAmount(KJob::Bytes, mBytesDownloaded);
    }
}

void FetchJob::handleJobDone(const Result &result)
{
    disconnect(mPOPSession->getProtocol(), &POP3Protocol::messageComplete, this, &FetchJob::slotMessageComplete);
    BaseJob::handleJobDone(result);
}

void FetchJob::slotMessageComplete()
{
    KMime::Message::Ptr msg(new KMime::Message);
    msg->setContent(KMime::CRLFtoLF(mCurrentMessage));
    msg->parse();

    mCurrentMessage.clear();
    const int idOfCurrentMessage = mIdsPendingDownload.takeFirst();
    Q_EMIT messageFinished(idOfCurrentMessage, msg);
}
