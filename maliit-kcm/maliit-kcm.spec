%global date 20210326
%global commit 605f3bb62baba2d13ba2162c51f61d6389e9a7d7
%global shortcommit %(c=%{commit}; echo ${c:0:7})

%global qt5_min_version 5.15.0
%global kf5_min_version 5.80.0

Name:    maliit-kcm
Version: 0.1
Release: 1.%{date}git%{shortcommit}%{?dist}
Summary: Maliit KCM

License: BSD and GPLv2 and GPLv3
URL:     https://invent.kde.org/nicolasfella/%{name}
Source:  %{url}/-/archive/%{commit}/%{name}-%{shortcommit}.tar.gz

BuildRequires: cmake
BuildRequires: gcc-c++
BuildRequires: gettext
BuildRequires: kf5-rpm-macros
BuildRequires: extra-cmake-modules
BuildRequires: desktop-file-utils
BuildRequires: libappstream-glib

BuildRequires: cmake(Qt5Quick)       >= %{qt5_min_version}
BuildRequires: cmake(Qt5Core)        >= %{qt5_min_version}

BuildRequires: cmake(KF5CoreAddons)  >= %{kf5_min_version}
BuildRequires: cmake(KF5I18n)        >= %{kf5_min_version}
BuildRequires: cmake(KF5Declarative) >= %{kf5_min_version}

BuildRequires: pkgconfig(gio-2.0)

Requires:      plasma-settings%{?_isa}

%description
%{summary}.


%prep
%autosetup -n %{name}-%{commit}


%build
%cmake_kf5
%cmake_build


%install
%cmake_install
%find_lang %{name} --with-qt --all-name || echo > %{name}.lang


%files -f %{name}.lang
%license LICENSES/*.txt
%{_kf5_qtplugindir}/kcms/kcm_maliit.so
%{_kf5_datadir}/kpackage/kcms/kcm_maliit/
%{_kf5_datadir}/kservices5/kcm_maliit.desktop
