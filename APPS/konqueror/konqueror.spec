## uncomment to enable bootstrap mode
#global bootstrap 1

## use webengine by default or not
%global webengine_default 1

## experimental ninja support
#global ninja 1
## FIXME: many tests require GLX, which doesn't appear to work as-is under koji
#global tests 1

Name:    konqueror
Version: 22.04.2
Release: 1%{?dist}
Summary: KDE File Manager and Browser

License: GPLv2+ and LGPLv2+ and GFDL
URL:     https://konqueror.org/

%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0: https://download.kde.org/%{stable}/release-service/%{version}/src/%{name}-%{version}.tar.xz

## upstream patches

## upstreamable patches
# toggle 'Always try to have one preloaded instance' to default off
# https://bugzilla.redhat.com/1523082
# https://bugs.kde.org/398996
Patch101: konqueror-18.12.2-preloaded.patch

## Fedora specific patches
# lower kwebenginepart to be lower than kwebkitpart (10<11)
# since kwebenginepart has crippling bug,
# https://bugzilla.redhat.com/show_bug.cgi?id=1523082
# https://bugs.kde.org/show_bug.cgi?id=401976
Patch200: konqueror-21.04.0-webenginepart_priority.patch

BuildRequires: desktop-file-utils

BuildRequires: extra-cmake-modules
BuildRequires: cmake(KDED)
BuildRequires: cmake(KF5Activities)
BuildRequires: cmake(KF5Archive)
BuildRequires: cmake(KF5Crash)
BuildRequires: cmake(KF5DocTools)
BuildRequires: cmake(KF5KCMUtils)
BuildRequires: cmake(KF5KDELibs4Support)
BuildRequires: cmake(KF5KHtml)
BuildRequires: cmake(KF5Parts)
BuildRequires: cmake(KF5Su)

BuildRequires: cmake(Qt5Core)
BuildRequires: cmake(Qt5Script)
%ifarch %{qt5_qtwebengine_arches}
%global webengine 1
%endif
BuildRequires: cmake(Qt5Widgets)
BuildRequires: cmake(Qt5X11Extras)

%if 0%{?ninja}
BuildRequires:  ninja-build
%endif

%if 0%{?tests}
BuildRequires: dbus-x11
BuildRequires: time
BuildRequires: xorg-x11-server-Xvfb
%endif

BuildRequires: pkgconfig(zlib)
BuildRequires: libtidy-devel

%if !0%{?bootstrap}
BuildRequires:  cmake(Qt5TextToSpeech)
%endif

%if 0%{?webengine} && 0%{?webengine_default}
Requires: kwebenginepart%{_isa} = %{version}-%{release}
%else
Requires: kwebkitpart%{?_isa} >= 1.4.0
%endif

# translations moved here
Conflicts: kde-l10n < 17.03

Requires:      %{name}-libs%{?_isa} = %{version}-%{release} 
Requires:      hicolor-icon-theme
Requires:      keditbookmarks

%description
Konqueror allows you to manage your files and browse the web in a
unified interface.

%package devel
Summary:       Development files for %{name}
Requires:      %{name}%{?_isa} = %{version}-%{release}
Requires:      %{name}-libs%{?_isa} = %{version}-%{release}
%description   devel
The %{name}-devel package contains libraries and header files for
developing applications that use %{name}.

%package libs
Summary:       Runtime libraries for %{name}
Requires:      %{name} = %{version}-%{release}
%description libs
%{summary}.

%if 0%{?webengine}
%package -n kwebenginepart
Summary:  A KPart based on QtWebEngine
BuildRequires: cmake(Qt5WebEngine)
BuildRequires: cmake(KF5Wallet)
%description -n kwebenginepart
KWebEnginePart is a web browser component for KDE (KPart)
based on (Qt)WebEngine. You can use it for example for
browsing the web in Konqueror.
%endif


%prep
%autosetup -p1


%build
%cmake_kf5 \
  %{?ninja:-G Ninja} \
  %{?tests:-DBUILD_TESTING:BOOL=ON}
%if 0%{?ninja}
%ninja_build -C %{_target_platform}
%else
%cmake_build
%endif


%install
%if 0%{?ninja}
%ninja_install -C %{_target_platform}
%else
%cmake_install
%endif

# omit some extraneous webenginepart files when building without webengine support
%if ! 0%{?webengine}
rm -rfv %{buildroot}%{_kf5_datadir}/webenginepart/
%endif


%find_lang %{name} --all-name --with-html


