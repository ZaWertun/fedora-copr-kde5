add_definitions(-DTRANSLATION_DOMAIN=\"akonadi_imap_resource\")

set(WITH_GMAIL_XOAUTH2 TRUE)

configure_file(config-imap.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-imap.h)

########### next target ###############

set( imapresource_LIB_SRCS
  addcollectiontask.cpp
  additemtask.cpp
  batchfetcher.cpp
  changecollectiontask.cpp
  changeitemsflagstask.cpp
  changeitemtask.cpp
  expungecollectiontask.cpp
  highestmodseqattribute.cpp
  imapaccount.cpp
  imapflags.cpp
  imapresourcebase.cpp
  messagehelper.cpp
  movecollectiontask.cpp
  moveitemstask.cpp
  noselectattribute.cpp
  noinferiorsattribute.cpp
  passwordrequesterinterface.cpp
  removecollectionrecursivetask.cpp
  resourcestateinterface.cpp
  resourcetask.cpp
  retrievecollectionmetadatatask.cpp
  retrievecollectionstask.cpp
  retrieveitemtask.cpp
  retrieveitemstask.cpp
  searchtask.cpp
  sessionpool.cpp
  uidvalidityattribute.cpp
  uidnextattribute.cpp
  settings.cpp
  subscriptiondialog.cpp
  imapidlemanager.cpp
  resourcestate.cpp
  collectionmetadatahelper.cpp
  replacemessagejob.cpp

  addcollectiontask.h
  additemtask.h
  batchfetcher.h
  changecollectiontask.h
  changeitemsflagstask.h
  changeitemtask.h
  expungecollectiontask.h
  highestmodseqattribute.h
  imapaccount.h
  imapflags.h
  imapresourcebase.h
  messagehelper.h
  movecollectiontask.h
  moveitemstask.h
  noselectattribute.h
  noinferiorsattribute.h
  passwordrequesterinterface.h
  removecollectionrecursivetask.h
  resourcestateinterface.h
  resourcetask.h
  retrievecollectionmetadatatask.h
  retrievecollectionstask.h
  retrieveitemtask.h
  retrieveitemstask.h
  searchtask.h
  sessionpool.h
  uidvalidityattribute.h
  uidnextattribute.h
  settings.h
  subscriptiondialog.h
  imapidlemanager.h
  resourcestate.h
  collectionmetadatahelper.h
  replacemessagejob.h
  ${AKONADI_COLLECTIONATTRIBUTES_SHARED_SOURCES}
  ${AKONADI_IMAPATTRIBUTES_SHARED_SOURCES}
)

ecm_qt_declare_logging_category(imapresource_LIB_SRCS HEADER imapresource_debug.h IDENTIFIER IMAPRESOURCE_LOG CATEGORY_NAME org.kde.pim.imapresource
        DESCRIPTION "imap resource (kdepim-runtime)"
        OLD_CATEGORY_NAMES log_imapresource
        EXPORT KDEPIMRUNTIME
    )
ecm_qt_declare_logging_category(imapresource_LIB_SRCS HEADER imapresource_trace.h
        IDENTIFIER IMAPRESOURCE_TRACE
        CATEGORY_NAME org.kde.pim.imapresource.trace
        DESCRIPTION "resource kolab trace (kdepim-runtime)"
        EXPORT KDEPIMRUNTIME
        )

kcfg_generate_dbus_interface( ${CMAKE_CURRENT_SOURCE_DIR}/imapresource.kcfg org.kde.Akonadi.Imap.Settings )
kconfig_add_kcfg_files(imapresource_LIB_SRCS settingsbase.kcfgc)

qt_add_dbus_adaptor( imapresource_LIB_SRCS
     ${CMAKE_CURRENT_BINARY_DIR}/org.kde.Akonadi.Imap.Settings.xml settings.h Settings
     )

qt_generate_dbus_interface( ${CMAKE_CURRENT_SOURCE_DIR}/imapresourcebase.h org.kde.Akonadi.Imap.Resource.xml OPTIONS -a )
qt_add_dbus_adaptor( imapresource_LIB_SRCS
     ${CMAKE_CURRENT_BINARY_DIR}/org.kde.Akonadi.Imap.Resource.xml
        imapresourcebase.h ImapResourceBase
     )

add_library(imapresource STATIC ${imapresource_LIB_SRCS})

target_link_libraries(imapresource
    Qt::DBus
    Qt::Network
    KPim${KF_MAJOR_VERSION}::AkonadiCore
    KPim${KF_MAJOR_VERSION}::IMAP
    KPim${KF_MAJOR_VERSION}::MailTransport
    KF${KF_MAJOR_VERSION}::KIOWidgets
    KPim${KF_MAJOR_VERSION}::Mime
    KPim${KF_MAJOR_VERSION}::AkonadiMime
    KPim${KF_MAJOR_VERSION}::IdentityManagementWidgets
    KPim${KF_MAJOR_VERSION}::AkonadiAgentBase
    KF${KF_MAJOR_VERSION}::I18n
    KF${KF_MAJOR_VERSION}::WindowSystem
    KF${KF_MAJOR_VERSION}::ConfigGui
    KF${KF_MAJOR_VERSION}::Wallet
    akonadi-singlefileresource
    KPim${KF_MAJOR_VERSION}::Libkdepim
    ${kdepim-keychain_lib}
)

########### next target ###############

add_executable(akonadi_imap_resource)
ki18n_wrap_ui(akonadi_imap_resource setupserverview_desktop.ui)
target_sources( akonadi_imap_resource PRIVATE
  main.cpp
  imapresource.cpp
  resourcestate.cpp
  settingspasswordrequester.cpp
  setupserver.cpp
  serverinfodialog.cpp
  passwordrequester.cpp
  gmailpasswordrequester.cpp

  imapresource.h
  resourcestate.h
  settingspasswordrequester.h
  setupserver.h
  serverinfodialog.h
  passwordrequester.h
  gmailpasswordrequester.h
  )

install( FILES imapresource.desktop DESTINATION "${KDE_INSTALL_DATAROOTDIR}/akonadi/agents" )



if (COMPILE_WITH_UNITY_CMAKE_SUPPORT)
    set_target_properties(akonadi_imap_resource PROPERTIES UNITY_BUILD ON)
endif()
if( APPLE )
  set_target_properties(akonadi_imap_resource PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/../Info.plist.template)
  set_target_properties(akonadi_imap_resource PROPERTIES MACOSX_BUNDLE_GUI_IDENTIFIER "org.kde.Akonadi.Imap")
  set_target_properties(akonadi_imap_resource PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "KDE Akonadi IMAP Resource")
endif ()

target_link_libraries(akonadi_imap_resource
    Qt::DBus
    KPim${KF_MAJOR_VERSION}::AkonadiCore
    KPim${KF_MAJOR_VERSION}::IMAP
    akonadi-singlefileresource
    KPim${KF_MAJOR_VERSION}::AkonadiWidgets
    KPim${KF_MAJOR_VERSION}::MailTransport
    KPim${KF_MAJOR_VERSION}::Mime
    KPim${KF_MAJOR_VERSION}::AkonadiMime
    KPim${KF_MAJOR_VERSION}::IdentityManagement
    KF${KF_MAJOR_VERSION}::I18n
    imapresource
    KF${KF_MAJOR_VERSION}::TextWidgets
    KF${KF_MAJOR_VERSION}::ConfigGui

    folderarchivesettings
    KPim${KF_MAJOR_VERSION}::GAPICore
)

install(
    FILES org.kde.akonadi_imap_resource.desktop
    DESTINATION ${KDE_INSTALL_APPDIR}
)

install(TARGETS akonadi_imap_resource ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

add_subdirectory( wizard )

if(BUILD_TESTING)
    add_subdirectory( autotests )
    add_subdirectory( tests )
endif()
