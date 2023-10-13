set(VCARD_FILE_DEFAULT_PATH "$HOME/.local/share/kaddressbook/contact.vcf")

configure_file(vcardwizard.es.cmake ${CMAKE_CURRENT_BINARY_DIR}/vcardwizard.es)
install ( FILES vcardwizard.desktop ${CMAKE_CURRENT_BINARY_DIR}/vcardwizard.es vcardwizard.ui DESTINATION ${KDE_INSTALL_DATADIR}/akonadi/accountwizard/vcard )
