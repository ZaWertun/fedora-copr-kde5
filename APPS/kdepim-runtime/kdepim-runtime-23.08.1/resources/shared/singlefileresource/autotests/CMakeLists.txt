include(ECMAddTests)

macro(_add_test _source)
  set(_test ${_source})
  get_filename_component(_name ${_source} NAME_WE)
  add_executable( ${_name} ${_test} )
  add_test( NAME ${_name} COMMAND ${_name} )
  ecm_mark_as_test(singlefileresource-${_name})
  target_link_libraries(${_name} Qt::Test   
                        KPim${KF_MAJOR_VERSION}::AkonadiCore KPim${KF_MAJOR_VERSION}::IMAP akonadi-singlefileresource)
endmacro()

_add_test( imapaclattributetest.cpp )
