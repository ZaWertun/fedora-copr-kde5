Name:           kio-admin
Version:        23.04.3
Release:        1%{?dist}
Summary:        Manage files as administrator using the admin:// KIO protocol
License:        (GPL-2.0-only or GPL-3.0-only) and BSD-3-Clause and CC0-1.0 and FSFAP
URL:            https://invent.kde.org/system/kio-admin

%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0:        https://download.kde.org/%{stable}/release-service/%{version}/src/%{name}-%{version}.tar.xz
Source1:        https://download.kde.org/%{stable}/release-service/%{version}/src/%{name}-%{version}.tar.xz.sig
Source2:        gpgkey-D81C0CB38EB725EF6691C385BB463350D6EF31EF.gpg

# RHEL 9 cmake is at 3.20, lower the minimum cmake required
Patch1:         kio-admin-lower-cmake-minimum.patch

BuildRequires:  gnupg2
BuildRequires:  extra-cmake-modules
BuildRequires:  gcc-c++
BuildRequires:  kf5-rpm-macros
BuildRequires:  zstd
BuildRequires:  cmake(Qt5Core)
BuildRequires:  cmake(KF5I18n)
BuildRequires:  cmake(KF5KIO)
BuildRequires:  cmake(PolkitQt5-1)


%description
kio-admin implements a new protocol "admin:///" 
which gives administrative access
to the entire system. This is achieved by talking, 
over dbus, with a root-level
helper binary that in turn uses 
existing KIO infrastructure to run file://
operations in root-scope.

%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -p1

%build
%cmake_kf5
%cmake_build

%install
%cmake_install
%find_lang kio5_admin %{name}.lang

%files -f %{name}.lang
%doc README.md
%license LICENSES/*
%{_kf5_metainfodir}/org.kde.kio.admin.metainfo.xml
%dir %{_kf5_plugindir}/kfileitemaction/
%{_kf5_plugindir}/kfileitemaction/kio-admin.so
%dir %{_kf5_plugindir}/kio/
%{_kf5_plugindir}/kio/admin.so
%{_kf5_libexecdir}/kio-admin-helper
%{_kf5_datadir}/dbus-1/system.d/org.kde.kio.admin.conf
%{_kf5_datadir}/dbus-1/system-services/org.kde.kio.admin.service
%{_kf5_datadir}/polkit-1/actions/org.kde.kio.admin.policy

%changelog	
* Wed Jul 19 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.04.3-1
- 23.04.3

* Tue Jun 06 2023 Marc Deop i Argemí <marcdeop@fedoraproject.org> - 23.04.2-1
- 23.04.2

* Sat May 13 2023 Marc Deop i Argemí <marcdeop@fedoraproject.org> - 23.04.1-1
- 23.04.1

* Thu Apr 20 2023 Marc Deop i Argemí <marcdeop@fedoraproject.org> - 23.04.0-1
- 23.04.0

* Fri Mar 31 2023 Marc Deop i Argemí <marcdeop@fedoraproject.org> - 23.03.90-1
- 23.03.90

* Mon Mar 20 2023 Marc Deop i Argemí <marcdeop@fedoraproject.org> - 23.03.80-1
- 23.03.80

* Mon Jan 23 2023 Onuralp SEZER <thunderbirdtr@fedoraproject.org> - 1.0.0-1
- initial kio-admin package
