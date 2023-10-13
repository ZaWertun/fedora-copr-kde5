add_definitions(-DTRANSLATION_DOMAIN=\"akonadi_tomboynotes_resource\")


########### next target ###############

set(tomboynotesresource_common_SRCS)

kconfig_add_kcfg_files(tomboynotesresource_common_SRCS
    settings.kcfgc
    )

set(tomboynotesresource_SRCS
    tomboynotesresource.cpp
    tomboycollectionsdownloadjob.cpp
    tomboyitemdownloadjob.cpp
    tomboyitemuploadjob.cpp
    tomboyitemsdownloadjob.cpp
    tomboyjobbase.cpp
    tomboyserverauthenticatejob.cpp
    o1tomboy.cpp
    o2/o0settingsstore.cpp
    o2/o0baseauth.cpp
    o2/o0abstractstore.h
    o2/o0globals.h
    o2/o1.cpp
    o2/o1requestor.cpp
    o2/o1timedreply.cpp
    o2/o2.cpp
    o2/o2reply.cpp
    o2/o2replyserver.cpp
    o2/o2requestor.cpp
    o2/o2simplecrypt.cpp

    tomboynotesresource.h
    tomboycollectionsdownloadjob.h
    tomboyitemdownloadjob.h
    tomboyitemuploadjob.h
    tomboyitemsdownloadjob.h
    tomboyjobbase.h
    tomboyserverauthenticatejob.h
    o1tomboy.h
    o2/o0settingsstore.h
    o2/o0baseauth.h
    o2/o0abstractstore.h
    o2/o0globals.h
    o2/o1.h
    o2/o1requestor.h
    o2/o1timedreply.h
    o2/o2.h
    o2/o2reply.h
    o2/o2replyserver.h
    o2/o2requestor.h
    )

ecm_qt_declare_logging_category(tomboynotesresource_SRCS
    HEADER debug.h
    IDENTIFIER TOMBOYNOTESRESOURCE_LOG
    CATEGORY_NAME org.kde.pim.tomboynotesresource
    DESCRIPTION "tomboynote resource  (kdepim-runtime)"
    OLD_CATEGORY_NAMES log_tomboynotesresource
    EXPORT KDEPIMRUNTIME
    )




kconfig_add_kcfg_files(tomboynotesresource_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/settings.kcfgc
    )

kcfg_generate_dbus_interface(
    ${CMAKE_CURRENT_SOURCE_DIR}/tomboynotesresource.kcfg
    org.kde.Akonadi.TomboyNotes.Settings
    )

qt_add_dbus_adaptor(tomboynotesresource_SRCS
    ${CMAKE_CURRENT_BINARY_DIR}/org.kde.Akonadi.TomboyNotes.Settings.xml
    ${CMAKE_CURRENT_BINARY_DIR}/settings.h
    Settings
    )


add_executable(akonadi_tomboynotes_resource ${tomboynotesresource_SRCS})
if (COMPILE_WITH_UNITY_CMAKE_SUPPORT)
    set_target_properties(akonadi_tomboynotes_resource PROPERTIES UNITY_BUILD ON)
endif()
target_link_libraries(akonadi_tomboynotes_resource
    Qt::DBus
    Qt::Gui
    Qt::Network
    Qt::WebEngineWidgets
    KPim${KF_MAJOR_VERSION}::AkonadiAgentBase
    KPim${KF_MAJOR_VERSION}::AkonadiNotes
    KF${KF_MAJOR_VERSION}::ConfigCore
    KF${KF_MAJOR_VERSION}::ConfigWidgets
    KF${KF_MAJOR_VERSION}::I18n
    KPim${KF_MAJOR_VERSION}::Mime
    )

install(TARGETS akonadi_tomboynotes_resource ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES tomboynotesresource.desktop
    DESTINATION "${KDE_INSTALL_DATAROOTDIR}/akonadi/agents"
    )

############################# Config plugin ################################
kcoreaddons_add_plugin(tomboynotesconfig
    INSTALL_NAMESPACE "pim${QT_MAJOR_VERSION}/akonadi/config"
    )
ki18n_wrap_ui(tomboynotesconfig tomboynotesagentconfigwidget.ui)
target_sources(tomboynotesconfig PRIVATE
    tomboynotesconfigwidget.cpp
    tomboynotesconfigwidget.h
    ${tomboynotesresource_common_SRCS}
    )


target_link_libraries(tomboynotesconfig
    KPim${KF_MAJOR_VERSION}::AkonadiCore
    KF${KF_MAJOR_VERSION}::CalendarCore
    KF${KF_MAJOR_VERSION}::Contacts
    KPim${KF_MAJOR_VERSION}::AkonadiWidgets
    KF${KF_MAJOR_VERSION}::I18n
    KF${KF_MAJOR_VERSION}::ConfigWidgets
    KF${KF_MAJOR_VERSION}::Completion
    )