%check
appstream-util validate-relax --nonet %{buildroot}%{_kf5_metainfodir}/org.kde.konqueror.appdata.xml ||:
desktop-file-validate %{buildroot}%{_kf5_datadir}/applications/kfmclient.desktop
desktop-file-validate %{buildroot}%{_kf5_datadir}/applications/kfmclient_html.desktop
desktop-file-validate %{buildroot}%{_kf5_datadir}/applications/kfmclient_war.desktop
desktop-file-validate %{buildroot}%{_kf5_datadir}/applications/konqbrowser.desktop
%if 0%{?tests}
export CTEST_OUTPUT_ON_FAILURE=1
## cant use %%ninja_test here for some reason, doesn't inherit env vars from xvfb or dbus -- rex
xvfb-run -a \
%if 0%{?ninja}
ninja test -v -C %{_target_platform} ||:
%else
make test -C %{_target_platform} ARGS="--output-on-failure --timeout 300" ||:
%endif
%endif


%files -f %{name}.lang
%license LICENSES/*.txt
%doc AUTHORS ChangeLog
%{_kf5_sysconfdir}/xdg/konqsidebartngrc
%{_kf5_bindir}/fsview
%{_kf5_bindir}/kfmclient
%{_kf5_bindir}/konqueror
%{_kf5_bindir}/kcreatewebarchive
%{_kf5_qtplugindir}/dolphinpart/kpartplugins/*.so
%{_kf5_qtplugindir}/khtml/kpartplugins/*.so
%{_kf5_qtplugindir}/konqueror/kpartplugins/*.so
%{_kf5_qtplugindir}/kwebkitpart/kpartplugins/*.so
%{_kf5_qtplugindir}/webenginepart/kpartplugins/*.so
%{_kf5_datadir}/akregator/pics/feed.png
%{_kf5_metainfodir}/org.kde.konqueror.appdata.xml
%{_kf5_datadir}/applications/*.desktop
%{_kf5_datadir}/config.kcfg/*.kcfg
%{_kf5_datadir}/dbus-1/interfaces/*.xml
%{_kf5_datadir}/icons/hicolor/*/*/*
%{_kf5_datadir}/kcmcss/
%{_kf5_datadir}/kcontrol/
%{_kf5_datadir}/kf5/kbookmark/
%{_kf5_datadir}/konqueror/
%{_kf5_datadir}/konqsidebartng/entries/*.desktop
%{_kf5_datadir}/konqsidebartng/plugins/*.desktop
%{_kf5_sysconfdir}/xdg/autostart/konqy_preload.desktop
%{_kf5_sysconfdir}/xdg/translaterc
%{_kf5_datadir}/qlogging-categories5/*.categories
%{_kf5_datadir}/kxmlgui5/fsview/fsview_part.rc
%{_kf5_datadir}/kconf_update/webenginepart.upd

%ldconfig_scriptlets libs

%files libs
%{_kf5_datadir}/kservices5/*
%{_kf5_libdir}/lib*.so.*
%{_kf5_libdir}/libkdeinit5*.so
%{_kf5_libdir}/libkonqsidebarplugin.so
%dir %{_kf5_qtplugindir}/konqueror_kcms/
%{_kf5_qtplugindir}/konqueror_kcms/kcm_bookmarks.so
%{_kf5_qtplugindir}/konqueror_kcms/kcm_history.so
%{_kf5_qtplugindir}/konqueror_kcms/kcm_konq.so
%{_kf5_qtplugindir}/konqueror_kcms/kcm_performance.so
%{_kf5_qtplugindir}/konqueror_kcms/khtml_appearance.so
%{_kf5_qtplugindir}/konqueror_kcms/khtml_behavior.so
%{_kf5_qtplugindir}/konqueror_kcms/khtml_filter.so
%{_kf5_qtplugindir}/konqueror_kcms/khtml_general.so
%{_kf5_qtplugindir}/konqueror_kcms/khtml_java_js.so
%{_kf5_qtplugindir}/*.so
%{_kf5_plugindir}/kfileitemaction/akregatorplugin.so
%dir %{_kf5_plugindir}/parts/
%{_kf5_plugindir}/parts/fsviewpart.so
%{_kf5_plugindir}/parts/konq_sidebar.so


%files devel
%{_includedir}/konqsidebarplugin.h
%{_kf5_includedir}/konq_*.h
%{_kf5_includedir}/libkonq_export.h
%{_kf5_libdir}/cmake/KF5Konq/
%{_kf5_libdir}/libKF5Konq.so

%if 0%{?webengine}
%files -n kwebenginepart
%{_kf5_datadir}/kxmlgui5/webenginepart/
%{_kf5_datadir}/webenginepart/
%{_kf5_libdir}/libkwebenginepart.so
%{_kf5_plugindir}/parts/webenginepart.so
%endif


%changelog
* Thu Jun 09 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.04.2-1
- 22.04.2

* Thu May 12 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.04.1-1
- 22.04.1

* Thu Apr 21 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.04.0-1
- 22.04.0

* Thu Mar 03 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 21.12.3-1
- 21.12.3

* Thu Feb 03 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 21.12.2-1
- 21.12.2

* Thu Jan 06 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 21.12.1-1
- 21.12.1

* Thu Dec 09 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 21.12.0-1
- 21.12.0

* Thu Nov 04 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 21.08.3-1
- 21.08.3

* Thu Oct 07 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 21.08.2-1
- 21.08.2

* Thu Sep 02 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 21.08.1-1
- 21.08.1

* Thu Aug 12 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 21.08.0-1
- 21.08.0

* Thu Jul 08 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 21.04.3-1
- 21.04.3

* Fri Jun 11 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 21.04.2-1
- 21.04.2

* Thu May 13 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 21.04.1-1
- 21.04.1

* Thu Apr 22 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 21.04.0-1
- 21.04.0

* Sat Mar 06 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.12.3-1
- 20.12.3

* Thu Feb 04 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.12.2-1
- 20.12.2

* Thu Jan  7 22:09:36 MSK 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.12.1-1
- 20.12.1

* Thu Dec 10 21:56:35 MSK 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.12.0-1
- 20.12.0

* Fri Nov  6 13:26:06 MSK 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.08.3-1
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

* Sat Apr 27 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 19.04.0-1
- 19.04.0

* Thu Feb 21 2019 Rex Dieter <rdieter@fedoraproject.org> - 18.12.2-2
- AlwaysHavePreloaded => false default (#1523082, kde#398996)

* Wed Feb 20 2019 Rex Dieter <rdieter@fedoraproject.org> - 18.12.2-1
- 18.12.2
- optional ninja/tests support (not enabled by default)

* Fri Feb 01 2019 Fedora Release Engineering <releng@fedoraproject.org> - 18.12.0-4
- Rebuilt for https://fedoraproject.org/wiki/Fedora_30_Mass_Rebuild

* Sat Dec 29 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.12.0-3
- revert back to kwebenginepart default, testing shows #1523082 occurs for all backends

* Sat Dec 29 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.12.0-2
- default to kwebkitpart until kwebenginepart works properly (#1523082,kde#401976)

* Sat Dec 08 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.12.0-1
- 18.12.0

* Tue Nov 06 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.08.3-1
- 18.08.3

* Wed Oct 10 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.08.2-1
- 18.08.2

* Fri Sep 07 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.08.1-1
- 18.08.1

* Wed Aug 15 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.08.0-1
- 18.08.0

* Thu Jul 12 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.04.3-1
- 18.04.3

* Tue Jun 05 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.04.2-1
- 18.04.2

* Tue May 08 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.04.1-1
- 18.04.1

* Sat Apr 14 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.04.0-1
- 18.04.0

* Wed Mar 21 2018 Rex Dieter <rdieter@fedoraproject.org> - 17.12.3-2
- make buildable on all archs (#1474171)
- -kwebenginepart subpkg

* Tue Mar 06 2018 Rex Dieter <rdieter@fedoraproject.org> - 17.12.3-1
- 17.12.3

* Tue Feb 06 2018 Rex Dieter <rdieter@fedoraproject.org> - 17.12.2-1
- 17.12.2

* Thu Jan 11 2018 Rex Dieter <rdieter@fedoraproject.org> - 17.12.1-1
- 17.12.1

* Thu Jan 11 2018 Igor Gnatenko <ignatenkobrain@fedoraproject.org> - 17.12.0-2
- Remove obsolete scriptlets

* Tue Dec 12 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.12.0-1
- 17.12.0

* Tue Nov 21 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.08.3-2
- BR: Qt5TextToSpeech

* Wed Nov 08 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.08.3-1
- 17.08.3

* Wed Oct 11 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.08.2-1
- 17.08.2

* Tue Sep 05 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.08.1-1
- 17.08.1

* Sat Aug 26 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.08.0-2
- Requires: keditbookmarks (#1474248)

* Sat Aug 26 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.08.0-1
- 17.08.0

* Thu Aug 03 2017 Fedora Release Engineering <releng@fedoraproject.org> - 17.04.3-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Binutils_Mass_Rebuild

* Fri Jul 28 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.04.3-1
- 17.04.3

* Wed Jul 26 2017 Fedora Release Engineering <releng@fedoraproject.org> - 17.04.2-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Mass_Rebuild

* Thu Jun 15 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.04.2-1
- 17.04.2

* Wed May 10 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.04.1-1
- 17.04.1

* Sat Apr 22 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.04.0-2
- use %%find_lang for handbooks

* Tue Apr 18 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.04.0-1
- 17.04.0

* Wed Mar 08 2017 Rex Dieter <rdieter@fedoraproject.org> - 16.12.3-1
- 16.12.3

* Wed Feb 08 2017 Rex Dieter <rdieter@fedoraproject.org> - 16.12.2-1
- 16.12.2

* Fri Jan 20 2017 Christian Dersch <lupinix@mailbox.org> - 16.12.1-1
- initial package (review: RHBZ #1413020)
