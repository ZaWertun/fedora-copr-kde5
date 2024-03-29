%global min_qt_version 5.14.0
%global min_kf_version 5.66.0

Name:    plasma-systemmonitor
Summary: New version of KSysGuard built on top of the new KStats daemon
Version: 5.27.11
Release: 1%{?dist}

License: GPLv2+
URL:     https://invent.kde.org/plasma/%{name}

Source0: https://download.kde.org/stable/plasma/%{version}/%{name}-%{version}.tar.xz
Source1: https://download.kde.org/stable/plasma/%{version}/%{name}-%{version}.tar.xz.sig
Source2: https://jriddell.org/esk-riddell.gpg

BuildRequires:  gnupg2
BuildRequires:  kf5-rpm-macros
BuildRequires:  desktop-file-utils
BuildRequires:  extra-cmake-modules

BuildRequires:  qt5-qtbase-devel      >= %{min_qt_version}
BuildRequires:  cmake(Qt5DBus)        >= %{min_qt_version}
BuildRequires:  cmake(Qt5Quick)       >= %{min_qt_version}
BuildRequires:  cmake(Qt5Widgets)     >= %{min_qt_version}
BuildRequires:  cmake(Qt5QuickControls2) >= %{min_qt_version}

BuildRequires:  cmake(KF5KIO)         >= %{min_kf_version}
BuildRequires:  cmake(KF5I18n)        >= %{min_kf_version}
BuildRequires:  cmake(KF5Config)      >= %{min_kf_version}
BuildRequires:  cmake(KF5Service)     >= %{min_kf_version}
BuildRequires:  cmake(KF5NewStuff)    >= %{min_kf_version}
BuildRequires:  cmake(KF5DBusAddons)  >= %{min_kf_version}
BuildRequires:  cmake(KF5IconThemes)  >= %{min_kf_version}
BuildRequires:  cmake(KF5ItemModels)  >= %{min_kf_version}
BuildRequires:  cmake(KF5Declarative) >= %{min_kf_version}
BuildRequires:  cmake(KF5GlobalAccel) >= %{min_kf_version}
BuildRequires:  cmake(KF5Kirigami2)   >= %{min_kf_version}

BuildRequires:  cmake(KSysGuard)


%description
New version of KSysGuard built on top of the new KStats daemon and using QML
for its UI.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -p1


%build
%cmake_kf5
%cmake_build


%install
%cmake_install
%find_lang %{name} --all-name


%check
desktop-file-validate %{buildroot}%{_kf5_datadir}/applications/org.kde.%{name}.desktop ||:
desktop-file-validate %{buildroot}%{_kf5_datadir}/plasma/kinfocenter/externalmodules/kcm_external_plasma-systemmonitor.desktop ||:


%files -f %{name}.lang
%doc README.md
%license LICENSES/*.txt
%{_kf5_bindir}/%{name}
%{_kf5_qmldir}/org/kde/ksysguard/page/*.qml
%{_kf5_qmldir}/org/kde/ksysguard/page/qmldir
%{_kf5_qmldir}/org/kde/ksysguard/page/libPagePlugin.so
%{_kf5_qmldir}/org/kde/ksysguard/table/*.qml
%{_kf5_qmldir}/org/kde/ksysguard/table/qmldir
%{_kf5_qmldir}/org/kde/ksysguard/table/libTablePlugin.so
%{_kf5_datadir}/%{name}
%{_kf5_datadir}/ksysguard/sensorfaces/org.kde.ksysguard.applicationstable/contents/config/main.xml
%{_kf5_datadir}/ksysguard/sensorfaces/org.kde.ksysguard.applicationstable/contents/faceproperties
%{_kf5_datadir}/ksysguard/sensorfaces/org.kde.ksysguard.applicationstable/contents/ui/*.qml
%{_kf5_datadir}/ksysguard/sensorfaces/org.kde.ksysguard.applicationstable/metadata.{desktop,json}
%{_kf5_datadir}/ksysguard/sensorfaces/org.kde.ksysguard.processtable/contents/config/main.xml
%{_kf5_datadir}/ksysguard/sensorfaces/org.kde.ksysguard.processtable/contents/faceproperties
%{_kf5_datadir}/ksysguard/sensorfaces/org.kde.ksysguard.processtable/contents/ui/*.qml
%{_kf5_datadir}/ksysguard/sensorfaces/org.kde.ksysguard.processtable/metadata.{desktop,json}
%{_kf5_datadir}/knsrcfiles/%{name}.knsrc
%{_kf5_datadir}/config.kcfg/systemmonitor.kcfg
%{_kf5_datadir}/metainfo/org.kde.plasma-systemmonitor.metainfo.xml
%{_kf5_datadir}/applications/org.kde.%{name}.desktop
%{_kf5_datadir}/plasma/kinfocenter/externalmodules/kcm_external_plasma-systemmonitor.desktop


%changelog
* Wed Mar 06 2024 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.27.11-1
- 5.27.11

* Wed Dec 06 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.27.10-1
- 5.27.10

* Tue Oct 24 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.27.9-1
- 5.27.9

* Tue Sep 12 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.27.8-1
- 5.27.8

* Tue Aug 01 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.27.7-1
- 5.27.7

* Tue Jun 20 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.27.6-1
- 5.27.6

* Tue May 09 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.27.5-1
- 5.27.5

* Tue Apr 04 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.27.4-1
- 5.27.4

* Tue Mar 14 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.27.3-1
- 5.27.3

* Tue Feb 28 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.27.2-1
- 5.27.2

* Tue Feb 21 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.27.1-1
- 5.27.1

* Tue Feb 14 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.27.0-1
- 5.27.0

* Wed Jan 04 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.26.5-1
- 5.26.5

* Tue Nov 29 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.26.4-1
- 5.26.4

* Tue Nov 08 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.26.3-1
- 5.26.3

* Wed Oct 26 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.26.2-1
- 5.26.2

* Tue Oct 18 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.26.1-1
- 5.26.1

* Tue Oct 11 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.26.0-1
- 5.26.0

* Tue Sep 06 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.25.5-1
- 5.25.5

* Tue Aug 02 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.25.4-1
- 5.25.4

* Mon Aug 01 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.25.3-2
- BR: Qt5QuickControls2, KF5Kirigami2

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

* Tue Feb 23 13:50:09 MSK 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.21.1-1
- 5.21.1

* Tue Feb 16 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.21.0-1
- 5.21.0

* Tue Nov  3 21:30:40 MSK 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.20.0-1
- first spec for version 5.20.0

