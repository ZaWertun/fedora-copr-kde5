/*
    SPDX-FileCopyrightText: 2008 Tobias Koenig <tokoe@kde.org>
    SPDX-FileCopyrightText: 2012 Sérgio Martins <iamsergio@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <Akonadi/ResourceBase>

#include <KCalendarCore/Incidence>

#include <QHash>

class ICalDirResource : public Akonadi::ResourceBase, public Akonadi::AgentBase::Observer
{
    Q_OBJECT

public:
    explicit ICalDirResource(const QString &id);
    ~ICalDirResource() override;

public Q_SLOTS:
    void aboutToQuit() override;

protected:
    using ResourceBase::retrieveItems; // Suppress -Woverload-virtual

protected Q_SLOTS:
    void retrieveCollections() override;
    void retrieveItems(const Akonadi::Collection &col) override;
    bool retrieveItem(const Akonadi::Item &item, const QSet<QByteArray> &parts) override;

protected:
    void itemAdded(const Akonadi::Item &item, const Akonadi::Collection &collection) override;
    void itemChanged(const Akonadi::Item &item, const QSet<QByteArray> &parts) override;
    void itemRemoved(const Akonadi::Item &item) override;

    void collectionChanged(const Akonadi::Collection &collection) override;

private:
    Q_REQUIRED_RESULT bool loadIncidences();
    Q_REQUIRED_RESULT QString iCalDirectoryName() const;
    QString iCalDirectoryFileName(const QString &file) const;
    void initializeICalDirectory() const;
    void slotReloadConfig();

private:
    QHash<QString, KCalendarCore::Incidence::Ptr> mIncidences;
};
