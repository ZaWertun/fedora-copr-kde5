/*
    SPDX-FileCopyrightText: 2016 Stefan Stäglich <sstaeglich@kdemail.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <Akonadi/ResourceBase>
#include <QNetworkAccessManager>

class TomboyNotesResource : public Akonadi::ResourceBase, public Akonadi::AgentBase::Observer
{
    Q_OBJECT

public:
    explicit TomboyNotesResource(const QString &id);
    ~TomboyNotesResource() override;

protected:
    using ResourceBase::retrieveItems; // Suppress -Woverload-virtual

protected Q_SLOTS:
    // Standard akonadi slots
    void retrieveCollections() override;
    void retrieveItems(const Akonadi::Collection &col) override;
    bool retrieveItem(const Akonadi::Item &item, const QSet<QByteArray> &parts) override;

    // Slots for Job result handling
    void onAuthorizationFinished(KJob *kjob);
    void onCollectionsRetrieved(KJob *kjob);
    void onItemChangeCommitted(KJob *kjob);
    void onItemRetrieved(KJob *kjob);
    void onItemsRetrieved(KJob *kjob);

private Q_SLOTS:
    // SSL error handling
    void onSslError(QNetworkReply *reply, const QList<QSslError> &errors);

protected:
    void aboutToQuit() override;

    // Standard akonadi
    void itemAdded(const Akonadi::Item &item, const Akonadi::Collection &collection) override;
    void itemChanged(const Akonadi::Item &item, const QSet<QByteArray> &parts) override;
    void itemRemoved(const Akonadi::Item &item) override;

private:
    bool configurationNotValid() const;
    void slotReloadConfig();

    void retryAfterFailure(const QString &errorMessage);
    // Status handling
    void showError(const QString &errorText);
    QTimer *mStatusMessageTimer = nullptr;

    // Only one UploadJob should run per time
    bool mUploadJobProcessRunning;

    QNetworkAccessManager *mManager = nullptr;
};
