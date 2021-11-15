%global git_date     20211115
%global git_commit   b8ced3f4f4accd51c3e3d24b5ce9277ee22e0d37
%global short_commit %(c=%{git_commit}; echo ${c:0:7})

Name:           kalendar
Version:        0.2.1
Release:        1.%{git_date}git%{short_commit}%{?dist}
Summary:        A calendar application using Akonadi to sync with external services

License:        LGPLv2+
URL:            https://invent.kde.org/pim/%{name}
Source0:        https://invent.kde.org/pim/%{name}/-/archive/%{short_commit}/%{name}-%{short_commit}.tar.bz2

BuildRequires:  gettext
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

Requires:       akonadi-calendar-tools
Requires:       kdepim-addons
Requires:       kdepim-runtime
Requires:       kf5-kirigami2
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

# Temporary disabling QML cache, see issue #55:
#  https://invent.kde.org/pim/kalendar/-/issues/55
sed -i 's|Exec=kalendar|Exec=env QML_DISABLE_DISK_CACHE=1 kalendar|' \
    %{buildroot}%{_datadir}/applications/org.kde.%{name}.desktop


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
