%global framework solid

Name:    kf5-%{framework}
Version: 5.114.0
Release: 1%{?dist}
Summary: KDE Frameworks 5 Tier 1 integration module that provides hardware information

License: LGPLv2+
URL:     https://solid.kde.org/

%global majmin %(echo %{version} | cut -d. -f1-2)
%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0: http://download.kde.org/%{stable}/frameworks/%{majmin}/%{framework}-%{version}.tar.xz
Source1: http://download.kde.org/%{stable}/frameworks/%{majmin}/%{framework}-%{version}.tar.xz.sig
Source2: gpgkey-53E6B47B45CEA3E0D5B7457758D0EE648A48B3BB.gpg

## upstreamable patches

BuildRequires:  gnupg2
BuildRequires:  extra-cmake-modules >= %{majmin}
BuildRequires:  kf5-rpm-macros >= %{majmin}
BuildRequires:  libupnp-devel
BuildRequires:  qt5-qtbase-devel
BuildRequires:  qt5-qtdeclarative-devel
BuildRequires:  qt5-qttools-devel
BuildRequires:  systemd-devel

%if ! 0%{?bootstrap}
# Predicate parser deps
BuildRequires:  bison
BuildRequires:  flex
# really runtime-only dep, but doesn't hurt to check availability at buildtime
BuildRequires:  media-player-info
BuildRequires:  pkgconfig(mount)
# For iOS device support backend
BuildRequires:  pkgconfig(libplist-2.0)
BuildRequires:  pkgconfig(libimobiledevice-1.0)
%if 0%{?fedora} > 23 || 0%{?rhel} > 7
Recommends:     media-player-info
Recommends:     udisks2
Recommends:     upower
%else
Requires:       media-player-info
Requires:       udisks2
Requires:       upower
%endif
%endif

Requires:       kf5-filesystem >= %{majmin}

Obsoletes:      kf5-solid-libs < 5.47.0-2
Provides:       kf5-solid-libs = %{version}-%{release}
Provides:       kf5-solid-libs%{?_isa} = %{version}-%{release}

%description
Solid provides the following features for application developers:
 - Hardware Discovery
 - Power Management
 - Network Management

%package        devel
Summary:        Development files for %{name}
Requires:       %{name}%{?_isa} = %{version}-%{release}
Requires:       qt5-qtbase-devel
%description    devel
The %{name}-devel package contains libraries and header files for
developing applications that use %{name}.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -n %{framework}-%{version} -p1
# do not build the solid-power executable, it conflicts with KF6 Solid
sed -i -e 's/if(WITH_NEW_SOLID_JOB AND WITH_NEW_POWER_ASYNC_API)/if(0)/g' \
    src/tools/CMakeLists.txt

%build
%cmake_kf5 \
    -DWITH_NEW_SOLID_JOB:BOOL=ON \
    -DWITH_NEW_POWER_ASYNC_API:BOOL=ON \
    -DWITH_NEW_POWER_ASYNC_FREEDESKTOP:BOOL=ON
%cmake_build


%install
%cmake_install
%find_lang_kf5 solid5_qt


%ldconfig_scriptlets

%files -f solid5_qt.lang
%doc README.md TODO
%license LICENSES/*.txt
%{_kf5_datadir}/qlogging-categories5/%{framework}.*
%{_kf5_bindir}/solid-hardware5
#files libs
%{_kf5_qmldir}/org/kde/solid/
%{_kf5_libdir}/libKF5Solid.so.*

%files devel
%{_kf5_includedir}/Solid/
%{_kf5_libdir}/libKF5Solid.so
%{_kf5_libdir}/cmake/KF5Solid/
%{_kf5_archdatadir}/mkspecs/modules/qt_Solid.pri


%changelog
* Fri Jan 19 2024 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.114.0-1
- 5.114.0

* Fri Dec 15 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.113.0-1
- 5.113.0

* Sun Nov 12 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.112.0-1
- 5.112.0

* Thu Oct 19 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.111.0-1
- 5.111.0

* Sun Sep 10 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.110.0-1
- 5.110.0

* Fri Aug 25 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.109.0-5
- merged upstream changes

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

