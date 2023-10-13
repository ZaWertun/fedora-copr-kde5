

add_subdirectory( wizard )

add_definitions(-DTRANSLATION_DOMAIN=\"akonadi_contacts_resource\")


set(contactsresource_common_SRCS)

kconfig_add_kcfg_files(contactsresource_common_SRCS
    settings.kcfgc
    )
########### next target ###############

set( contactsresource_SRCS
    contactsresource.cpp
    contactsresource.h
    ${contactsresource_common_SRCS}
    )

kcfg_generate_dbus_interface(${CMAKE_CURRENT_SOURCE_DIR}/contactsresource.kcfg org.kde.Akonadi.Contacts.Settings)
qt_add_dbus_adaptor(contactsresource_SRCS
    ${CMAKE_CURRENT_BINARY_DIR}/org.kde.Akonadi.Contacts.Settings.xml settings.h ContactsResourceSettings contactsresourcesettingsadaptor ContactsResourceSettingsAdaptor
    )

ecm_qt_declare_logging_category(contactsresource_SRCS HEADER contacts_resources_debug.h IDENTIFIER CONTACTSRESOURCES_LOG CATEGORY_NAME org.kde.pim.resources_contacts
    DESCRIPTION "contacts resource (kdepim-runtime)"
    OLD_CATEGORY_NAMES log_resources_contacts
    EXPORT KDEPIMRUNTIME
    )


install( FILES contactsresource.desktop DESTINATION "${KDE_INSTALL_DATAROOTDIR}/akonadi/agents" )

add_executable(akonadi_contacts_resource ${contactsresource_SRCS})
if (COMPILE_WITH_UNITY_CMAKE_SUPPORT)
    set_target_properties(akonadi_contacts_resource PROPERTIES UNITY_BUILD ON)
endif()
target_link_libraries(akonadi_contacts_resource
    KPim${KF_MAJOR_VERSION}::AkonadiCore
    KPim${KF_MAJOR_VERSION}::AkonadiAgentBase
    KF${KF_MAJOR_VERSION}::Contacts
    KF${KF_MAJOR_VERSION}::I18n
    KF${KF_MAJOR_VERSION}::KIOWidgets
    KF${KF_MAJOR_VERSION}::ConfigWidgets
    )

install(TARGETS akonadi_contacts_resource ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

install(
    FILES org.kde.akonadi_contacts_resource.desktop
    DESTINATION ${KDE_INSTALL_APPDIR}
)

############################## Config plugin ################################
kcoreaddons_add_plugin(contactsconfig
    INSTALL_NAMESPACE "pim${QT_MAJOR_VERSION}/akonadi/config"
    )
ki18n_wrap_ui(contactsconfig contactsagentsettingswidget.ui)

target_sources(contactsconfig PRIVATE
    contactssettingswidget.cpp
    contactssettingswidget.h
    ${contactsresource_common_SRCS}
    )


target_link_libraries(contactsconfig
    KPim${KF_MAJOR_VERSION}::AkonadiCore
    KF${KF_MAJOR_VERSION}::CalendarCore
    KF${KF_MAJOR_VERSION}::Codecs
    KPim${KF_MAJOR_VERSION}::AkonadiAgentBase
    KF${KF_MAJOR_VERSION}::Contacts
    KPim${KF_MAJOR_VERSION}::AkonadiWidgets
    KF${KF_MAJOR_VERSION}::I18n
    KF${KF_MAJOR_VERSION}::KIOWidgets
    )
