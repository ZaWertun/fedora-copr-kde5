set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Stolen from kdepimlibs/akonadi/tests
kde_enable_exceptions()

set(_srcList ${_source} fakeserver/fakeserver.cpp)

kcfg_generate_dbus_interface(${CMAKE_CURRENT_SOURCE_DIR}/../settings.kcfg org.kde.Akonadi.POP3.Settings)
set(pop3settingsinterface_xml ${CMAKE_CURRENT_BINARY_DIR}/org.kde.Akonadi.POP3.Settings.xml)
kcfg_generate_dbus_interface(${CMAKE_CURRENT_SOURCE_DIR}/../../maildir/maildirresource.kcfg org.kde.Akonadi.Maildir.Settings)
set(maildirsettingsinterface_xml ${CMAKE_CURRENT_BINARY_DIR}/org.kde.Akonadi.Maildir.Settings.xml)
set_source_files_properties(${pop3settingsinterface_xml} PROPERTIES INCLUDE "../metatype.h")
qt_add_dbus_interface(_srcList ${pop3settingsinterface_xml} pop3settings)
qt_add_dbus_interface(_srcList ${maildirsettingsinterface_xml} maildirsettings)
# add the dbus interface to every test (easier than adding to particular tests only)
#qt_add_dbus_interface(_srcList ../org.kde.krss.xml krssinterface)

if (KDEPIM_RUN_AKONADI_TEST)
	add_akonadi_isolated_test_advanced(pop3test.cpp "${_srcList}" "KPim${KF_MAJOR_VERSION}::Mime")
endif()	
