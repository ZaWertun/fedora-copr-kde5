%global qt5_min_version 5.15.2
%global kf5_min_version 5.98.0

Name:    plank-player
Version: 5.25.90
Release: 1%{?dist}
Summary: Multimedia Player for playing local files on Plasma Bigscreen

License: GPLv2+
URL:     https://invent.kde.org/plasma-bigscreen/%{name}
Source0: https://invent.kde.org/plasma-bigscreen/%{name}/-/archive/v%{version}/%{name}-v%{version}.tar.bz2

BuildRequires: cmake
BuildRequires: gcc-c++
BuildRequires: gettext
BuildRequires: kf5-rpm-macros
BuildRequires: extra-cmake-modules
BuildRequires: desktop-file-utils
BuildRequires: libappstream-glib

BuildRequires: cmake(Qt5Core)           >= %{qt5_min_version}
BuildRequires: cmake(Qt5Qml)            >= %{qt5_min_version}
BuildRequires: cmake(Qt5Quick)          >= %{qt5_min_version}
BuildRequires: cmake(Qt5QuickControls2) >= %{qt5_min_version}
BuildRequires: cmake(Qt5Multimedia)     >= %{qt5_min_version}

BuildRequires: cmake(KF5Kirigami2)      >= %{kf5_min_version}
BuildRequires: cmake(KF5I18n)           >= %{kf5_min_version}

Requires:      hicolor-icon-theme

%description
%{summary}.


%prep
%autosetup -n %{name}-v%{version}


%build
%cmake_kf5
%cmake_build


%install
%cmake_install
%find_lang %{name} || echo > %{name}.lang


%check
desktop-file-validate %{buildroot}%{_kf5_datadir}/applications/org.plank.player.desktop
appstream-util validate-relax --nonet %{buildroot}%{_kf5_metainfodir}/org.kde.invent.plank_player.metainfo.xml


%files -f %{name}.lang
%doc %doc README.md
%license LICENSES/*.txt
%{_bindir}/%{name}
%{_kf5_datadir}/icons/hicolor/*/apps/%{name}.png
%{_kf5_datadir}/applications/org.plank.player.desktop
%{_kf5_metainfodir}/org.kde.invent.plank_player.metainfo.xml


%changelog
* Mon Sep 19 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.25.90-1
- first spec for version 5.25.90

