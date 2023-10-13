add_definitions(-DTEST_PATH=${CMAKE_CURRENT_SOURCE_DIR})

set( testidxreader_SRCS
    testidxreader.cpp
    testidxreader.h
)
add_executable( testidxreader  ${testidxreader_SRCS} )
add_test(NAME testidxreader COMMAND testidxreader )
ecm_mark_as_test(testidxreader)

target_link_libraries( testidxreader  Qt::Test kmindexreader KPim${KF_MAJOR_VERSION}::AkonadiMime )
