add_definitions(-DTRANSLATION_DOMAIN=\"akonadi_etesync_resource\")

set(etesyncresource_common_SRCS
    etebaseadapter.cpp etebaseadapter.h
)

kconfig_add_kcfg_files(etesyncresource_common_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/settings.kcfgc
)

set(etesyncconfig_SRCS
  setupwizard.cpp
  ${etesyncresource_common_SRCS}
)

set(etesyncresource_SRCS
    etesyncresource.cpp etesyncresource.h
    etesyncclientstate.cpp etesyncclientstate.h
    journalsfetchjob.cpp journalsfetchjob.h
    entriesfetchjob.cpp entriesfetchjob.h
    loginjob.cpp loginjob.h 
    setupwizard.cpp

    ${etesyncconfig_SRCS}
)

ecm_qt_declare_logging_category(etesyncresource_SRCS
    HEADER etesync_debug.h
    IDENTIFIER ETESYNC_LOG
    CATEGORY_NAME org.kde.pim.etesync
    DESCRIPTION "resource etesync (kdepim-runtime)"
    EXPORT KDEPIMRUNTIME
)

kcfg_generate_dbus_interface(
    ${CMAKE_CURRENT_SOURCE_DIR}/etesyncresource.kcfg
    org.kde.Akonadi.etesync.Settings
)

qt_add_dbus_adaptor(etesyncresource_SRCS
    ${CMAKE_CURRENT_BINARY_DIR}/org.kde.Akonadi.etesync.Settings.xml
    ${CMAKE_CURRENT_BINARY_DIR}/settings.h
    Settings
)

add_executable(akonadi_etesync_resource ${etesyncresource_SRCS})
set_target_properties(akonadi_etesync_resource PROPERTIES MACOSX_BUNDLE FALSE)
target_include_directories(akonadi_etesync_resource PRIVATE ${ETEBASE_INCLUDE_DIRS})
if (COMPILE_WITH_UNITY_CMAKE_SUPPORT)
    set_target_properties(akonadi_etesync_resource PROPERTIES UNITY_BUILD ON)
endif()
target_link_libraries(akonadi_etesync_resource
    ${ETEBASE_LIBRARIES}
    Qt::DBus
    KPim${KF_MAJOR_VERSION}::AkonadiAgentBase
    KF${KF_MAJOR_VERSION}::ConfigCore
    KF${KF_MAJOR_VERSION}::Contacts
    KF${KF_MAJOR_VERSION}::CalendarCore
    KF${KF_MAJOR_VERSION}::KIOCore
    KF${KF_MAJOR_VERSION}::ConfigWidgets
    KF${KF_MAJOR_VERSION}::WindowSystem
    KF${KF_MAJOR_VERSION}::Wallet
    KF${KF_MAJOR_VERSION}::I18n
)

ecm_install_icons(
    ICONS
    icons/16-apps-akonadi-etesync.png
    icons/24-apps-akonadi-etesync.png
    icons/32-apps-akonadi-etesync.png
    icons/48-apps-akonadi-etesync.png
    icons/64-apps-akonadi-etesync.png
    icons/72-apps-akonadi-etesync.png
    icons/128-apps-akonadi-etesync.png
    DESTINATION ${KDE_INSTALL_ICONDIR}
    THEME hicolor
    )

install(TARGETS akonadi_etesync_resource ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES etesyncresource.desktop
    DESTINATION ${KDE_INSTALL_DATAROOTDIR}/akonadi/agents
)
