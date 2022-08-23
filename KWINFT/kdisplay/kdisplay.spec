%global min_qt_version     5.14.0
%global min_kf_version     5.66.0
%global min_disman_version 0.522.0

Name:    kdisplay
Version: 5.25.0
Release: 2%{?dist}
Summary: App and daemon for display managing

Conflicts: kdisplay

%global  real_version %(echo %{version} |sed 's/~/-/')
License: GPLv2
URL:     https://gitlab.com/kwinft/%{name}
Source0: %{url}/-/archive/%{name}@%{real_version}/%{name}-%{name}@%{real_version}.tar.bz2

BuildRequires:  extra-cmake-modules
BuildRequires:  kf5-rpm-macros
BuildRequires:  desktop-file-utils

BuildRequires:  qt5-qtbase-devel  >= %{min_qt_version}
BuildRequires:  cmake(Qt5Test)    >= %{min_qt_version}
BuildRequires:  cmake(Qt5Sensors) >= %{min_qt_version}

BuildRequires:  cmake(KF5Config)      >= %{min_kf_version}
BuildRequires:  cmake(KF5DBusAddons)  >= %{min_kf_version}
BuildRequires:  cmake(KF5Declarative) >= %{min_kf_version}
BuildRequires:  cmake(KF5GlobalAccel) >= %{min_kf_version}
BuildRequires:  cmake(KF5I18n)        >= %{min_kf_version}
BuildRequires:  cmake(KF5IconThemes)  >= %{min_kf_version}
BuildRequires:  cmake(KF5KCMUtils)    >= %{min_kf_version}
BuildRequires:  cmake(KF5Plasma)      >= %{min_kf_version}
BuildRequires:  cmake(KF5XmlGui)      >= %{min_kf_version}
BuildRequires:  cmake(KF5Kirigami2)   >= %{min_kf_version}

BuildRequires:  cmake(disman) >= %{min_disman_version}


%description
%{summary}.

%prep
%autosetup -p1 -n %{name}-%{name}@%{real_version}


%build
%cmake_kf5
%cmake_build


%install
%cmake_install
%find_lang %{name} --all-name


%check
desktop-file-validate %{buildroot}%{_kf5_datadir}/applications/org.kwinft.%{name}.desktop


%files -f %{name}.lang
%doc README.md
%license COPYING COPYING.LGPL
%{_bindir}/kdisplay
%{_kf5_datadir}/kded_kdisplay/qml/*.qml
%{_kf5_datadir}/kpackage/kcms/kcm_kdisplay/
%{_kf5_datadir}/kservices5/*.desktop
%{_kf5_datadir}/plasma/plasmoids/org.kwinft.kdisplay/
%{_kf5_datadir}/qlogging-categories5/kdisplay.categories
%{_kf5_metainfodir}/org.kwinft.kdisplay.appdata.xml
%{_kf5_qtplugindir}/kcm_kdisplay.so
%{_kf5_qtplugindir}/plasma/applets/plasma_applet_kdisplay.so
%{_kf5_plugindir}/kded/kdisplayd.so
%{_kf5_datadir}/applications/org.kwinft.%{name}.desktop


%changelog
* Tue Aug 23 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.25.0-2
- Conflicts: kdisplay

* Sat Jun 18 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.25.0-1
- 5.25.0

* Mon Apr 11 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.24.0-2
- rebuild

* Wed Feb 09 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.24.0-1
- 5.24.0

* Fri Oct 15 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.23.0-1
- 5.23.0

* Thu Jun 10 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 0.522.0-1
- 0.522.0

* Wed Feb 17 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.21.0-1
- 5.21.0

* Wed Oct 14 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.20.0-1
- version 5.20.0

* Tue Jun 16 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.19.0-1
- first spec for version 5.19.0

