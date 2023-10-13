#
#  SPDX-FileCopyrightText: 2017 Krzysztof Nowicki <krissn@op.pl>
#
#  SPDX-License-Identifier: LGPL-2.0-or-later

include_directories(../)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/../)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/fakeserver)

set(isolatestestcommon_SRCS isolatedtestbase.cpp isolatedtestbase.h statemonitor.cpp statemonitor.h)

kcfg_generate_dbus_interface(${CMAKE_CURRENT_SOURCE_DIR}/../ewsresource.kcfg org.kde.Akonadi.Ews.Settings)
set(ewssettingsinterface_xml ${CMAKE_CURRENT_BINARY_DIR}/org.kde.Akonadi.Ews.Settings.xml)
qt_generate_dbus_interface( ${CMAKE_CURRENT_SOURCE_DIR}/../ewssettings.h org.kde.Akonadi.Ews.Wallet.xml OPTIONS -a )
set(ewswalletinterface_xml ${CMAKE_CURRENT_BINARY_DIR}/org.kde.Akonadi.Ews.Wallet.xml)
qt_generate_dbus_interface( ${CMAKE_CURRENT_SOURCE_DIR}/../ewsresource.h org.kde.Akonadi.Ews.Resource.xml OPTIONS -a )
set(ewsresourceinterface_xml ${CMAKE_CURRENT_BINARY_DIR}/org.kde.Akonadi.Ews.Resource.xml)

qt_add_dbus_interface(isolatestestcommon_SRCS ${ewssettingsinterface_xml} ewssettings)
qt_add_dbus_interface(isolatestestcommon_SRCS ${ewswalletinterface_xml} ewswallet)
qt_add_dbus_interface(isolatestestcommon_SRCS ${ewsresourceinterface_xml} ewsresource)

add_library(isolatedtestcommon STATIC ${isolatestestcommon_SRCS})
target_link_libraries(isolatedtestcommon
  KPim${KF_MAJOR_VERSION}::AkonadiCore
  KPim${KF_MAJOR_VERSION}::AkonadiMime
  Qt::Core
  Qt::Network
  Qt::Test
  fakeewsserver
)

add_subdirectory(unittests)
# XmlPattern was removed in qt6
if (QT_MAJOR_VERSION STREQUAL "5")
    add_subdirectory(fakeserver)
endif()
if (KDEPIM_RUN_AKONADI_TEST)
	add_akonadi_isolated_test_advanced(ewstest.cpp isolatedtestcommon.qrc "isolatedtestcommon")
	add_akonadi_isolated_test_advanced(serverbusytest.cpp isolatedtestcommon.qrc "isolatedtestcommon")
endif()
