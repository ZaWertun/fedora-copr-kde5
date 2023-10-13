add_executable(kolabformatchecker kolabformatchecker.cpp)
target_link_libraries(kolabformatchecker pimkolab Qt::Core KPim${KF_MAJOR_VERSION}::AkonadiCore KF${KF_MAJOR_VERSION}::Contacts KF${KF_MAJOR_VERSION}::CalendarCore KPim${KF_MAJOR_VERSION}::Mime)
