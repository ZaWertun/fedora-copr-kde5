Name:    kaccounts-providers
Version: 23.08.1
Release: 1%{?dist}
Summary: Additional service providers for KAccounts framework
License: GPLv2
URL:     https://cgit.kde.org/%{name}.git

%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0:        https://download.kde.org/%{stable}/release-service/%{version}/src/%{name}-%{version}.tar.xz
Source1:        https://download.kde.org/%{stable}/release-service/%{version}/src/%{name}-%{version}.tar.xz.sig
Source2:        gpgkey-D81C0CB38EB725EF6691C385BB463350D6EF31EF.gpg

BuildRequires:  gnupg2
BuildRequires:  extra-cmake-modules
BuildRequires:  intltool
%global majmin_ver %(echo %{version} | cut -d. -f1,2)
BuildRequires:  kaccounts-integration-devel >= %{majmin_ver}
BuildRequires:  kf5-kdeclarative-devel
BuildRequires:  kf5-ki18n-devel
BuildRequires:  kf5-kio-devel
BuildRequires:  kf5-rpm-macros
BuildRequires:  pkgconfig(libaccounts-glib)
BuildRequires:  pkgconfig(Qt5Gui)
BuildRequires:  pkgconfig(Qt5WebEngineWidgets)

Requires:       signon-ui
Requires:       accounts-qml-module%{?_isa}

# switched to arch'd pkg
Obsoletes: kaccounts-providers < 15.12.0

%description
%{summary}.

%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -p1


%build
%cmake_kf5
%cmake_build


%install
%cmake_install

%find_lang %{name} --all-name


%files -f %{name}.lang
%license LICENSES/*.txt
%config %{_sysconfdir}/signon-ui/webkit-options.d/*
%{_datadir}/accounts/providers/kde/
%{_datadir}/accounts/services/kde/*.service
%dir %{_kf5_datadir}/kpackage/genericqml
%{_kf5_datadir}/kpackage/genericqml/org.kde.kaccounts.owncloud/
%{_kf5_datadir}/kpackage/genericqml/org.kde.kaccounts.nextcloud/
%{_kf5_datadir}/icons/hicolor/256x256/apps/kaccounts-owncloud.png
%{_kf5_datadir}/icons/hicolor/scalable/apps/kaccounts-nextcloud.svg
%dir %{_qt5_plugindir}/kaccounts/
%dir %{_qt5_plugindir}/kaccounts/ui/
%{_qt5_plugindir}/kaccounts/ui/owncloud_plugin_kaccounts.so
%{_qt5_plugindir}/kaccounts/ui/nextcloud_plugin_kaccounts.so
%{_kf5_metainfodir}/org.kde.kaccounts.owncloud.appdata.xml
%{_kf5_metainfodir}/org.kde.kaccounts.nextcloud.appdata.xml


%changelog
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

