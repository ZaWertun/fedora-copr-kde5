include(ECMAddTests)

if(${EXECUTABLE_OUTPUT_PATH})
    set( PREVIOUS_EXEC_OUTPUT_PATH ${EXECUTABLE_OUTPUT_PATH} )
else()
    set( PREVIOUS_EXEC_OUTPUT_PATH . )
endif()
set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

kde_enable_exceptions()

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/../
  ${CMAKE_CURRENT_BINARY_DIR}/../
  ${CMAKE_CURRENT_SOURCE_DIR}
)

# test data
set( testdata_SRCS
  ${testdata_generated_SRCS}
  testdatautil.cpp testdatautil.h
  ../mixedmaildir_debug.cpp
  testdata.qrc
)


add_executable(testdatatest testdatatest.cpp ${testdata_SRCS})
add_test(NAME testdatatest COMMAND testdatatest)
ecm_mark_as_test(mixedmaildir-testdatatest)

target_link_libraries(
  testdatatest
  Qt::Test
)

# put the libraries all tests link against into a variable and use it
# in target_link_libraries instead
set( common_link_libraries
  kmindexreader
  maildir
  akonadi-filestore
  KPim${KF_MAJOR_VERSION}::AkonadiMime
  KPim${KF_MAJOR_VERSION}::AkonadiCore
  KPim${KF_MAJOR_VERSION}::Mbox
  KPim${KF_MAJOR_VERSION}::Mime
  Qt::Test
  KF${KF_MAJOR_VERSION}::I18n
)

# test for overwritten methods
add_executable(templatemethodstest ../mixedmaildirstore.cpp templatemethodstest.cpp ../mixedmaildir_debug.cpp ../mixedmaildirresource_debug.cpp)
add_test(NAME templatemethodstest COMMAND templatemethodstest)
ecm_mark_as_test(mixedmaildir-templatemethodtest)


target_link_libraries(
  templatemethodstest
  ${common_link_libraries}
)

# test for collection creation handling
add_executable(collectioncreatetest  ../mixedmaildirstore.cpp collectioncreatetest.cpp ../mixedmaildir_debug.cpp ../mixedmaildirresource_debug.cpp)
add_test(NAME collectioncreatetest COMMAND collectioncreatetest)
ecm_mark_as_test(mixedmaildir-collectioncreatetest)


target_link_libraries(
  collectioncreatetest
  ${common_link_libraries}
)

# test for collection deletion handling
add_executable(collectiondeletetest   ../mixedmaildirstore.cpp collectiondeletetest.cpp ../mixedmaildir_debug.cpp ../mixedmaildirresource_debug.cpp)
add_test(NAME collectiondeletetest COMMAND collectiondeletetest)
ecm_mark_as_test(mixedmaildir-collectiondeletetest)

target_link_libraries(
  collectiondeletetest
  ${common_link_libraries}
)

# test for collection fetching handling
add_executable( collectionfetchtest  ../mixedmaildirstore.cpp collectionfetchtest.cpp ../mixedmaildir_debug.cpp ../mixedmaildirresource_debug.cpp)
add_test(NAME collectionfetchtest COMMAND collectionfetchtest)
ecm_mark_as_test(mixedmaildir-collectionfetchtest)


target_link_libraries(
  collectionfetchtest
  ${common_link_libraries}
)

# test for collection modification handling
add_executable( collectionmodifytest  ../mixedmaildirstore.cpp
 collectionmodifytest.cpp ../mixedmaildir_debug.cpp ../mixedmaildirresource_debug.cpp
 ${testdata_SRCS}
)
add_test(NAME collectionmodifytest COMMAND collectionmodifytest)
ecm_mark_as_test(mixedmaildir-collectionmodifytest)


target_link_libraries(
  collectionmodifytest
  ${common_link_libraries}
)

# test for collection move handling
add_executable( collectionmovetest  ../mixedmaildirstore.cpp ../mixedmaildir_debug.cpp
 collectionmovetest.cpp ../mixedmaildirresource_debug.cpp
 ${testdata_SRCS}
)
add_test(NAME collectionmovetest COMMAND collectionmovetest)
ecm_mark_as_test(mixedmaildir-collectionmovetest)

target_link_libraries(
  collectionmovetest
  ${common_link_libraries}
)

# test for item creation handling
add_executable(itemcreatetest   ../mixedmaildirstore.cpp ../mixedmaildir_debug.cpp ../mixedmaildirresource_debug.cpp
 itemcreatetest.cpp
 ${testdata_SRCS}
)
add_test(NAME itemcreatetest COMMAND itemcreatetest)
ecm_mark_as_test(mixedmaildir-itemcreatetest)


target_link_libraries(
  itemcreatetest
  ${common_link_libraries}
)

# test for item creation handling
add_executable(itemdeletetest  ../mixedmaildirstore.cpp
 itemdeletetest.cpp ../mixedmaildirresource_debug.cpp
 ${testdata_SRCS}
)
add_test(NAME itemdeletetest COMMAND itemdeletetest)
ecm_mark_as_test(mixedmaildir-itemdeletetest)


target_link_libraries(
  itemdeletetest
  ${common_link_libraries}
)
#REACTIVATE IT
if (0)
# test for item retrieval handling
add_executable(itemfetchtest   ../mixedmaildirstore.cpp ../mixedmaildirresource_debug.cpp
 itemfetchtest.cpp
 ${testdata_SRCS}
)
add_test(NAME itemfetchtest COMMAND itemfetchtest)
ecm_mark_as_test(mixedmaildir-itemfetchtest)

target_link_libraries(
  itemfetchtest
  ${common_link_libraries}
)
endif()

# test for item modification handling
add_executable(itemmodifytest   ../mixedmaildirstore.cpp ../mixedmaildirresource_debug.cpp
 itemmodifytest.cpp
 ${testdata_SRCS}
)
add_test(NAME itemmodifytest COMMAND itemmodifytest)
ecm_mark_as_test( mixedmaildir-itemmodifytest)

target_link_libraries(
  itemmodifytest
  ${common_link_libraries}
)

# test for item move handling
add_executable(itemmovetest  ../mixedmaildirstore.cpp ../mixedmaildirresource_debug.cpp
 itemmovetest.cpp
 ${testdata_SRCS}
)
add_test(NAME itemmovetest COMMAND itemmovetest)
ecm_mark_as_test(mixedmaildir-itemmovetest)


target_link_libraries(
  itemmovetest
  ${common_link_libraries}
)

# test for store compact handling
add_executable(storecompacttest  ../mixedmaildirstore.cpp  ../mixedmaildirresource_debug.cpp
 storecompacttest.cpp
 ${testdata_SRCS}
)
add_test(NAME storecompacttest COMMAND storecompacttest)
ecm_mark_as_test(mixedmaildir-storecompacttest)


target_link_libraries(
  storecompacttest
  ${common_link_libraries}
)
