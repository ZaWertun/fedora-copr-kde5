add_definitions(-DTRANSLATION_DOMAIN=\"akonadi_vcarddir_resource\")

########### next target ###############

set(vcarddirresource_common_SRCS)

kconfig_add_kcfg_files(vcarddirresource_common_SRCS
    settings.kcfgc
  )

set( vcarddirresource_SRCS
    ${vcarddirresource_common_SRCS}
  vcarddirresource.cpp
  vcarddirresource.h
)
ecm_qt_declare_logging_category(vcarddirresource_SRCS HEADER vcarddirresource_debug.h IDENTIFIER VCARDDIRRESOURCE_LOG CATEGORY_NAME org.kde.pim.vcarddirresource
        DESCRIPTION "vcarddir resource (kdepim-runtime)"
        EXPORT KDEPIMRUNTIME
    )



kcfg_generate_dbus_interface(${CMAKE_CURRENT_SOURCE_DIR}/vcarddirresource.kcfg org.kde.Akonadi.VCardDirectory.Settings)
qt_add_dbus_adaptor(vcarddirresource_SRCS
  ${CMAKE_CURRENT_BINARY_DIR}/org.kde.Akonadi.VCardDirectory.Settings.xml settings.h VcardDirResourceSettings
)

install( FILES vcarddirresource.desktop DESTINATION "${KDE_INSTALL_DATAROOTDIR}/akonadi/agents" )

add_executable(akonadi_vcarddir_resource ${vcarddirresource_SRCS})

if( APPLE )
  set_target_properties(akonadi_vcarddir_resource PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/../Info.plist.template)
  set_target_properties(akonadi_vcarddir_resource PROPERTIES MACOSX_BUNDLE_GUI_IDENTIFIER "org.kde.Akonadi.VCardDirectory")
  set_target_properties(akonadi_vcarddir_resource PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "KDE Akonadi VCardDirectory Resource")
endif ()


target_link_libraries(akonadi_vcarddir_resource
  KPim${KF_MAJOR_VERSION}::AkonadiCore
  KPim${KF_MAJOR_VERSION}::AkonadiAgentBase
  KF${KF_MAJOR_VERSION}::Contacts
  KF${KF_MAJOR_VERSION}::I18n
  KF${KF_MAJOR_VERSION}::TextWidgets
  KF${KF_MAJOR_VERSION}::KIOWidgets
  KF${KF_MAJOR_VERSION}::ConfigWidgets
  KF${KF_MAJOR_VERSION}::WindowSystem
  Qt::DBus
)

add_dependencies(akonadi_vcarddir_resource akonadi-singlefileresource)

add_subdirectory(wizard)

install(TARGETS akonadi_vcarddir_resource ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

install(
    FILES org.kde.akonadi_vcarddir_resource.desktop
    DESTINATION ${KDE_INSTALL_APPDIR}
)

############################## Config plugin ################################
kcoreaddons_add_plugin(vcarddirconfig
    INSTALL_NAMESPACE "pim${QT_MAJOR_VERSION}/akonadi/config"
    )
ki18n_wrap_ui(vcarddirconfig vcarddiragentsettingswidget.ui)

target_sources(vcarddirconfig PRIVATE
    vcarddirsettingswidget.cpp
    vcarddirsettingswidget.h
    ${vcarddirconfig_ui_SRCS}
    ${vcarddirresource_common_SRCS}
    )


target_link_libraries(vcarddirconfig
    KPim${KF_MAJOR_VERSION}::AkonadiCore
    KF${KF_MAJOR_VERSION}::CalendarCore
    KPim${KF_MAJOR_VERSION}::AkonadiAgentBase
    KPim${KF_MAJOR_VERSION}::AkonadiWidgets
    KF${KF_MAJOR_VERSION}::I18n
    KF${KF_MAJOR_VERSION}::KIOWidgets
    KF${KF_MAJOR_VERSION}::TextWidgets
    )
