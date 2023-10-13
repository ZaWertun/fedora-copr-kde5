/*
    SPDX-FileCopyrightText: 2013 Christian Mollekopf <mollekopf@kolabsys.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <Akonadi/ControlGui>
#include <KAboutData>

#include <KLocalizedString>

#include "migration_debug.h"
#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QIcon>
#include <infodialog.h>

#include "gidmigrator.h"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    KLocalizedString::setApplicationDomain("gid-migrator");
    KAboutData aboutData(QStringLiteral("gid-migrator"),
                         i18n("GID Migration Tool"),
                         QStringLiteral("0.1"),
                         i18n("Migration of Akonadi Items to support GID"),
                         KAboutLicense::LGPL,
                         i18n("(c) 2013-2022 the Akonadi developers"),
                         QStringLiteral("https://community.kde.org/KDE_PIM/Akonadi"));
    aboutData.setProductName(QByteArrayLiteral("Akonadi/Migration"));
    aboutData.addAuthor(i18n("Christian Mollekopf"), i18n("Author"), QStringLiteral("mollekopf@kolabsys.com"));

    QCommandLineParser parser;
    parser.addOption(QCommandLineOption(QStringList() << QStringLiteral("interactive"), i18n("Show reporting dialog")));
    parser.addOption(QCommandLineOption(QStringList() << QStringLiteral("interactive-on-change"), i18n("Show report only if changes were made")));
    parser.addOption(QCommandLineOption(QStringLiteral("mimetype"), i18n("MIME type to migrate"), QStringLiteral("mimetype")));

    aboutData.setupCommandLine(&parser);
    parser.process(app);
    aboutData.processCommandLine(&parser);

    app.setQuitOnLastWindowClosed(false);
    app.setWindowIcon(QIcon::fromTheme(QStringLiteral("akonadi")));

    if (!Akonadi::ControlGui::start(nullptr)) {
        return 2;
    }

    InfoDialog *infoDialog = nullptr;
    if (parser.isSet(QStringLiteral("interactive")) || parser.isSet(QStringLiteral("interactive-on-change"))) {
        infoDialog = new InfoDialog(parser.isSet(QStringLiteral("interactive-on-change")));
        Akonadi::ControlGui::widgetNeedsAkonadi(infoDialog);
        infoDialog->show();
    }

    const QString mimeType = parser.value(QStringLiteral("mimetype"));
    if (mimeType.isEmpty()) {
        qCWarning(MIGRATION_LOG) << "set the mimetype to migrate";
        return 5;
    }

    auto migrator = new GidMigrator(mimeType);
    if (infoDialog) {
        infoDialog->migratorAdded();
        QObject::connect(migrator, &MigratorBase::message, infoDialog, qOverload<MigratorBase::MessageType, const QString &>(&InfoDialog::message));
        QObject::connect(migrator, &QObject::destroyed, infoDialog, &InfoDialog::migratorDone);
        QObject::connect(migrator, qOverload<int>(&MigratorBase::progress), infoDialog, qOverload<int>(&InfoDialog::progress));
    }
    QObject::connect(migrator, &GidMigrator::stoppedProcessing, &app, &QApplication::quit);
    migrator->start();
    const int result = app.exec();
    if (InfoDialog::hasError() || migrator->migrationState() == MigratorBase::Failed) {
        return 3;
    }

    return result;
}
