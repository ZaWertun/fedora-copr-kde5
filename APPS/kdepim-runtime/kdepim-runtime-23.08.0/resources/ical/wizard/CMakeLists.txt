set(ICAL_FILE_DEFAULT_PATH "$HOME/.local/share/korganizer/calendar.ics")

configure_file(icalwizard.es.cmake ${CMAKE_CURRENT_BINARY_DIR}/icalwizard.es)
install ( FILES icalwizard.desktop ${CMAKE_CURRENT_BINARY_DIR}/icalwizard.es icalwizard.ui DESTINATION ${KDE_INSTALL_DATADIR}/akonadi/accountwizard/ical )
