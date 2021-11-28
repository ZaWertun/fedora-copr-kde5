%global git_date     20211128
%global git_commit   0cbfd4745767a8ff5165e2924344ea7f4bf6a102
%global short_commit %(c=%{git_commit}; echo ${c:0:7})

Name:           kalendar
Version:        0.2.1
Release:        3.%{git_date}git%{short_commit}%{?dist}
Summary:        A calendar application using Akonadi to sync with external services

License:        LGPLv2+
URL:            https://invent.kde.org/pim/%{name}
Source0:        https://invent.kde.org/pim/%{name}/-/archive/%{short_commit}/%{name}-%{short_commit}.tar.bz2

## upstream patches

## upstreamable patches
Patch0:         find_package_kf5_service.patch

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
BuildRequires:  cmake(KF5Service)         >= %{kf5_min_version}
BuildRequires:  cmake(KF5Solid)           >= %{kf5_min_version}
BuildRequires:  cmake(KF5WindowSystem)    >= %{kf5_min_version}
BuildRequires:  cmake(KF5XmlGui)          >= %{kf5_min_version}

BuildRequires:  cmake(KF5Akonadi)         >= %{akonadi_min_version}
BuildRequires:  cmake(KF5AkonadiContact)  >= %{akonadi_min_version}
BuildRequires:  cmake(KF5CalendarSupport) >= %{akonadi_min_version}
BuildRequires:  cmake(KF5EventViews)      >= %{akonadi_min_version}
BuildRequires:  cmake(KF5GrantleeTheme)   >= %{akonadi_min_version}

Requires:       akonadi-calendar-tools
Requires:       kdepim-addons
Requires:       kdepim-runtime
Requires:       kf5-kirigami2
Requires:       kf5-kirigami2-addons
Requires:       kf5-kirigami2-addons-treeview

Requires:       hicolor-icon-theme

Requires:       %{name}-reminder-daemon = %{version}-%{release}

%description
Kalendar is a Kirigami-based calendar application that uses Akonadi. It lets
you add, edit and delete events from local and remote accounts of your choice,
while keeping changes syncronised across your Plasma desktop or phone.

%package        reminder-daemon
Summary:        Kalendar Reminder Daemon
%description    reminder-daemon
%{summary}.

%description    reminder-daemon
Kalendar Reminder Daemon.


%prep
%autosetup -p1 -n %{name}-%{short_commit}


%build
%cmake_kf5
%cmake_build


%install
%cmake_install

# Temporary disabling QML cache, see issue #55:
#  https://invent.kde.org/pim/kalendar/-/issues/55
sed -i 's|Exec=kalendar|Exec=env QML_DISABLE_DISK_CACHE=1 kalendar|' \
    %{buildroot}%{_datadir}/applications/org.kde.%{name}.desktop


%check
desktop-file-validate %{buildroot}%{_datadir}/applications/org.kde.%{name}.desktop
appstream-util validate-relax --nonet %{buildroot}%{_kf5_metainfodir}/org.kde.%{name}.appdata.xml


%files
%license LICENSES/*.txt
%doc README.md
%{_bindir}/%{name}
%{_datadir}/applications/org.kde.%{name}.desktop
%{_datadir}/icons/hicolor/scalable/apps/org.kde.%{name}.svg
%{_kf5_datadir}/qlogging-categories5/%{name}.categories
%{_kf5_metainfodir}/org.kde.%{name}.appdata.xml

%files reminder-daemon
%{_bindir}/%{name}ac
%{_sysconfdir}/xdg/autostart/org.kde.%{name}ac.desktop
%{_kf5_datadir}/knotifications5/%{name}ac.notifyrc
%{_kf5_datadir}/dbus-1/services/org.kde.%{name}ac.service


%changelog
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
