%global min_qt_version     5.14.0
%global min_kf_version     5.66.0

Name:    plasma-systemmonitor
Summary: New version of KSysGuard built on top of the new KStats daemon
Version: 5.21.0
Release: 1%{?dist}

License: GPLv2+
URL:     https://invent.kde.org/plasma/%{name}

Source0: https://download.kde.org/stable/plasma/%{version}/%{name}-%{version}.tar.xz

BuildRequires:  kf5-rpm-macros
BuildRequires:  desktop-file-utils
BuildRequires:  extra-cmake-modules

BuildRequires:  qt5-qtbase-devel      >= %{min_qt_version}
BuildRequires:  cmake(Qt5DBus)        >= %{min_qt_version}
BuildRequires:  cmake(Qt5Quick)       >= %{min_qt_version}
BuildRequires:  cmake(Qt5Widgets)     >= %{min_qt_version}

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

BuildRequires:  cmake(KSysGuard)


%description
New version of KSysGuard built on top of the new KStats daemon and using QML
for its UI.


%prep
%autosetup -p1


%build
%cmake_kf5
%cmake_build


%install
%cmake_install

%find_lang %{name} --all-name


%check
desktop-file-validate %{buildroot}%{_kf5_datadir}/applications/org.kde.%{name}.desktop


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
%{_kf5_datadir}/applications/org.kde.%{name}.desktop


%changelog
* Tue Feb 16 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.21.0-1
- 5.21.0

* Tue Nov  3 21:30:40 MSK 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.20.0-1
- first spec for version 5.20.0

