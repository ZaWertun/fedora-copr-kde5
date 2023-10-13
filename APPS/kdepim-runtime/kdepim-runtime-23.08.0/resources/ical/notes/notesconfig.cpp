/*
    SPDX-FileCopyrightText: 2018 Daniel Vrátil <dvratil@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "settings.h"
#include "singlefileresourceconfigbase.h"

using ConfigBase = SingleFileResourceConfigBase<SETTINGS_NAMESPACE::Settings>;

class NotesConfigBase : public ConfigBase
{
public:
    using ConfigBase::ConfigBase;
};

class NotesConfig : public NotesConfigBase
{
    Q_OBJECT
public:
    using NotesConfigBase::NotesConfigBase;
};

AKONADI_AGENTCONFIG_FACTORY(NotesConfigFactory, "notesconfig.json", NotesConfig)

#include "notesconfig.moc"
