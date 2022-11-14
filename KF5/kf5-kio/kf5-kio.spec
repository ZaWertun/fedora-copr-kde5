%global framework kio

Name:    kf5-%{framework}
Version: 5.100.0
Release: 1%{?dist}
Summary: KDE Frameworks 5 Tier 3 solution for filesystem abstraction

License: GPLv2+ and MIT and BSD
URL:     https://invent.kde.org/frameworks/%{framework}

%global majmin %(echo %{version} | cut -d. -f1-2)
%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0: http://download.kde.org/%{stable}/frameworks/%{majmin}/%{framework}-%{version}.tar.xz
Source1: http://download.kde.org/%{stable}/frameworks/%{majmin}/%{framework}-%{version}.tar.xz.sig
Source2: gpgkey-53E6B47B45CEA3E0D5B7457758D0EE648A48B3BB.gpg

## upstream patches (lookaside)

## upstreamable patches

%if 0%{?flatpak}
# Disable the help: and ghelp: protocol for Flatpak builds, to avoid depending
# on the docbook stack.
Patch101: kio-no-help-protocol.patch
%endif

# filter plugin provides
%global __provides_exclude_from ^(%{_kf5_qtplugindir}/.*\\.so)$

BuildRequires:  gnupg2
BuildRequires:  extra-cmake-modules >= %{majmin}
BuildRequires:  kf5-rpm-macros
# core
BuildRequires:  kf5-karchive-devel >= %{majmin}
BuildRequires:  kf5-kconfig-devel >= %{majmin}
BuildRequires:  kf5-kcoreaddons-devel >= %{majmin}
BuildRequires:  kf5-kcrash-devel >= %{majmin}
BuildRequires:  kf5-kdoctools-devel >= %{majmin}
BuildRequires:  kf5-kdbusaddons-devel >= %{majmin}
BuildRequires:  kf5-kguiaddons-devel >= %{majmin}
BuildRequires:  kf5-ki18n-devel >= %{majmin}
BuildRequires:  kf5-kservice-devel >= %{majmin}
BuildRequires:  kf5-solid-devel >= %{majmin}
# extras
BuildRequires:  kf5-kbookmarks-devel >= %{majmin}
BuildRequires:  kf5-kcompletion-devel >= %{majmin}
BuildRequires:  kf5-kconfigwidgets-devel >= %{majmin}
BuildRequires:  kf5-kiconthemes-devel >= %{majmin}
BuildRequires:  kf5-kitemviews-devel >= %{majmin}
BuildRequires:  kf5-kjobwidgets-devel >= %{majmin}
BuildRequires:  kf5-kwindowsystem-devel >= %{majmin}
# others
BuildRequires:  kf5-knotifications-devel >= %{majmin}
BuildRequires:  kf5-ktextwidgets-devel >= %{majmin}
BuildRequires:  kf5-kwallet-devel >= %{majmin}
BuildRequires:  kf5-kwidgetsaddons-devel >= %{majmin}
BuildRequires:  kf5-kxmlgui-devel >= %{majmin}

BuildRequires:  krb5-devel
BuildRequires:  libacl-devel
%if !0%{?flatpak}
BuildRequires:  libxml2-devel
BuildRequires:  libxslt-devel
%endif
BuildRequires:  zlib-devel
BuildRequires:  pkgconfig(mount)

BuildRequires:  qt5-qtbase-devel
BuildRequires:  qt5-qtscript-devel
BuildRequires:  qt5-qtx11extras-devel
BuildRequires:  cmake(Qt5UiPlugin)
BuildRequires:  cmake(Qt5Qml)


%if ! 0%{?bootstrap}
# really runtime dep, but will make cmake happier when building
BuildRequires: kf5-kded-devel
# (apparently?) requires org.kde.klauncher5 service provided by kf5-kinit -- rex
# not versioned to allow update without bootstrap
# <skip!>
BuildRequires:  kf5-kinit-devel
%endif

