%global with_markdown 1

Name:    kdepim-addons
Version: 23.08.2
Release: 1%{?dist}
Summary: Additional plugins for KDE PIM applications

License: GPLv2 and LGPLv2+
URL:     https://cgit.kde.org/%{name}.git/

%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0: https://download.kde.org/%{stable}/release-service/%{version}/src/%{name}-%{version}.tar.xz
Source1: https://download.kde.org/%{stable}/release-service/%{version}/src/%{name}-%{version}.tar.xz.sig
Source2: gpgkey-D81C0CB38EB725EF6691C385BB463350D6EF31EF.gpg

## upstream patches

## upstream patches (master)

# handled by qt5-srpm-macros, which defines %%qt5_qtwebengine_arches
%{?qt5_qtwebengine_arches:ExclusiveArch: %{qt5_qtwebengine_arches}}

BuildRequires:  gnupg2
BuildRequires:  extra-cmake-modules >= 5.39.0
BuildRequires:  kf5-rpm-macros

BuildRequires:  cmake(Qt5Widgets)
BuildRequires:  cmake(Qt5Test)
BuildRequires:  pkgconfig(Qt5WebEngine)
BuildRequires:  cmake(Grantlee5)

BuildRequires:  cmake(KF5Declarative)
BuildRequires:  cmake(KF5I18n)
BuildRequires:  cmake(KF5KHtml)
BuildRequires:  cmake(KF5Config)
BuildRequires:  cmake(KF5DBusAddons)
BuildRequires:  cmake(KF5Holidays)
BuildRequires:  cmake(KF5Prison)
BuildRequires:  cmake(KF5XmlGui)

BuildRequires:  cmake(KF5TextAutoCorrection)

BuildRequires:  cmake(KPim5Akonadi)
BuildRequires:  cmake(KPim5AkonadiNotes)
BuildRequires:  cmake(KF5CalendarSupport)
BuildRequires:  cmake(KPim5EventViews)
BuildRequires:  cmake(KPim5GrantleeTheme)
BuildRequires:  cmake(KPim5Gravatar)
BuildRequires:  cmake(KPim5IncidenceEditor)
BuildRequires:  cmake(KPimImportWizard)
BuildRequires:  cmake(KPimItinerary)
BuildRequires:  cmake(KPim5PkPass)
BuildRequires:  cmake(KPimAddressbookImportExport)
BuildRequires:  cmake(KPim5Libkdepim)
BuildRequires:  cmake(KPim5Libkleo)
BuildRequires:  cmake(KPim5MailCommon)
BuildRequires:  cmake(KPim5MailImporterAkonadi)
BuildRequires:  cmake(KPim5MessageComposer)
BuildRequires:  cmake(KPim5MessageCore)
BuildRequires:  cmake(KPim5MessageList)
BuildRequires:  cmake(KPim5MessageViewer)
BuildRequires:  cmake(KPim5PimCommon)
BuildRequires:  cmake(KPim5Tnef)
BuildRequires:  cmake(KPim5KontactInterface)

#global majmin_ver %%(echo %%{version} | cut -d. -f1,2)
%global majmin_ver %{version}
BuildRequires:  akonadi-import-wizard-devel >= %{majmin_ver}
BuildRequires:  kf5-akonadi-notes-devel >= %{majmin_ver}
BuildRequires:  kf5-akonadi-server-devel >= %{majmin_ver}
BuildRequires:  kf5-calendarsupport-devel >= %{majmin_ver}
BuildRequires:  kf5-eventviews-devel >= %{majmin_ver}
BuildRequires:  kf5-grantleetheme-devel >= %{majmin_ver}
BuildRequires:  kf5-incidenceeditor-devel >= %{majmin_ver}
BuildRequires:  kf5-kitinerary-devel >= %{majmin_ver}
BuildRequires:  kf5-kpkpass-devel >= %{majmin_ver}
BuildRequires:  kf5-ktnef-devel >= %{majmin_ver}
BuildRequires:  kf5-libgravatar-devel >= %{majmin_ver}
BuildRequires:  kf5-libkleo-devel >= %{majmin_ver}
BuildRequires:  kf5-libksieve-devel >= %{majmin_ver}
BuildRequires:  kf5-mailcommon-devel >= %{majmin_ver}
BuildRequires:  kf5-mailimporter-devel >= %{majmin_ver}
BuildRequires:  kf5-messagelib-devel >= %{majmin_ver}
BuildRequires:  kf5-pimcommon-devel >= %{majmin_ver}
BuildRequires:  libkgapi-devel >= %{majmin_ver}

