Name:    libkdegames
Summary: Common code and data for many KDE games
Version: 23.04.3
Release: 1%{?dist}

# libKF5KDEGames is LGPLv2, libKF5KDEGamesPrivate is GPLv2+
License: LGPLv2 and GPLv2+
URL:     https://cgit.kde.org/%{name}.git

%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0: https://download.kde.org/%{stable}/release-service/%{version}/src/libkdegames-%{version}.tar.xz
Source1: https://download.kde.org/%{stable}/release-service/%{version}/src/libkdegames-%{version}.tar.xz.sig
Source2: gpgkey-D81C0CB38EB725EF6691C385BB463350D6EF31EF.gpg

BuildRequires: gnupg2
BuildRequires: extra-cmake-modules
BuildRequires: kf5-karchive-devel
BuildRequires: kf5-kbookmarks-devel
BuildRequires: kf5-kcodecs-devel
BuildRequires: kf5-kcompletion-devel
BuildRequires: kf5-kcompletion-devel
BuildRequires: kf5-kconfig-devel
BuildRequires: kf5-kconfigwidgets-devel
BuildRequires: kf5-kcoreaddons-devel
BuildRequires: kf5-kcrash-devel
BuildRequires: kf5-kdbusaddons-devel
BuildRequires: kf5-kdeclarative-devel
BuildRequires: kf5-kdnssd-devel
BuildRequires: kf5-kdoctools-devel
BuildRequires: kf5-kglobalaccel-devel
BuildRequires: kf5-kguiaddons-devel
BuildRequires: kf5-khtml-devel
BuildRequires: kf5-ki18n-devel
BuildRequires: kf5-kiconthemes-devel
BuildRequires: kf5-kio-devel
BuildRequires: kf5-kitemviews-devel
BuildRequires: kf5-kjobwidgets-devel
BuildRequires: kf5-knewstuff-devel
BuildRequires: kf5-kservice-devel
BuildRequires: kf5-ktextwidgets-devel
BuildRequires: kf5-kwidgetsaddons-devel
BuildRequires: kf5-kxmlgui-devel
BuildRequires: kf5-rpm-macros

BuildRequires: pkgconfig(Qt5Widgets) pkgconfig(Qt5Qml) pkgconfig(Qt5Quick) pkgconfig(Qt5QuickWidgets) pkgconfig(Qt5Svg) pkgconfig(Qt5Test)

BuildRequires: gettext
BuildRequires: pkgconfig(openal)
BuildRequires: pkgconfig(sndfile)

Provides: libkdegames-kf5 = %{version}-%{release}
Provides: libkdegames-kf5%{?_isa} = %{version}-%{release}

%global __provides_exclude_from ^%{_qt5_archdatadir}/qml/.*\\.so$

%description
%{summary}.

%package devel
Summary:  Development files for %{name} 
Provides: libkdegames-kf5-devel = %{version}-%{release}
Provides: libkdegames-kf5-devel%{?_isa} = %{version}-%{release}
Requires: %{name}%{?_isa} = %{version}-%{release}
Requires: pkgconfig(Qt5Network) pkgconfig(Qt5Widgets) pkgconfig(Qt5Qml) pkgconfig(Qt5QuickWidgets) pkgconfig(Qt5Xml)
Requires: kf5-kconfig-devel
Requires: kf5-kconfigwidgets-devel
Requires: kf5-kcompletion-devel
Requires: kf5-ki18n-devel
Requires: kf5-kwidgetsaddons-devel
%description devel
%{summary}.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -p1


%build
%cmake_kf5
%cmake_build


%install
%cmake_install
%find_lang %{name} --all-name --with-html


%ldconfig_scriptlets

%files -f %{name}.lang
%doc README TODO
%license LICENSES/*.txt
%{_kf5_libdir}/libKF5KDEGames.so.7*
%{_kf5_libdir}/libKF5KDEGamesPrivate.so.*
%{_qt5_archdatadir}/qml/org/kde/games/
%{_kf5_datadir}/kconf_update/kgthemeprovider-migration.upd
# consider common/noarch subpkg
%{_kf5_datadir}/carddecks/
%{_kf5_datadir}/qlogging-categories5/*.categories

%files devel
%{_kf5_libdir}/libKF5KDEGames.so
%{_kf5_libdir}/libKF5KDEGamesPrivate.so
%{_kf5_includedir}/KDEGames/
%{_kf5_libdir}/cmake/KF5KDEGames/


%changelog
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