Requires:       %{name}-core%{?_isa} = %{version}-%{release}
Requires:       %{name}-widgets%{?_isa} = %{version}-%{release}
Requires:       %{name}-file-widgets%{?_isa} = %{version}-%{release}
Requires:       %{name}-ntlm%{?_isa} = %{version}-%{release}
Requires:       %{name}-gui%{?_isa} = %{version}-%{release}

Requires: kf5-kded

%if 0%{?fedora} || 0%{?rhel} > 7
%global _with_html --with-html
%endif

%description
KDE Frameworks 5 Tier 3 solution for filesystem abstraction

%package        devel
Summary:        Development files for %{name}
Requires:       %{name}%{?_isa} = %{version}-%{release}
Requires:       kf5-kbookmarks-devel >= %{majmin}
Requires:       kf5-kcompletion-devel >= %{majmin}
Requires:       kf5-kconfig-devel >= %{majmin}
Requires:       kf5-kcoreaddons-devel >= %{majmin}
Requires:       kf5-kitemviews-devel >= %{majmin}
Requires:       kf5-kjobwidgets-devel >= %{majmin}
Requires:       kf5-kservice-devel >= %{majmin}
Requires:       kf5-solid-devel >= %{majmin}
Requires:       kf5-kxmlgui-devel >= %{majmin}
Requires:       kf5-kwindowsystem-devel >= %{majmin}
Requires:       qt5-qtbase-devel
%description    devel
The %{name}-devel package contains libraries and header files for
developing applications that use %{name}.

%package        doc
Summary:        Documentation files for %{name}
Requires:       %{name}-core = %{version}-%{release}
Obsoletes:      kf5-kio-doc < 5.11.0-3
BuildArch:      noarch
%description    doc
Documentation for %{name}.

%package        core
Summary:        Core components of the KIO Framework
## org.kde.klauncher5 service referenced from : src/core/slave.cpp
%{?kf5_kinit_requires}
Requires:       %{name}-core-libs%{?_isa} = %{version}-%{release}
Requires:       %{name}-doc = %{version}-%{release}
%description    core
KIOCore library provides core non-GUI components for working with KIO.

%package        core-libs
Summary:        Runtime libraries for KIO Core
Requires:       %{name}-core = %{version}-%{release}
%description    core-libs
%{summary}.

%package        widgets
Summary:        Widgets for KIO Framework
## org.kde.klauncher5 service referenced from : widgets/krun.cpp
## included here for completeness, even those -core already has a dependency.
%{?kf5_kinit_requires}
Requires:       %{name}-core%{?_isa} = %{version}-%{release}
%description    widgets
KIOWidgets contains classes that provide generic job control, progress
reporting, etc.

%package        widgets-libs
Summary:        Runtime libraries for KIO Widgets library
Requires:       %{name}-widgets = %{version}-%{release}
%description    widgets-libs
%{summary}.

%package        file-widgets
Summary:        Widgets for file-handling for KIO Framework
Requires:       %{name}-widgets%{?_isa} = %{version}-%{release}
%description    file-widgets
The KIOFileWidgets library provides the file selection dialog and
its components.

%package        gui
Summary:        Gui components for the KIO Framework
Requires:       %{name}-core%{?_isa} = %{version}-%{release}
%description    gui
%{summary}.

%package        ntlm
Summary:        NTLM support for KIO Framework
%description    ntlm
KIONTLM provides support for NTLM authentication mechanism in KIO


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -n %{framework}-%{version} -p1


%build
%cmake_kf5
%cmake_build


%install
%cmake_install

%find_lang %{name} --all-name --with-man %{?_with_html}


