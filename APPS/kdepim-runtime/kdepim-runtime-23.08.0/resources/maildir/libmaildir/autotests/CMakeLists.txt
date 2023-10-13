include(ECMAddTests)

set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/..   )

########### next target ###############

set(testmaildir_SRCS testmaildir.cpp testmaildir.h)


add_executable( testmaildir ${testmaildir_SRCS} )
add_test(NAME testmaildir COMMAND testmaildir )
ecm_mark_as_test(maildir-testmaildir)

target_link_libraries(testmaildir  Qt::Test  KPim${KF_MAJOR_VERSION}::AkonadiMime maildir)

