%global real_name kup

Name:           kde-kup
Version:        0.9.1
Release:        1%{?dist}
Summary:        Backup scheduler for the Plasma desktop

License:        GPLv2+
URL:            https://invent.kde.org/system/%{real_name}
Source0:        https://invent.kde.org/system/%{real_name}/-/archive/%{real_name}-%{version}/%{real_name}-%{real_name}-%{version}.tar.bz2

## upstream patches

## upstreamable patches

BuildRequires: gettext
BuildRequires: desktop-file-utils
BuildRequires: libappstream-glib

BuildRequires: extra-cmake-modules
BuildRequires: kf5-rpm-macros

BuildRequires: cmake(Qt5Widgets)

BuildRequires: cmake(KF5Solid)
BuildRequires: cmake(KF5KIO)
BuildRequires: cmake(KF5IdleTime)
BuildRequires: cmake(KF5I18n)
BuildRequires: cmake(KF5Notifications)
BuildRequires: cmake(KF5CoreAddons)
BuildRequires: cmake(KF5DBusAddons)
BuildRequires: cmake(KF5Config)
BuildRequires: cmake(KF5Init)
BuildRequires: cmake(KF5JobWidgets)
BuildRequires: cmake(KF5Plasma)
BuildRequires: cmake(KF5WidgetsAddons)

BuildRequires: pkgconfig(libgit2)

Requires:      bup
Requires:      rsync
Requires:      hicolor-icon-theme

Requires:      kf5-kservice
Requires:      plasma-workspace
Requires:      plasma-workspace-libs

%description
%{summary}.


%prep
%autosetup -p1 -n %{real_name}-%{real_name}-%{version}


%build
%cmake_kf5
%cmake_build


%install
%cmake_install


%check
desktop-file-validate %{buildroot}%{_qt5_settingsdir}/autostart/%{real_name}-daemon.desktop
desktop-file-validate %{buildroot}%{_kf5_datadir}/kservices5/kcm_%{real_name}.desktop ||:
desktop-file-validate %{buildroot}%{_kf5_datadir}/kservices5/plasma-applet-org.kde.%{real_name}applet.desktop
desktop-file-validate %{buildroot}%{_kf5_datadir}/kservices5/plasma-dataengine-%{real_name}.desktop
appstream-util validate-relax --nonet %{buildroot}%{_kf5_metainfodir}/org.kde.%{real_name}.appdata.xml ||:
appstream-util validate-relax --nonet %{buildroot}%{_kf5_metainfodir}/org.kde.%{real_name}applet.appdata.xml ||:


%files
%license LICENSES/*.txt
%doc README.md
%{_kf5_bindir}/%{real_name}-daemon
%{_kf5_bindir}/%{real_name}-filedigger
%{_kf5_bindir}/%{real_name}-purger
%{_kf5_libdir}/libkdeinit5_%{real_name}-daemon.so
%{_kf5_qtplugindir}/kio_bup.so
%{_kf5_qtplugindir}/kcm_%{real_name}.so
%{_kf5_qtplugindir}/plasma/dataengine/plasma_engine_%{real_name}.so
%{_qt5_settingsdir}/autostart/%{real_name}-daemon.desktop
%{_kf5_datadir}/icons/hicolor/scalable/apps/%{real_name}.svg
%{_kf5_datadir}/knotifications5/%{real_name}daemon.notifyrc
%{_kf5_datadir}/kservices5/bup.protocol
%{_kf5_datadir}/kservices5/kcm_%{real_name}.desktop
%{_kf5_datadir}/kservices5/plasma-applet-org.kde.%{real_name}applet.desktop
%{_kf5_datadir}/kservices5/plasma-dataengine-%{real_name}.desktop
%{_kf5_datadir}/plasma/plasmoids/org.kde.kupapplet/
%{_kf5_datadir}/plasma/services/%{real_name}daemonservice.operations
%{_kf5_datadir}/plasma/services/%{real_name}service.operations
%{_kf5_metainfodir}/org.kde.%{real_name}.appdata.xml
%{_kf5_metainfodir}/org.kde.%{real_name}applet.appdata.xml
%{_kf5_datadir}/qlogging-categories5/%{real_name}.categories


%changelog
* Tue Dec 07 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 0.9.1-1
- first spec for version 0.9.1
