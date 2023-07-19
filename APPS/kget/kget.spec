Name:    kget
Summary: Download manager
Version: 23.04.3
Release: 1%{?dist}

License: GPLv2+ and GFDL
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

## upstream patches

BuildRequires: gnupg2
BuildRequires: desktop-file-utils
BuildRequires: libappstream-glib

BuildRequires: extra-cmake-modules
BuildRequires: kf5-rpm-macros

BuildRequires: cmake(KF5Completion)
BuildRequires: cmake(KF5Config)
BuildRequires: cmake(KF5ConfigWidgets)
BuildRequires: cmake(KF5CoreAddons)
BuildRequires: cmake(KF5DBusAddons)
BuildRequires: cmake(KF5DocTools)
BuildRequires: cmake(KF5I18n)
BuildRequires: cmake(KF5IconThemes)
BuildRequires: cmake(KF5KCMUtils)
BuildRequires: cmake(KF5KDELibs4Support)
BuildRequires: cmake(KF5KIO)
BuildRequires: cmake(KF5Notifications)
BuildRequires: cmake(KF5NotifyConfig)
BuildRequires: cmake(KF5Parts)
BuildRequires: cmake(KF5Service)
BuildRequires: cmake(KF5Solid)
BuildRequires: cmake(KF5TextWidgets)
BuildRequires: cmake(KF5Wallet)
BuildRequires: cmake(KF5WidgetsAddons)
BuildRequires: cmake(KF5WindowSystem)
BuildRequires: cmake(KF5XmlGui)

BuildRequires: cmake(Qt5DBus)
BuildRequires: cmake(Qt5Gui)
BuildRequires: cmake(Qt5Network)
BuildRequires: cmake(Qt5Sql)
BuildRequires: cmake(Qt5Test)
BuildRequires: cmake(Qt5Widgets)
BuildRequires: cmake(Qt5Xml)

BuildRequires: kf5-libktorrent-devel

BuildRequires: cmake(Gpgmepp)
BuildRequires: cmake(QGpgme)
BuildRequires: pkgconfig(qca2-qt5)
BuildRequires: pkgconfig(sqlite3)

Requires: %{name}-libs%{?_isa} = %{version}-%{release}

# when split occurred
Conflicts: kdenetwork-common < 7:4.10.80
Obsoletes: kdenetwork-kget < 7:4.10.80
Provides:  kdenetwork-kget = 7:%{version}-%{release}

Conflicts: kde-l10n < 17.08.3-2

%description
%{summary}.

%package libs
Summary: Runtime libraries for %{name}
Requires: %{name} = %{version}-%{release}
Obsoletes: kdenetwork-kget-libs < 7:4.10.80
Provides:  kdenetwork-kget-libs = 7:%{version}-%{release}
%description libs
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

## unpackaged files
rm -fv %{buildroot}%{_kf5_libdir}/libkgetcore.so


%check
appstream-util validate-relax --nonet %{buildroot}%{_kf5_metainfodir}/org.kde.%{name}.appdata.xml
desktop-file-validate %{buildroot}%{_kf5_datadir}/applications/org.kde.%{name}.desktop


%files -f %{name}.lang
%doc AUTHORS README TODO
%license COPYING*
%{_kf5_bindir}/kget
%{_kf5_metainfodir}/org.kde.%{name}.appdata.xml
%{_kf5_datadir}/kservicetypes5/kget_plugin.desktop
%{_kf5_datadir}/applications/org.kde.%{name}.desktop
%{_kf5_datadir}/kio/servicemenus/kget_download.desktop
%{_kf5_datadir}/kget/
%{_kf5_datadir}/config.kcfg/kget*
%{_kf5_datadir}/dbus-1/services/org.kde.kget.service
%{_kf5_datadir}/icons/hicolor/*/apps/kget.*
%{_kf5_datadir}/knotifications5/kget*
%{_kf5_datadir}/kxmlgui5/kget/
%{_kf5_datadir}/qlogging-categories5/*.categories

%ldconfig_scriptlets libs

%files libs
%{_kf5_libdir}/libkgetcore.so.5*
%{_kf5_qtplugindir}/kget/
%{_kf5_qtplugindir}/kget_kcms/


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

