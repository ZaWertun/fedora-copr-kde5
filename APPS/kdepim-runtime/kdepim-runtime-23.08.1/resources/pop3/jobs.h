/*
   SPDX-FileCopyrightText: 2009 Thomas McGuire <mcguire@kde.org>

   SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#pragma once

#include "result.h"

#include <KMime/Message>

#include <KJob>

#include <QMap>
#include <QObject>

class KSslErrorUiData;
class BaseJob;
class POP3Protocol;
class Settings;

class POPSession : public QObject
{
    Q_OBJECT
public:
    explicit POPSession(Settings &settings, const QString &password);
    ~POPSession() override;

    // Warning: this object lives in a different thread
    // Do not make direct method calls to it
    POP3Protocol *getProtocol() const;

    void abortCurrentJob();
    void closeSession();

    // Sets the current BaseJob that is using the POPSession.
    void setCurrentJob(BaseJob *job);

private:
    void handleSslError(const KSslErrorUiData &);

    std::unique_ptr<POP3Protocol> mProtocol;
    BaseJob *mCurrentJob = nullptr;
    std::unique_ptr<QThread> mThread;
};

class BaseJob : public KJob
{
    Q_OBJECT
public:
    explicit BaseJob(POPSession *POPSession);
    ~BaseJob() override;

Q_SIGNALS:
    // internal signal
    void jobDone(const Result &result);

protected:
    virtual void slotData(const QByteArray &data);
    virtual void handleJobDone(const Result &result);
    void startJob(const QString &path);

    POPSession *const mPOPSession;
};

class LoginJob : public BaseJob
{
    Q_OBJECT
public:
    explicit LoginJob(POPSession *popSession);
    void start() override;
};

class ListJob : public BaseJob
{
    Q_OBJECT
public:
    explicit ListJob(POPSession *popSession);
    QMap<int, int> idList() const;
    void start() override;

private:
    void slotData(const QByteArray &data) override;

private:
    QMap<int, int> mIdList;
};

class UIDListJob : public BaseJob
{
    Q_OBJECT
public:
    explicit UIDListJob(POPSession *popSession);
    QMap<int, QString> uidList() const;
    QMap<QString, int> idList() const;
    void start() override;

private:
    void slotData(const QByteArray &data) override;

    QMap<int, QString> mUidList;
    QMap<QString, int> mIdList;
};

class DeleteJob : public BaseJob
{
    Q_OBJECT
public:
    explicit DeleteJob(POPSession *popSession);
    void setDeleteIds(const QList<int> &ids);
    void start() override;
    QList<int> deletedIDs() const;

private:
    QList<int> mIdsToDelete;
};

class QuitJob : public BaseJob
{
    Q_OBJECT

public:
    explicit QuitJob(POPSession *popSession);
    void start() override;
};

class FetchJob : public BaseJob
{
    Q_OBJECT
public:
    explicit FetchJob(POPSession *session);
    void setFetchIds(const QList<int> &ids, const QList<int> &sizes);
    void start() override;

Q_SIGNALS:
    void messageFinished(int id, KMime::Message::Ptr message);

private:
    void slotData(const QByteArray &data) override;
    void handleJobDone(const Result &result) override;
    void slotMessageComplete();

    QList<int> mIdsPendingDownload;
    QByteArray mCurrentMessage;
    int mBytesDownloaded;
    int mTotalBytesToDownload;
    uint mDataCounter;
};
