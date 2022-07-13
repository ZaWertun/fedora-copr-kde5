%global min_qt_version 5.15.0
%global min_kf_version 5.74.0

Name:    plasma-disks
Summary: Monitors S.M.A.R.T. capable devices for imminent failure
Version: 5.25.3
Release: 1%{?dist}

License: GPLv2+
URL:     https://cgit.kde.org/%{base_name}.git

%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0:        http://download.kde.org/%{stable}/plasma/%(echo %{version} |cut -d. -f1-3)/%{name}-%{version}.tar.xz

BuildRequires:  extra-cmake-modules
BuildRequires:  kf5-rpm-macros

BuildRequires:  cmake(Qt5Gui)  >= %{min_qt_version}
BuildRequires:  cmake(Qt5Core) >= %{min_qt_version}

BuildRequires:  cmake(KF5CoreAddons)    >= %{min_kf_version}
BuildRequires:  cmake(KF5DBusAddons)    >= %{min_kf_version}
BuildRequires:  cmake(KF5Notifications) >= %{min_kf_version}
BuildRequires:  cmake(KF5I18n)          >= %{min_kf_version}
BuildRequires:  cmake(KF5Solid)         >= %{min_kf_version}
BuildRequires:  cmake(KF5Service)       >= %{min_kf_version}
BuildRequires:  cmake(KF5KIO)           >= %{min_kf_version}
BuildRequires:  cmake(KF5Auth)          >= %{min_kf_version}
BuildRequires:  cmake(KF5Declarative)   >= %{min_kf_version}

Requires:       kf5-filesystem

%description
%{summary}.


%prep
%autosetup -p1


%build
%cmake_kf5
%cmake_build


%install
%cmake_install
%find_lang %{name} --all-name


%files -f %{name}.lang
%license LICENSES/*.txt
%{_qt5_plugindir}/plasma/kcms/kinfocenter/smart.so
%{_kf5_plugindir}/kded/smart.so
%{_kf5_libexecdir}/kauth/kded-smart-helper
%{_kf5_datadir}/dbus-1/system-services/org.kde.kded.smart.service
%{_kf5_datadir}/dbus-1/system.d/org.kde.kded.smart.conf
%{_kf5_datadir}/knotifications5/org.kde.kded.smart.notifyrc
%{_kf5_datadir}/kpackage/kcms/plasma_disks/contents/main.qml
%{_kf5_datadir}/kpackage/kcms/plasma_disks/contents/ui/ReportPage.qml
%{_kf5_datadir}/kpackage/kcms/plasma_disks/metadata.{desktop,json}
%{_kf5_datadir}/metainfo/org.kde.plasma.disks.metainfo.xml
%{_kf5_datadir}/polkit-1/actions/org.kde.kded.smart.policy


%changelog
* Wed Jul 13 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.25.3-1
- 5.25.3

* Tue Jun 28 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.25.2-1
- 5.25.2

* Tue Jun 21 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.25.1-1
- 5.25.1

* Tue Jun 14 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.25.0-1
- 5.25.0

* Tue May 03 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.24.5-1
- 5.24.5

* Tue Mar 29 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.24.4-1
- 5.24.4

* Tue Mar 08 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.24.3-1
- 5.24.3

* Tue Feb 22 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.24.2-1
- 5.24.2

* Tue Feb 15 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.24.1-1
- 5.24.1

* Tue Feb 08 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.24.0-1
- 5.24.0

* Tue Jan 04 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.23.5-1
- 5.23.5

* Tue Nov 30 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.23.4-1
- 5.23.4

* Tue Nov 09 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.23.3-1
- 5.23.3

* Tue Oct 26 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.23.2-1
- 5.23.2

* Tue Oct 19 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.23.1-1
- 5.23.1

* Thu Oct 14 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.23.0-1
- 5.23.0

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

* Tue Jun 08 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.22.0-1
- 5.22.0

* Tue May 04 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.21.5-1
- 5.21.5

* Tue Apr 06 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.21.4-1
- 5.21.4

* Tue Mar 16 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.21.3-1
- 5.21.3

* Wed Mar 03 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.21.2-1
- 5.21.2

* Tue Feb 23 13:50:06 MSK 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.21.1-1
- 5.21.1

* Tue Feb 16 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.21.0-1
- 5.21.0

* Tue Jan  5 22:06:20 MSK 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.20.5-1
- 5.20.5

* Tue Dec  1 22:30:46 MSK 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.20.4-1
- 5.20.4

* Wed Nov 11 11:10:20 MSK 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.20.3-1
- 5.20.3

* Tue Oct 27 16:56:29 MSK 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.20.2-1
- 5.20.2

* Tue Oct 20 17:02:45 MSK 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.20.1-1
- 5.20.1

* Thu Oct 15 00:06:34 MSK 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.20.0-1
- first spec for version 5.20.0


