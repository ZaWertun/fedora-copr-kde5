#if the kimap project wasn't built with tests then we can't build anything in here
if(NOT TARGET KPim${KF_MAJOR_VERSION}::kimaptest)
  message(STATUS "Warning: Unable to build the imap autotests since the kimap project did not build the kimaptest library. You need to rebuild kimap with the -DBUILD_TESTING=on option")
  return()
endif()
if (KDEPIM_RUN_AKONADI_TEST)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})
set(KDEPIMLIBS_RUN_ISOLATED_TESTS TRUE)
set(KDEPIMLIBS_RUN_SQLITE_ISOLATED_TESTS TRUE)

MACRO(KOLAB_RESOURCE_ISOLATED_TESTS)
  FOREACH(_testname ${ARGN})
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/.. ${CMAKE_CURRENT_BINARY_DIR}/.. ../../imap/autotests/)
    add_akonadi_isolated_test_advanced(${_testname}.cpp "../../imap/autotests/dummypasswordrequester.cpp;../../imap/autotests/dummyresourcestate.cpp;../../imap/autotests/imaptestbase.cpp" "KPim${KF_MAJOR_VERSION}::IMAP;KPim${KF_MAJOR_VERSION}::kimaptest;Qt::Test;imapresource;kolabresource;akonaditest;${Libkolab_LIBRARIES}")
    kde_enable_exceptions()
  ENDFOREACH(_testname)
ENDMACRO(KOLAB_RESOURCE_ISOLATED_TESTS)

KOLAB_RESOURCE_ISOLATED_TESTS (
  testretrievetagstask
  testchangeitemstagstask
)
endif()
