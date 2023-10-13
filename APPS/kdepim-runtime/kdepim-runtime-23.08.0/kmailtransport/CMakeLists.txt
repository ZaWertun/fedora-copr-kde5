kcoreaddons_add_plugin(mailtransport_akonadiplugin INSTALL_NAMESPACE "pim${QT_MAJOR_VERSION}/mailtransport")
target_sources(mailtransport_akonadiplugin PRIVATE
    akonadimailtransportplugin.cpp
    resourcesendjob.cpp
    resourcesendjob_p.h
    akonadimailtransportplugin.h
    )

ecm_qt_declare_logging_category(mailtransport_akonadiplugin HEADER mailtransportplugin_akonadi_debug.h IDENTIFIER
        MAILTRANSPORT_AKONADI_LOG CATEGORY_NAME org.kde.pim.mailtransport.akonadiplugin
        DESCRIPTION "kmailtransport (akonadi plugin)"
        EXPORT KDEPIMRUNTIME
    )




target_link_libraries(mailtransport_akonadiplugin
    KPim${KF_MAJOR_VERSION}::AkonadiMime
    KPim${KF_MAJOR_VERSION}::Mime
    KPim${KF_MAJOR_VERSION}::MailTransport
    KF${KF_MAJOR_VERSION}::CoreAddons
    KPim${KF_MAJOR_VERSION}::AkonadiCore
    KPim${KF_MAJOR_VERSION}::AkonadiWidgets
    KF${KF_MAJOR_VERSION}::I18n
    KF${KF_MAJOR_VERSION}::ConfigWidgets
    Qt::DBus
    )
