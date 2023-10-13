/*
    SPDX-FileCopyrightText: 2014 Christian Mollekopf <mollekopf@kolabsys.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <resourcestate.h>

class ImapResource;

class KolabResourceState : public ::ResourceState
{
public:
    explicit KolabResourceState(ImapResourceBase *resource, const TaskArguments &arguments);

private:
    void collectionAttributesRetrieved(const Akonadi::Collection &collection) override;
    void collectionsRetrieved(const Akonadi::Collection::List &collections) override;
    MessageHelper::Ptr messageHelper() const override;
};
