include_directories(${CMAKE_BINARY_DIR}/resources/google-groupware)

set(googlegroupwaremigration_SRCS
    googleresourcemigrator.cpp
    googleresourcemigrator.h
    ${MIGRATION_AKONADI_SHARED_SOURCES}
    )

kcfg_generate_dbus_interface(
    ${CMAKE_SOURCE_DIR}/resources/google-groupware/settingsbase.kcfg
    org.kde.Akonadi.Google.Settings
)

qt_add_dbus_interface(googlegroupwaremigration_SRCS
  ${CMAKE_CURRENT_BINARY_DIR}/org.kde.Akonadi.Google.Settings.xml
  googlesettingsinterface
)

add_library(googlegroupwaremigration STATIC ${googlegroupwaremigration_SRCS})
if (COMPILE_WITH_UNITY_CMAKE_SUPPORT)
    set_target_properties(googlegroupwaremigration PROPERTIES UNITY_BUILD ON)
endif()
target_link_libraries(googlegroupwaremigration
    KPim${KF_MAJOR_VERSION}::AkonadiCore
    KF${KF_MAJOR_VERSION}::ConfigGui
    KF${KF_MAJOR_VERSION}::I18n
    KF${KF_MAJOR_VERSION}::Wallet
    Qt::DBus
    migrationshared
)
