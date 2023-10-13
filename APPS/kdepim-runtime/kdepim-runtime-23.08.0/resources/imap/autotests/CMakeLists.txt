#if the kimap project wasn't built with tests then we can't build anything in here
if(NOT TARGET KPim${KF_MAJOR_VERSION}::kimaptest)
  message(STATUS "Warning: Unable to build the imap autotests since the kimap project did not build the kimaptest library. You need to rebuild kimap with the -DBUILD_TESTING=on option")
  return()
endif()

kde_enable_exceptions()
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

include(ECMMarkAsTest)


MACRO(IMAP_RESOURCE_UNIT_TESTS)
  FOREACH(_testname ${ARGN})
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/.. ${CMAKE_CURRENT_BINARY_DIR}/..)
    add_executable( ${_testname} ${_testname}.cpp dummypasswordrequester.cpp dummyresourcestate.cpp imaptestbase.cpp
        dummypasswordrequester.h dummyresourcestate.h imaptestbase.h)
    add_test(NAME ${_testname} COMMAND ${_testname} )
    ecm_mark_as_test(imap-${_testname})

    target_link_libraries(${_testname}
      KPim${KF_MAJOR_VERSION}::IMAP KPim${KF_MAJOR_VERSION}::kimaptest Qt::Gui Qt::Core Qt::Test KF${KF_MAJOR_VERSION}::ConfigGui imapresource akonadi-singlefileresource)
    add_definitions(-DTEST_DATA="\\"${CMAKE_CURRENT_SOURCE_DIR}\\"")
  ENDFOREACH(_testname)
ENDMACRO(IMAP_RESOURCE_UNIT_TESTS)

IMAP_RESOURCE_UNIT_TESTS(
  testresourcetask
  testsessionpool

  testaddcollectiontask
  testadditemtask
  testchangecollectiontask
  testchangeitemtask
  testexpungecollectiontask
  testmovecollectiontask
  testmoveitemstask
  testremovecollectionrecursivetask
  testretrievecollectionmetadatatask
  testretrievecollectionstask
  testretrieveitemtask
  testretrieveitemstask
)
