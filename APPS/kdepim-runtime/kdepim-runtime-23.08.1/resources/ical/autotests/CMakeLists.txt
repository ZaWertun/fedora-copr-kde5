configure_file(${CMAKE_CURRENT_SOURCE_DIR}/ical-empty.xml ${CMAKE_CURRENT_BINARY_DIR}/ical-empty.xml COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/ical-step1.xml ${CMAKE_CURRENT_BINARY_DIR}/ical-step1.xml COPYONLY)

akonadi_add_resourcetest( ical icaltest.es )
