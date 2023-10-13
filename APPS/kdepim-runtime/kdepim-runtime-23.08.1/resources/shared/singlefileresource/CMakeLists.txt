
add_definitions(-DTRANSLATION_DOMAIN=\"akonadi_singlefile_resource\")

set( AKONADI_SINGLEFILERESOURCE_SHARED_SOURCES
  singlefileresourcebase.cpp singlefileresourcebase.h
  singlefileresourceconfigwidgetbase.cpp singlefileresourceconfigwidgetbase.h
)

set( AKONADI_SINGLEFILERESOURCE_SHARED_UI
  singlefileresourceconfigwidget_desktop.ui
  singlefileresourceconfigwidget.ui
)

set( AKONADI_COLLECTIONATTRIBUTES_SHARED_SOURCES
  collectionflagsattribute.cpp
  collectionflagsattribute.h
)

set( AKONADI_IMAPATTRIBUTES_SHARED_SOURCES
  imapaclattribute.cpp
  imapquotaattribute.cpp
  imapaclattribute.h
  imapquotaattribute.h
)


set(akonadi-singlefileresource_SRCS
  ${AKONADI_SINGLEFILERESOURCE_SHARED_SOURCES}
  ${AKONADI_COLLECTIONATTRIBUTES_SHARED_SOURCES}
  ${AKONADI_IMAPATTRIBUTES_SHARED_SOURCES}
  createandsettagsjob.cpp createandsettagsjob.h
)

ki18n_wrap_ui(akonadi-singlefileresource_SRCS ${AKONADI_SINGLEFILERESOURCE_SHARED_UI} settingsdialog.ui)

add_library(akonadi-singlefileresource  ${akonadi-singlefileresource_SRCS} )
generate_export_header(akonadi-singlefileresource BASE_NAME akonadi-singlefileresource)
if (COMPILE_WITH_UNITY_CMAKE_SUPPORT)
    set_target_properties(akonadi-singlefileresource PROPERTIES UNITY_BUILD ON)
endif()

target_link_libraries(akonadi-singlefileresource
PUBLIC
    KF${KF_MAJOR_VERSION}::I18n
    KF${KF_MAJOR_VERSION}::Completion
    KF${KF_MAJOR_VERSION}::KIOWidgets
    KF${KF_MAJOR_VERSION}::ConfigWidgets
PRIVATE
    KPim${KF_MAJOR_VERSION}::AkonadiCore
    KPim${KF_MAJOR_VERSION}::AkonadiAgentBase
    KF${KF_MAJOR_VERSION}::KIOCore
    KPim${KF_MAJOR_VERSION}::IMAP
)

set_target_properties(akonadi-singlefileresource PROPERTIES VERSION ${KDEPIMRUNTIME_LIB_VERSION} SOVERSION ${KDEPIMRUNTIME_LIB_SOVERSION} )
install(TARGETS akonadi-singlefileresource ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} LIBRARY NAMELINK_SKIP)

if (BUILD_TESTING)
   add_subdirectory(autotests)
endif()
