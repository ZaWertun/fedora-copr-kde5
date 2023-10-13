include_directories(
    ${kdepim-runtime_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/shared
)

add_definitions(-DTRANSLATION_DOMAIN=\"akonadi_ical_resource\")

add_subdirectory( wizard )
add_subdirectory( notes )

if(BUILD_TESTING)
    add_subdirectory( autotests )
endif()

set(icalresource_common_SRCS)
kconfig_add_kcfg_files(icalresource_common_SRCS settings.kcfgc)

############################# Resource #################################
add_definitions( -DSETTINGS_NAMESPACE=Akonadi_ICal_Resource )
add_executable(akonadi_ical_resource)



kcfg_generate_dbus_interface(${CMAKE_CURRENT_SOURCE_DIR}/icalresource.kcfg org.kde.Akonadi.ICal.Settings)
qt_add_dbus_adaptor(icalresource_SRCS
  ${CMAKE_CURRENT_BINARY_DIR}/org.kde.Akonadi.ICal.Settings.xml settings.h Akonadi_ICal_Resource::Settings icalsettingsadaptor ICalSettingsAdaptor
)

if (COMPILE_WITH_UNITY_CMAKE_SUPPORT)
    set_target_properties(akonadi_ical_resource PROPERTIES UNITY_BUILD ON)
endif()
target_sources( akonadi_ical_resource PRIVATE
  icalresource.cpp
  shared/icalresourcebase.cpp
  shared/icalresource.cpp
  shared/icalresourcebase.h
  shared/icalresource.h
  ${icalresource_common_SRCS}
  ${icalresource_SRCS}
)

if( APPLE )
  set_target_properties(akonadi_ical_resource PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/../Info.plist.template)
  set_target_properties(akonadi_ical_resource PROPERTIES MACOSX_BUNDLE_GUI_IDENTIFIER "org.kde.Akonadi.ICal")
  set_target_properties(akonadi_ical_resource PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "KDE Akonadi iCal Resource")
endif ()

install( FILES icalresource.desktop DESTINATION "${KDE_INSTALL_DATAROOTDIR}/akonadi/agents" )
target_link_libraries(akonadi_ical_resource KPim${KF_MAJOR_VERSION}::AkonadiCore   KF${KF_MAJOR_VERSION}::KIOCore KF${KF_MAJOR_VERSION}::CalendarCore KPim${KF_MAJOR_VERSION}::AkonadiAgentBase akonadi-singlefileresource)
if (COMPILE_WITH_UNITY_CMAKE_SUPPORT)
    set_target_properties(akonadi_ical_resource PROPERTIES UNITY_BUILD ON)
endif()
install(TARGETS akonadi_ical_resource ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

################################ Config plugin ###############################

kcoreaddons_add_plugin(icalconfig
    INSTALL_NAMESPACE "pim${QT_MAJOR_VERSION}/akonadi/config"
)
target_sources(icalconfig PRIVATE icalconfig.cpp ${icalresource_common_SRCS})
target_link_libraries(icalconfig
    KPim${KF_MAJOR_VERSION}::AkonadiCore
    akonadi-singlefileresource
)
