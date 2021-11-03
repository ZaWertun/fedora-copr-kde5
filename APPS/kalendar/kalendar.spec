%global git_date     20211103
%global git_commit   edd5276400d4c48adf991c04ffcf022df7ad6be6
%global short_commit %(c=%{git_commit}; echo ${c:0:7})

Name:           kalendar
Version:        0.1
Release:        2.%{git_date}git%{short_commit}%{?dist}
Summary:        A calendar application using Akonadi to sync with external services

License:        LGPLv2+
URL:            https://invent.kde.org/pim/%{name}
Source0:        https://invent.kde.org/pim/%{name}/-/archive/%{short_commit}/%{name}-%{short_commit}.tar.bz2

BuildRequires:  desktop-file-utils
BuildRequires:  libappstream-glib

%global kf5_min_version 5.86.0

BuildRequires:  extra-cmake-modules >= %{kf5_min_version}
BuildRequires:  kf5-rpm-macros      >= %{kf5_min_version}

BuildRequires:  cmake(Qt5Core)
BuildRequires:  cmake(Qt5Gui)
BuildRequires:  cmake(Qt5Qml)
BuildRequires:  cmake(Qt5QuickControls2)
BuildRequires:  cmake(Qt5Svg)
BuildRequires:  cmake(Qt5Location)

BuildRequires:  cmake(KF5Kirigami2)     >= %{kf5_min_version}
BuildRequires:  cmake(KF5I18n)          >= %{kf5_min_version}
BuildRequires:  cmake(KF5CalendarCore)  >= %{kf5_min_version}
BuildRequires:  cmake(KF5ConfigWidgets) >= %{kf5_min_version}
BuildRequires:  cmake(KF5WindowSystem)  >= %{kf5_min_version}
BuildRequires:  cmake(KF5CoreAddons)    >= %{kf5_min_version}
BuildRequires:  cmake(KF5People)        >= %{kf5_min_version}
BuildRequires:  cmake(KF5Contacts)      >= %{kf5_min_version}
BuildRequires:  cmake(KF5ItemModels)    >= %{kf5_min_version}
BuildRequires:  cmake(KF5XmlGui)        >= %{kf5_min_version}

BuildRequires:  cmake(KF5Akonadi)
BuildRequires:  cmake(KF5AkonadiContact)
BuildRequires:  cmake(KF5CalendarSupport)
BuildRequires:  cmake(KF5EventViews)
BuildRequires:  cmake(KF5GrantleeTheme)

Requires:       kf5-kirigami2-addons-treeview

%description
Kalendar is a Kirigami-based calendar application that uses Akonadi. It lets
you add, edit and delete events from local and remote accounts of your choice,
while keeping changes syncronised across your Plasma desktop or phone.


%prep
%autosetup -n %{name}-%{short_commit}


%build
%cmake_kf5
%cmake_build


%install
%cmake_install


%check
desktop-file-validate %{buildroot}%{_datadir}/applications/org.kde.%{name}.desktop
appstream-util validate-relax --nonet %{buildroot}%{_metainfodir}/org.kde.%{name}.appdata.xml


%files
%license LICENSES/*.txt
%doc README.md
%{_bindir}/%{name}
%{_datadir}/applications/org.kde.%{name}.desktop
%{_datadir}/icons/hicolor/scalable/apps/org.kde.%{name}.svg
%{_metainfodir}/org.kde.%{name}.appdata.xml


%changelog
* Sun Oct 24 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 0.1-2.20211021git20b34f7
- 2021-10-24, commit ba5a1306970c3ef90a94694dada81c85b61f80f1

* Fri Oct 22 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 0.1-2.20211021git20b34f7
- first spec for version 0.1 (2021-10-21, commit 20b34f7)
