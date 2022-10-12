%global qt5_min_version 5.15.2
%global kf5_min_version 5.98.0

Name:    aura-browser
Version: 5.26.0
Release: 1%{?dist}
Summary: Multimedia Player for playing local files on Plasma Bigscreen

License: BSD and GPLv2+ and GPLv3+ and LGPLv2+ and LGPLv3 and MPLv2.0
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
BuildRequires: cmake(Qt5WebEngine)      >= %{qt5_min_version}
# Test
BuildRequires: cmake(Qt5Widgets)        >= %{qt5_min_version}
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
desktop-file-validate %{buildroot}%{_kf5_datadir}/applications/org.aura.browser.desktop ||:
appstream-util validate-relax --nonet %{buildroot}%{_kf5_metainfodir}/org.kde.invent.aura_browser.metainfo.xml


%files -f %{name}.lang
%doc %doc README.md
%license LICENSES/*.txt
%{_bindir}/%{name}
%{_kf5_datadir}/icons/hicolor/*/apps/%{name}.png
%{_kf5_datadir}/applications/org.aura.browser.desktop
%{_kf5_metainfodir}/org.kde.invent.aura_browser.metainfo.xml


%changelog
* Wed Oct 12 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.26.0-1
- 5.26.0

* Mon Sep 19 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.25.90-1
- first spec for version 5.25.90

