Name:    ksystemstats
Version: 5.22.5
Release: 1%{?dist}
Summary: KSystemStats is a daemon that collects statistics about the running system.
 
# KDE e.V. may determine that future GPL versions are accepted
License: GPLv2 or GPLv3
URL:     https://invent.kde.org/plasma/%{name}
 
%global majmin %(echo %{version} | cut -d. -f1-2)
%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif

Source0: http://download.kde.org/%{stable}/plasma/%{version}/%{name}-%{version}.tar.xz
 
BuildRequires: desktop-file-utils
BuildRequires: libappstream-glib
BuildRequires: libksysguard-devel >= %{majmin}
 
BuildRequires: kf5-rpm-macros
BuildRequires: extra-cmake-modules >= 5.82
BuildRequires: cmake(KF5Config)
BuildRequires: cmake(KF5CoreAddons)
BuildRequires: cmake(KF5DBusAddons)
BuildRequires: cmake(KF5DocTools)
BuildRequires: cmake(KF5I18n)
BuildRequires: cmake(KF5IconThemes)
BuildRequires: cmake(KF5ItemViews)
BuildRequires: cmake(KF5KIO)
BuildRequires: cmake(KF5NewStuff)
BuildRequires: cmake(KF5Notifications)
BuildRequires: cmake(KF5Solid)
BuildRequires: cmake(KF5WindowSystem)
 
BuildRequires: cmake(KF5NetworkManagerQt)
 
BuildRequires: cmake(Qt5Widgets)
 
BuildRequires:  libnl3-devel
BuildRequires:  lm_sensors-devel
BuildRequires:  systemd-devel
BuildRequires:  pkgconfig(libpcap)
 
%description
KSystemStats is a daemon that collects statistics about the running system.
 
%package devel
Summary:  Developer files for %{name}
Requires: %{name}%{?_isa} = %{version}-%{release}
%description devel
%{summary}.
 
 
%prep
%autosetup -n %{name}-%{version}


%build
%cmake_kf5

%cmake_build


%install
%cmake_install
%find_lang ksystemstats_plugins


%files -f ksystemstats_plugins.lang
%doc README.md
%license LICENSES/*
%{_kf5_bindir}/ksystemstats
%{_kf5_bindir}/kstatsviewer
%{_datadir}/dbus-1/services/org.kde.ksystemstats.service
%{_userunitdir}/plasma-ksystemstats.service
%{_qt5_plugindir}/ksystemstats/


%changelog
* Tue Aug 31 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.22.5-1
- 5.22.5

* Tue Jul 27 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.22.4-1
- 5.22.4

* Thu Jul 08 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.22.3-1
- 5.22.3

* Wed Jun 23 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.22.2.1-1
- 5.22.2.1

* Tue Jun 22 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.22.2-1
- 5.22.2

* Tue Jun 15 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.22.1-1
- 5.22.1

* Tue Jun 08 2021 Jan Grulich <jgrulich@redhat.com> - 5.22.0-1
- 5.22.0

* Tue May 18 2021 Rex Dieter <rdieter@fedoraproject.org> - 5.21.90-2
- and comment about licensing
- .spec cosmetics

* Tue May 18 2021 Onuralp SEZER <thunderbirdtr@fedoraproject.org> - 5.21.90-1
- Initial Package
