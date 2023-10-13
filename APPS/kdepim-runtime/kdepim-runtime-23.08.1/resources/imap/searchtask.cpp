/*
 * SPDX-FileCopyrightText: 2013 Daniel Vrátil <dvratil@redhat.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 *
 */

#include "searchtask.h"
#include "imapresource_debug.h"
#include <Akonadi/MessageFlags>
#include <Akonadi/SearchQuery>
#include <KIMAP/SearchJob>
#include <KIMAP/SelectJob>
#include <KIMAP/Session>
#include <KLocalizedString>
Q_DECLARE_METATYPE(KIMAP::Session *)

SearchTask::SearchTask(const ResourceStateInterface::Ptr &state, const QString &query, QObject *parent)
    : ResourceTask(ResourceTask::DeferIfNoSession, state, parent)
    , m_query(query)
{
}

SearchTask::~SearchTask() = default;

void SearchTask::doStart(KIMAP::Session *session)
{
    qCDebug(IMAPRESOURCE_LOG) << collection().remoteId();

    const QString mailbox = mailBoxForCollection(collection());
    if (session->selectedMailBox() == mailbox) {
        doSearch(session);
        return;
    }

    auto select = new KIMAP::SelectJob(session);
    select->setMailBox(mailbox);
    connect(select, &KJob::finished, this, &SearchTask::onSelectDone);
    select->start();
}

void SearchTask::onSelectDone(KJob *job)
{
    if (job->error()) {
        searchFinished(QVector<qint64>());
        cancelTask(job->errorText());
        return;
    }

    doSearch(qobject_cast<KIMAP::SelectJob *>(job)->session());
}

static KIMAP::Term::Relation mapRelation(Akonadi::SearchTerm::Relation relation)
{
    if (relation == Akonadi::SearchTerm::RelAnd) {
        return KIMAP::Term::And;
    }
    return KIMAP::Term::Or;
}

