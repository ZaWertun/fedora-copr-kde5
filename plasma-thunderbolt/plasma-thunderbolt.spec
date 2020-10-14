%undefine __cmake_in_source_build
%global base_name    plasma-thunderbolt

Name:    plasma-thunderbolt
Summary: Plasma integration for controlling Thunderbolt devices
Version: 5.20.0
Release: 1%{?dist}

License: GPLv2+
URL:     https://cgit.kde.org/%{base_name}.git

%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0:        http://download.kde.org/%{stable}/plasma/%(echo %{version} |cut -d. -f1-3)/%{base_name}-%{version}.tar.xz

BuildRequires:  extra-cmake-modules
BuildRequires:  kf5-rpm-macros
BuildRequires:  cmake(KF5CoreAddons)
BuildRequires:  cmake(KF5KCMUtils)
BuildRequires:  cmake(KF5Declarative)
BuildRequires:  cmake(KF5I18n)
BuildRequires:  cmake(KF5DBusAddons)
BuildRequires:  cmake(KF5Notifications)

Requires:       kf5-filesystem
Requires:       bolt%{?_isa}

%description
%{summary}.


%prep
%autosetup -n %{base_name}-%{version} -p1


%build
%{cmake_kf5}

%cmake_build


%install
%cmake_install
%find_lang %{name} --all-name


%ldconfig_scriptlets

%files -f %{name}.lang
%license COPYING
%{_kf5_libdir}/libkbolt.so
%{_kf5_qtplugindir}/kcms/kcm_bolt.so
%{_kf5_plugindir}/kded/kded_bolt.so
%{_kf5_datadir}/knotifications5/kded_bolt.notifyrc
%{_kf5_datadir}/kpackage/kcms/kcm_bolt/contents/ui/*.{qml,js}
%{_kf5_datadir}/kpackage/kcms/kcm_bolt/metadata.{desktop,json}
%{_kf5_datadir}/kservices5/kcm_bolt.desktop


%changelog
* Tue Oct 13 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.20.0-1
- 5.20.0

* Tue Sep 01 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.19.5-1
- 5.19.5

* Tue Jul 28 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.19.4-1
- 5.19.4

* Tue Jul 07 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.19.3-1
- 5.19.3

* Tue Jun 23 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.19.2-1
- 5.19.2

* Tue Jun 16 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.19.1-1
- 5.19.1

* Mon Jun 15 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.19.0-1
- 5.19.0

* Wed May 06 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.18.5-1
- 5.18.5

* Wed Apr 01 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.18.4.1-1
- 5.18.4.1

* Wed Mar 11 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.18.3-1
- 5.18.3

* Wed Feb 26 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.18.2-1
- 5.18.2

* Wed Feb 19 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.18.1-1
- 5.18.1

* Tue Feb 11 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.18.0-1
- 5.18.0

* Thu Jan 09 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.17.5-1
- 5.17.5

* Tue Dec 03 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.17.4-1
- 5.17.4

* Tue Nov 12 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.17.3-1
- 5.17.3

* Wed Oct 30 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.17.2-1
- 5.17.2

* Wed Oct 23 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.17.1-1
- 5.17.1

* Tue Oct 15 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.17.0-1
- 5.17.0

