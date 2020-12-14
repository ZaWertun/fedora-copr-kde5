%undefine __cmake_in_source_build
%if 0%{?fedora} >= 32
# Qt >= 5.14 needed
%global with_markdown 1
%endif

Name:    kdepim-addons
Version: 20.12.0
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

## upstream patches

## upstream patches (master)

# handled by qt5-srpm-macros, which defines %%qt5_qtwebengine_arches
%{?qt5_qtwebengine_arches:ExclusiveArch: %{qt5_qtwebengine_arches}}

BuildRequires:  extra-cmake-modules >= 5.39.0
BuildRequires:  kf5-rpm-macros

BuildRequires:  cmake(Qt5Widgets)
BuildRequires:  cmake(Qt5Test)
BuildRequires:  pkgconfig(Qt5WebEngine)

BuildRequires:  cmake(KF5Declarative)
BuildRequires:  cmake(KF5I18n)
BuildRequires:  cmake(KF5KHtml)
BuildRequires:  cmake(KF5Config)
BuildRequires:  cmake(KF5DBusAddons)
BuildRequires:  cmake(KF5Holidays)
BuildRequires:  cmake(KF5Prison)
BuildRequires:  cmake(KF5XmlGui)

BuildRequires:  cmake(KF5Akonadi)
BuildRequires:  cmake(KF5AkonadiNotes)
BuildRequires:  cmake(KF5CalendarSupport)
BuildRequires:  cmake(KF5EventViews)
BuildRequires:  cmake(KF5GrantleeTheme)
BuildRequires:  cmake(KF5Gravatar)
BuildRequires:  cmake(KF5IncidenceEditor)
BuildRequires:  cmake(KF5KaddressbookGrantlee)
BuildRequires:  cmake(KPimImportWizard)
BuildRequires:  cmake(KPimItinerary)
BuildRequires:  cmake(KPimPkPass)
BuildRequires:  cmake(KF5Libkdepim)
BuildRequires:  cmake(KF5Libkleo)
BuildRequires:  cmake(KF5MailCommon)
BuildRequires:  cmake(KF5MailImporterAkonadi)
BuildRequires:  cmake(KF5MessageComposer)
BuildRequires:  cmake(KF5MessageCore)
BuildRequires:  cmake(KF5MessageList)
BuildRequires:  cmake(KF5MessageViewer)
BuildRequires:  cmake(KF5PimCommon)
BuildRequires:  cmake(KF5Tnef)
BuildRequires:  cmake(KF5KontactInterface)

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

Conflicts:      kdepim-common < 16.04.0

# at least until we have subpkgs for each -- rex
Supplements:    kaddressbook
Supplements:    kmail
Supplements:    korganizer

%description
%{summary}.


%prep
%autosetup -n %{name}-%{version} -p1


%build
%{cmake_kf5} \
  -DKDEPIMADDONS_BUILD_EXAMPLES:BOOL=FALSE

%cmake_build


%install
%cmake_install

%find_lang %{name} --all-name --with-html


%ldconfig_scriptlets

