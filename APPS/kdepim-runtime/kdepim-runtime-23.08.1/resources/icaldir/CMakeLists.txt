
add_definitions(-DTRANSLATION_DOMAIN=\"akonadi_icaldir_resource\")


kconfig_add_kcfg_files(icaldirresource_common_SRCS
    settings.kcfgc
  )
########### next target ###############

set( icaldirresource_SRCS
  icaldirresource.cpp
  icaldirresource.h
  ${icaldirresource_common_SRCS}
)

kcfg_generate_dbus_interface(${CMAKE_CURRENT_SOURCE_DIR}/icaldirresource.kcfg org.kde.Akonadi.ICalDirectory.Settings)
qt_add_dbus_adaptor(icaldirresource_SRCS
  ${CMAKE_CURRENT_BINARY_DIR}/org.kde.Akonadi.ICalDirectory.Settings.xml settings.h IcalDirResourceSettings
)

install( FILES icaldirresource.desktop DESTINATION "${KDE_INSTALL_DATAROOTDIR}/akonadi/agents" )

add_executable(akonadi_icaldir_resource ${icaldirresource_SRCS})
if (COMPILE_WITH_UNITY_CMAKE_SUPPORT)
    set_target_properties(akonadi_icaldir_resource PROPERTIES UNITY_BUILD ON)
endif()

if( APPLE )
  set_target_properties(akonadi_icaldir_resource PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/../Info.plist.template)
  set_target_properties(akonadi_icaldir_resource PROPERTIES MACOSX_BUNDLE_GUI_IDENTIFIER "org.kde.Akonadi.ICalDirectory")
  set_target_properties(akonadi_icaldir_resource PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "KDE Akonadi ICalDirectory Resource")
endif ()


target_link_libraries(akonadi_icaldir_resource
  KPim${KF_MAJOR_VERSION}::AkonadiCore
  KPim${KF_MAJOR_VERSION}::AkonadiAgentBase
  KF${KF_MAJOR_VERSION}::CalendarCore
  KF${KF_MAJOR_VERSION}::I18n
  KF${KF_MAJOR_VERSION}::TextWidgets
  KF${KF_MAJOR_VERSION}::KIOWidgets
  KF${KF_MAJOR_VERSION}::ConfigWidgets
)

install(TARGETS akonadi_icaldir_resource ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

############################## Config plugin ################################
kcoreaddons_add_plugin(icaldirconfig
    INSTALL_NAMESPACE "pim${QT_MAJOR_VERSION}/akonadi/config"
    )
ki18n_wrap_ui(icaldirconfig icaldirsagentsettingswidget.ui)

target_sources(icaldirconfig PRIVATE
    icaldirsettingswidget.cpp
    icaldirsettingswidget.h
    ${icaldirresource_common_SRCS}
    )


target_link_libraries(icaldirconfig
    KPim${KF_MAJOR_VERSION}::AkonadiCore
    KF${KF_MAJOR_VERSION}::CalendarCore
    KPim${KF_MAJOR_VERSION}::AkonadiAgentBase
    KF${KF_MAJOR_VERSION}::Contacts
    KPim${KF_MAJOR_VERSION}::AkonadiWidgets
    KF${KF_MAJOR_VERSION}::I18n
    KF${KF_MAJOR_VERSION}::KIOWidgets
    KF${KF_MAJOR_VERSION}::TextWidgets
    )
