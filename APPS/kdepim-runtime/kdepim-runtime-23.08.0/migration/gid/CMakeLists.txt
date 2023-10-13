add_library(gidmigration STATIC)
target_sources(gidmigration PRIVATE
    gidmigrator.cpp
    gidmigrationjob.cpp
    gidmigrator.h
    gidmigrationjob.h
)

if (COMPILE_WITH_UNITY_CMAKE_SUPPORT)
    set_target_properties(gidmigration PROPERTIES UNITY_BUILD ON)
endif()

target_link_libraries(gidmigration
    migrationshared
    KF${KF_MAJOR_VERSION}::I18n
)

add_executable(gidmigrator main.cpp)
target_link_libraries(gidmigrator
    gidmigration
    KPim${KF_MAJOR_VERSION}::AkonadiWidgets
    KPim${KF_MAJOR_VERSION}::Mime
    )
install(TARGETS gidmigrator ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
