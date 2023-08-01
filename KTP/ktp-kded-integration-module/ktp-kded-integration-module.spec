%global module ktp-kded-module

Name:    ktp-kded-integration-module
Summary: KDE integration for telepathy
Version: 23.04.3
Release: 1%{?dist}

License: LGPLv2+
URL:     https://cgit.kde.org/%{module}.git

%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0: http://download.kde.org/stable/release-service/%{version}/src/%{module}-%{version}.tar.xz
Source1: http://download.kde.org/stable/release-service/%{version}/src/%{module}-%{version}.tar.xz.sig
Source2: gpgkey-D81C0CB38EB725EF6691C385BB463350D6EF31EF.gpg

# due to kaccounts-providers
# handled by qt5-srpm-macros, which defines %%qt5_qtwebengine_arches
%{?qt5_qtwebengine_arches:ExclusiveArch: %{qt5_qtwebengine_arches}}

## upstream patches

BuildRequires:  gnupg2
BuildRequires:  extra-cmake-modules
BuildRequires:  gettext
BuildRequires:  kf5-rpm-macros

BuildRequires: cmake(KF5DBusAddons)
BuildRequires: cmake(KF5WidgetsAddons)
BuildRequires: cmake(KF5Config)
BuildRequires: cmake(KF5I18n)
BuildRequires: cmake(KF5KIO)
BuildRequires: cmake(KF5Notifications)
BuildRequires: cmake(KF5ConfigWidgets)
BuildRequires: cmake(KF5IdleTime)
BuildRequires: cmake(KF5KCMUtils)
BuildRequires: cmake(KF5Activities)

%global majmin_ver %(echo %{version} | cut -d. -f1,2)
BuildRequires:  ktp-common-internals-devel >= %{majmin_ver}
BuildRequires:  qt5-qtbase-devel

BuildRequires: /usr/bin/dbus-send
Requires: /usr/bin/dbus-send

Obsoletes:      telepathy-kde-integration-module < 0.3.0
Provides:       telepathy-kde-integration-module = %{version}-%{release}

# upstream name
Provides: %{name} = %{version}-%{release}

%description
This module sits in KDED and takes care of various bits of system
integration like setting user to auto-away or handling connection errors.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -n %{module}-%{version} -p1


%build
%cmake_kf5
%cmake_build


%install
%cmake_install

%find_lang %{name} --all-name --with-html


%files -f %{name}.lang
%license COPYING*
%{_kf5_qtplugindir}/kcm_ktp_integration_module.so
%{_kf5_plugindir}/kded/ktp_integration_module.so
%{_kf5_datadir}/kservices5/kcm_ktp_integration_module.desktop
%{_datadir}/dbus-1/services/org.freedesktop.Telepathy.Client.KTp.KdedIntegrationModule.service


%changelog
* Thu Jul 06 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.04.3-1
- 23.04.3

* Thu Jun 08 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.04.2-1
- 23.04.2

* Thu May 11 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.04.1-1
- 23.04.1

* Fri Apr 21 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.04.0-1
- 23.04.0

* Thu Mar 02 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.12.3-1
- 22.12.3

* Thu Jan 05 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.12.1-1
- 22.12.1

