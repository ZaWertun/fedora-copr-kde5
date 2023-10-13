
add_definitions(-DTRANSLATION_DOMAIN=\"akonadi_maildispatcher_agent\")
add_executable(akonadi_maildispatcher_agent)


ecm_qt_declare_logging_category(akonadi_maildispatcher_agent HEADER maildispatcher_debug.h IDENTIFIER MAILDISPATCHER_LOG CATEGORY_NAME org.kde.pim.maildispatcher
        DESCRIPTION "maildispacher agent (kdepim-runtime)"
        OLD_CATEGORY_NAMES log_maildispatcher
        EXPORT KDEPIMRUNTIME
    )

kconfig_add_kcfg_files(maildispatcheragent_SRCS settings.kcfgc)
kcfg_generate_dbus_interface(${CMAKE_CURRENT_SOURCE_DIR}/maildispatcheragent.kcfg org.kde.Akonadi.MailDispatcher.Settings)
qt_add_dbus_adaptor(maildispatcheragent_SRCS
  ${CMAKE_CURRENT_BINARY_DIR}/org.kde.Akonadi.MailDispatcher.Settings.xml settings.h Settings
)
qt_add_dbus_adaptor( maildispatcheragent_SRCS
  org.freedesktop.Akonadi.MailDispatcherAgent.xml maildispatcheragent.h MailDispatcherAgent
)
target_sources( akonadi_maildispatcher_agent PRIVATE
  maildispatcheragent.cpp
  outboxqueue.cpp
  sendjob.cpp
  sentactionhandler.cpp
  storeresultjob.cpp

  maildispatcheragent.h
  outboxqueue.h
  sendjob.h
  sentactionhandler.h
  storeresultjob.h
  ${maildispatcheragent_SRCS}
)

if (COMPILE_WITH_UNITY_CMAKE_SUPPORT)
    set_target_properties(akonadi_maildispatcher_agent PROPERTIES UNITY_BUILD ON)
endif()

if( APPLE )
  set_target_properties(akonadi_maildispatcher_agent PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/../Info.plist.template)
  set_target_properties(akonadi_maildispatcher_agent PROPERTIES MACOSX_BUNDLE_GUI_IDENTIFIER "org.kde.Akonadi.maildispatcher")
  set_target_properties(akonadi_maildispatcher_agent PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "KDE Akonadi Maildispatcher")
endif ()

target_link_libraries(akonadi_maildispatcher_agent 
    KPim${KF_MAJOR_VERSION}::AkonadiCore
    KPim${KF_MAJOR_VERSION}::AkonadiMime
    KPim${KF_MAJOR_VERSION}::Mime
    KPim${KF_MAJOR_VERSION}::MailTransport
    KPim${KF_MAJOR_VERSION}::AkonadiAgentBase
    KF${KF_MAJOR_VERSION}::I18n
    KF${KF_MAJOR_VERSION}::Notifications
)

install( TARGETS akonadi_maildispatcher_agent ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} )
install( FILES maildispatcheragent.desktop DESTINATION "${KDE_INSTALL_DATAROOTDIR}/akonadi/agents" )
install( FILES akonadi_maildispatcher_agent.notifyrc DESTINATION ${KDE_INSTALL_KNOTIFYRCDIR} )

############################## Configuration ##############################

kcoreaddons_add_plugin(maildispatcherconfig
    SOURCES maildispatcherconfig.cpp maildispatcherconfig.h
    INSTALL_NAMESPACE "pim${QT_MAJOR_VERSION}/akonadi/config")
target_link_libraries(maildispatcherconfig
    KPim${KF_MAJOR_VERSION}::AkonadiCore
    KF${KF_MAJOR_VERSION}::NotifyConfig
    KF${KF_MAJOR_VERSION}::ConfigCore
)
