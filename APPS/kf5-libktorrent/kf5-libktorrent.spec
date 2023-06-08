%global framework libktorrent

Name:    kf5-libktorrent
Summary: Library providing torrent downloading code
Version: 23.04.2
Release: 1%{?dist}

License: GPLv2+
URL:     https://invent.kde.org/network/%{framework}

%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0: http://download.kde.org/stable/release-service/%{version}/src/%{framework}-%{version}.tar.xz
Source1: http://download.kde.org/stable/release-service/%{version}/src/%{framework}-%{version}.tar.xz.sig
Source2: gpgkey-D81C0CB38EB725EF6691C385BB463350D6EF31EF.gpg

## upstream patches

BuildRequires: gnupg2
BuildRequires: boost-devel
BuildRequires: gettext
BuildRequires: gmp-devel >= 6.0.0
BuildRequires: libgcrypt-devel >= 1.4.5
BuildRequires: cmake(Qca-qt5)
BuildRequires: cmake(Qt5Network)

# kf5 deps
BuildRequires: extra-cmake-modules
BuildRequires: kf5-rpm-macros
BuildRequires: cmake(KF5Archive)
BuildRequires: cmake(KF5Crash)
BuildRequires: cmake(KF5I18n)
BuildRequires: cmake(KF5KIO)
BuildRequires: cmake(KF5Solid)

%description
%{summary}.

%package devel
Summary: Developer files for %{name}
Requires: %{name}%{?_isa} = %{version}-%{release}
Requires: boost-devel
# mse/bigint.h:#include <gmp.h>
Requires: gmp-devel%{?_isa}
Requires: libgcrypt-devel
Requires: cmake(KF5Archive)
Requires: cmake(KF5Config)
Requires: cmake(KF5KIO)
Requires: cmake(Qt5Network)
%description devel
%{summary}.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -n %{framework}-%{version} -p1


%build
%cmake_kf5

%cmake_build


%install
%cmake_install

%find_lang libktorrent5

%ldconfig_scriptlets

%files -f libktorrent5.lang
%doc ChangeLog
%license LICENSES/*
%{_kf5_libdir}/libKF5Torrent.so.6*
%{_kf5_libdir}/libKF5Torrent.so.%{version}

%files devel
%{_kf5_includedir}/libktorrent/
%{_kf5_libdir}/libKF5Torrent.so
%{_kf5_libdir}/cmake/KF5Torrent/


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

