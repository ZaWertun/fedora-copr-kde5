add_definitions(-DTRANSLATION_DOMAIN=\"akonadi_google_resource\")

set(googleresource_SRCS
    googleresource.cpp
    googlesettings.cpp
    googlesettingsdialog.cpp
    defaultreminderattribute.cpp
    googleresourcestate.cpp
    generichandler.cpp
    calendarhandler.cpp
    taskhandler.cpp
    peopleconversionjob.cpp
    personhandler.cpp
    googleresource.h
    googlesettings.h
    googlesettingsdialog.h
    defaultreminderattribute.h
    googleresourcestate.h
    generichandler.h
    calendarhandler.h
    taskhandler.h
    peopleconversionjob.h
    personhandler.h
    )

ecm_qt_declare_logging_category(googleresource_SRCS
    HEADER googleresource_debug.h
    IDENTIFIER GOOGLE_LOG
    CATEGORY_NAME org.kde.pim.google
    DESCRIPTION "resource google (kdepim-runtime)"
    EXPORT KDEPIMRUNTIME)
ecm_qt_declare_logging_category(googleresource_SRCS
    HEADER googlecalendar_debug.h
    IDENTIFIER GOOGLE_CALENDAR_LOG
    CATEGORY_NAME org.kde.pim.google.calendar
    DESCRIPTION "resource google calendar (kdepim-runtime)"
    EXPORT KDEPIMRUNTIME)
ecm_qt_declare_logging_category(googleresource_SRCS
    HEADER googletasks_debug.h
    IDENTIFIER GOOGLE_TASKS_LOG
    CATEGORY_NAME org.kde.pim.google.tasks
    DESCRIPTION "resource google tasks (kdepim-runtime)"
    EXPORT KDEPIMRUNTIME)
ecm_qt_declare_logging_category(googleresource_SRCS
    HEADER googlepeople_debug.h
    IDENTIFIER GOOGLE_PEOPLE_LOG
    CATEGORY_NAME org.kde.pim.google.people
    DESCRIPTION "resource google people (kdepim-runtime)"
    EXPORT KDEPIMRUNTIME)

ki18n_wrap_ui(googleresource_SRCS googlesettingsdialog.ui)

kconfig_add_kcfg_files(googleresource_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/settingsbase.kcfgc)

kcfg_generate_dbus_interface(
  ${CMAKE_CURRENT_SOURCE_DIR}/settingsbase.kcfg
  org.kde.Akonadi.Google.Settings
)

qt_add_dbus_adaptor(googleresource_SRCS
  ${CMAKE_CURRENT_BINARY_DIR}/org.kde.Akonadi.Google.Settings.xml
  ${CMAKE_CURRENT_SOURCE_DIR}/googlesettings.h GoogleSettings
)

add_executable(akonadi_google_resource ${googleresource_SRCS})
if (COMPILE_WITH_UNITY_CMAKE_SUPPORT)
    set_target_properties(akonadi_google_resource PROPERTIES UNITY_BUILD ON)
endif()

if( APPLE )
  set_target_properties(akonadi_google_resource PROPERTIES
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/../../Info.plist.template
  )
  set_target_properties(akonadi_google_resource PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER "org.kde.Akonadi.google"
  )
  set_target_properties(akonadi_google_resource PROPERTIES
    MACOSX_BUNDLE_BUNDLE_NAME "KDE Akonadi Google Resource"
  )
endif()

target_link_libraries(akonadi_google_resource
  KPim${KF_MAJOR_VERSION}::AkonadiCalendar
  KPim${KF_MAJOR_VERSION}::AkonadiCore
  KPim${KF_MAJOR_VERSION}::AkonadiAgentBase
  KF${KF_MAJOR_VERSION}::CalendarCore
  KF${KF_MAJOR_VERSION}::Contacts
  KF${KF_MAJOR_VERSION}::I18n
  KF${KF_MAJOR_VERSION}::WindowSystem
  KF${KF_MAJOR_VERSION}::TextWidgets
  KF${KF_MAJOR_VERSION}::Notifications
  KPim${KF_MAJOR_VERSION}::GAPICalendar
  KPim${KF_MAJOR_VERSION}::GAPIPeople
  KPim${KF_MAJOR_VERSION}::GAPICore
  KPim${KF_MAJOR_VERSION}::GAPITasks
  qt${KF_MAJOR_VERSION}keychain
)

install(TARGETS akonadi_google_resource ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

install(
  FILES googleresource.desktop
  DESTINATION "${KDE_INSTALL_DATAROOTDIR}/akonadi/agents"
)

install(
    FILES org.kde.akonadi_google_resource.desktop
    DESTINATION ${KDE_INSTALL_APPDIR}
)

install(
  FILES akonadi_google_resource.notifyrc
  DESTINATION ${KDE_INSTALL_KNOTIFYRCDIR}
)
