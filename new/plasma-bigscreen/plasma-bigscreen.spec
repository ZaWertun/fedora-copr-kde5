%global qt5_min_version 5.15.2
%global kf5_min_version 5.98.0

Name:    plasma-bigscreen
Version: 5.25.90
Release: 1%{?dist}
Summary: A big launcher giving you easy access to any installed apps and skills

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

BuildRequires: gnupg2
BuildRequires: cmake
BuildRequires: gcc-c++
BuildRequires: gettext
BuildRequires: kf5-rpm-macros
BuildRequires: extra-cmake-modules
BuildRequires: desktop-file-utils
BuildRequires: libappstream-glib

BuildRequires: cmake(Qt5Quick)           >= %{qt5_min_version}
BuildRequires: cmake(Qt5Core)            >= %{qt5_min_version}
BuildRequires: cmake(Qt5Qml)             >= %{qt5_min_version}
BuildRequires: cmake(Qt5DBus)            >= %{qt5_min_version}
BuildRequires: cmake(Qt5Network)         >= %{qt5_min_version}
BuildRequires: cmake(Qt5Multimedia)      >= %{qt5_min_version}

BuildRequires: cmake(KF5Activities)      >= %{kf5_min_version}
BuildRequires: cmake(KF5ActivitiesStats) >= %{kf5_min_version}
BuildRequires: cmake(KF5Plasma)          >= %{kf5_min_version}
BuildRequires: cmake(KF5I18n)            >= %{kf5_min_version}
BuildRequires: cmake(KF5Kirigami2)       >= %{kf5_min_version}
BuildRequires: cmake(KF5Declarative)     >= %{kf5_min_version}
BuildRequires: cmake(KF5KCMUtils)        >= %{kf5_min_version}
BuildRequires: cmake(KF5Notifications)   >= %{kf5_min_version}
BuildRequires: cmake(KF5PlasmaQuick)     >= %{kf5_min_version}
BuildRequires: cmake(KF5KIO)             >= %{kf5_min_version}
BuildRequires: cmake(KF5Wayland)         >= %{kf5_min_version}
BuildRequires: cmake(KF5WindowSystem)    >= %{kf5_min_version}

BuildRequires: pkgconfig(libcec) >= 6
BuildRequires: pkgconfig(libevdev)
BuildRequires: cmake(LibTaskManager)

BuildRequires: pkgconfig(wayland-client)
BuildRequires: pkgconfig(wayland-scanner)
BuildRequires: cmake(PlasmaWaylandProtocols)

Requires:      kf5-plasma%{?_isa}
Requires:      aura-browser%{?_isa}
Requires:      plank-player%{?_isa}
Requires:      plasma-remotecontrollers%{?_isa}

%description
A big launcher giving you easy access to any installed apps and skills.
Controllable via voice or TV remote. This project is using various open-source
components like Plasma Bigscreen, Mycroft AI and libcec.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup


%build
%cmake_kf5
%cmake_build


%install
%cmake_install
%find_lang %{name} --with-qt --all-name


%check
find %{buildroot}%{_kf5_datadir} -name '*.desktop' -exec desktop-file-validate {} \;
find %{buildroot}%{_kf5_metainfodir} -name '*.xml' -exec appstream-util validate-relax --nonet {} \;


%files -f %{name}.lang
%doc %doc README.md
%license LICENSES/*.txt
%{_bindir}/%{name}-x11
%{_bindir}/%{name}-wayland
%{_bindir}/mycroft-skill-launcher.py
%{_kf5_qmldir}/org/kde/mycroft/bigscreen/
%{_qt5_plugindir}/kcms/kcm_mediacenter_*.so
%{_qt5_plugindir}/plasma/applets/plasma_containment_biglauncherhomescreen.so
%{_kf5_datadir}/sounds/%{name}/
%{_kf5_datadir}/kservices5/*.desktop
%{_kf5_datadir}/kpackage/kcms/kcm_mediacenter_*/
%{_kf5_datadir}/kpackage/genericqml/org.kde.plasma.settings/
%{_kf5_datadir}/plasma/shells/org.kde.plasma.mycroft.bigscreen/
%{_kf5_datadir}/plasma/look-and-feel/org.kde.plasma.mycroft.bigscreen/
%{_kf5_datadir}/plasma/plasmoids/org.kde.mycroft.bigscreen.homescreen/
%{_kf5_datadir}/xsessions/%{name}-x11.desktop
%{_kf5_datadir}/wayland-sessions/%{name}-wayland.desktop
%{_kf5_metainfodir}/org.kde.*.xml


%changelog
* Mon Sep 19 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.25.90-1
- first spec for version 5.25.90

