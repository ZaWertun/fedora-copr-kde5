include_directories(
    ${kdepim-runtime_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../maildir
    ${CMAKE_CURRENT_BINARY_DIR}/../maildir
    ${CMAKE_CURRENT_SOURCE_DIR}/../maildir/libmaildir
)


add_definitions(-DTRANSLATION_DOMAIN=\"akonadi_akonotes_resource\")

set(akonotes_common_SRCS)
kconfig_add_kcfg_files(akonotes_common_SRCS ../maildir/settings.kcfgc)

################################# Resource #################################

set( akonotesresource_SRCS
  ../maildir/maildirresource.cpp
  ../maildir/retrieveitemsjob.cpp
  ../maildir/maildirresource_debug.cpp
  akonotesresource.cpp
  ../maildir/maildirresource.h
  ../maildir/retrieveitemsjob.h
  ../maildir/maildirresource_debug.h
  akonotesresource.h
  ${akonotes_common_SRCS}
)

kcfg_generate_dbus_interface(${CMAKE_CURRENT_SOURCE_DIR}/../maildir/maildirresource.kcfg org.kde.Akonadi.Maildir.Settings)

qt_add_dbus_adaptor(akonotesresource_SRCS
  ${CMAKE_CURRENT_BINARY_DIR}/org.kde.Akonadi.Maildir.Settings.xml settings.h Akonadi_Maildir_Resource::MaildirSettings maildirsettingsadaptor MaildirSettingsAdaptor
)

install( FILES akonotesresource.desktop DESTINATION "${KDE_INSTALL_DATAROOTDIR}/akonadi/agents" )

add_executable(akonadi_akonotes_resource ${akonotesresource_SRCS})
if (COMPILE_WITH_UNITY_CMAKE_SUPPORT)
    set_target_properties(akonadi_akonotes_resource PROPERTIES UNITY_BUILD ON)
endif()

target_link_libraries(akonadi_akonotes_resource
    KPim${KF_MAJOR_VERSION}::AkonadiCore
    KPim${KF_MAJOR_VERSION}::AkonadiMime
    KPim${KF_MAJOR_VERSION}::Mime
    KPim${KF_MAJOR_VERSION}::AkonadiAgentBase
    KF${KF_MAJOR_VERSION}::I18n
    maildir
    folderarchivesettings
)

install(TARGETS akonadi_akonotes_resource ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

############################### Config plugin #################################


kcoreaddons_add_plugin(akonotesconfig
    INSTALL_NAMESPACE pim${QT_MAJOR_VERSION}/akonadi/config
)

ki18n_wrap_ui(akonotesconfig ../maildir/settings.ui)
target_sources(akonotesconfig PRIVATE
    ../maildir/configwidget.cpp
    ../maildir/maildirconfig.cpp
    akonotesconfig.cpp
    ../maildir/configwidget.h
    ../maildir/maildirconfig.h
    ${akonotes_common_SRCS}
)
if (COMPILE_WITH_UNITY_CMAKE_SUPPORT)
    set_target_properties(akonotesconfig PROPERTIES UNITY_BUILD ON)
endif()

target_link_libraries(akonotesconfig
    KPim${KF_MAJOR_VERSION}::AkonadiCore
    KF${KF_MAJOR_VERSION}::KIOWidgets
    KF${KF_MAJOR_VERSION}::ConfigWidgets
    KF${KF_MAJOR_VERSION}::I18n
    maildir
    folderarchivesettings
)
