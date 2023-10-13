#
#  SPDX-FileCopyrightText: 2015-2018 Krzysztof Nowicki <krissn@op.pl>
#
#  SPDX-License-Identifier: LGPL-2.0-or-later

kde_enable_exceptions()

add_library(uttesthelpers STATIC faketransferjob.cpp faketransferjob.h)
target_link_libraries(uttesthelpers Qt::Core KF${KF_MAJOR_VERSION}::KIOCore)

macro(akonadi_ews_add_ut_advanced utname extra_SRCS)
  add_executable(${utname} ${utname}.cpp ${extra_SRCS})
  target_link_libraries(${utname} Qt::Test uttesthelpers)
  add_test(NAME ${utname} COMMAND ${utname})
endmacro(akonadi_ews_add_ut_advanced utname)

macro(akonadi_ews_add_ut utname)
  akonadi_ews_add_ut_advanced(${utname} "")
  target_link_libraries(${utname} ewsclient)
endmacro(akonadi_ews_add_ut utname)

akonadi_ews_add_ut(ewsmoveitemrequest_ut)
akonadi_ews_add_ut(ewsdeleteitemrequest_ut)
akonadi_ews_add_ut(ewsgetitemrequest_ut)
akonadi_ews_add_ut(ewsunsubscriberequest_ut)
akonadi_ews_add_ut(ewsattachment_ut)

qt_wrap_cpp(ewssettings_ut_SRCS ewssettings_ut_mock.h)
akonadi_ews_add_ut_advanced(ewssettings_ut "${CMAKE_CURRENT_SOURCE_DIR}/../../ewssettings.cpp;${ewssettings_ut_SRCS}")
target_link_libraries(ewssettings_ut
  KPim${KF_MAJOR_VERSION}::AkonadiCore
  KF${KF_MAJOR_VERSION}::WidgetsAddons
  KF${KF_MAJOR_VERSION}::I18n
  KF${KF_MAJOR_VERSION}::ConfigCore
  KF${KF_MAJOR_VERSION}::ConfigGui
  KF${KF_MAJOR_VERSION}::CoreAddons
  KF${KF_MAJOR_VERSION}::Wallet
  )
target_compile_definitions(ewssettings_ut PUBLIC -DEWSSETTINGS_UNITTEST)

if (TARGET Qt::NetworkAuth)
  set(ewsoauth_ut_SRCS
    ewsoauth_ut_mock.cpp ewsoauth_ut_mock.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../../ewsclient/auth/ewsabstractauth.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../ewsclient/auth/ewsoauth.cpp)
  akonadi_ews_add_ut_advanced(ewsoauth_ut "${ewsoauth_ut_SRCS}")
  target_link_libraries(ewsoauth_ut
    Qt::Widgets
    KF${KF_MAJOR_VERSION}::I18n
    Qt::WebEngineWidgets
    )
  target_compile_definitions(ewsoauth_ut PUBLIC -DEWSOAUTH_UNITTEST)
endif ()
