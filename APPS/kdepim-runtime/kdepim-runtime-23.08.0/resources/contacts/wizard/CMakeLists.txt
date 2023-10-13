
set(CONTACTS_DIRECTORY_DEFAULT_PATH "$HOME/.local/share/contacts/")

configure_file(contactswizard.es.cmake ${CMAKE_CURRENT_BINARY_DIR}/contactswizard.es)
install ( FILES contactswizard.desktop ${CMAKE_CURRENT_BINARY_DIR}/contactswizard.es contactswizard.ui DESTINATION ${KDE_INSTALL_DATADIR}/akonadi/accountwizard/contacts/ )
