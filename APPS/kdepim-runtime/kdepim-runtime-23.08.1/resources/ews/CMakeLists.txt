#
#  SPDX-FileCopyrightText: 2015-2020 Krzysztof Nowicki <krissn@op.pl>
#
#  SPDX-License-Identifier: LGPL-2.0-or-later

# Build a separate resource for sending e-mail. This is needed for KMail which assumes that
# a single resource is not able to both send and receive e-mail.
set(HAVE_SEPARATE_MTA_RESOURCE TRUE)

set(AKONADI_EWS_VERSION ${KDEPIM_RUNTIME_VERSION})

add_subdirectory(ewsclient)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/ewsconfig.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/ewsconfig.h)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/ewsclient)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/ewsclient)

add_definitions(-DTRANSLATION_DOMAIN=\"akonadi_ews_resource\")

set(ewsresource_SRCS
    abchperson/ewsabchpersonhandler.cpp
    abchperson/ewscreateabchpersonjob.cpp
    abchperson/ewsfetchabchpersondetailjob.cpp
    abchperson/ewsmodifyabchpersonjob.cpp
    calendar/ewscalendarhandler.cpp
    calendar/ewscreatecalendarjob.cpp
    calendar/ewsfetchcalendardetailjob.cpp
    calendar/ewsmodifycalendarjob.cpp
    contact/ewscontacthandler.cpp
    contact/ewscreatecontactjob.cpp
    contact/ewsfetchcontactdetailjob.cpp
    contact/ewsmodifycontactjob.cpp
    mail/ewscreatemailjob.cpp
    mail/ewsfetchmaildetailjob.cpp
    mail/ewsmailhandler.cpp
    mail/ewsmodifymailjob.cpp
    tags/ewsakonaditagssyncjob.cpp
    tags/ewsglobaltagsreadjob.cpp
    tags/ewsglobaltagswritejob.cpp
    tags/ewstagstore.cpp
    tags/ewsupdateitemstagsjob.cpp
    task/ewscreatetaskjob.cpp
    task/ewsfetchtaskdetailjob.cpp
    task/ewsmodifytaskjob.cpp
    task/ewstaskhandler.cpp
    ewsautodiscoveryjob.cpp
    ewsconfigdialog.cpp
    ewscreateitemjob.cpp
    ewsfetchfoldersincrjob.cpp
    ewsfetchfoldersjob.cpp
    ewsfetchitemdetailjob.cpp
    ewsfetchitempayloadjob.cpp
    ewsfetchitemsjob.cpp
    ewsitemhandler.cpp
    ewsmodifyitemflagsjob.cpp
    ewsmodifyitemjob.cpp
    ewsprogressdialog.cpp
    ewsresource.cpp
    ewssettings.cpp
    ewssubscribedfoldersjob.cpp
    ewssubscriptionmanager.cpp
    ewssubscriptionwidget.cpp
    ewssyncstateattribute.cpp

    abchperson/ewsabchpersonhandler.h
    abchperson/ewscreateabchpersonjob.h
    abchperson/ewsfetchabchpersondetailjob.h
    abchperson/ewsmodifyabchpersonjob.h
    calendar/ewscalendarhandler.h
    calendar/ewscreatecalendarjob.h
    calendar/ewsfetchcalendardetailjob.h
    calendar/ewsmodifycalendarjob.h
    contact/ewscontacthandler.h
    contact/ewscreatecontactjob.h
    contact/ewsfetchcontactdetailjob.h
    contact/ewsmodifycontactjob.h
    mail/ewscreatemailjob.h
    mail/ewsfetchmaildetailjob.h
    mail/ewsmailhandler.h
    mail/ewsmodifymailjob.h
    tags/ewsakonaditagssyncjob.h
    tags/ewsglobaltagsreadjob.h
    tags/ewsglobaltagswritejob.h
    tags/ewstagstore.h
    tags/ewsupdateitemstagsjob.h
    task/ewscreatetaskjob.h
    task/ewsfetchtaskdetailjob.h
    task/ewsmodifytaskjob.h
    task/ewstaskhandler.h
    ewsautodiscoveryjob.h
    ewsconfigdialog.h
    ewscreateitemjob.h
    ewsfetchfoldersincrjob.h
    ewsfetchfoldersjob.h
    ewsfetchitemdetailjob.h
    ewsfetchitempayloadjob.h
    ewsfetchitemsjob.h
    ewsitemhandler.h
    ewsmodifyitemflagsjob.h
    ewsmodifyitemjob.h
    ewsprogressdialog.h
    ewsresource.h
    ewssettings.h
    ewssubscribedfoldersjob.h
    ewssubscriptionmanager.h
    ewssubscriptionwidget.h
    ewssyncstateattribute.h)

ecm_qt_declare_logging_category(ewsresource_SRCS
    HEADER ewsres_debug.h
    IDENTIFIER EWSRES_LOG
    CATEGORY_NAME org.kde.pim.ews
    DESCRIPTION "ews resource (kdepim-runtime)"
    EXPORT KDEPIMRUNTIME
    )


ecm_qt_declare_logging_category(ewsresource_SRCS
    HEADER ewsres_agentif_debug.h
    IDENTIFIER EWSRES_AGENTIF_LOG
    CATEGORY_NAME org.kde.pim.ews.agentif
    DESCRIPTION "exchange ews agent (kdepim-runtime)"
    EXPORT KDEPIMRUNTIME
    )


ki18n_wrap_ui(ewsresource_SRCS ewsconfigdialog.ui)

kconfig_add_kcfg_files(ewsresource_SRCS ewssettingsbase.kcfgc)
kcfg_generate_dbus_interface(${CMAKE_CURRENT_SOURCE_DIR}/ewsresource.kcfg org.kde.Akonadi.Ews.Settings)
qt_add_dbus_adaptor(ewsresource_SRCS
    ${CMAKE_CURRENT_BINARY_DIR}/org.kde.Akonadi.Ews.Settings.xml ewssettings.h EwsSettings ewssettingsadaptor EwsSettingsAdaptor
    )

qt_generate_dbus_interface( ${CMAKE_CURRENT_SOURCE_DIR}/ewsresource.h org.kde.Akonadi.Ews.Resource.xml OPTIONS -a )
qt_add_dbus_adaptor(ewsresource_SRCS
    ${CMAKE_CURRENT_BINARY_DIR}/org.kde.Akonadi.Ews.Resource.xml ewsresource.h EwsResource ewsresourceadaptor EwsResourceAdaptor
    )
qt_generate_dbus_interface( ${CMAKE_CURRENT_SOURCE_DIR}/ewssettings.h org.kde.Akonadi.Ews.Wallet.xml OPTIONS -a )
qt_add_dbus_adaptor(ewsresource_SRCS
    ${CMAKE_CURRENT_BINARY_DIR}/org.kde.Akonadi.Ews.Wallet.xml ewssettings.h EwsSettings ewswalletadaptor EwsWalletAdaptor
    )



add_executable(akonadi_ews_resource ${ewsresource_SRCS})
target_link_libraries(akonadi_ews_resource
    KPim${KF_MAJOR_VERSION}::AkonadiAgentBase
    KPim${KF_MAJOR_VERSION}::AkonadiCore
    KPim${KF_MAJOR_VERSION}::AkonadiMime
    KF${KF_MAJOR_VERSION}::KIOCore
    KPim${KF_MAJOR_VERSION}::Mime
    KF${KF_MAJOR_VERSION}::CalendarCore
    KF${KF_MAJOR_VERSION}::Contacts
    KF${KF_MAJOR_VERSION}::Wallet
    KF${KF_MAJOR_VERSION}::WidgetsAddons
    KF${KF_MAJOR_VERSION}::I18n
    KF${KF_MAJOR_VERSION}::WindowSystem
    KF${KF_MAJOR_VERSION}::Notifications
    KF${KF_MAJOR_VERSION}::ConfigWidgets
    ewsclient)

if (HAVE_SEPARATE_MTA_RESOURCE)
    set(ewsmtaresource_SRCS
        ewsmtaconfigdialog.cpp
        ewsmtaresource.cpp
        ewsmtaconfigdialog.h
        ewsmtaresource.h)
    ecm_qt_declare_logging_category(ewsmtaresource_SRCS
        HEADER ewsres_mta_debug.h
        IDENTIFIER EWSRES_MTA_LOG
        CATEGORY_NAME org.kde.pim.ews.mta
        DESCRIPTION "ews mta (kdepim-runtime)"
        EXPORT KDEPIMRUNTIME
        )


    ki18n_wrap_ui(ewsmtaresource_SRCS ewsmtaconfigdialog.ui)

    kconfig_add_kcfg_files(ewsmtaresource_SRCS ewsmtasettings.kcfgc)
    kcfg_generate_dbus_interface(${CMAKE_CURRENT_SOURCE_DIR}/ewsmtaresource.kcfg org.kde.Akonadi.EwsMta.Settings)
    qt_add_dbus_adaptor(ewsmtaresource_SRCS
        ${CMAKE_CURRENT_BINARY_DIR}/org.kde.Akonadi.EwsMta.Settings.xml ewsmtasettings.h EwsMtaSettings ewsmtasettingsadaptor
        )
    qt_add_dbus_interface(ewsmtaresource_SRCS ${CMAKE_CURRENT_BINARY_DIR}/org.kde.Akonadi.Ews.Resource.xml ewsresourceinterface)

    add_executable(akonadi_ewsmta_resource ${ewsmtaresource_SRCS})
    target_link_libraries(akonadi_ewsmta_resource
        KPim${KF_MAJOR_VERSION}::AkonadiAgentBase
        KPim${KF_MAJOR_VERSION}::AkonadiCore
        KPim${KF_MAJOR_VERSION}::AkonadiWidgets
        KF${KF_MAJOR_VERSION}::I18n
        KF${KF_MAJOR_VERSION}::WindowSystem
        KPim${KF_MAJOR_VERSION}::Mime)

    set(EWS_MTA_CAPABILITIES "X-EwsMailTransport")
else ()
    set(EWS_MTA_CAPABILITIES "MailTransport,Synchronizable")
endif ()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/ewsresource.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/ewsresource.desktop)

