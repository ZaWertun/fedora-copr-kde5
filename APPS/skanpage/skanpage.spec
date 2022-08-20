Name:           skanpage
Version:        22.08.0
Release:        1%{?dist}
Summary:        Utility to scan images and multi-page documents
License:        BSD and GPLv2 and GPLv3
URL:            https://www.kde.org/applications/graphics/%{name}/
Source0:        https://invent.kde.org/utilities/%{name}/-/archive/v%{version}/%{name}-v%{version}.tar.bz2

BuildRequires:  libappstream-glib
BuildRequires:  desktop-file-utils
BuildRequires:  extra-cmake-modules

BuildRequires:  cmake
BuildRequires:  gcc-c++

%global qt5_min_version 5.15.2
BuildRequires:  cmake(Qt5Qml)            >= %{qt5_min_version}
BuildRequires:  cmake(Qt5Core)           >= %{qt5_min_version}
BuildRequires:  cmake(Qt5Quick)          >= %{qt5_min_version}
BuildRequires:  cmake(Qt5Widgets)        >= %{qt5_min_version}
BuildRequires:  cmake(Qt5Concurrent)     >= %{qt5_min_version}
BuildRequires:  cmake(Qt5PrintSupport)   >= %{qt5_min_version}
BuildRequires:  cmake(Qt5QuickControls2) >= %{qt5_min_version}

%global kf5_min_version 5.91.0
BuildRequires:  cmake(KF5I18n)       >= %{kf5_min_version}
BuildRequires:  cmake(KF5Sane)       >= %{kf5_min_version}
BuildRequires:  cmake(KF5Crash)      >= %{kf5_min_version}
BuildRequires:  cmake(KF5Config)     >= %{kf5_min_version}
BuildRequires:  cmake(KF5Purpose)    >= %{kf5_min_version}
BuildRequires:  cmake(KF5Kirigami2)  >= %{kf5_min_version}
BuildRequires:  cmake(KF5CoreAddons) >= %{kf5_min_version}

BuildRequires:  cmake(KSaneCore)

BuildRequires:  pkgconfig(lept)
BuildRequires:  pkgconfig(tesseract)

Requires:       hicolor-icon-theme
Requires:       qt5-qtquickcontrols2
Requires:       kf5-kirigami2        >= %{kf5_min_version}

Recommends:     sane-backends-drivers-scanners

%description
Skanpage is a multi-page scanning application built using the libksane library
and a QML interface. It supports saving to image and PDF files.


%prep
%autosetup -p1 -n %{name}-v%{version}


%build
sed -i 's|set(CMAKE_CXX_FLAGS "-fopenmp")|set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")|' \
  CMakeLists.txt

%cmake_kf5
%cmake_build


%install
%cmake_install


%check
desktop-file-validate %{buildroot}%{_kf5_datadir}/applications/org.kde.%{name}.desktop
appstream-util validate-relax --nonet %{buildroot}%{_kf5_metainfodir}/org.kde.%{name}.appdata.xml


%files
%license LICENSES/*.txt
%{_bindir}/%{name}
%{_kf5_metainfodir}/org.kde.%{name}.appdata.xml
%{_kf5_datadir}/applications/org.kde.%{name}.desktop
%{_kf5_datadir}/icons/hicolor/*/apps/%{name}.{svg,png}
%{_kf5_datadir}/qlogging-categories5/%{name}.categories


%changelog
* Fri Aug 19 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.08.0-1
- 22.08.0

* Sun Mar 20 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.03.80-1
- first spec for version 22.03.80

