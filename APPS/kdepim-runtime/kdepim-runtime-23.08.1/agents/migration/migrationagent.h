/*
 * SPDX-FileCopyrightText: 2013 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 *
 */

#pragma once

#include "migrationscheduler.h"
#include <Akonadi/AgentBase>

namespace Akonadi
{
class MigrationAgent : public AgentBase, public AgentBase::ObserverV2
{
    Q_OBJECT
public:
    explicit MigrationAgent(const QString &id);
    void configure(WId windowId) override;

private:
    MigrationScheduler mScheduler;
};
}
