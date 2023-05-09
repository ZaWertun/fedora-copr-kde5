Name:           plasma-welcome
Version:        5.27.5
Release:        1%{?dist}
Summary:        A friendly onboarding wizard for Plasma

License:        GPLv2+
URL:            https://invent.kde.org/plasma/%{name}
Source0:        http://download.kde.org/stable/plasma/%(echo %{version} |cut -d. -f1-3)/%{name}-%{version}.tar.xz
Source1:        http://download.kde.org/stable/plasma/%(echo %{version} |cut -d. -f1-3)/%{name}-%{version}.tar.xz.sig
Source2:        https://jriddell.org/esk-riddell.gpg

BuildRequires:  gnupg2
BuildRequires:  libappstream-glib
BuildRequires:  desktop-file-utils
BuildRequires:  make cmake gcc-c++
BuildRequires:  cmake(Qt5Core)
BuildRequires:  cmake(Qt5Gui)
BuildRequires:  cmake(Qt5Network)
BuildRequires:  cmake(Qt5Qml)
BuildRequires:  cmake(Qt5QuickControls2)
BuildRequires:  cmake(Qt5Svg)
BuildRequires:  cmake(KF5Kirigami2)
BuildRequires:  cmake(KF5CoreAddons)
BuildRequires:  cmake(KF5ConfigWidgets)
BuildRequires:  cmake(KF5DBusAddons)
BuildRequires:  cmake(KF5Declarative)
BuildRequires:  cmake(KF5I18n)
BuildRequires:  cmake(KF5KIO)
BuildRequires:  cmake(KF5NewStuff)
BuildRequires:  cmake(KF5Notifications)
BuildRequires:  cmake(KF5Plasma)
BuildRequires:  cmake(KF5Service)
BuildRequires:  cmake(KF5WindowSystem)
BuildRequires:  cmake(KAccounts)
BuildRequires:  cmake(KUserFeedback)

%description
A Friendly onboarding wizard for Plasma

Welcome Center is the perfect introduction to KDE Plasma! It can help you learn
how to connect to the internet, install apps, customize the system, and more!


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup


%build
%cmake_kf5
%cmake_build


%install
%cmake_install
%find_lang %{name} --with-qt


%check
desktop-file-validate %{buildroot}%{_kf5_datadir}/applications/org.kde.%{name}.desktop
appstream-util validate-relax --nonet %{buildroot}%{_kf5_metainfodir}/org.kde.%{name}.appdata.xml ||:


%files -f %{name}.lang
%doc README.md
%license LICENSES/*.txt
%{_bindir}/%{name}
%{_kf5_qmldir}/org/kde/plasma/welcome/
%{_kf5_datadir}/applications/org.kde.%{name}.desktop
%{_sysconfdir}/xdg/autostart/org.kde.%{name}.desktop
%{_kf5_metainfodir}/org.kde.%{name}.appdata.xml


%changelog
* Tue May 09 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.27.5-1
- 5.27.5

* Tue Apr 04 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.27.4.1-1
- version 5.27.4.1

* Tue Apr 04 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.27.4-1
- 5.27.4

* Tue Mar 14 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.27.3-1
- 5.27.3

* Tue Feb 28 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.27.2-1
- 5.27.2

* Tue Feb 21 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.27.1-1
- 5.27.1

* Fri Feb 17 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.27.0-1
- first spec for version 5.27.0

