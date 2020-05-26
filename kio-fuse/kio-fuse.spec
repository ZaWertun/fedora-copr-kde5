%global         min_qt_version 5.12
%global         min_kf_version 5.66

Name:           kio-fuse
Version:        4.95.0
Release:        1%{?dist}
Summary:        KIO FUSE

Group:          System Environment/Libraries
License:        GPLv3+
URL:            https://invent.kde.org/system/kio-fuse
Source0:        %{url}/-/archive/v%{version}/%{name}-v%{version}.tar.bz2

BuildRequires:  kf5-rpm-macros
BuildRequires:  extra-cmake-modules  >= %{min_kf_version}

BuildRequires:  pkgconfig(fuse3)

BuildRequires:  cmake(Qt5Gui)        >= %{min_qt_version}
BuildRequires:  cmake(Qt5Core)       >= %{min_qt_version}

BuildRequires:  cmake(KF5KIO)        >= %{min_kf_version}
BuildRequires:  cmake(KF5Auth)       >= %{min_kf_version}
BuildRequires:  cmake(KF5Solid)      >= %{min_kf_version}
BuildRequires:  cmake(KF5Codecs)     >= %{min_kf_version}
BuildRequires:  cmake(KF5Service)    >= %{min_kf_version}
BuildRequires:  cmake(KF5CoreAddons) >= %{min_kf_version}
BuildRequires:  cmake(KF5DBusAddons) >= %{min_kf_version}
BuildRequires:  cmake(KF5JobWidgets) >= %{min_kf_version}

%description
KioFuse works by acting as a bridge between KDE's KIO filesystem design and FUSE.


%prep
%autosetup -p1 -n %{name}-v%{version}


%build
mkdir %{_target_platform}
pushd %{_target_platform}
%{cmake_kf5} ..
popd

%make_build -C %{_target_platform}


%install
make install/fast DESTDIR=%{buildroot} -C %{_target_platform}


%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig


%files
%license LICENSES/GPL-3.0-or-later.txt
%doc README
%{_libexecdir}/kio-fuse
%{_kf5_datadir}/dbus-1/services/org.kde.KIOFuse.service
%{_tmpfilesdir}/%{name}-tmpfiles.conf


%changelog
* Tue May 26 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 4.95.0-1
- first spec for version 4.95.0