%files
%license LICENSES/*.txt
%doc README.md

%files core
%{_kf5_sysconfdir}/xdg/accept-languages.codes
%{_kf5_datadir}/qlogging-categories5/*categories
%{_kf5_libexecdir}/kio_http_cache_cleaner
%{_kf5_libexecdir}/kpac_dhcp_helper
%{_kf5_libexecdir}/kioexec
%{_kf5_libexecdir}/kioslave5
%{_kf5_libexecdir}/kiod5
%{_kf5_bindir}/ktelnetservice5
%{_kf5_bindir}/kcookiejar5
%{_kf5_bindir}/ktrash5
%{_kf5_plugindir}/kio/
%{_kf5_plugindir}/kded/
%{_kf5_qtplugindir}/kcm_trash.so
%{_kf5_qtplugindir}/kcm_proxy.so
%{_kf5_qtplugindir}/plasma/kcms/systemsettings/kcm_smb.so
%{_kf5_qtplugindir}/plasma/kcms/systemsettings_qwidgets/kcm_cookies.so
%{_kf5_qtplugindir}/plasma/kcms/systemsettings_qwidgets/kcm_netpref.so
%{_kf5_qtplugindir}/plasma/kcms/systemsettings_qwidgets/kcm_proxy.so
%{_kf5_qtplugindir}/plasma/kcms/systemsettings_qwidgets/kcm_webshortcuts.so
%{_kf5_plugindir}/kiod/
%{_kf5_datadir}/kservices5/cookies.desktop
%{_kf5_datadir}/kservices5/netpref.desktop
%{_kf5_datadir}/kservices5/proxy.desktop
%{_kf5_datadir}/kservices5/smb.desktop
%{_kf5_datadir}/kservices5/http_cache_cleaner.desktop
%{_kf5_datadir}/kservices5/kcmtrash.desktop
%{_kf5_datadir}/knotifications5/proxyscout.*
%{_kf5_datadir}/kf5/kcookiejar/domain_info
%{_kf5_datadir}/applications/*.desktop
%{_kf5_datadir}/kconf_update/*
%{_datadir}/dbus-1/services/org.kde.*.service

## omitted since 5.45, security concerns? -- rex
%if 0
# file_helper
%{_kf5_sysconfdir}/dbus-1/system.d/org.kde.kio.file.conf
%{_kf5_libexecdir}/kauth/file_helper
%{_kf5_datadir}/dbus-1/system-services/org.kde.kio.file.service
%{_kf5_datadir}/polkit-1/actions/org.kde.kio.file.policy
%endif

%ldconfig_scriptlets core-libs

%files core-libs
%{_kf5_libdir}/libKF5KIOCore.so.*

%files doc -f %{name}.lang
%{_kf5_mandir}/man8/kcookiejar5.8*
%if !0%{?_with_html:1}
%{_kf5_docdir}/HTML/*/*
%endif

%ldconfig_scriptlets gui

%files gui
%{_kf5_libdir}/libKF5KIOGui.so.*

