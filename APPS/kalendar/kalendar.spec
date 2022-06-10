Name:    kalendar
Version: 22.04.2
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

## upstream patches

## upstreamable patches

%{?qt5_qtwebengine_arches:ExclusiveArch: %{qt5_qtwebengine_arches}}

BuildRequires:  gettext
BuildRequires:  desktop-file-utils
BuildRequires:  libappstream-glib

%global qt5_min_version     5.15.2
%global kf5_min_version     5.86.0
%global akonadi_min_version 5.18.0

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
BuildRequires:  cmake(KF5Contacts)        >= %{kf5_min_version}
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

BuildRequires:  cmake(KF5Akonadi)         >= %{akonadi_min_version}
BuildRequires:  cmake(KF5AkonadiContact)  >= %{akonadi_min_version}
BuildRequires:  cmake(KF5CalendarSupport) >= %{akonadi_min_version}
BuildRequires:  cmake(KF5EventViews)      >= %{akonadi_min_version}
BuildRequires:  cmake(KF5GrantleeTheme)   >= %{akonadi_min_version}

BuildRequires:  cmake(Grantlee5)

Requires:       akonadi-calendar-tools
Requires:       kdepim-addons
Requires:       kdepim-runtime
Requires:       kf5-kirigami2
Requires:       kf5-kirigami2-addons
Requires:       kf5-kirigami2-addons-treeview

Requires:       hicolor-icon-theme

Provides:       %{name}-reminder-daemon = %{version}
Obsoletes:      %{name}-reminder-daemon < 1.0.0-2

%description
Kalendar is a Kirigami-based calendar application that uses Akonadi. It lets
you add, edit and delete events from local and remote accounts of your choice,
while keeping changes syncronised across your Plasma desktop or phone.


%prep
%autosetup -p1


%build
%cmake_kf5
%cmake_build


%install
%cmake_install
%find_lang %{name}


%check
desktop-file-validate %{buildroot}%{_kf5_datadir}/applications/org.kde.%{name}.desktop ||:
appstream-util validate-relax --nonet %{buildroot}%{_kf5_metainfodir}/org.kde.%{name}.appdata.xml


%files -f %{name}.lang
%license LICENSES/*.txt
%doc README.md
%{_kf5_bindir}/%{name}
%{_kf5_datadir}/applications/org.kde.%{name}.desktop
%{_kf5_datadir}/icons/hicolor/scalable/apps/org.kde.%{name}.svg
%{_kf5_datadir}/qlogging-categories5/%{name}.categories
%{_kf5_metainfodir}/org.kde.%{name}.appdata.xml


%changelog
* Thu Jun 09 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.04.2-1
- 22.04.2

* Thu May 12 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.04.1-1
- 22.04.1

* Thu Apr 21 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.04.0-1
- 22.04.0

* Sun Feb 27 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 1.0.0-2
- package kalendar-reminder-daemon obsolete

* Sun Feb 13 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 1.0.0-1
- 1.0.0

* Sat Dec 11 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 0.3.1-2
- rebuild

* Thu Dec 02 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 0.3.1-1
- version 0.3.1

* Wed Dec 01 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 0.3.0-1
- version 0.3.0, commit cbae6952de88034b148ce449f06083eace3521b5

* Sun Nov 28 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 0.2.1-3.20211128git0cbfd47
- 2021-11-28, commit 0cbfd4745767a8ff5165e2924344ea7f4bf6a102

* Sun Nov 21 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 0.2.1-2.20211121git79116ec
- 2021-11-21, commit 79116ec8b5bc7f5738ec9f2ced73793057f8a73e

* Mon Nov 15 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 0.2.1-1.20211115gitb8ced3f
- 2021-11-15, commit b8ced3f4f4accd51c3e3d24b5ce9277ee22e0d37

* Sat Nov 13 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 0.2.0-1.20211113git66c6563
- 2021-11-13, commit 66c65630bc61192615e06f118cab3861728e6d8b

* Wed Nov 10 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 0.1-5.20211110git9f24a9a
- 2021-11-10, commit 9f24a9a7070b5740fa554c40f36ab0d36a22fbe9

* Tue Nov 09 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 0.1-4.20211108gitab0b5bd
- 2021-11-08, commit ab0b5bd9eb75c4b86083a79190afc7bcfc76e4fe

* Sat Nov 06 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 0.1-3.20211106git1ec5c7f
- 2021-11-06, commit 1ec5c7ff82ad24ca70c7f7a55993daa00663dad6

* Sun Oct 24 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 0.1-2.20211021git20b34f7
- 2021-10-24, commit ba5a1306970c3ef90a94694dada81c85b61f80f1

* Fri Oct 22 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 0.1-2.20211021git20b34f7
- first spec for version 0.1 (2021-10-21, commit 20b34f7)
