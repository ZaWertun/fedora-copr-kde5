add_definitions(-DTRANSLATION_DOMAIN=\"kcm_ldap\")

add_library(kcm_ldap MODULE)
target_sources( kcm_ldap PRIVATE
    kcmldap.cpp
    kcmldap.h
    )


target_link_libraries(kcm_ldap KF${KF_MAJOR_VERSION}::CoreAddons KF${KF_MAJOR_VERSION}::KCMUtils KF${KF_MAJOR_VERSION}::I18n KPim${KF_MAJOR_VERSION}::Ldap)

install(TARGETS kcm_ldap DESTINATION ${KDE_INSTALL_PLUGINDIR}/pim${QT_MAJOR_VERSION}/kcms/kaddressbook)