if (BUILD_TESTING)
    if (QT_MAJOR_VERSION STREQUAL "5")
        add_subdirectory(test)
    else()
        MESSAGE(STATUS "ews test was disable in qt6 XmlPattern was removed in qt6")
    endif()
endif ()


install(FILES ${CMAKE_CURRENT_BINARY_DIR}/ewsresource.desktop
    DESTINATION "${KDE_INSTALL_DATAROOTDIR}/akonadi/agents" )
install(TARGETS akonadi_ews_resource ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

if (HAVE_SEPARATE_MTA_RESOURCE)
    install(FILES ewsmtaresource.desktop
        DESTINATION "${KDE_INSTALL_DATAROOTDIR}/akonadi/agents" )
    install(TARGETS akonadi_ewsmta_resource ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
endif ()

ecm_install_icons(
    ICONS
    icons/16-apps-akonadi-ews.png
    icons/22-apps-akonadi-ews.png
    icons/24-apps-akonadi-ews.png
    icons/32-apps-akonadi-ews.png
    icons/48-apps-akonadi-ews.png
    icons/64-apps-akonadi-ews.png
    icons/72-apps-akonadi-ews.png
    icons/96-apps-akonadi-ews.png
    icons/128-apps-akonadi-ews.png
    DESTINATION ${KDE_INSTALL_ICONDIR}
    THEME hicolor
    )

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/akonadi_ews_resource.notifyrc
    DESTINATION "${KDE_INSTALL_KNOTIFYRCDIR}")

install(
    FILES org.kde.akonadi_ews_resource.desktop
    DESTINATION ${KDE_INSTALL_APPDIR}
)
