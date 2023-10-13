set(newmailnotifieragent_common_SRCS)
    ecm_qt_declare_logging_category(newmailnotifieragent_common_SRCS HEADER newmailnotifier_debug.h IDENTIFIER NEWMAILNOTIFIER_LOG CATEGORY_NAME org.kde.pim.newmailnotifier)



set(newmailnotifierselectcollectionwidget_SRCS newmailnotifierselectcollectionwidget_test.cpp ../newmailnotifierselectcollectionwidget.cpp ${newmailnotifieragent_common_SRCS})
add_executable(newmailnotifierselectcollectionwidgettest ${newmailnotifierselectcollectionwidget_SRCS})
target_link_libraries(newmailnotifierselectcollectionwidgettest
    Qt::Widgets
    KF${KF_MAJOR_VERSION}::I18n
    KPim${KF_MAJOR_VERSION}::AkonadiCore
    KPim${KF_MAJOR_VERSION}::Mime
    KF${KF_MAJOR_VERSION}::Completion
    KPim${KF_MAJOR_VERSION}::AkonadiMime)