%files widgets
%config %{_kf5_sysconfdir}/xdg/kshorturifilterrc
%{_kf5_qtplugindir}/kcm_webshortcuts.so
%dir %{_kf5_plugindir}/urifilters/
%{_kf5_plugindir}/urifilters/*.so
%{_kf5_datadir}/kservices5/webshortcuts.desktop
%{_kf5_datadir}/kservices5/searchproviders
%{_kf5_datadir}/kservicetypes5/*.desktop

%ldconfig_scriptlets widgets-libs

%files widgets-libs
%{_kf5_libdir}/libKF5KIOWidgets.so.*
%{_kf5_qtplugindir}/designer/*5widgets.so

%ldconfig_scriptlets file-widgets

%files file-widgets
%{_kf5_libdir}/libKF5KIOFileWidgets.so.*

%ldconfig_scriptlets ntlm

%files ntlm
%{_kf5_libdir}/libKF5KIONTLM.so.*

%files devel
%{_kf5_includedir}/*
%{_kf5_libdir}/*.so
%{_kf5_libdir}/cmake/KF5KIO/
%{_kf5_archdatadir}/mkspecs/modules/qt_KIOCore.pri
%{_kf5_archdatadir}/mkspecs/modules/qt_KIOFileWidgets.pri
%{_kf5_archdatadir}/mkspecs/modules/qt_KNTLM.pri
%{_kf5_archdatadir}/mkspecs/modules/qt_KIOWidgets.pri
%{_kf5_archdatadir}/mkspecs/modules/qt_KIOGui.pri
%{_datadir}/dbus-1/interfaces/*.xml
%{_kf5_bindir}/protocoltojson
%{_kf5_datadir}/kdevappwizard/templates/kioworker.tar.bz2


%changelog
* Mon Nov 14 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.100.0-1
- 5.100.0

* Mon Oct 10 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.99.0-1
- 5.99.0

* Mon Sep 12 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.98.0-1
- 5.98.0

* Sun Aug 14 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.97.0-1
- 5.97.0

* Sun Jul 10 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.96.0-1
- 5.96.0

* Mon Jun 13 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.95.0-1
- 5.95.0

* Sat May 14 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.94.0-1
- 5.94.0

* Sun Apr 10 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.93.0-1
- 5.93.0

* Sun Mar 13 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.92.0-1
- 5.92.0

* Mon Feb 14 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.91.0-1
- 5.91.0

* Sat Jan 08 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.90.0-1
- 5.90.0

* Mon Dec 13 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.89.0-1
- 5.89.0

* Sat Nov 13 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.88.0-1
- 5.88.0

* Sat Oct 09 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.87.0-1
- 5.87.0

* Sat Sep 11 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.86.0-1
- 5.86.0

* Sat Aug 14 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.85.0-1
- 5.85.0

* Sat Jul 10 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.84.0-1
- 5.84.0

* Sun Jun 13 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.83.0-1
- 5.83.0

* Wed May 26 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.82.0-4
- added patch to fix #437153

* Sat May 15 2021 Rex Dieter <rdieter@fedoraproject.org> - 5.82.0-3
- 2 more kio_file-related backports

* Fri May 14 2021 Rex Dieter <rdieter@fedoraproject.org> - 5.82.0-2
- backport KMimeTypeFinderJob fixes

* Mon May 03 2021 Rex Dieter <rdieter@fedoraproject.org> - 5.82.0-1
- 5.82.0

* Tue Apr 06 2021 Rex Dieter <rdieter@fedoraproject.org> - 5.81.0-1
- 5.81.0

* Tue Mar 30 2021 Jonathan Wakely <jwakely@redhat.com> - 5.80.1-2
- Rebuilt for removed libstdc++ symbol (#1937698)

* Mon Mar 15 2021 Rex Dieter <rdieter@fedoraproject.org> - 5.80.1-1
- 5.80.1

* Tue Mar 09 2021 Rex Dieter <rdieter@fedoraproject.org> - 5.80.0-1
- 5.80.0

* Sun Feb 07 2021 Rex Dieter <rdieter@fedoraproject.org> - 5.79.0-3
- BR: Qt5Qml, kf5-kded-devel

* Sat Feb 06 2021 Rex Dieter <rdieter@fedoraproject.org> - 5.79.0-2
- respin

* Sat Feb 06 2021 Rex Dieter <rdieter@fedoraproject.org> - 5.79.0-1
- 5.79.0

* Tue Jan 26 2021 Fedora Release Engineering <releng@fedoraproject.org> - 5.78.0-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_34_Mass_Rebuild

* Mon Jan  4 08:48:59 CST 2021 Rex Dieter <rdieter@fedoraproject.org> - 5.78.0-1
- 5.78.0

* Sun Dec 13 14:14:59 CST 2020 Rex Dieter <rdieter@fedoraproject.org> - 5.77.0-1
- 5.77.0

* Thu Nov 19 09:05:04 CST 2020 Rex Dieter <rdieter@fedoraproject.org> - 5.76.0-1
- 5.76.0

* Fri Oct 16 2020 Jeff Law <law@redhat.com> - 5.75.0-2
- Fix missing #include for gcc-11

* Wed Oct 14 09:57:16 CDT 2020 Rex Dieter <rdieter@fedoraproject.org> - 5.75.0-1
- 5.75.0

* Fri Sep 18 2020 Jan Grulich <jgrulich@redhat.com> - 5.74.1-1
- 5.74.1

* Mon Aug 03 2020 Rex Dieter <rdieter@fedoraproject.org> - 5.73.0-1
- 5.73.0

* Tue Jul 28 2020 Fedora Release Engineering <releng@fedoraproject.org> - 5.72.0-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_33_Mass_Rebuild

* Tue Jul 07 2020 Rex Dieter <rdieter@fedoraproject.org> - 5.72.0-1
- 5.72.0

* Tue Jun 16 2020 Rex Dieter <rdieter@fedoraproject.org> - 5.71.0-1
- 5.71.0

* Sun May 31 2020 Rex Dieter <rdieter@fedoraproject.org> - 5.70.1-2
- pull in upstream fix for "run in terminal" regression (#1841860)

* Sat May 16 2020 Rex Dieter <rdieter@fedoraproject.org> - 5.70.1-1
- 5.70.1

* Fri May 15 2020 Rex Dieter <rdieter@fedoraproject.org> - 5.70.0-3
- pull in upstream regression fix, copy dest to symlinked folder (kde#421213)

* Wed May 13 2020 Rex Dieter <rdieter@fedoraproject.org> - 5.70.0-2
- Requires: kf5-kded (#1835467)

* Mon May 04 2020 Rex Dieter <rdieter@fedoraproject.org> - 5.70.0-1
- 5.70.0

* Wed Apr 22 2020 Rex Dieter <rdieter@fedoraproject.org> - 5.69.0-2
- -devel: Requires: kf5-kwindowsystem

* Tue Apr 21 2020 Rex Dieter <rdieter@fedoraproject.org> - 5.69.0-1
- 5.69.0

* Fri Mar 20 2020 Rex Dieter <rdieter@fedoraproject.org> - 5.68.0-1
- 5.68.0

* Mon Feb 03 2020 Rex Dieter <rdieter@fedoraproject.org> - 5.67.0-1
- 5.67.0

* Wed Jan 29 2020 Fedora Release Engineering <releng@fedoraproject.org> - 5.66.0-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_32_Mass_Rebuild

* Tue Jan 07 2020 Rex Dieter <rdieter@fedoraproject.org> - 5.66.0-1
- 5.66.0

* Tue Dec 17 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.65.0-1
- 5.65.0

* Fri Nov 08 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.64.0-1
- 5.64.0

* Tue Oct 22 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.63.0-1
- 5.63.0

* Mon Sep 30 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.62.1-2
- pull in upstream crash-on-close fix

* Tue Sep 17 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.61.1-1
- 5.62.1

* Wed Aug 07 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.61.0-1
- 5.61.0

* Thu Jul 25 2019 Fedora Release Engineering <releng@fedoraproject.org> - 5.60.0-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_31_Mass_Rebuild

* Sat Jul 13 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.60.0-1
- 5.60.0

* Thu Jun 06 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.59.0-1
- 5.59.0

* Tue May 07 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.58.0-1
- 5.58.0

* Tue Apr 09 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.57.0-1
- 5.57.0

* Tue Mar 05 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.56.0-1
- 5.56.0

* Mon Feb 04 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.55.0-1
- 5.55.0

* Fri Feb 01 2019 Fedora Release Engineering <releng@fedoraproject.org> - 5.54.1-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_30_Mass_Rebuild

* Mon Jan 14 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.54.1-1
- 5.54.1

* Fri Jan 11 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.54.0-2
- respin

* Tue Jan 08 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.54.0-1
- 5.54.0

* Sun Dec 09 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.53.0-1
- 5.53.0

* Sun Nov 04 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.52.0-1
- 5.52.0

* Sat Oct 13 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.51.0-2
- pull in candidate fix for kio_help regression (kde#399709)

* Wed Oct 10 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.51.0-1
- 5.51.0

* Fri Sep 28 2018 Owen Taylor <otaylor@redhat.com> - 5.50.0-3
- Disable help/ghelp protocols for Flatpak builds

* Fri Sep 21 2018 Jan Grulich <jgrulich@redhat.com> - 5.50.0-2
- rebuild (qt5)

* Tue Sep 04 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.50.0-1
- 5.50.0

* Tue Aug 07 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.49.0-1
- 5.49.0

* Sun Jul 08 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.48.0-1
- 5.48.0

* Wed Jun 06 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.47.0-2
- respin

* Sat Jun 02 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.47.0-1
- 5.47.0

* Sun May 06 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.46.0-2
- respin

* Sat May 05 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.46.0-1
- 5.46.0

* Sun Apr 08 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.45.0-1
- 5.45.0

* Sun Mar 04 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.44.0-2
- use %%make_build %%ldconfig_scriptlets
- tarball respin

* Sat Mar 03 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.44.0-1
- 5.44.0

* Thu Feb 08 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.43.0-3
- respin

* Wed Feb 07 2018 Fedora Release Engineering <releng@fedoraproject.org> - 5.43.0-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_28_Mass_Rebuild

* Wed Feb 07 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.43.0-1
- 5.43.0

* Mon Jan 08 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.42.0-1
- 5.42.0

* Tue Dec 19 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.41.0-3
- backport upstream fix: forwardsocket errors (D9249)

* Thu Dec 07 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.41.0-2
- cleanup

* Mon Dec 04 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.41.0-1
- 5.41.0

* Tue Nov 28 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.40.0-3
- pull in upstream fixes for Qt 5.9.3/5.10-beta changes

* Mon Nov 13 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.40.0-2
- kf5-kio runs wrong kioslave binary (#1512418)

* Fri Nov 10 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.40.0-1
- 5.40.0

* Sun Oct 08 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.39.0-1
- 5.39.0

* Mon Sep 11 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.38.0-1
- 5.38.0

* Fri Aug 25 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.37.0-1
- 5.37.0

* Thu Aug 03 2017 Fedora Release Engineering <releng@fedoraproject.org> - 5.36.0-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Binutils_Mass_Rebuild

* Wed Jul 26 2017 Fedora Release Engineering <releng@fedoraproject.org> - 5.36.0-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Mass_Rebuild

* Mon Jul 03 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.36.0-1
- 5.36.0

* Sun Jun 04 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.35.0-1
- 5.35.0

* Mon May 15 2017 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 5.34.0-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_26_27_Mass_Rebuild

* Mon May 15 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.34.0-1
- 5.34.0

* Mon Apr 03 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.33.0-1
- 5.33.0

* Sat Mar 04 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.32.0-1
- 5.32.0

* Thu Mar 02 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.31.0-2
- CVE-2017-6410 (#1427808)
- .spec cosmetics, update URL

* Mon Feb 06 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.31.0-1
- 5.31.0

* Mon Jan 02 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.29.0-2
- filter plugin provides, own plugindir/urifilters

* Fri Dec 16 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.29.0-1
- 5.29.0

* Tue Oct 04 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.27.0-1
- 5.27.0

* Fri Sep 30 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.26.0-3
- more upstream fixes (nfsv4 detection, ktcpsocket::secureprotocols)

* Sat Sep 17 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.26.0-2
- Fix launching terminal .desktop files with konsole (#1376981,kde#368949)

* Wed Sep 07 2016 Daniel Vrátil <dvratil@fedoraproject.org> - 5.26.0-1
- KDE Frameworks 5.26.0

* Mon Aug 08 2016 Daniel Vrátil <dvratil@fedoraproject.org> - 5.25.0-1
- KDE Frameworks 5.25.0

* Thu Jul 28 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.24.0-2
- -core/-widgets: +%%{?kf5_kinit_requires}

* Wed Jul 06 2016 Daniel Vrátil <dvratil@fedoraproject.org> - 5.24.0-1
- KDE Frameworks 5.24.0

* Tue Jun 14 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.23.0-2
- BR: kf5-kinit-devel (provider of %%{?kf5_kinit_requires} macro)

* Tue Jun 07 2016 Daniel Vrátil <dvratil@fedoraproject.org> - 5.23.0-1
- KDE Frameworks 5.23.0

* Tue Jun 07 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.22.0-2
- update URL

* Mon May 16 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.22.0-1
- KDE Frameworks 5.22.0

* Mon Apr 04 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.21.0-1
- KDE Frameworks 5.21.0

* Thu Mar 31 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.20.0-3
- backport crash fix (kde#360488)

* Sat Mar 26 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.20.0-2
- -core: move Requires: -doc here (from empty main pkg)

* Mon Mar 14 2016 Daniel Vrátil <dvratil@fedoraproject.org> - 5.20.0-1
- KDE Frameworks 5.20.0

* Thu Feb 11 2016 Daniel Vrátil <dvratil@fedoraproject.org> - 5.19.0-1
- KDE Frameworks 5.19.0

* Thu Feb 04 2016 Fedora Release Engineering <releng@fedoraproject.org> - 5.18.0-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_24_Mass_Rebuild

* Thu Jan 14 2016 Rex Dieter <rdieter@fedoraproject.org> 5.18.0-2
- -BR: cmake

* Sun Jan 03 2016 Daniel Vrátil <dvratil@fedoraproject.org> - 5.18.0-1
- KDE Frameworks 5.18.0

* Tue Dec 22 2015 Rex Dieter <rdieter@fedoraproject.org> - 5.17.0-3
- use %%_kf5_kinit_requires macro
- -core: own plugindir/{kio,kded,kiod}

* Tue Dec 08 2015 Daniel Vrátil <dvratil@fedoraproject.org> - 5.17.0-1
- KDE Frameworks 5.17.0

* Tue Nov 10 2015 Daniel Vrátil <dvratil@fedoraproject.org> - 5.16.0-2
- kf5-kinit dep must be unversioned (my silly script ignored that)

* Sun Nov 08 2015 Daniel Vrátil <dvratil@fedoraproject.org> - 5.16.0-1
- KDE Frameworks 5.16.0

* Mon Nov 02 2015 Rex Dieter <rdieter@fedoraproject.org> 5.15.0-2
- .spec cleanup/cosmetics, update URL, (noarch) -doc subpkg

* Thu Oct 08 2015 Daniel Vrátil <dvratil@redhat.com> - 5.15.0-1
- KDE Frameworks 5.15.0

* Thu Sep 17 2015 Daniel Vrátil <dvratil@redhat.com> - 5.14.0-2
- kf5-kinit dependency must be unversioned

* Wed Sep 16 2015 Daniel Vrátil <dvratil@redhat.com> - 5.14.0-1
- KDE Frameworks 5.14.0

* Thu Aug 20 2015 Daniel Vrátil <dvratil@redhat.com> - 5.13.0-2
- Make kf5-kinit dependency unversioned to allow non-bootstrapped update

* Wed Aug 19 2015 Daniel Vrátil <dvratil@redhat.com> - 5.13.0-1
- KDE Frameworks 5.13.0

* Wed Aug 19 2015 Daniel Vrátil <dvratil@redhat.com> - 5.13.0-1
- KDE Frameworks 5.13.0

* Tue Aug 11 2015 Daniel Vrátil <dvratil@redhat.com> - 5.13.0-0.1
- KDE Frameworks 5.13

* Fri Jul 10 2015 Rex Dieter <rdieter@fedoraproject.org> - 5.12.0-1
- 5.12.0

* Thu Jul 02 2015 Rex Dieter <rdieter@fedoraproject.org> - 5.11.0-3
- .spec cleanup, cleanup Conflicts
- use %%license
- drop -doc subpkg
- Requires: kf5-kinit (kio apparently needs org.kde.klauncher5 service provided by kinit)

* Wed Jun 17 2015 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 5.11.0-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_23_Mass_Rebuild

* Wed Jun 10 2015 Daniel Vrátil <dvratil@redhat.com> - 5.11.0-1
- KDE Frameworks 5.11.0

* Mon May 11 2015 Daniel Vrátil <dvratil@redhat.com> - 5.10.0-1
- KDE Frameworks 5.10.0

* Fri May 01 2015 Rex Dieter <rdieter@fedoraproject.org> - 5.9.0-3
- Added folders to left panel "Places" disappear (kde#345174)
- optimize scriptlets
- .spec cosmetics

* Thu Apr 30 2015 Rex Dieter <rdieter@fedoraproject.org> 5.9.0-2
- BR: krb5-devel libacl-devel

* Tue Apr 07 2015 Daniel Vrátil <dvratil@redhat.com> - 5.9.0-1
- KDE Frameworks 5.9.0

* Fri Apr 03 2015 Rex Dieter <rdieter@fedoraproject.org> 5.8.0-2
- -core: own %%{_kf5_datadir}/kservices5/kded/

* Mon Mar 16 2015 Daniel Vrátil <dvratil@redhat.com> - 5.8.0-1
- KDE Frameworks 5.8.0

* Fri Feb 27 2015 Daniel Vrátil <dvratil@redhat.com> - 5.7.0-2
- Rebuild (GCC 5)

* Mon Feb 16 2015 Daniel Vrátil <dvratil@redhat.com> - 5.7.0-1
- KDE Frameworks 5.7.0

* Mon Feb 09 2015 Daniel Vrátil <dvratil@redhat.com> - 5.7.0-1
- KDE Frameworks 5.7.0

* Thu Jan 08 2015 Daniel Vrátil <dvratil@redhat.com> - 5.6.0-1
- KDE Frameworks 5.6.0

* Mon Dec 08 2014 Daniel Vrátil <dvratil@redhat.com> - 5.5.0-1
- KDE Frameworks 5.5.0

* Mon Nov 03 2014 Daniel Vrátil <dvratil@redhat.com> - 5.4.0-1
- KDE Frameworks 5.4.0

* Thu Oct 30 2014 Daniel Vrátil <dvratil@redhat.com> - 5.3.0-3
- Fix typo in deps

* Wed Oct 29 2014 Daniel Vrátil <dvratil@redhat.com> - 5.3.0-2
- Split into subpackages

* Tue Oct 07 2014 Daniel Vrátil <dvratil@redhat.com> - 5.3.0-1
- KDE Frameworks 5.3.0

* Mon Sep 15 2014 Daniel Vrátil <dvratil@redhat.com> - 5.2.0-1
- KDE Frameworks 5.2.0

* Sat Aug 16 2014 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 5.1.0-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_21_22_Mass_Rebuild

* Wed Aug 06 2014 Daniel Vrátil <dvratil@redhat.com> - 5.1.0-1
- KDE Frameworks 5.1.0

* Mon Jul 14 2014 Daniel Vrátil <dvratil@redhat.com> - 5.0.0-2
- Fix plugin install path

* Wed Jul 09 2014 Daniel Vrátil <dvratil@redhat.com> - 5.0.0-1
- KDE Frameworks 5.0.0

* Sat Jun 28 2014 Daniel Vrátil <dvratil@redhat.com> - 4.100.0-2
- fixed licenses
- added %%config
- added update-desktop-database

* Tue Jun 03 2014 Daniel Vrátil <dvratil@redhat.com> - 4.100.0-1
- KDE Frameworks 4.100.0

* Mon May 05 2014 Daniel Vrátil <dvratil@redhat.com> - 4.99.0
- KDE Frameworks 4.99.0

* Wed Apr 02 2014 Daniel Vrátil <dvratil@redhat.com> 4.98.0-2
- Fix conflict of kf5-kio-doc with kdelibs4

* Mon Mar 31 2014 Jan Grulich <jgrulich@redhat.com> 4.98.0-1
- Update to KDE Frameworks 5 Beta 1 (4.98.0)

* Tue Mar 11 2014 Jan Grulich <jgrulich@redhat.com> 4.97.0-2
- remove public dependencies

* Wed Mar 05 2014 Jan Grulich <jgrulich@redhat.com> 4.97.0-1
- Update to KDE Frameworks 5 Alpha 1 (4.97.0)

* Wed Feb 12 2014 Daniel Vrátil <dvratil@redhat.com> 4.96.0-1
- Update to KDE Frameworks 5 Alpha 1 (4.96.0)

* Wed Feb 05 2014 Daniel Vrátil <dvratil@redhat.com> 4.96.0-0.1.20140205git
- Update to pre-relase snapshot of 4.96.0

* Mon Jan 20 2014 Daniel Vrátil <dvratil@redhat.com> 4.95.0-2
- rebuild against new kf5-filesystem

* Thu Jan 09 2014 Daniel Vrátil <dvratil@redhat.com> 4.95.0-1
- Update to KDE Frameworks 5 TP1 (4.95.0)

* Sat Jan  4 2014 Daniel Vrátil <dvratil@redhat.com>
- initial version

