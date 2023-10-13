
add_definitions(-DTRANSLATION_DOMAIN=\"akonadi-filestore\")
add_library(akonadi-filestore)

target_sources(akonadi-filestore PRIVATE
    abstractlocalstore.cpp
    collectioncreatejob.cpp
    collectiondeletejob.cpp
    collectionfetchjob.cpp
    collectionmodifyjob.cpp
    collectionmovejob.cpp
    entitycompactchangeattribute.cpp
    itemcreatejob.cpp
    itemdeletejob.cpp
    itemfetchjob.cpp
    itemmodifyjob.cpp
    itemmovejob.cpp
    job.cpp
    session.cpp
    sessionimpls.cpp
    storecompactjob.cpp

    abstractlocalstore.h
    collectioncreatejob.h
    collectiondeletejob.h
    collectionfetchjob.h
    collectionmodifyjob.h
    collectionmovejob.h
    entitycompactchangeattribute.h
    itemcreatejob.h
    itemdeletejob.h
    itemfetchjob.h
    itemmodifyjob.h
    itemmovejob.h
    job.h
    storecompactjob.h
    )

ecm_qt_declare_logging_category(akonadi-filestore HEADER akonadifilestore_debug.h IDENTIFIER AKONADIFILESTORE_LOG CATEGORY_NAME org.kde.pim.filestore
    DESCRIPTION "resource filestore lib (kdepim-runtime)"
    EXPORT KDEPIMRUNTIME
    )


if (COMPILE_WITH_UNITY_CMAKE_SUPPORT)
    set_target_properties(akonadi-filestore PROPERTIES UNITY_BUILD ON)
endif()
generate_export_header(akonadi-filestore BASE_NAME akonadi-filestore)

target_link_libraries(akonadi-filestore
    PUBLIC
    KPim${KF_MAJOR_VERSION}::AkonadiCore
    PRIVATE
    KF${KF_MAJOR_VERSION}::I18n
    )

set_target_properties(akonadi-filestore PROPERTIES VERSION ${KDEPIMRUNTIME_LIB_VERSION} SOVERSION ${KDEPIMRUNTIME_LIB_SOVERSION} )

install(TARGETS akonadi-filestore ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} LIBRARY NAMELINK_SKIP)

if (BUILD_TESTING)
    add_subdirectory( autotests )
endif ()

