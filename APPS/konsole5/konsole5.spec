%global tests 1
%global base_name konsole

Name:    konsole5
Summary: KDE Terminal emulator
Version: 23.08.4
Release: 1%{?dist}

# sources: MIT and LGPLv2 and LGPLv2+ and GPLv2+
License: GPLv2 and GFDL
URL:     http://www.kde.org/applications/system/konsole/
#URL:    http://konsole.kde.org/

%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0: https://download.kde.org/%{stable}/release-service/%{version}/src/%{base_name}-%{version}.tar.xz
Source1: https://download.kde.org/%{stable}/release-service/%{version}/src/%{base_name}-%{version}.tar.xz.sig
Source2: gpgkey-D81C0CB38EB725EF6691C385BB463350D6EF31EF.gpg

## upstreamable patches

## upstream patches

## downstream patches
Patch200: konsole-history_location_default.patch

# custom konsolerc that sets default to cache as well
Source10: konsolerc

Obsoletes: konsole < 14.12
Provides:  konsole = %{version}-%{release}

%global maj_ver %(echo %{version} | cut -d. -f1)

BuildRequires: gnupg2
BuildRequires: desktop-file-utils
BuildRequires: gettext
BuildRequires: libicu-devel
BuildRequires: pkgconfig(x11)
BuildRequires: pkgconfig(zlib)

BuildRequires: extra-cmake-modules
BuildRequires: kf5-rpm-macros
BuildRequires: cmake(KF5Bookmarks)
BuildRequires: cmake(KF5Completion)
BuildRequires: cmake(KF5Config)
BuildRequires: cmake(KF5ConfigWidgets)
BuildRequires: cmake(KF5CoreAddons)
BuildRequires: cmake(KF5Crash)
BuildRequires: cmake(KF5DBusAddons)
BuildRequires: cmake(KF5DBusAddons)
BuildRequires: cmake(KF5DocTools)
BuildRequires: cmake(KF5GlobalAccel)
BuildRequires: cmake(KF5GuiAddons)
BuildRequires: cmake(KF5I18n)
BuildRequires: cmake(KF5IconThemes)
BuildRequires: cmake(KF5Init)
BuildRequires: cmake(KF5KIO)
BuildRequires: cmake(KF5NewStuff)
BuildRequires: cmake(KF5NewStuffCore)
BuildRequires: cmake(KF5Notifications)
BuildRequires: cmake(KF5NotifyConfig)
BuildRequires: cmake(KF5Parts)
BuildRequires: cmake(KF5Pty)
BuildRequires: cmake(KF5Service)
BuildRequires: cmake(KF5TextWidgets)
BuildRequires: cmake(KF5WidgetsAddons)
BuildRequires: cmake(KF5WindowSystem)
BuildRequires: cmake(KF5XmlGui)

## TODO?
#BuildRequires: kf5-konq-devel
BuildRequires: libappstream-glib
BuildRequires: qt5-qtbase-devel
BuildRequires: qt5-qtscript-devel
BuildRequires: cmake(Qt5Multimedia)

%if 0%{?tests}
BuildRequires: xorg-x11-server-Xvfb dbus-x11
%endif

# translations moved here
Conflicts: kde-l10n < 17.03

Requires: %{name}-part%{?_isa} = %{version}-%{release}
Requires: keditbookmarks

%{?kf5_kinit_requires}

%description
%{summary}.

%package part
Summary: Konsole5 kpart plugin
%description part
%{summary}.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -n %{base_name}-%{version} -p1


%build
%cmake_kf5 \
  %{?tests:-DBUILD_TESTING:BOOL=ON} \
  -DENABLE_PLUGIN_SSHMANAGER=ON
%cmake_build


%install
%cmake_install
install -m644 -p -b -D %{SOURCE10} %{buildroot}%{_kf5_sysconfdir}/xdg/konsolerc
%find_lang konsole --with-html
rm -v %{buildroot}%{_kf5_sysconfdir}/xdg/konsolerc~


%check
appstream-util validate-relax --nonet %{buildroot}%{_kf5_metainfodir}/org.kde.konsole.appdata.xml
desktop-file-validate %{buildroot}%{_kf5_datadir}/applications/org.kde.konsole.desktop

%if 0%{?tests}
test "$(xvfb-run -a %{_vpath_builddir}/bin/konsole --version)" = "konsole %{version}" ||:
export CTEST_OUTPUT_ON_FAILURE=1
DBUS_SESSION_BUS_ADDRESS=
xvfb-run -a bash -c "%{ctest --timeout 30}" ||:
%endif


%files -f konsole.lang
%doc README.md
%{_kf5_bindir}/konsole
%{_kf5_bindir}/konsoleprofile
%{_kf5_libdir}/libkonsoleapp.so.%{maj_ver}*
%{_kf5_libdir}/kconf_update_bin/konsole_globalaccel
%{_kf5_libdir}/kconf_update_bin/konsole_show_menubar
%{_kf5_metainfodir}/org.kde.konsole.appdata.xml
%{_kf5_datadir}/applications/org.kde.konsole.desktop
%{_kf5_datadir}/kconf_update/konsole.upd
%{_kf5_datadir}/kconf_update/konsole_add_hamburgermenu_to_toolbar.sh
%{_kf5_datadir}/kglobalaccel/org.kde.%{base_name}.desktop
%{_kf5_datadir}/kio/servicemenus/konsolerun.desktop
%{_kf5_datadir}/knotifications5/konsole.notifyrc
%{_kf5_datadir}/knsrcfiles/*.knsrc
%{_kf5_datadir}/kservicetypes5/terminalemulator.desktop
%{_kf5_datadir}/qlogging-categories5/*.categories
%{_kf5_datadir}/zsh/site-functions/_konsole

%ldconfig_scriptlets part

%files part
%license COPYING*
%config(noreplace) %{_kf5_sysconfdir}/xdg/konsolerc
%{_kf5_datadir}/konsole/
%{_kf5_libdir}/libkonsoleapp.so.1
%{_kf5_libdir}/libkonsoleprivate.so.1
%{_kf5_libdir}/libkonsoleprivate.so.%{maj_ver}*
%{_kf5_qtplugindir}/konsolepart.so
%{_kf5_qtplugindir}/konsoleplugins/konsole_sshmanagerplugin.so
%{_kf5_qtplugindir}/konsoleplugins/konsole_quickcommandsplugin.so
%{_kf5_datadir}/kservices5/konsolepart.desktop


%changelog
* Fri Dec 08 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.08.4-1
- 23.08.4

* Fri Nov 10 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.08.3-1
- 23.08.3

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

* Thu Mar 02 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.12.3-1
- 22.12.3

* Thu Feb 02 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.12.2-1
- 22.12.2

