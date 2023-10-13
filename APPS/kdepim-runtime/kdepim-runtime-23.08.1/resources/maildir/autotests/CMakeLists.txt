include(ECMMarkAsTest)

if (KDEPIM_RUN_AKONADI_TEST)
	set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})
	set(KDEPIMLIBS_RUN_ISOLATED_TESTS TRUE)
	set(KDEPIMLIBS_RUN_SQLITE_ISOLATED_TESTS TRUE)

	kcfg_generate_dbus_interface(${CMAKE_CURRENT_SOURCE_DIR}/../maildirresource.kcfg org.kde.Akonadi.Maildir.Settings)
	qt_add_dbus_interface(extra_SRC ${CMAKE_CURRENT_BINARY_DIR}/org.kde.Akonadi.Maildir.Settings.xml maildirsettings)

	add_akonadi_isolated_test_advanced(synctest.cpp "${extra_SRC}" "")
endif()
