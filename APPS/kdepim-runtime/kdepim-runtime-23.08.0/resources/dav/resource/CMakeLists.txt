add_definitions(-DTRANSLATION_DOMAIN=\"akonadi_davgroupware_resource\")

if(WIN32)
    set(LIB_INSTALL_DIR ${KDE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${KDE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${KDE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${KDE_INSTALL_LIBDIR} )
endif()

########### next target ###############
set(davgroupwareresource_common_SRCS)
kconfig_add_kcfg_files(davgroupwareresource_common_SRCS settingsbase.kcfgc)

set(davgroupwareconfig_SRCS
    configdialog.cpp
    searchdialog.cpp
    setupwizard.cpp
    configdialog.h
    searchdialog.h
    setupwizard.h

    )
ki18n_wrap_ui(davgroupwareconfig_SRCS configdialog.ui urlconfigurationdialog.ui searchdialog.ui)


set( davgroupwareresource_SRCS
    ${davgroupwareresource_common_SRCS}
    ${davgroupwareconfig_SRCS}
    daverror-kdepim-runtime.cpp
    akonadietagcache.cpp
    utils.cpp

    ctagattribute.cpp
    davfreebusyhandler.cpp
    davprotocolattribute.cpp
    settings.cpp
    urlconfigurationdialog.cpp
    davgroupwareresource.cpp
    daverror-kdepim-runtime.h
    akonadietagcache.h
    utils.h

    ctagattribute.h
    davfreebusyhandler.h
    davprotocolattribute.h
    settings.h
    urlconfigurationdialog.h
    davgroupwareresource.h
    )

ecm_qt_declare_logging_category(davgroupwareresource_SRCS HEADER davresource_debug.h IDENTIFIER DAVRESOURCE_LOG CATEGORY_NAME org.kde.pim.davresource
    DESCRIPTION "dav resource (kdepim-runtime)"
    OLD_CATEGORY_NAMES log_davresource
    EXPORT KDEPIMRUNTIME
    )


install( FILES davgroupwareresource.desktop DESTINATION "${KDE_INSTALL_DATAROOTDIR}/akonadi/agents" )
install( FILES org.kde.akonadi_davgroupware_resource.desktop DESTINATION ${KDE_INSTALL_APPDIR} )

file( GLOB providersFiles "../services/*.desktop" )
install( FILES ${providersFiles} DESTINATION "${KDE_INSTALL_KSERVICESDIR}/akonadi/davgroupware-providers" )

kcfg_generate_dbus_interface(${CMAKE_CURRENT_SOURCE_DIR}/davgroupwareresource.kcfg org.kde.Akonadi.davGroupware.Settings)
qt_add_dbus_adaptor(davgroupwareresource_SRCS
    ${CMAKE_CURRENT_BINARY_DIR}/org.kde.Akonadi.davGroupware.Settings.xml settings.h Settings
    )

add_executable(akonadi_davgroupware_resource ${davgroupwareresource_SRCS})
if (COMPILE_WITH_UNITY_CMAKE_SUPPORT)
    set_target_properties(akonadi_davgroupware_resource PROPERTIES UNITY_BUILD ON)
endif()

if( APPLE )
    set_target_properties(akonadi_davgroupware_resource PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/../../Info.plist.template)
    set_target_properties(akonadi_davgroupware_resource PROPERTIES MACOSX_BUNDLE_GUI_IDENTIFIER "org.kde.Akonadi.davGroupware")
    set_target_properties(akonadi_davgroupware_resource PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "KDE Akonadi davGroupware Resource")
endif ()

target_link_libraries(akonadi_davgroupware_resource
    KF${KF_MAJOR_VERSION}::Contacts
    KPim${KF_MAJOR_VERSION}::AkonadiCalendar
    KF${KF_MAJOR_VERSION}::KIOCore
    KPim${KF_MAJOR_VERSION}::AkonadiAgentBase
    KPim${KF_MAJOR_VERSION}::AkonadiCore
    KF${KF_MAJOR_VERSION}::CalendarCore
    KF${KF_MAJOR_VERSION}::Completion
    KF${KF_MAJOR_VERSION}::ConfigCore
    KF${KF_MAJOR_VERSION}::ConfigWidgets
    KF${KF_MAJOR_VERSION}::WindowSystem
    KF${KF_MAJOR_VERSION}::DAV
    KF${KF_MAJOR_VERSION}::Service
    Qt::Xml
    qt${KF_MAJOR_VERSION}keychain
    )

install(TARGETS akonadi_davgroupware_resource ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

