%global qt5_min_version 5.15.2
%global kf5_min_version 5.98.0

Name:    plasma-remotecontrollers
Version: 5.26.1
Release: 1%{?dist}
Summary: Multimedia Player for playing local files on Plasma Bigscreen

License: BSD and GPLv2 and GPLv3
URL:     https://invent.kde.org/plasma-bigscreen/%{name}

%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0: https://download.kde.org/%{stable}/plasma/%{version}/%{name}-%{version}.tar.xz
Source1: https://download.kde.org/%{stable}/plasma/%{version}/%{name}-%{version}.tar.xz.sig
Source2: https://jriddell.org/esk-riddell.gpg

BuildRequires: cmake
BuildRequires: gcc-c++
BuildRequires: gettext
BuildRequires: kf5-rpm-macros
BuildRequires: systemd-rpm-macros
BuildRequires: extra-cmake-modules
BuildRequires: desktop-file-utils
BuildRequires: libappstream-glib

BuildRequires: cmake(Qt5Core)           >= %{qt5_min_version}
BuildRequires: cmake(Qt5DBus)           >= %{qt5_min_version}
BuildRequires: cmake(Qt5Gui)            >= %{qt5_min_version}
BuildRequires: cmake(Qt5WaylandClient)  >= %{qt5_min_version}

BuildRequires: cmake(KF5Config)         >= %{kf5_min_version}
BuildRequires: cmake(KF5Notifications)  >= %{kf5_min_version}
BuildRequires: cmake(KF5I18n)           >= %{kf5_min_version}
BuildRequires: cmake(KF5Solid)          >= %{kf5_min_version}
BuildRequires: cmake(KF5CoreAddons)     >= %{kf5_min_version}
BuildRequires: cmake(KF5Declarative)    >= %{kf5_min_version}
BuildRequires: cmake(KF5KCMUtils)       >= %{kf5_min_version}
BuildRequires: cmake(KF5Package)        >= %{kf5_min_version}
BuildRequires: cmake(KF5WindowSystem)   >= %{kf5_min_version}
BuildRequires: cmake(KF5ItemModels)     >= %{kf5_min_version}

BuildRequires: pkgconfig(libcec) >= 6
BuildRequires: pkgconfig(libevdev)
BuildRequires: cmake(LibTaskManager)

BuildRequires: pkgconfig(wayland-client)
BuildRequires: pkgconfig(wayland-scanner)
BuildRequires: cmake(PlasmaWaylandProtocols)

Requires:      systemd-udev

%description
%{summary}.


%prep
%autosetup


%build
sed -i 's|${KDE_INSTALL_LIBDIR}/udev/rules.d|${CMAKE_INSTALL_PREFIX}/lib/udev/rules.d|' CMakeLists.txt
%cmake_kf5
%cmake_build


%install
%cmake_install
%find_lang %{name} --with-qt --all-name


%check
desktop-file-validate %{buildroot}%{_kf5_datadir}/applications/org.kde.%{name}.desktop ||:
appstream-util validate-relax --nonet %{buildroot}%{_kf5_metainfodir}/org.kde.plasma.remotecontrollers.metainfo.xml


%files -f %{name}.lang
%doc %doc README.md
%license LICENSES/*.txt
%{_bindir}/%{name}
%{_qt5_qmldir}/org/kde/plasma/remotecontrollers/
%{_qt5_plugindir}/kcms/kcm_mediacenter_remotecontrollers.so
%{_kf5_sysconfdir}/xdg/autostart/org.kde.%{name}.desktop
%{_kf5_sysconfdir}/xdg/%{name}rc
%{_kf5_datadir}/dbus-1/interfaces/org.kde.%{name}.*.xml
%{_kf5_datadir}/knotifications5/%{name}.notifyrc
%{_kf5_datadir}/kpackage/kcms/kcm_mediacenter_remotecontrollers/
%{_kf5_datadir}/kservices5/kcm_mediacenter_remotecontrollers.desktop
%{_kf5_datadir}/applications/org.kde.%{name}.desktop
%{_kf5_metainfodir}/org.kde.plasma.remotecontrollers.metainfo.xml
%{_kf5_datadir}/qlogging-categories5/%{name}.categories
%{_udevrulesdir}/40-uinput.rules


%changelog
* Tue Oct 18 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.26.1-1
- 5.26.1

* Wed Oct 12 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.26.0-1
- 5.26.0

* Mon Sep 19 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.25.90-1
- first spec for version 5.25.90

