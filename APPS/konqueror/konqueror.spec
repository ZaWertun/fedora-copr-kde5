## use webengine by default or not
%global webengine_default 1

## FIXME: many tests require GLX, which doesn't appear to work as-is under koji
#global tests 1

Name:    konqueror
Version: 23.04.3
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
Source1: https://download.kde.org/%{stable}/release-service/%{version}/src/%{name}-%{version}.tar.xz.sig
Source2: gpgkey-D81C0CB38EB725EF6691C385BB463350D6EF31EF.gpg

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

BuildRequires: gnupg2
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
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -p1


%build
%cmake_kf5 \
  %{?tests:-DBUILD_TESTING:BOOL=ON}
%cmake_build


%install
%cmake_install

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
xvfb-run -a \
make test -C %{_vpath_builddir} ARGS="--output-on-failure --timeout 300" ||:
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
%{_kf5_qtplugindir}/konqueror_kcms/khtml_cache.so
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
* Thu Jul 06 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.04.3-1
- 23.04.3

* Thu Jun 08 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.04.2-1
- 23.04.2

* Thu May 11 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.04.1-1
- 23.04.1

* Thu Apr 20 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.04.0-1
- 23.04.0

* Thu Mar 02 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.12.3-1
- 22.12.3

* Thu Feb 02 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.12.2-1
- 22.12.2

