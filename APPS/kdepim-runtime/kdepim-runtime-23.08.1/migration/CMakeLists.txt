include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
)

add_library(migrationshared STATIC )

target_sources(migrationshared PRIVATE
  kmigratorbase.cpp
  infodialog.cpp
  entitytreecreatejob.cpp
  migratorbase.cpp
  kmigratorbase.h
  infodialog.h
  entitytreecreatejob.h
  migratorbase.h
)

ecm_qt_declare_logging_category(migrationshared
    HEADER migration_debug.h
    IDENTIFIER MIGRATION_LOG
    CATEGORY_NAME org.kde.pim.migration
    DESCRIPTION "migration (kdepim-runtime)"
    EXPORT KDEPIMRUNTIME
)

if (COMPILE_WITH_UNITY_CMAKE_SUPPORT)
    set_target_properties(migrationshared PROPERTIES UNITY_BUILD ON)
endif()
target_link_libraries(migrationshared
    KPim${KF_MAJOR_VERSION}::AkonadiCore
    KF${KF_MAJOR_VERSION}::ConfigCore
    KF${KF_MAJOR_VERSION}::I18n
    Qt::Widgets
)


add_subdirectory(gid)
add_subdirectory(googlegroupware)

