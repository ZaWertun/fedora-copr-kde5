/*
    SPDX-FileCopyrightText: 2008 Bertjan Broeksema <b.broeksema@kdemail.org>
    SPDX-FileCopyrightText: 2008 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "akonadi-singlefileresource_export.h"
#include "singlefileresourceconfigwidgetbase.h"

#include <KConfigDialogManager>

namespace Akonadi
{
/**
 * Configuration widget for single file resources.
 */
template<typename Settings>
class SingleFileResourceConfigWidget : public SingleFileResourceConfigWidgetBase
{
    Settings *mSettings = nullptr;

public:
    explicit SingleFileResourceConfigWidget(QWidget *parent, Settings *settings)
        : SingleFileResourceConfigWidgetBase(parent)
        , mSettings(settings)
    {
        mManager = new KConfigDialogManager(this, mSettings);
    }

    bool save() const override
    {
        mManager->updateSettings();
        mSettings->setPath(ui.kcfg_Path->url().toString());
        mSettings->save();
        return true;
    }

    void load() override
    {
        ui.kcfg_Path->setUrl(QUrl::fromUserInput(mSettings->path()));
        mManager = new KConfigDialogManager(this, mSettings);
        mManager->updateWidgets();
    }
};
}
