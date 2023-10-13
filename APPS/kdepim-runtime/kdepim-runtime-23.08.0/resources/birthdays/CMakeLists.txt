add_definitions(-DTRANSLATION_DOMAIN=\"akonadi_birthdays_resource\")

set(birthdayresource_common_SRCS)

kconfig_add_kcfg_files(birthdayresource_common_SRCS
    settings.kcfgc
    )

set( birthdayresource_srcs
    birthdaysresource.cpp
    birthdaysresource.h
    ${birthdayresource_common_SRCS}
    )


kcfg_generate_dbus_interface(${CMAKE_CURRENT_SOURCE_DIR}/birthdaysresource.kcfg org.kde.Akonadi.Birthdays.Settings)

qt_add_dbus_adaptor(birthdayresource_srcs
    ${CMAKE_CURRENT_BINARY_DIR}/org.kde.Akonadi.Birthdays.Settings.xml settings.h Settings
    )

ecm_qt_declare_logging_category(birthdayresource_srcs HEADER birthdays_debug.h IDENTIFIER BIRTHDAYS_LOG CATEGORY_NAME org.kde.pim.birthdays
    DESCRIPTION "birthdays resource (kdepim-runtime)"
    OLD_CATEGORY_NAMES log_birthdays
    EXPORT KDEPIMRUNTIME
    )



add_executable(akonadi_birthdays_resource ${birthdayresource_srcs})
if (COMPILE_WITH_UNITY_CMAKE_SUPPORT)
    set_target_properties(akonadi_birthdays_resource PROPERTIES UNITY_BUILD ON)
endif()

if( APPLE )
    set_target_properties(akonadi_birthdays_resource PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/../Info.plist.template)
    set_target_properties(akonadi_birthdays_resource PROPERTIES MACOSX_BUNDLE_GUI_IDENTIFIER "org.kde.Akonadi.Birthdays")
    set_target_properties(akonadi_birthdays_resource PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "KDE Akonadi Birthdays Resource")
endif ()

target_link_libraries(akonadi_birthdays_resource
    KPim${KF_MAJOR_VERSION}::AkonadiCore
    KF${KF_MAJOR_VERSION}::CalendarCore
    KPim${KF_MAJOR_VERSION}::AkonadiAgentBase
    KF${KF_MAJOR_VERSION}::Contacts
    KPim${KF_MAJOR_VERSION}::AkonadiWidgets
    KF${KF_MAJOR_VERSION}::I18n
    KF${KF_MAJOR_VERSION}::Codecs
    )

install( TARGETS akonadi_birthdays_resource ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} )
install( FILES birthdaysresource.desktop DESTINATION "${KDE_INSTALL_DATAROOTDIR}/akonadi/agents" )


############################# Config plugin ################################
kcoreaddons_add_plugin(birthdaysconfig
    INSTALL_NAMESPACE "pim${QT_MAJOR_VERSION}/akonadi/config"
    )
ki18n_wrap_ui(birthdaysconfig birthdaysconfigwidget.ui)
target_sources(birthdaysconfig PRIVATE
    birthdaysconfigagentwidget.cpp
    birthdaysconfigagentwidget.h
    ${birthdayresource_common_SRCS}
    )

target_link_libraries(birthdaysconfig
    KPim${KF_MAJOR_VERSION}::AkonadiCore
    KF${KF_MAJOR_VERSION}::CalendarCore
    KF${KF_MAJOR_VERSION}::Contacts
    KPim${KF_MAJOR_VERSION}::AkonadiWidgets
    KF${KF_MAJOR_VERSION}::I18n
    KF${KF_MAJOR_VERSION}::TextWidgets
    )
