%global qt5_min_version 5.15.2
%global kf5_min_version 5.98.0

Name:    plank-player
Version: 5.26.3
Release: 1%{?dist}
Summary: Multimedia Player for playing local files on Plasma Bigscreen

License: GPLv2+
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
%autosetup


%build
%cmake_kf5
%cmake_build


%install
%cmake_install
%find_lang %{name} --with-qt --all-name


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
* Tue Nov 08 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.26.3-1
- 5.26.3

* Wed Oct 26 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.26.2-1
- 5.26.2

* Tue Oct 18 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.26.1-1
- 5.26.1

* Wed Oct 12 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.26.0-1
- 5.26.0

* Mon Sep 19 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.25.90-1
- first spec for version 5.25.90

