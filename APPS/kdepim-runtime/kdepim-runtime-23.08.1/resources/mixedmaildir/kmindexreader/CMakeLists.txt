if (BUILD_TESTING)
   add_subdirectory(autotests)
endif()

########### next target ###############

include(CheckIncludeFileCXX)
check_include_file_CXX(byteswap.h HAVE_BYTESWAP_H)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/kmindexreader-config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/kmindexreader-config.h)

set(kmindexreader_LIB_SRCS
 kmindexreader.cpp kmindexreader.h
 ../mixedmaildir_debug.cpp
 ../mixedmaildirresource_debug.cpp
)

add_library(kmindexreader  ${kmindexreader_LIB_SRCS})
generate_export_header(kmindexreader BASE_NAME kmindexreader)

target_link_libraries(kmindexreader
    KPim${KF_MAJOR_VERSION}::AkonadiMime
)

set_target_properties(kmindexreader PROPERTIES VERSION ${KDEPIMRUNTIME_LIB_VERSION} SOVERSION ${KDEPIMRUNTIME_LIB_SOVERSION} )

install(TARGETS kmindexreader ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} LIBRARY NAMELINK_SKIP)