static KIMAP::Term recursiveEmailTermMapping(const Akonadi::SearchTerm &term)
{
    if (!term.subTerms().isEmpty()) {
        QVector<KIMAP::Term> subterms;
        const QList<Akonadi::SearchTerm> lstSearchTermsList = term.subTerms();
        for (const Akonadi::SearchTerm &subterm : lstSearchTermsList) {
            const KIMAP::Term newTerm = recursiveEmailTermMapping(subterm);
            if (!newTerm.isNull()) {
                subterms << newTerm;
            }
        }
        return KIMAP::Term(mapRelation(term.relation()), subterms);
    } else {
        const Akonadi::EmailSearchTerm::EmailSearchField field = Akonadi::EmailSearchTerm::fromKey(term.key());
        switch (field) {
        case Akonadi::EmailSearchTerm::Message:
            return KIMAP::Term(KIMAP::Term::Text, term.value().toString()).setNegated(term.isNegated());
        case Akonadi::EmailSearchTerm::Body:
            return KIMAP::Term(KIMAP::Term::Body, term.value().toString()).setNegated(term.isNegated());
        case Akonadi::EmailSearchTerm::Headers:
            // FIXME
            //                 return KIMAP::Term(KIMAP::Term::Header, term.value()).setNegated(term.isNegated());
            break;
        case Akonadi::EmailSearchTerm::ByteSize: {
            int value = term.value().toInt();
            switch (term.condition()) {
            case Akonadi::SearchTerm::CondGreaterOrEqual:
                value--;
                Q_FALLTHROUGH();
            case Akonadi::SearchTerm::CondGreaterThan:
                return KIMAP::Term(KIMAP::Term::Larger, value).setNegated(term.isNegated());
            case Akonadi::SearchTerm::CondLessOrEqual:
                value++;
                Q_FALLTHROUGH();
            case Akonadi::SearchTerm::CondLessThan:
                return KIMAP::Term(KIMAP::Term::Smaller, value).setNegated(term.isNegated());
            case Akonadi::SearchTerm::CondEqual:
                return KIMAP::Term(KIMAP::Term::And,
                                   QVector<KIMAP::Term>() << KIMAP::Term(KIMAP::Term::Smaller, value + 1) << KIMAP::Term(KIMAP::Term::Larger, value + 1))
                    .setNegated(term.isNegated());
            case Akonadi::SearchTerm::CondContains:
                qCDebug(IMAPRESOURCE_LOG) << " invalid condition for ByteSize";
                break;
            }
            break;
        }
        case Akonadi::EmailSearchTerm::HeaderOnlyDate:
        case Akonadi::EmailSearchTerm::HeaderDate: {
            QDate value = term.value().toDateTime().date();
            switch (term.condition()) {
            case Akonadi::SearchTerm::CondGreaterOrEqual:
                value = value.addDays(-1);
                Q_FALLTHROUGH();
            case Akonadi::SearchTerm::CondGreaterThan:
                return KIMAP::Term(KIMAP::Term::SentSince, value).setNegated(term.isNegated());
            case Akonadi::SearchTerm::CondLessOrEqual:
                value = value.addDays(1);
                Q_FALLTHROUGH();
            case Akonadi::SearchTerm::CondLessThan:
                return KIMAP::Term(KIMAP::Term::SentBefore, value).setNegated(term.isNegated());
            case Akonadi::SearchTerm::CondEqual:
                return KIMAP::Term(KIMAP::Term::SentOn, value).setNegated(term.isNegated());
            case Akonadi::SearchTerm::CondContains:
                qCDebug(IMAPRESOURCE_LOG) << " invalid condition for Date";
                return {};
            default:
                qCWarning(IMAPRESOURCE_LOG) << "unknown term for date" << term.key();
                return {};
            }
        }
        case Akonadi::EmailSearchTerm::Subject:
            return KIMAP::Term(KIMAP::Term::Subject, term.value().toString()).setNegated(term.isNegated());
        case Akonadi::EmailSearchTerm::HeaderFrom:
            return KIMAP::Term(KIMAP::Term::From, term.value().toString()).setNegated(term.isNegated());
        case Akonadi::EmailSearchTerm::HeaderTo:
            return KIMAP::Term(KIMAP::Term::To, term.value().toString()).setNegated(term.isNegated());
        case Akonadi::EmailSearchTerm::HeaderCC:
            return KIMAP::Term(KIMAP::Term::Cc, term.value().toString()).setNegated(term.isNegated());
        case Akonadi::EmailSearchTerm::HeaderBCC:
            return KIMAP::Term(KIMAP::Term::Bcc, term.value().toString()).setNegated(term.isNegated());
        case Akonadi::EmailSearchTerm::MessageStatus: {
            const QString termStr = term.value().toString();
            if (termStr == QString::fromLatin1(Akonadi::MessageFlags::Flagged)) {
                return KIMAP::Term(KIMAP::Term::Flagged).setNegated(term.isNegated());
            }
            if (termStr == QString::fromLatin1(Akonadi::MessageFlags::Deleted)) {
                return KIMAP::Term(KIMAP::Term::Deleted).setNegated(term.isNegated());
            }
            if (termStr == QString::fromLatin1(Akonadi::MessageFlags::Replied)) {
                return KIMAP::Term(KIMAP::Term::Answered).setNegated(term.isNegated());
            }
            if (termStr == QString::fromLatin1(Akonadi::MessageFlags::Seen)) {
                return KIMAP::Term(KIMAP::Term::Seen).setNegated(term.isNegated());
            }
            break;
        }
        case Akonadi::EmailSearchTerm::MessageTag:
            break;
        case Akonadi::EmailSearchTerm::HeaderReplyTo:
            break;
        case Akonadi::EmailSearchTerm::HeaderOrganization:
            break;
        case Akonadi::EmailSearchTerm::HeaderListId:
            break;
        case Akonadi::EmailSearchTerm::HeaderResentFrom:
            break;
        case Akonadi::EmailSearchTerm::HeaderXLoop:
            break;
        case Akonadi::EmailSearchTerm::HeaderXMailingList:
            break;
        case Akonadi::EmailSearchTerm::HeaderXSpamFlag:
            break;
        case Akonadi::EmailSearchTerm::Unknown:
        default:
            qCWarning(IMAPRESOURCE_LOG) << "unknown term " << term.key();
        }
    }
    return {};
}

void SearchTask::doSearch(KIMAP::Session *session)
{
    qCDebug(IMAPRESOURCE_LOG) << m_query;

    Akonadi::SearchQuery query = Akonadi::SearchQuery::fromJSON(m_query.toLatin1());
    auto searchJob = new KIMAP::SearchJob(session);
    searchJob->setUidBased(true);

    KIMAP::Term term = recursiveEmailTermMapping(query.term());
    if (term.isNull()) {
        qCWarning(IMAPRESOURCE_LOG) << "failed to translate query " << m_query;
        searchFinished(QVector<qint64>());
        cancelTask(i18n("Invalid search"));
        return;
    }
    searchJob->setTerm(term);

    connect(searchJob, &KJob::finished, this, &SearchTask::onSearchDone);
    searchJob->start();
}

void SearchTask::onSearchDone(KJob *job)
{
    if (job->error()) {
        qCWarning(IMAPRESOURCE_LOG) << "Failed to execute search " << job->errorString();
        qCDebug(IMAPRESOURCE_LOG) << m_query;
        searchFinished(QVector<qint64>());
        cancelTask(job->errorString());
        return;
    }

    auto searchJob = qobject_cast<KIMAP::SearchJob *>(job);
    const QVector<qint64> result = searchJob->results();
    qCDebug(IMAPRESOURCE_LOG) << result.count() << "matches";

    searchFinished(result);
    taskDone();
}
