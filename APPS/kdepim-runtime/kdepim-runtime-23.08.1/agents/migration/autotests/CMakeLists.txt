
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

add_executable(schedulertest schedulertest.cpp ../migrationscheduler.cpp ../migrationexecutor.cpp ../migrationscheduler.h ../migrationexecutor.h)
target_link_libraries(schedulertest
    gidmigration
    KPim${KF_MAJOR_VERSION}::AkonadiCore
    Qt::Test
)
add_test(NAME schedulertest COMMAND schedulertest)
