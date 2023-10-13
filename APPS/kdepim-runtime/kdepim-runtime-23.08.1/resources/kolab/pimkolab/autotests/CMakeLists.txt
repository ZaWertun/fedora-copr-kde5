
include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_definitions(-DTEST_DATA_PATH="${CMAKE_CURRENT_SOURCE_DIR}")


macro(addTest TEST_NAME)
    add_executable(${TEST_NAME} ${TEST_NAME}.cpp ${TEST_NAME}.h)
    target_link_libraries(${TEST_NAME} Qt::Test kolab_static)
    add_test(${TEST_NAME} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TEST_NAME})
endmacro()

add_executable(benchmarktest benchmark.cpp benchmark.h)
target_link_libraries(benchmarktest ${QT_QTTEST_LIBRARY} kolab_static)

addTest(formattest)
addTest(upgradetest)
addTest(kcalconversiontest)
addTest(calendaringtest)
addTest(icalendartest)
addTest(freebusytest)
addTest(kolabobjecttest)
addTest(timezonetest)
addTest(mimeobjecttest)
addTest(xmlobjecttest)
addTest(debugstreamtest)
addTest(legacyformattest)

