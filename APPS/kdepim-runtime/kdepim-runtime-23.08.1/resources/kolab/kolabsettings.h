/*
    SPDX-FileCopyrightText: 2014 Sandro Knauß <knauss@kolabsys.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "settings.h"

class KolabSettings : public Settings
{
    Q_OBJECT
public:
    explicit KolabSettings(WId = 0);

protected:
    virtual void changeDefaults();
};
