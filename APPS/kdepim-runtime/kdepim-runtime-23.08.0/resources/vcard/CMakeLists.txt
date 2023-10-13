

if(BUILD_TESTING)
    add_subdirectory( autotests )
endif()

add_definitions(-DTRANSLATION_DOMAIN=\"akonadi_vcard_resource\")

set(vcardresource_common_SRCS)
kconfig_add_kcfg_files(vcardresource_common_SRCS settings.kcfgc)

############################## Resource ####################################

add_definitions(-DSETTINGS_NAMESPACE=Akonadi_VCard_Resource)

set( vcardresource_SRCS
  vcardresource.cpp
  vcardresource.h
  ${vcardresource_common_SRCS}
)

install( FILES vcardresource.desktop DESTINATION "${KDE_INSTALL_DATAROOTDIR}/akonadi/agents" )

kcfg_generate_dbus_interface(${CMAKE_CURRENT_SOURCE_DIR}/vcardresource.kcfg org.kde.Akonadi.VCard.Settings)
qt_add_dbus_adaptor(vcardresource_SRCS
  ${CMAKE_CURRENT_BINARY_DIR}/org.kde.Akonadi.VCard.Settings.xml settings.h Akonadi_VCard_Resource::Settings vcardsettingsadaptor VCardSettingsAdaptor
)

add_executable(akonadi_vcard_resource ${vcardresource_SRCS})

add_subdirectory( wizard )

if( APPLE )
  set_target_properties(akonadi_vcard_resource PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/../Info.plist.template)
  set_target_properties(akonadi_vcard_resource PROPERTIES MACOSX_BUNDLE_GUI_IDENTIFIER "org.kde.Akonadi.VCard")
  set_target_properties(akonadi_vcard_resource PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "KDE Akonadi VCard Resource")
endif ()

target_link_libraries(akonadi_vcard_resource KPim${KF_MAJOR_VERSION}::AkonadiCore KF${KF_MAJOR_VERSION}::KIOCore   KPim${KF_MAJOR_VERSION}::AkonadiAgentBase KF${KF_MAJOR_VERSION}::Contacts akonadi-singlefileresource KF${KF_MAJOR_VERSION}::Completion KF${KF_MAJOR_VERSION}::KIOWidgets KF${KF_MAJOR_VERSION}::ConfigWidgets)

install(TARGETS akonadi_vcard_resource ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

install(
    FILES org.kde.akonadi_vcard_resource.desktop
    DESTINATION ${KDE_INSTALL_APPDIR}
)

############################# Config plugin #################################

kcoreaddons_add_plugin(vcardconfig
    SOURCES vcardconfig.cpp ${vcardresource_common_SRCS}
    INSTALL_NAMESPACE "pim${QT_MAJOR_VERSION}/akonadi/config"
)

target_link_libraries(vcardconfig
    KPim${KF_MAJOR_VERSION}::AkonadiCore
    akonadi-singlefileresource
)
