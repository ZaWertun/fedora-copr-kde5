kde_enable_exceptions()
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

set(testsubscriptiondialog_SRCS
  testsubscriptiondialog.cpp
  ../imapaccount.cpp
  ../subscriptiondialog.cpp
  ../imapresource_debug.cpp
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/.. ${CMAKE_CURRENT_BINARY_DIR}/..)
add_executable(testsubscriptiondialog ${testsubscriptiondialog_SRCS})

target_link_libraries(testsubscriptiondialog KF${KF_MAJOR_VERSION}::KIOCore KPim${KF_MAJOR_VERSION}::IMAP KPim${KF_MAJOR_VERSION}::Mime KF${KF_MAJOR_VERSION}::ItemModels KF${KF_MAJOR_VERSION}::ConfigGui Qt::Widgets KF${KF_MAJOR_VERSION}::I18n KF${KF_MAJOR_VERSION}::KIOWidgets KPim${KF_MAJOR_VERSION}::AkonadiCore KPim${KF_MAJOR_VERSION}::MailTransport)

