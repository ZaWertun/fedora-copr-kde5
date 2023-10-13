

add_library(kio_akonadi MODULE akonadiworker.cpp akonadiworker.h)

ecm_qt_declare_logging_category(kio_akonadi HEADER akonadiworker_debug.h IDENTIFIER AKONADIWORKER_LOG CATEGORY_NAME org.kde.pim.akonadiworker
    DESCRIPTION "kioworker (akonadi)"
    OLD_CATEGORY_NAMES log_akonadislave org.kde.pim.akonadislave
    EXPORT KDEPIMRUNTIME
    )


target_link_libraries(kio_akonadi  
                      KF${KF_MAJOR_VERSION}::KIOCore 
                      KPim${KF_MAJOR_VERSION}::AkonadiCore 
                      KF${KF_MAJOR_VERSION}::I18n
                      Qt::DBus
                      Qt::Widgets
                      Qt::Network
                      )
set_target_properties(kio_akonadi PROPERTIES OUTPUT_NAME "akonadi")

install(TARGETS kio_akonadi DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf${QT_MAJOR_VERSION}/kio )
