Name:    spectacle
Summary: Screenshot capture utility
Version: 23.08.2
Release: 1%{?dist}

License: GPLv2
URL:     https://www.kde.org/applications/graphics/spectacle/

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

%global majmin %(echo %{version} | cut -d. -f1,2)

BuildRequires: gnupg2
BuildRequires: desktop-file-utils
BuildRequires: libappstream-glib
BuildRequires: systemd-rpm-macros

BuildRequires: extra-cmake-modules
BuildRequires: kf5-rpm-macros
BuildRequires: kf5-kcoreaddons-devel
BuildRequires: kf5-kdeclarative-devel
BuildRequires: kf5-kwidgetsaddons-devel
BuildRequires: kf5-kdbusaddons-devel
BuildRequires: kf5-knotifications-devel
BuildRequires: kf5-kconfig-devel
BuildRequires: kf5-ki18n-devel
BuildRequires: kf5-kio-devel
BuildRequires: kf5-kxmlgui-devel
BuildRequires: kf5-kwindowsystem-devel
BuildRequires: kf5-kdoctools-devel
BuildRequires: kf5-kwayland-devel

BuildRequires: cmake(KF5NewStuff)
BuildRequires: cmake(KF5Screen)
BuildRequires: cmake(KF5GlobalAccel)
BuildRequires: cmake(KF5Kipi)
BuildRequires: cmake(KF5GuiAddons)
BuildRequires: cmake(KF5Kirigami2)
BuildRequires: kf5-libkipi-devel

BuildRequires: cmake(Qt5DBus)
BuildRequires: cmake(Qt5PrintSupport)
BuildRequires: cmake(Qt5Quick)
BuildRequires: cmake(Qt5X11Extras)
BuildRequires: cmake(Qt5Svg)
BuildRequires: cmake(Qt5QuickControls2)
BuildRequires: qt5-qtbase-private-devel

BuildRequires: pkgconfig(xcb-cursor)
BuildRequires: pkgconfig(xcb-image)
BuildRequires: pkgconfig(xcb-util)
BuildRequires: pkgconfig(xcb-xfixes)

BuildRequires: cmake(KDEExperimentalPurpose)

BuildRequires: kimageannotator-devel
BuildRequires: kcolorpicker-devel

BuildRequires: pkgconfig(wayland-client)
BuildRequires: cmake(PlasmaWaylandProtocols)
BuildRequires: cmake(Qt5WaylandClient)

BuildRequires: cmake(KPipeWire)
BuildRequires: pkgconfig(libpipewire-0.3)

# f26+ upgrade path
%if 0%{?fedora} > 25
Obsoletes: ksnapshot <= 15.08.3
%endif

# translations moved here
Conflicts: kde-l10n < 17.03

# For kconf_update/50-clipboard_settings_change.py
Requires:  python3

%description
%{summary}.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -p1


%build
%cmake_kf5
%cmake_build


%install
%cmake_install
%find_lang %{name} --all-name --with-html --with-man


%check
appstream-util validate-relax --nonet %{buildroot}%{_kf5_metainfodir}/org.kde.spectacle.appdata.xml ||:
desktop-file-validate %{buildroot}%{_kf5_datadir}/applications/org.kde.spectacle.desktop


%post
%systemd_user_post app-org.kde.%{name}.service


%preun
%systemd_user_preun app-org.kde.%{name}.service


%files -f %{name}.lang
%license LICENSES/*.txt
%{_kf5_bindir}/spectacle
%{_kf5_metainfodir}/org.kde.spectacle.appdata.xml
%{_kf5_libdir}/kconf_update_bin/spectacle-migrate-shortcuts
%{_kf5_libdir}/kconf_update_bin/spectacle-migrate-rememberregion
%{_kf5_datadir}/kconf_update/spectacle_rememberregion.upd
%{_kf5_datadir}/applications/org.kde.spectacle.desktop
%{_kf5_datadir}/dbus-1/interfaces/org.kde.Spectacle.xml
%{_kf5_datadir}/dbus-1/services/org.kde.Spectacle.service
%{_kf5_datadir}/icons/hicolor/*/apps/spectacle.*
%{_kf5_datadir}/knotifications5/spectacle.notifyrc
%{_kf5_datadir}/kconf_update/spectacle_shortcuts.upd
%{_kf5_datadir}/kconf_update/spectacle_newConfig.upd
%{_kf5_datadir}/kconf_update/spectacle_clipboard.upd
%{_kf5_datadir}/kconf_update/50-clipboard_settings_change.py
%{_kf5_datadir}/kglobalaccel/*.desktop
%{_kf5_datadir}/qlogging-categories5/*.categories
%{_userunitdir}/app-org.kde.%{name}.service
%{_mandir}/man1/%{name}.1*


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

* Thu Mar 02 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.12.3-1
- 22.12.3

* Thu Feb 02 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.12.2-1
- 22.12.2

