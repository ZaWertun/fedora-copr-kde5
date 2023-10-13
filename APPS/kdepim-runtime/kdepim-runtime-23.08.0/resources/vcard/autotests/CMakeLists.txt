configure_file(${CMAKE_CURRENT_SOURCE_DIR}/vcardtest.xml ${CMAKE_CURRENT_BINARY_DIR}/vcardtest.xml COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/vcardtest-readonly.xml ${CMAKE_CURRENT_BINARY_DIR}/vcardtest-readonly.xml COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/vcardtest.vcf ${CMAKE_CURRENT_BINARY_DIR}/vcardtest.vcf COPYONLY)

akonadi_add_resourcetest( vcard-read vcardtest.js )
akonadi_add_resourcetest( vcard-read-readonly vcardtest-readonly.js )
