Name:    cervisia
Summary: CVS frontend
Version: 23.04.2
Release: 1%{?dist}

License: GPLv2+ and LGPLv2+ and GFDL
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
BuildRequires: libappstream-glib
BuildRequires: extra-cmake-modules
BuildRequires: kf5-rpm-macros
BuildRequires: cmake(KF5Parts)
BuildRequires: cmake(KF5DBusAddons)
BuildRequires: cmake(KF5DocTools)
BuildRequires: cmake(KF5Init)
BuildRequires: cmake(KF5IconThemes)
BuildRequires: cmake(KF5Notifications)
BuildRequires: cmake(KF5WidgetsAddons)
BuildRequires: cmake(KF5ItemViews)
BuildRequires: cmake(KF5Su)
BuildRequires: cmake(Qt5DBus)
BuildRequires: cmake(Qt5Widgets)

%{?kf5_kinit_requires}

# translations moved here
Conflicts: kde-l10n < 17.03

Conflicts:      kdesdk-common < 4.10.80
Provides:       kdesdk-cervisia = %{version}-%{release}
Obsoletes:      kdesdk-cervisia < 4.10.80

%description
Cervisia is a CVS frontend for KDE


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup


%build
%cmake_kf5
%cmake_build


%install
%cmake_install

%find_lang %{name} --all-name --with-html --with-man


%check
appstream-util validate-relax --nonet %{buildroot}%{_kf5_metainfodir}/org.kde.cervisia.appdata.xml ||:
desktop-file-validate %{buildroot}%{_kf5_datadir}/applications/org.kde.cervisia.desktop


%files -f %{name}.lang
%license COPYING*
%doc ChangeLog README
%{_kf5_bindir}/cervisia
%{_kf5_datadir}/icons/hicolor/*/*/*
%{_kf5_qtplugindir}/cervisiapart5.so
%{_kf5_libdir}/libkdeinit5_cervisia.so
%{_kf5_datadir}/applications/org.kde.cervisia.desktop
%{_kf5_metainfodir}/org.kde.cervisia.appdata.xml
%{_kf5_datadir}/config.kcfg/cervisiapart.kcfg
%{_kf5_datadir}/dbus-1/interfaces/org.kde.cervisia5.*.xml
%{_kf5_datadir}/kservices5/org.kde.cervisiapart5.desktop
%{_mandir}/man1/cervisia*
%{_kf5_bindir}/cvsaskpass
%{_kf5_bindir}/cvsservice5
%{_kf5_libdir}/libkdeinit5_cvsaskpass.so
%{_kf5_libdir}/libkdeinit5_cvsservice.so
%{_kf5_datadir}/knotifications5/cervisia.notifyrc
%{_kf5_datadir}/kservices5/org.kde.cvsservice5.desktop
%{_kf5_datadir}/kxmlgui5/cervisia/
%{_kf5_datadir}/kxmlgui5/cervisiapart/


%changelog
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

