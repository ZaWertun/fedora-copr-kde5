configure_file(defaultaddressbook.desktop ${CMAKE_CURRENT_BINARY_DIR}/defaultaddressbook)
configure_file(defaultcalendar.desktop ${CMAKE_CURRENT_BINARY_DIR}/defaultcalendar)
configure_file(defaultnotebook.desktop ${CMAKE_CURRENT_BINARY_DIR}/defaultnotebook)
configure_file(birthdaycalendar.desktop ${CMAKE_CURRENT_BINARY_DIR}/birthdaycalendar)

install ( FILES ${CMAKE_CURRENT_BINARY_DIR}/defaultcalendar
                ${CMAKE_CURRENT_BINARY_DIR}/defaultaddressbook
                ${CMAKE_CURRENT_BINARY_DIR}/defaultnotebook
                ${CMAKE_CURRENT_BINARY_DIR}/birthdaycalendar
                DESTINATION ${KDE_INSTALL_DATADIR}/akonadi/firstrun )
