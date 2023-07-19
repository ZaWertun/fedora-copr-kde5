Name:    sweeper 
Summary: Clean unwanted traces the user leaves on the system 
Version: 23.04.3
Release: 1%{?dist}

License: LGPLv2+
#URL:     http://utils.kde.org/projects/%{name}
URL:     https://cgit.kde.org/%{name}.git

%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0: https://download.kde.org/%{stable}/release-service/%{version}/src/%{name}-%{version}.tar.xz
Source1: https://download.kde.org/%{stable}/release-service/%{version}/src/%{name}-%{version}.tar.xz.sig
Source2: gpgkey-D81C0CB38EB725EF6691C385BB463350D6EF31EF.gpg

BuildRequires: gnupg2
BuildRequires: desktop-file-utils

BuildRequires: extra-cmake-modules
BuildRequires: cmake(KF5ActivitiesStats)
BuildRequires: cmake(KF5Bookmarks)
BuildRequires: cmake(KF5Crash)
BuildRequires: cmake(KF5Config)
BuildRequires: cmake(KF5ConfigWidgets)
BuildRequires: cmake(KF5CoreAddons)
BuildRequires: cmake(KF5DocTools)
BuildRequires: cmake(KF5I18n)
BuildRequires: cmake(KF5KIO)
BuildRequires: cmake(KF5TextWidgets)
BuildRequires: cmake(KF5XmlGui)

BuildRequires: cmake(Qt5DBus)
BuildRequires: cmake(Qt5Widgets)

# when split occured
Conflicts: kdeutils-common < 6:4.7.80

Obsoletes: kdeutils-sweeper < 6:4.7.80
Provides:  kdeutils-sweeper = 6:%{version}-%{release}

Conflicts: kde-l10n < 17.08.3-2

%description
Sweeper helps to clean unwanted traces the user leaves on the system.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -p1 


%build
%cmake_kf5
%cmake_build


%install
%cmake_install
%find_lang %{name} --all-name --with-html


%check
desktop-file-validate %{buildroot}%{_kf5_datadir}/applications/org.kde.sweeper.desktop


%files -f %{name}.lang
%license LICENSES/*.txt
%{_kf5_bindir}/sweeper
%{_kf5_datadir}/applications/org.kde.sweeper.desktop
%{_kf5_metainfodir}/org.kde.sweeper.appdata.xml
%{_kf5_datadir}/kxmlgui5/sweeper/
%{_datadir}/dbus-1/interfaces/org.kde.sweeper.xml
%{_kf5_datadir}/qlogging-categories5/%{name}.categories


%changelog
* Tue Jul 18 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.04.3-1
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

