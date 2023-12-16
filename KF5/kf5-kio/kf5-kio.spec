%global framework kio

Name:    kf5-%{framework}
Version: 5.113.0
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
Recommends:     switcheroo-control
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
* Fri Dec 15 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.113.0-1
- 5.113.0

* Sun Nov 12 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.112.0-1
- 5.112.0

* Thu Oct 19 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.111.0-1
- 5.111.0

* Sun Sep 10 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.110.0-1
- 5.110.0

* Thu Aug 24 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.109.0-5
- rebuild

* Wed Aug 23 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.109.0-2
- upstream changes

* Fri Aug 18 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.109.0-1
- 5.109.0

* Sun Jul 09 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.108.0-1
- 5.108.0

* Sat Jun 10 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.107.0-1
- 5.107.0

* Sat May 13 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.106.0-1
- 5.106.0

* Sat Apr 08 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.105.0-1
- 5.105.0

* Sat Mar 11 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.104.0-1
- 5.104.0

* Sun Feb 12 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.103.0-1
- 5.103.0

