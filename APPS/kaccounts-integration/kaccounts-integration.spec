Name:    kaccounts-integration
Version: 23.08.4
Release: 1%{?dist}
Summary: Small system to administer web accounts across the KDE desktop
License: GPLv2+
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

## upstream fixes

## upstreamable fixes

BuildRequires:  gnupg2
BuildRequires:  desktop-file-utils
BuildRequires:  extra-cmake-modules
BuildRequires:  kf5-rpm-macros
BuildRequires:  kf5-kcmutils-devel
BuildRequires:  kf5-kdeclarative-devel
BuildRequires:  kf5-kio-devel
BuildRequires:  kf5-ki18n-devel
BuildRequires:  kf5-kwidgetsaddons-devel
BuildRequires:  kf5-kcoreaddons-devel
BuildRequires:  kf5-kiconthemes-devel
BuildRequires:  kf5-kconfig-devel
BuildRequires:  kf5-kwallet-devel
BuildRequires:  kf5-kdbusaddons-devel

BuildRequires:  pkgconfig(Qt5Gui)
BuildRequires:  pkgconfig(Qt5Qml)
BuildRequires:  cmake(QCoro5)

BuildRequires:  pkgconfig(accounts-qt5) >= 1.13
BuildRequires:  pkgconfig(libaccounts-glib) >= 1.21
BuildRequires:  pkgconfig(libsignon-qt5) >= 8.55

# For AutoReq cmake-filesystem
BuildRequires:  cmake

Requires:       signon-plugin-oauth2%{?_isa}
Requires:       accounts-qml-module%{?_isa}

Obsoletes:      kaccounts < 15.03
Provides:       kaccounts = %{version}-%{release}

# translations moved here
Conflicts: kde-l10n < 17.03

%description
Small system to administer web accounts for the sites and services
across the KDE desktop.

%package        devel
Summary:        Development files for %{name}
Requires:       %{name}%{?_isa} = %{version}-%{release}
Requires:       qt5-qtbase-devel%{?_isa}
Requires:       kf5-kcoreaddons-devel%{?_isa}
Requires:       libaccounts-glib-devel%{?_isa}
Requires:       libaccounts-qt5-devel%{?_isa}
Requires:       signon-devel%{?_isa}
%description    devel
Headers, development libraries and documentation for %{name}.


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
desktop-file-validate %{buildroot}%{_kf5_datadir}/applications/kcm_kaccounts.desktop


%ldconfig_scriptlets

%files -f %{name}.lang
%doc README
%license LICENSES/*.txt
%{_kf5_qtplugindir}/plasma/kcms/systemsettings/kcm_kaccounts.so
%{_kf5_qtplugindir}/kaccounts/daemonplugins/kaccounts_kio_webdav_plugin.so
%{_kf5_datadir}/applications/kcm_kaccounts.desktop
%{_kf5_datadir}/kpackage/kcms/kcm_kaccounts/contents/ui/*.qml
%{_kf5_plugindir}/kded/kded_accounts.so
%{_kf5_libdir}/libkaccounts.so.2
%{_kf5_libdir}/libkaccounts.so.%{version}
%{_kf5_qmldir}/org/kde/kaccounts/

%files devel
%{_kf5_libdir}/libkaccounts.so
%{_kf5_libdir}/cmake/KAccounts/
%{_includedir}/KAccounts/


%changelog
* Fri Dec 08 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.08.4-1
- 23.08.4

* Fri Nov 10 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.08.3-1
- 23.08.3

* Fri Oct 13 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.08.2-1
- 23.08.2

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

