
include_directories(${kdepim-runtime_BINARY_DIR})
add_definitions(-DTRANSLATION_DOMAIN=\"akonadi_newmailnotifier_agent\")
configure_file(config-newmailnotifier.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-newmailnotifier.h)

add_executable( akonadi_newmailnotifier_agent)
set(newmailnotifier_common_SRCS)
kconfig_add_kcfg_files(newmailnotifier_common_SRCS
    newmailnotifieragentsettings.kcfgc
  )

ecm_qt_declare_logging_category(newmailnotifier_common_SRCS HEADER newmailnotifier_debug.h IDENTIFIER NEWMAILNOTIFIER_LOG CATEGORY_NAME org.kde.pim.newmailnotifier
        DESCRIPTION "mailnotifier agent (kdepim-runtime)"
        OLD_CATEGORY_NAMES log_newmailnotifier
        EXPORT KDEPIMRUNTIME
    )



qt_add_dbus_adaptor(newmailnotifieragent_SRCS org.freedesktop.Akonadi.NewMailNotifier.xml newmailnotifieragent.h NewMailNotifierAgent)
target_sources(akonadi_newmailnotifier_agent PRIVATE
  newmailnotifieragent.cpp
  specialnotifierjob.cpp
  newmailnotifiershowmessagejob.cpp
  newmailnotifierreplymessagejob.cpp
  newmailnotifieragent.h
  specialnotifierjob.h
  newmailnotifiershowmessagejob.h
  newmailnotifierreplymessagejob.h

  newmailnotificationhistorywidget.h
  newmailnotificationhistorywidget.cpp
  newmailnotificationhistorydialog.h
  newmailnotificationhistorydialog.cpp
  newmailnotificationhistorymanager.h
  newmailnotificationhistorymanager.cpp
  newmailnotificationhistoryplaintextedit.h
  newmailnotificationhistoryplaintextedit.cpp

  newmailnotificationhistoryplaintexteditor.h
  newmailnotificationhistoryplaintexteditor.cpp

  ${newmailnotifier_common_SRCS}
  ${newmailnotifieragent_SRCS}
)

if (COMPILE_WITH_UNITY_CMAKE_SUPPORT)
    set_target_properties(akonadi_newmailnotifier_agent PROPERTIES UNITY_BUILD ON)
endif()

target_link_libraries( akonadi_newmailnotifier_agent
  KPim${KF_MAJOR_VERSION}::AkonadiCore
  KPim${KF_MAJOR_VERSION}::Mime
  KPim${KF_MAJOR_VERSION}::AkonadiMime
  KPim${KF_MAJOR_VERSION}::AkonadiContact
  KF${KF_MAJOR_VERSION}::Codecs
  KPim${KF_MAJOR_VERSION}::IdentityManagement
  KPim${KF_MAJOR_VERSION}::AkonadiAgentBase
  KF${KF_MAJOR_VERSION}::Notifications
  KF${KF_MAJOR_VERSION}::Service
  KF${KF_MAJOR_VERSION}::I18n
  KF${KF_MAJOR_VERSION}::KIOGui
  KF${KF_MAJOR_VERSION}::WindowSystem
  KPim${KF_MAJOR_VERSION}::PimTextEdit
)

if(Qt${QT_MAJOR_VERSION}TextToSpeech_FOUND)
    target_link_libraries( akonadi_newmailnotifier_agent
      Qt::TextToSpeech
    )
endif()

if( APPLE )
  set_target_properties( akonadi_newmailnotifier_agent PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/Info.plist.template)
  set_target_properties( akonadi_newmailnotifier_agent PROPERTIES MACOSX_BUNDLE_GUI_IDENTIFIER "org.kde.Akonadi.newmailnotifier")
  set_target_properties( akonadi_newmailnotifier_agent PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "KDE New Mail Notifier")
endif ()

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
install(TARGETS akonadi_newmailnotifier_agent ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} )


install(FILES newmailnotifieragent.desktop DESTINATION "${KDE_INSTALL_DATAROOTDIR}/akonadi/agents")
install(FILES akonadi_newmailnotifier_agent.notifyrc DESTINATION ${KDE_INSTALL_KNOTIFYRCDIR} )

########################## Config plugin #################################

kcoreaddons_add_plugin(newmailnotifierconfig
    INSTALL_NAMESPACE "pim${QT_MAJOR_VERSION}/akonadi/config")

target_sources(newmailnotifierconfig PRIVATE
    newmailnotifiersettingswidget.cpp
    newmailnotifierselectcollectionwidget.cpp
    newmailnotifiersettingswidget.h
    newmailnotifierselectcollectionwidget.h
    ${newmailnotifier_common_SRCS}
    )

target_link_libraries(newmailnotifierconfig
    KPim${KF_MAJOR_VERSION}::AkonadiCore
    KPim${KF_MAJOR_VERSION}::AkonadiMime
    Qt::Widgets
    KF${KF_MAJOR_VERSION}::NotifyConfig
    KF${KF_MAJOR_VERSION}::ConfigGui
    KF${KF_MAJOR_VERSION}::I18n
    KF${KF_MAJOR_VERSION}::XmlGui
    KPim${KF_MAJOR_VERSION}::Mime
    KF${KF_MAJOR_VERSION}::Completion
)
