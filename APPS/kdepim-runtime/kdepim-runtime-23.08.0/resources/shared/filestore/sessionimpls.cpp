/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2009 Kevin Krammer <kevin.krammer@gmx.at>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "sessionimpls_p.h"

#include "collectioncreatejob.h"
#include "collectiondeletejob.h"
#include "collectionfetchjob.h"
#include "collectionmodifyjob.h"
#include "collectionmovejob.h"
#include "itemcreatejob.h"
#include "itemdeletejob.h"
#include "itemfetchjob.h"
#include "itemmodifyjob.h"
#include "itemmovejob.h"
#include "storecompactjob.h"

#include <QQueue>
#include <QTimer>

using namespace Akonadi;

class AbstractEnqueueVisitor : public FileStore::Job::Visitor
{
public:
    bool visit(FileStore::Job *job) override
    {
        enqueue(job, "Job");
        return true;
    }

    bool visit(FileStore::CollectionCreateJob *job) override
    {
        enqueue(job, "CollectionCreateJob");
        return true;
    }

    bool visit(FileStore::CollectionDeleteJob *job) override
    {
        enqueue(job, "CollectionDeleteJob");
        return true;
    }

    bool visit(FileStore::CollectionFetchJob *job) override
    {
        enqueue(job, "CollectionFetchJob");
        return true;
    }

    bool visit(FileStore::CollectionModifyJob *job) override
    {
        enqueue(job, "CollectionModifyJob");
        return true;
    }

    bool visit(FileStore::CollectionMoveJob *job) override
    {
        enqueue(job, "CollectionMoveJob");
        return true;
    }

    bool visit(FileStore::ItemCreateJob *job) override
    {
        enqueue(job, "ItemCreateJob");
        return true;
    }

    bool visit(FileStore::ItemDeleteJob *job) override
    {
        enqueue(job, "ItemDeleteJob");
        return true;
    }

    bool visit(FileStore::ItemFetchJob *job) override
    {
        enqueue(job, "ItemFetchJob");
        return true;
    }

    bool visit(FileStore::ItemModifyJob *job) override
    {
        enqueue(job, "ItemModifyJob");
        return true;
    }

    bool visit(FileStore::ItemMoveJob *job) override
    {
        enqueue(job, "ItemMoveJob");
        return true;
    }

    bool visit(FileStore::StoreCompactJob *job) override
    {
        enqueue(job, "StoreCompactJob");
        return true;
    }

protected:
    virtual void enqueue(FileStore::Job *job, const char *className) = 0;
};

class FileStore::FiFoQueueJobSessionPrivate : public AbstractEnqueueVisitor
{
public:
    explicit FiFoQueueJobSessionPrivate(FileStore::FiFoQueueJobSession *parent)
        : mParent(parent)
    {
        QObject::connect(&mJobRunTimer, &QTimer::timeout, mParent, [this]() {
            runNextJob();
        });
    }

    void runNextJob()
    {
        /*      qDebug() << "Queue with" << mJobQueue.count() << "entries";*/
        if (mJobQueue.isEmpty()) {
            mJobRunTimer.stop();
            return;
        }

        FileStore::Job *job = mJobQueue.dequeue();
        while (job != nullptr && job->error() != 0) {
            /*        qDebug() << "Dequeued job" << job << "has error ("
                             << job->error() << "," << job->errorText() << ")";*/
            mParent->emitResult(job);
            if (!mJobQueue.isEmpty()) {
                job = mJobQueue.dequeue();
            } else {
                job = nullptr;
            }
        }

        if (job != nullptr) {
            /*        qDebug() << "Dequeued job" << job << "is ready";*/
            QList<FileStore::Job *> jobs;
            jobs << job;

            Q_EMIT mParent->jobsReady(jobs);
        } else {
            /*        qDebug() << "Queue now empty";*/
            mJobRunTimer.stop();
        }
    }

public:
    QQueue<FileStore::Job *> mJobQueue;

    QTimer mJobRunTimer;

protected:
    void enqueue(FileStore::Job *job, const char *className) override
    {
        Q_UNUSED(className)
        mJobQueue.enqueue(job);

        //       qDebug() << "adding" << className << ". Queue now with"
        //                << mJobQueue.count() << "entries";

        mJobRunTimer.start(0);
    }

private:
    FileStore::FiFoQueueJobSession *mParent = nullptr;
};

FileStore::FiFoQueueJobSession::FiFoQueueJobSession(QObject *parent)
    : FileStore::AbstractJobSession(parent)
    , d(new FiFoQueueJobSessionPrivate(this))
{
}

FileStore::FiFoQueueJobSession::~FiFoQueueJobSession()
{
    cancelAllJobs();
}

void FileStore::FiFoQueueJobSession::addJob(FileStore::Job *job)
{
    job->accept(d.get());
}

void FileStore::FiFoQueueJobSession::cancelAllJobs()
{
    // KJob::kill() also deletes the job
    for (FileStore::Job *job : std::as_const(d->mJobQueue)) {
        job->kill(KJob::EmitResult);
    }

    d->mJobQueue.clear();
}

void FileStore::FiFoQueueJobSession::removeJob(FileStore::Job *job)
{
    d->mJobQueue.removeAll(job);
}

#include "moc_sessionimpls_p.cpp"