%files -f %{name}.lang
%license LICENSES/*.txt
%{_kf5_libdir}/libgrammarcommon.so.5*
%{_kf5_libdir}/libkmailgrammalecte.so.5*
%{_kf5_libdir}/libkmaillanguagetool.so.5*
%{_kf5_libdir}/libadblocklibprivate.so.5*
%{_kf5_libdir}/libdkimverifyconfigure.so.5*
%{_kf5_libdir}/libfolderconfiguresettings.so.5*
%{_kf5_libdir}/libkmailquicktextpluginprivate.so.5*
%{_kf5_libdir}/libexpireaccounttrashfolderconfig.so.5*
%{_kf5_qtplugindir}/plasmacalendarplugins/pimevents.so
%{_kf5_qtplugindir}/plasmacalendarplugins/pimevents/
%{_kf5_qtplugindir}/webengineviewer/urlinterceptor/webengineviewer_adblockplugin.so
%{_kf5_qtplugindir}/webengineviewer/urlinterceptor/webengineviewer_donottrackplugin.so
%{_kf5_datadir}/kconf_update/*.upd
%{_kf5_qmldir}/org/kde/plasma/PimCalendars/
%{_kf5_datadir}/qlogging-categories5/*categories

# TODO: Split to per-app subpackages?
# KAddressBook
%{_kf5_libdir}/libkaddressbookmergelibprivate.so*
%{_kf5_libdir}/contacteditor/editorpageplugins/cryptopageplugin.so

%dir %{_kf5_qtplugindir}/kaddressbook/
%{_kf5_qtplugindir}/kaddressbook/mainview/kaddressbook_checkgravatarplugin.so
%{_kf5_qtplugindir}/kaddressbook/mainview/kaddressbook_mergecontactsplugin.so
%{_kf5_qtplugindir}/kaddressbook/mainview/kaddressbook_searchduplicatesplugin.so
%{_kf5_qtplugindir}/kaddressbook/mainview/kaddressbook_sendmailplugin.so
%{_kf5_qtplugindir}/kaddressbook/mainview/kaddressbook_sendvcardsplugin.so

# Akonadi
%{_kf5_qtplugindir}/akonadi/emailaddressselectionldapdialogplugin.so

# KMail
%{_kf5_bindir}/kmail_*.sh
%dir %{_kf5_qtplugindir}/kmail/
%{_kf5_qtplugindir}/kmail/plugineditor/kmail_autocorrectioneditorplugin.so
%{_kf5_qtplugindir}/kmail/plugineditor/kmail_changecaseeditorplugin.so
%{_kf5_qtplugindir}/kmail/plugineditor/kmail_insertemaileditorplugin.so
%{_kf5_qtplugindir}/kmail/plugineditor/kmail_insertshorturleditorplugin.so
%{_kf5_qtplugindir}/kmail/plugineditor/kmail_insertspecialcharactereditorplugin.so
%{_kf5_qtplugindir}/kmail/plugineditor/kmail_nonbreakingspaceeditorplugin.so
%{_kf5_qtplugindir}/kmail/plugineditor/kmail_quicktextplugin.so
%{_kf5_qtplugindir}/kmail/plugineditor/kmail_sharetexteditorplugin.so
%{_kf5_qtplugindir}/kmail/plugineditor/kmail_zoomtexteditorplugin.so
%{_kf5_qtplugindir}/kmail/mainview/kmail_antispamplugin.so
%{_kf5_qtplugindir}/kmail/mainview/kmail_antivirusplugin.so
%{_kf5_qtplugindir}/kmail/mainview/kmail_expertplugin.so
%{_kf5_qtplugindir}/kmail/mainview/kmail_logactivitiesplugin.so
%{_kf5_qtplugindir}/kmail/plugincheckbeforesend/kmail_automaticaddcontactseditorplugin.so
%{_kf5_qtplugindir}/kmail/plugincheckbeforesend/kmail_checkbeforesendeditorplugin.so
%{_kf5_qtplugindir}/kmail/plugincheckbeforesend/kmail_confirmaddresseditorplugin.so
%{_kf5_qtplugindir}/kmail/plugineditorgrammar/kmail_grammalecteplugin.so
%{_kf5_qtplugindir}/kmail/plugineditorgrammar/kmail_languagetoolplugin.so
%{_kf5_qtplugindir}/kmail/plugineditorinit/kmail_externalcomposereditorplugin.so
%{_kf5_sysconfdir}/xdg/kmail.antispamrc
%{_kf5_sysconfdir}/xdg/kmail.antivirusrc

%{_kf5_qtplugindir}/libksieve/emaillineeditplugin.so
%{_kf5_qtplugindir}/libksieve/imapfoldercompletionplugin.so

%{_kf5_qtplugindir}/importwizard/evolutionv1importerplugin.so
%{_kf5_qtplugindir}/importwizard/evolutionv2importerplugin.so
%{_kf5_qtplugindir}/importwizard/gearyimporterplugin.so
%{_kf5_qtplugindir}/importwizard/nylasmailimporterplugin.so
%{_kf5_qtplugindir}/importwizard/operaimporterplugin.so
%{_kf5_qtplugindir}/libksieve/regexpeditorlineeditplugin.so
%{_kf5_qtplugindir}/mailtransport/mailtransport_sendplugin.so
%{_kf5_qtplugindir}/templateparser/templateparseraddressrequesterplugin.so

# KOrganizer
%{_kf5_qtplugindir}/korg_datenums.so
%{_kf5_datadir}/kservices5/korganizer/datenums.desktop
%{_kf5_qtplugindir}/korg_picoftheday.so
%{_kf5_datadir}/kservices5/korganizer/picoftheday.desktop
%{_kf5_qtplugindir}/korg_thisdayinhistory.so
%{_kf5_datadir}/kservices5/korganizer/thisdayinhistory.desktop

# PimCommon
%dir %{_kf5_qtplugindir}/pimcommon/
%{_kf5_libdir}/libshorturlpluginprivate.so*
%{_kf5_qtplugindir}/pimcommon/customtools/pimcommon_translatorplugin.so
%{_kf5_qtplugindir}/pimcommon/shorturlengine/pimcommon_isgdshorturlengineplugin.so
%{_kf5_qtplugindir}/pimcommon/shorturlengine/pimcommon_tinyurlengineplugin.so
%{_kf5_qtplugindir}/pimcommon/shorturlengine/pimcommon_triopabshorturlengineplugin.so

# BodyPartFormatter
%dir %{_kf5_qtplugindir}/messageviewer/bodypartformatter/
%{_kf5_qtplugindir}/messageviewer/bodypartformatter/messageviewer_bodypartformatter_application_gnupgwks.so
%{_kf5_qtplugindir}/messageviewer/bodypartformatter/messageviewer_bodypartformatter_application_mstnef.so
%{_kf5_qtplugindir}/messageviewer/bodypartformatter/messageviewer_bodypartformatter_pkpass.so
%{_kf5_qtplugindir}/messageviewer/bodypartformatter/messageviewer_bodypartformatter_semantic.so
%{_kf5_qtplugindir}/messageviewer/bodypartformatter/messageviewer_bodypartformatter_text_calendar.so
%{_kf5_qtplugindir}/messageviewer/bodypartformatter/messageviewer_bodypartformatter_text_highlighter.so
%{_kf5_qtplugindir}/messageviewer/bodypartformatter/messageviewer_bodypartformatter_text_vcard.so
%if 0%{?with_markdown}
%{_kf5_qtplugindir}/messageviewer/bodypartformatter/messageviewer_bodypartformatter_text_markdown.so
%endif

# MessageViewer headers
%dir %{_kf5_qtplugindir}/messageviewer/
%{_kf5_qtplugindir}/messageviewer/headerstyle/messageviewer_briefheaderstyleplugin.so
%{_kf5_qtplugindir}/messageviewer/headerstyle/messageviewer_fancyheaderstyleplugin.so
%{_kf5_qtplugindir}/messageviewer/headerstyle/messageviewer_grantleeheaderstyleplugin.so
%{_kf5_qtplugindir}/messageviewer/headerstyle/messageviewer_longheaderstyleplugin.so
%{_kf5_qtplugindir}/messageviewer/headerstyle/messageviewer_standardsheaderstyleplugin.so

# MessageViewer
%{_kf5_qtplugindir}/messageviewer/grantlee/
%{_kf5_qtplugindir}/messageviewer/viewerplugin/messageviewer_createeventplugin.so
%{_kf5_qtplugindir}/messageviewer/viewerplugin/messageviewer_createnoteplugin.so
%{_kf5_qtplugindir}/messageviewer/viewerplugin/messageviewer_createtodoplugin.so
%{_kf5_qtplugindir}/messageviewer/viewerplugin/messageviewer_externalscriptplugin.so 
%{_kf5_qtplugindir}/messageviewer/viewercommonplugin/messageviewer_expandurlplugin.so
%{_kf5_qtplugindir}/messageviewer/viewercommonplugin/messageviewer_translatorplugin.so
%{_kf5_qtplugindir}/messageviewer/configuresettings/messageviewer_dkimconfigplugin.so
%{_kf5_qtplugindir}/messageviewer/configuresettings/messageviewer_gravatarconfigplugin.so
%{_kf5_qtplugindir}/messageviewer/configuresettings/messageviewer_folderconfiguresettingsplugin.so
%{_kf5_qtplugindir}/messageviewer/configuresettings/messageviewer_expireaccounttrashfolderconfigplugin.so

# qtcreator templates
%dir %{_datadir}/qtcreator
%dir %{_datadir}/qtcreator/templates
%{_datadir}/qtcreator/templates/kmaileditorconvertertextplugins/
%{_datadir}/qtcreator/templates/kmaileditorplugins/


%changelog
* Thu Dec 10 21:56:11 MSK 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.12.0-1
- 20.12.0

* Fri Nov  6 13:25:39 MSK 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.08.3-1
- 20.08.3

* Fri Oct 09 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.08.2-1
- 20.08.2

* Thu Sep 03 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.08.1-1
- 20.08.1

* Fri Aug 14 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.08.0-1
- 20.08.0

* Thu Jul 09 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.04.3-1
- 20.04.3

* Fri Jun 12 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.04.2-1
- 20.04.2

* Tue May 19 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.04.1-1
- 20.04.1

* Fri Apr 24 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.04.0-1
- 20.04.0

* Fri Mar 06 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 19.12.3-1
- 19.12.3

* Fri Feb 07 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 19.12.2-1
- 19.12.2

* Fri Jan 10 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 19.12.1-1
- 19.12.1

* Thu Dec 12 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 19.12.0-1
- 19.12.0

* Fri Nov 08 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 19.08.3-1
- 19.08.3

* Thu Oct 10 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 19.08.2-1
- 19.08.2

* Thu Sep 05 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 19.08.1-1
- 19.08.1

* Thu Aug 15 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 19.08.0-1
- 19.08.0

* Thu Jul 11 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 19.04.3-1
- 19.04.3

* Thu Jun 06 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 19.04.2-1
- 19.04.2

* Thu May 09 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 19.04.1-1
- 19.04.1

* Mon May 06 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 19.04.0-1
- 19.04.0

* Tue Feb 05 2019 Rex Dieter <rdieter@fedoraproject.org> - 18.12.2-1
- 18.12.2

* Fri Feb 01 2019 Fedora Release Engineering <releng@fedoraproject.org> - 18.12.1-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_30_Mass_Rebuild

* Tue Jan 08 2019 Rex Dieter <rdieter@fedoraproject.org> - 18.12.1-1
- 18.12.1

* Fri Dec 14 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.12.0-1
- 18.12.0

* Tue Nov 06 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.08.3-1
- 18.08.3

* Wed Oct 10 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.08.2-1
- 18.08.2

* Mon Oct 01 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.08.1-1
- 18.08.1

* Fri Jul 13 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.04.3-1
- 18.04.3

* Fri Jul 13 2018 Fedora Release Engineering <releng@fedoraproject.org> - 18.04.2-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_29_Mass_Rebuild

* Wed Jun 06 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.04.2-1
- 18.04.2

* Wed May 09 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.04.1-1
- 18.04.1

* Fri Apr 20 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.04.0-1
- 18.04.0

* Fri Mar 16 2018 Rex Dieter <rdieter@fedoraproject.org> - 17.12.3-2
- Supplements: kaddressbook kmail korganizer
- use %%make_build %%ldconfig_scriptlets

* Tue Mar 06 2018 Rex Dieter <rdieter@fedoraproject.org> - 17.12.3-1
- 17.12.3

* Wed Feb 07 2018 Fedora Release Engineering <releng@fedoraproject.org> - 17.12.2-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_28_Mass_Rebuild

* Tue Feb 06 2018 Rex Dieter <rdieter@fedoraproject.org> - 17.12.2-1
- 17.12.2

* Fri Jan 12 2018 Rex Dieter <rdieter@fedoraproject.org> - 17.12.1-2
- pull in upstream fixes in particular...
- High memory usage when adding PIM Events in Digital Clock Widget (kde#367541)

* Thu Jan 11 2018 Rex Dieter <rdieter@fedoraproject.org> - 17.12.1-1
- 17.12.1

* Tue Dec 12 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.12.0-1
- 17.12.0

* Wed Dec 06 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.11.90-1
- 17.11.90

* Wed Nov 22 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.11.80-1
- 17.11.80

* Wed Nov 08 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.08.3-1
- 17.08.3

* Mon Sep 25 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.08.1-1
- 17.08.1

* Thu Aug 03 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.04.3-3
- rebuild

* Thu Aug 03 2017 Fedora Release Engineering <releng@fedoraproject.org> - 17.04.3-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Binutils_Mass_Rebuild

* Fri Jul 28 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.04.3-1
- 17.04.3

* Wed Jul 26 2017 Fedora Release Engineering <releng@fedoraproject.org> - 17.04.2-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Mass_Rebuild

* Thu Jun 15 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.04.2-1
- 17.04.2

* Mon May 15 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.04.1-1
- 17.04.1

* Thu Mar 09 2017 Rex Dieter <rdieter@fedoraproject.org> - 16.12.3-1
- 16.12.3

* Thu Feb 09 2017 Rex Dieter <rdieter@fedoraproject.org> - 16.12.2-1
- 16.12.2

* Mon Jan 16 2017 Rex Dieter <rdieter@fedoraproject.org> - 16.12.1-1
- 16.12.1

* Mon Jan 16 2017 Rex Dieter <rdieter@fedoraproject.org> - 16.08.3-2
- rebuild (gpgme)

* Mon Dec 05 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.08.3-1
- 16.08.3

* Fri Oct 28 2016 Than Ngo <than@redhat.com> - 16.08.2-2
- don't build on ppc64/s390x as qtwebengine is not supported yet

* Thu Oct 13 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.08.2-1
- 16.08.2

* Thu Sep 08 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.08.1-1
- 16.08.1

* Sun Sep 04 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.08.0-1
- 16.08.0

* Sun Jul 10 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.04.3-1
- 16.04.3

* Sun Jun 12 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.04.2-1
- 16.04.2

* Tue May 03 2016 Daniel Vrátil <dvratil@fedoraproject.org> - 16.04.0-1
- Initial version