BuildRequires:  pkgconfig(poppler-qt5)
BuildRequires:  pkgconfig(shared-mime-info)
BuildRequires:  discount

Conflicts:      kdepim-common < 16.04.0

# at least until we have subpkgs for each -- rex
Supplements:    kaddressbook
Supplements:    kmail
Supplements:    korganizer

%description
%{summary}.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -n %{name}-%{version} -p1


%build
%cmake_kf5 \
  -DKDEPIMADDONS_BUILD_EXAMPLES:BOOL=FALSE
%cmake_build


%install
%cmake_install
%find_lang %{name} --all-name --with-html


%ldconfig_scriptlets

%files -f %{name}.lang
%license LICENSES/*.txt
#{_kf5_libdir}/libgrammarcommon.so.5*
#{_kf5_libdir}/libkmailgrammalecte.so.5*
#{_kf5_libdir}/libkmaillanguagetool.so.5*
%{_kf5_libdir}/libadblocklibprivate.so.5*
%{_kf5_libdir}/libakonadidatasetools.so.5*
%{_kf5_libdir}/libdkimverifyconfigure.so.5*
%{_kf5_libdir}/libopenurlwithconfigure.so.5*
%{_kf5_libdir}/libscamconfiguresettings.so.5*
%{_kf5_libdir}/libfolderconfiguresettings.so.5*
%{_kf5_libdir}/libkmailconfirmbeforedeleting.so.5*
%{_kf5_libdir}/libkmailquicktextpluginprivate.so.5*
%{_kf5_libdir}/libexpireaccounttrashfolderconfig.so.5*
%{_kf5_qtplugindir}/plasmacalendarplugins/pimevents.so
%{_kf5_qtplugindir}/plasmacalendarplugins/pimevents/
%{_kf5_qtplugindir}/pim5/mailtransport/mailtransport_sendplugin.so
%{_kf5_qtplugindir}/pim5/webengineviewer/urlinterceptor/webengineviewer_adblockplugin.so
%{_kf5_qtplugindir}/pim5/webengineviewer/urlinterceptor/webengineviewer_donottrackplugin.so
%{_kf5_qtplugindir}/pim5/messageviewer/configuresettings/messageviewer_openurlwithconfigplugin.so
%{_kf5_qtplugindir}/pim5/messageviewer/configuresettings/messageviewer_scamconfiguresettingsplugin.so
%{_kf5_datadir}/kconf_update/*.upd
%{_kf5_datadir}/icons/hicolor/scalable/status/moon-phase-*.svg
%{_kf5_datadir}/qlogging-categories5/*categories
%{_kf5_qmldir}/org/kde/plasma/PimCalendars/

# TODO: Split to per-app subpackages?
# KAddressBook
%{_kf5_libdir}/libkaddressbookmergelibprivate.so*
%{_kf5_qtplugindir}/pim5/contacteditor/editorpageplugins/cryptopageplugin.so

%{_kf5_qtplugindir}/pim5/kaddressbook/mainview/kaddressbook_checkgravatarplugin.so
%{_kf5_qtplugindir}/pim5/kaddressbook/mainview/kaddressbook_mergecontactsplugin.so
%{_kf5_qtplugindir}/pim5/kaddressbook/mainview/kaddressbook_searchduplicatesplugin.so
%{_kf5_qtplugindir}/pim5/kaddressbook/mainview/kaddressbook_sendmailplugin.so
%{_kf5_qtplugindir}/pim5/kaddressbook/mainview/kaddressbook_sendvcardsplugin.so
%{_kf5_qtplugindir}/pim5/kaddressbook/importexportplugin/kaddressbook_importexportcsvplugin.so
%{_kf5_qtplugindir}/pim5/kaddressbook/importexportplugin/kaddressbook_importexportgmxplugin.so
%{_kf5_qtplugindir}/pim5/kaddressbook/importexportplugin/kaddressbook_importexportldapplugin.so
%{_kf5_qtplugindir}/pim5/kaddressbook/importexportplugin/kaddressbook_importexportldifplugin.so
%{_kf5_qtplugindir}/pim5/kaddressbook/importexportplugin/kaddressbook_importexportvcardplugin.so
%{_kf5_qtplugindir}/pim5/kaddressbook/importexportplugin/kaddressbook_importexportwindowscontactplugin.so

# Akonadi
%{_kf5_qtplugindir}/pim5/akonadi/emailaddressselectionldapdialogplugin.so

# KMail
%{_kf5_bindir}/kmail_*.sh
%{_kf5_qtplugindir}/pim5/kmail/plugineditor/kmail_autocorrectioneditorplugin.so
%{_kf5_qtplugindir}/pim5/kmail/plugineditor/kmail_changecaseeditorplugin.so
%{_kf5_qtplugindir}/pim5/kmail/plugineditor/kmail_insertemaileditorplugin.so
%{_kf5_qtplugindir}/pim5/kmail/plugineditor/kmail_insertshorturleditorplugin.so
%{_kf5_qtplugindir}/pim5/kmail/plugineditor/kmail_insertspecialcharactereditorplugin.so
%{_kf5_qtplugindir}/pim5/kmail/plugineditor/kmail_nonbreakingspaceeditorplugin.so
%{_kf5_qtplugindir}/pim5/kmail/plugineditor/kmail_quicktextplugin.so
%{_kf5_qtplugindir}/pim5/kmail/plugineditor/kmail_sharetexteditorplugin.so
%{_kf5_qtplugindir}/pim5/kmail/plugineditor/kmail_zoomtexteditorplugin.so
%{_kf5_qtplugindir}/pim5/kmail/mainview/kmail_akonadidatabasetoolplugin.so
%{_kf5_qtplugindir}/pim5/kmail/mainview/kmail_antispamplugin.so
%{_kf5_qtplugindir}/pim5/kmail/mainview/kmail_antivirusplugin.so
%{_kf5_qtplugindir}/pim5/kmail/mainview/kmail_expertplugin.so
%{_kf5_qtplugindir}/pim5/kmail/plugincheckbeforesend/kmail_automaticaddcontactseditorplugin.so
%{_kf5_qtplugindir}/pim5/kmail/plugincheckbeforesend/kmail_checkbeforesendeditorplugin.so
%{_kf5_qtplugindir}/pim5/kmail/plugincheckbeforesend/kmail_confirmaddresseditorplugin.so
%{_kf5_qtplugindir}/pim5/kmail/plugineditorgrammar/kmail_grammalecteplugin.so
%{_kf5_qtplugindir}/pim5/kmail/plugineditorgrammar/kmail_languagetoolplugin.so
%{_kf5_qtplugindir}/pim5/kmail/plugineditorinit/kmail_externalcomposereditorplugin.so
%{_kf5_sysconfdir}/xdg/kmail.antispamrc
%{_kf5_sysconfdir}/xdg/kmail.antivirusrc

%{_kf5_qtplugindir}/pim5/libksieve/emaillineeditplugin.so
%{_kf5_qtplugindir}/pim5/libksieve/imapfoldercompletionplugin.so

%{_kf5_qtplugindir}/pim5/importwizard/evolutionv1importerplugin.so
%{_kf5_qtplugindir}/pim5/importwizard/evolutionv2importerplugin.so
%{_kf5_qtplugindir}/pim5/importwizard/gearyimporterplugin.so
%{_kf5_qtplugindir}/pim5/importwizard/operaimporterplugin.so
%{_kf5_qtplugindir}/pim5/templateparser/templateparseraddressrequesterplugin.so

# KOrganizer
%{_kf5_qtplugindir}/pim5/korganizer/datenums.so
%{_kf5_qtplugindir}/pim5/korganizer/lunarphases.so
%{_kf5_qtplugindir}/pim5/korganizer/picoftheday.so
%{_kf5_qtplugindir}/pim5/korganizer/thisdayinhistory.so

# PimCommon
%{_kf5_libdir}/libshorturlpluginprivate.so*
%{_kf5_qtplugindir}/pim5/pimcommon/customtools/pimcommon_translatorplugin.so
%{_kf5_qtplugindir}/pim5/pimcommon/shorturlengine/pimcommon_isgdshorturlengineplugin.so
%{_kf5_qtplugindir}/pim5/pimcommon/shorturlengine/pimcommon_tinyurlengineplugin.so
%{_kf5_qtplugindir}/pim5/pimcommon/shorturlengine/pimcommon_triopabshorturlengineplugin.so

# BodyPartFormatter
%{_kf5_qtplugindir}/pim5/messageviewer/bodypartformatter/messageviewer_bodypartformatter_application_gnupgwks.so
%{_kf5_qtplugindir}/pim5/messageviewer/bodypartformatter/messageviewer_bodypartformatter_application_mstnef.so
%{_kf5_qtplugindir}/pim5/messageviewer/bodypartformatter/messageviewer_bodypartformatter_pkpass.so
%{_kf5_qtplugindir}/pim5/messageviewer/bodypartformatter/messageviewer_bodypartformatter_semantic.so
%{_kf5_qtplugindir}/pim5/messageviewer/bodypartformatter/messageviewer_bodypartformatter_text_calendar.so
%{_kf5_qtplugindir}/pim5/messageviewer/bodypartformatter/messageviewer_bodypartformatter_text_highlighter.so
%{_kf5_qtplugindir}/pim5/messageviewer/bodypartformatter/messageviewer_bodypartformatter_text_vcard.so
%if 0%{?with_markdown}
%{_kf5_qtplugindir}/pim5/messageviewer/bodypartformatter/messageviewer_bodypartformatter_text_markdown.so
%endif

# MessageViewer headers
%{_kf5_qtplugindir}/pim5/messageviewer/headerstyle/messageviewer_briefheaderstyleplugin.so
%{_kf5_qtplugindir}/pim5/messageviewer/headerstyle/messageviewer_fancyheaderstyleplugin.so
%{_kf5_qtplugindir}/pim5/messageviewer/headerstyle/messageviewer_grantleeheaderstyleplugin.so
%{_kf5_qtplugindir}/pim5/messageviewer/headerstyle/messageviewer_longheaderstyleplugin.so
%{_kf5_qtplugindir}/pim5/messageviewer/headerstyle/messageviewer_standardsheaderstyleplugin.so

# MessageViewer
%{_kf5_qtplugindir}/pim5/messageviewer/grantlee/
%{_kf5_qtplugindir}/pim5/messageviewer/viewerplugin/messageviewer_createeventplugin.so
%{_kf5_qtplugindir}/pim5/messageviewer/viewerplugin/messageviewer_createnoteplugin.so
%{_kf5_qtplugindir}/pim5/messageviewer/viewerplugin/messageviewer_createtodoplugin.so
%{_kf5_qtplugindir}/pim5/messageviewer/viewerplugin/messageviewer_externalscriptplugin.so 
%{_kf5_qtplugindir}/pim5/messageviewer/viewercommonplugin/messageviewer_expandurlplugin.so
%{_kf5_qtplugindir}/pim5/messageviewer/viewercommonplugin/messageviewer_translatorplugin.so
%{_kf5_qtplugindir}/pim5/messageviewer/configuresettings/messageviewer_dkimconfigplugin.so
%{_kf5_qtplugindir}/pim5/messageviewer/configuresettings/messageviewer_gravatarconfigplugin.so
%{_kf5_qtplugindir}/pim5/messageviewer/configuresettings/messageviewer_folderconfiguresettingsplugin.so
%{_kf5_qtplugindir}/pim5/messageviewer/configuresettings/messageviewer_expireaccounttrashfolderconfigplugin.so
%{_kf5_qtplugindir}/pim5/messageviewer/checkbeforedeleting/kmail_confirmbeforedeletingplugin.so

# qtcreator templates
%dir %{_datadir}/qtcreator
%dir %{_datadir}/qtcreator/templates
%{_datadir}/qtcreator/templates/kmaileditorconvertertextplugins/
%{_datadir}/qtcreator/templates/kmaileditorplugins/


%changelog
* Fri Oct 13 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.08.2-1
- 23.08.2

* Thu Sep 14 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.08.1-1
- 23.08.1

* Sun Aug 27 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.08.0-1
- 23.08.0

* Thu Jul 06 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.04.3-1
- 23.04.3

* Thu Jun 08 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.04.2-1
- 23.04.2

* Thu May 11 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.04.1-1
- 23.04.1

* Thu Apr 20 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.04.0-1
- 23.04.0

* Sat Mar 25 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.12.3-6
- rebuild

* Thu Mar 02 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.12.3-1
- 22.12.3

