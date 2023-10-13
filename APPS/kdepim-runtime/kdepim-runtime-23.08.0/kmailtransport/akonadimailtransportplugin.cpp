/*
    SPDX-FileCopyrightText: 2017-2023 Laurent Montel <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "akonadimailtransportplugin.h"
#include "mailtransportplugin_akonadi_debug.h"
#include "resourcesendjob_p.h"
#include <Akonadi/AgentConfigurationDialog>
#include <Akonadi/AgentInstanceCreateJob>
#include <Akonadi/AgentManager>
#include <KPluginFactory>
#include <MailTransport/Transport>
#include <QPointer>

using namespace Akonadi;

K_PLUGIN_CLASS_WITH_JSON(AkonadiMailTransportPlugin, "akonadimailtransport.json")

AkonadiMailTransportPlugin::AkonadiMailTransportPlugin(QObject *parent, const QList<QVariant> &)
    : MailTransport::TransportAbstractPlugin(parent)
{
    // Watch for appearing and disappearing types.
    connect(AgentManager::self(), &AgentManager::typeAdded, this, &AkonadiMailTransportPlugin::slotUpdatePluginList);
    connect(AgentManager::self(), &AgentManager::typeRemoved, this, &AkonadiMailTransportPlugin::slotUpdatePluginList);
}

AkonadiMailTransportPlugin::~AkonadiMailTransportPlugin() = default;

void AkonadiMailTransportPlugin::slotUpdatePluginList(const Akonadi::AgentType &type)
{
    if (type.capabilities().contains(QLatin1String("MailTransport"))) {
        Q_EMIT updatePluginList();
    }
}

void AkonadiMailTransportPlugin::cleanUp(MailTransport::Transport *t)
{
    const AgentInstance instance = AgentManager::self()->instance(t->host());
    if (!instance.isValid()) {
        qCWarning(MAILTRANSPORT_AKONADI_LOG) << "Could not find resource instance for name:" << t->host();
    }
    AgentManager::self()->removeInstance(instance);
}

QVector<MailTransport::TransportAbstractPluginInfo> AkonadiMailTransportPlugin::names() const
{
    QVector<MailTransport::TransportAbstractPluginInfo> lst;

    const auto types{AgentManager::self()->types()};
    for (const AgentType &atype : types) {
        // TODO probably the string "MailTransport" should be #defined somewhere
        // and used like that in the resources (?)
        if (atype.capabilities().contains(QLatin1String("MailTransport"))) {
            MailTransport::TransportAbstractPluginInfo info;
            info.name = atype.name();
            info.description = atype.description();
            info.identifier = atype.identifier();
            info.isAkonadi = true;
            lst << std::move(info);
        }
    }
    return lst;
}

bool AkonadiMailTransportPlugin::configureTransport(const QString &identifier, MailTransport::Transport *transport, QWidget *parent)
{
    Q_UNUSED(identifier)
    AgentInstance instance = AgentManager::self()->instance(transport->host());
    if (!instance.isValid()) {
        qCWarning(MAILTRANSPORT_AKONADI_LOG) << "Invalid resource instance" << transport->host();
        return false;
    }
    Akonadi::AgentConfigurationDialog dlg(instance, parent); // Async...
    dlg.exec();
    transport->save();
    return true; // No way to know here if the user cancelled or not.
}

MailTransport::TransportJob *AkonadiMailTransportPlugin::createTransportJob(MailTransport::Transport *t, const QString &identifier)
{
    Q_UNUSED(identifier)
    return new MailTransport::ResourceSendJob(t, this);
}

void AkonadiMailTransportPlugin::initializeTransport(MailTransport::Transport *t, const QString &identifier)
{
    auto cjob = new AgentInstanceCreateJob(identifier);
    if (!cjob->exec()) {
        qCWarning(MAILTRANSPORT_AKONADI_LOG) << "Failed to create agent instance of type" << identifier;
        return;
    }
    t->setHost(cjob->instance().identifier());
}

#include "akonadimailtransportplugin.moc"
