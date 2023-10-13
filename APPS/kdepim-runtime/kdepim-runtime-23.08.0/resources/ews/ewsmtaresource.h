/*
    SPDX-FileCopyrightText: 2015-2016 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <Akonadi/Item>
#include <Akonadi/ResourceBase>
#include <Akonadi/TransportResourceBase>

class OrgKdeAkonadiEwsResourceInterface;

class EwsMtaResource : public Akonadi::ResourceBase, public Akonadi::TransportResourceBase
{
    Q_OBJECT
public:
    explicit EwsMtaResource(const QString &id);
    ~EwsMtaResource() override;

    void sendItem(const Akonadi::Item &item) override;
public Q_SLOTS:
    void configure(WId windowId) override;
protected Q_SLOTS:
    void retrieveCollections() override;
    void retrieveItems(const Akonadi::Collection &collection) override;
    bool retrieveItems(const Akonadi::Item::List &items, const QSet<QByteArray> &parts) override;

private:
    void messageSent(const QString &id, const QString &error);
    Q_REQUIRED_RESULT bool connectEws();

    OrgKdeAkonadiEwsResourceInterface *mEwsResource = nullptr;
    QHash<QString, Akonadi::Item> mItemHash;
};
