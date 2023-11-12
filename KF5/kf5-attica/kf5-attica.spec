%global framework attica

Name:    kf5-attica
Version: 5.112.0
Release: 1%{?dist}
Summary: KDE Frameworks Tier 1 Addon with Open Collaboration Services API

License: LGPLv2+
URL:     https://cgit.kde.org/%{framework}.git

%global majmin %(echo %{version} | cut -d. -f1-2)
%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0:        http://download.kde.org/%{stable}/frameworks/%{majmin}/%{framework}-%{version}.tar.xz
Source1:        http://download.kde.org/%{stable}/frameworks/%{majmin}/%{framework}-%{version}.tar.xz.sig
Source2:        gpgkey-53E6B47B45CEA3E0D5B7457758D0EE648A48B3BB.gpg

## upstreamable patches
# https://bugs.kde.org/show_bug.cgi?id=451165
Patch0:         kf5-attica-5.92.0-fix-new-stuff-loading.patch

BuildRequires:  gnupg2
BuildRequires:  extra-cmake-modules >= %{version}
BuildRequires:  kf5-rpm-macros >= %{version}
BuildRequires:  qt5-qtbase-devel

Requires:       kf5-filesystem >= %{majmin}

%description
Attica is a Qt library that implements the Open Collaboration Services
API version 1.4.

%package        devel
Summary:        Development files for %{name}
Requires:       %{name}%{?_isa} = %{version}-%{release}
Requires:       qt5-qtbase-devel
%description    devel
%{summary}.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -n %{framework}-%{version} -p1


%build
%cmake_kf5
%cmake_build


%install
%cmake_install


%ldconfig_scriptlets

%files
%doc AUTHORS ChangeLog README.md
%license LICENSES/*.txt
%{_kf5_datadir}/qlogging-categories5/%{framework}.*
%{_kf5_libdir}/libKF5Attica.so.*

%files devel
%{_kf5_libdir}/cmake/KF5Attica/
%{_kf5_includedir}/Attica/
%{_kf5_libdir}/libKF5Attica.so
%{_kf5_archdatadir}/mkspecs/modules/qt_Attica.pri
%{_kf5_libdir}/pkgconfig/libKF5Attica.pc


%changelog
* Sun Nov 12 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.112.0-1
- 5.112.0

* Thu Oct 19 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.111.0-1
- 5.111.0

* Sun Sep 10 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.110.0-1
- 5.110.0

* Fri Aug 18 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.109.0-1
- 5.109.0

* Sun Jul 09 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.108.0-1
- 5.108.0

* Sat Jun 10 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.107.0-1
- 5.107.0

* Sat May 13 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.106.0-1
- 5.106.0

* Sat Apr 08 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.105.0-1
- 5.105.0

* Sat Mar 11 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.104.0-1
- 5.104.0

* Sun Feb 12 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.103.0-1
- 5.103.0

