Name:    merkuro
Version: 23.08.4
Release: 1%{?dist}
Summary: A calendar application using Akonadi to sync with external services

License: LGPLv2+
URL:     https://invent.kde.org/pim/%{name}

%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0: https://download.kde.org/%{stable}/release-service/%{version}/src/%{name}-%{version}.tar.xz
Source1: https://download.kde.org/%{stable}/release-service/%{version}/src/%{name}-%{version}.tar.xz.sig
Source2: gpgkey-D81C0CB38EB725EF6691C385BB463350D6EF31EF.gpg

## upstream patches

## upstreamable patches

%{?qt5_qtwebengine_arches:ExclusiveArch: %{qt5_qtwebengine_arches}}

BuildRequires:  gnupg2
BuildRequires:  gettext
BuildRequires:  desktop-file-utils
BuildRequires:  libappstream-glib

%global qt5_min_version     5.15.2
%global kf5_min_version     5.86.0
%global akonadi_min_version 5.21.0

BuildRequires:  extra-cmake-modules       >= %{kf5_min_version}
BuildRequires:  kf5-rpm-macros            >= %{kf5_min_version}

BuildRequires:  cmake(Qt5Core)            >= %{qt5_min_version}
BuildRequires:  cmake(Qt5Gui)             >= %{qt5_min_version}
BuildRequires:  cmake(Qt5Qml)             >= %{qt5_min_version}
BuildRequires:  cmake(Qt5QuickControls2)  >= %{qt5_min_version}
BuildRequires:  cmake(Qt5Svg)             >= %{qt5_min_version}
BuildRequires:  cmake(Qt5Location)        >= %{qt5_min_version}

BuildRequires:  cmake(KF5CalendarCore)    >= %{kf5_min_version}
BuildRequires:  cmake(KF5Completion)      >= %{kf5_min_version}
BuildRequires:  cmake(KF5Config)          >= %{kf5_min_version}
BuildRequires:  cmake(KF5ConfigWidgets)   >= %{kf5_min_version}
BuildRequires:  cmake(KF5CoreAddons)      >= %{kf5_min_version}
BuildRequires:  cmake(KF5I18n)            >= %{kf5_min_version}
BuildRequires:  cmake(KF5ItemModels)      >= %{kf5_min_version}
BuildRequires:  cmake(KF5ItemViews)       >= %{kf5_min_version}
BuildRequires:  cmake(KF5JobWidgets)      >= %{kf5_min_version}
BuildRequires:  cmake(KF5KIO)             >= %{kf5_min_version}
BuildRequires:  cmake(KF5Kirigami2)       >= %{kf5_min_version}
BuildRequires:  cmake(KF5People)          >= %{kf5_min_version}
BuildRequires:  cmake(KF5Solid)           >= %{kf5_min_version}
BuildRequires:  cmake(KF5WindowSystem)    >= %{kf5_min_version}
BuildRequires:  cmake(KF5XmlGui)          >= %{kf5_min_version}
BuildRequires:  cmake(KF5QQC2DesktopStyle) >= %{kf5_min_version}
BuildRequires:  cmake(KF5Plasma)          >= %{kf5_min_version}
BuildRequires:  cmake(KF5KirigamiAddons)

BuildRequires:  cmake(KPim5Akonadi)       >= %{akonadi_min_version}
BuildRequires:  cmake(KPim5AkonadiContact) >= %{akonadi_min_version}
BuildRequires:  cmake(KF5CalendarSupport) >= %{akonadi_min_version}
BuildRequires:  cmake(KPim5EventViews)    >= %{akonadi_min_version}
BuildRequires:  cmake(KPim5GrantleeTheme) >= %{akonadi_min_version}
BuildRequires:  cmake(KF5MailCommon)      >= %{akonadi_min_version}
BuildRequires:  cmake(KPim5Libkdepim)       >= %{akonadi_min_version}
BuildRequires:  cmake(KPim5PimCommon)
BuildRequires:  cmake(KPim5PimCommonAkonadi)
BuildRequires:  cmake(KPim5AkonadiCalendar)

BuildRequires:  cmake(KF5TextAutoCorrectionCore)
BuildRequires:  cmake(Grantlee5)

Requires:       akonadi-calendar-tools
Requires:       kdepim-addons
Requires:       kdepim-runtime
Requires:       kf5-kirigami2
Requires:       kf5-kirigami2-addons
Requires:       kf5-kirigami2-addons-treeview
Requires:       hicolor-icon-theme

Provides:       kalendar-reminder-daemon = %{version}
Obsoletes:      kalendar-reminder-daemon < 1.0.0-2

Provides:       kalendar = %{version}-%{release}
Obsoletes:      kalendar < 23.08.0-1

%description
Kalendar is a Kirigami-based calendar application that uses Akonadi. It lets
you add, edit and delete events from local and remote accounts of your choice,
while keeping changes syncronised across your Plasma desktop or phone.


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
desktop-file-validate %{buildroot}%{_kf5_datadir}/applications/org.kde.%{name}.*.desktop
appstream-util validate-relax --nonet %{buildroot}%{_kf5_metainfodir}/org.kde.%{name}.*.xml ||:


%files -f %{name}.lang
%license LICENSES/*.txt
%doc README.md
%{_kf5_bindir}/%{name}-{calendar,contact,mail}
%{_qt5_qmldir}/org/kde/akonadi/
%{_qt5_qmldir}/org/kde/%{name}/
%{_kf5_datadir}/plasma/plasmoids/org.kde.%{name}.contact/
%{_kf5_datadir}/applications/org.kde.%{name}.{calendar,contact,mail}.desktop
%{_kf5_datadir}/icons/hicolor/*/apps/org.kde.%{name}.{calendar,contact,mail}.png
%{_kf5_datadir}/qlogging-categories5/*.categories
%{_kf5_metainfodir}/org.kde.%{name}.contact.appdata.xml
%{_kf5_metainfodir}/org.kde.%{name}.{calendar,contact,mail}.metainfo.xml


%changelog
* Fri Dec 08 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.08.4-1
- 23.08.4

* Fri Nov 10 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.08.3-1
- 23.08.3

* Sun Oct 15 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.08.2-5
- rebuilt

* Fri Oct 13 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.08.2-1
- 23.08.2

* Thu Sep 14 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.08.1-1
- 23.08.1

* Sun Aug 27 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.08.0-1
- 23.08.0

* Thu Jul 06 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.04.3-1
- 23.04.3

* Thu Jun 08 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.04.2-1
- 23.04.2

* Thu May 11 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.04.1-1
- 23.04.1

* Thu Apr 20 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.04.0-1
- 23.04.0

* Thu Mar 02 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.12.3-1
- 22.12.3

* Thu Feb 02 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.12.2-1
- 22.12.2

