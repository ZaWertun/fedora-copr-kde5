%global framework kcalendarcore

# uncomment to enable bootstrap mode
#global bootstrap 1

%if !0%{?bootstrap}
%global tests 1
%endif

Name:    kf5-kcalendarcore
Epoch:   1
Version: 5.115.0
Release: 1%{?dist}
Summary: KDE Frameworks 5 Tier 1 KCalendarCore Library

License: LGPLv2+ and GPLv3+
URL:     https://cgit.kde.org/%{framework}.git

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

# libical (and thus kcalendarcore) not on all arches for RHEL8.
%if 0%{?rhel} == 8
ExclusiveArch: x86_64 ppc64le %{arm}
%endif

BuildRequires:  gnupg2
BuildRequires:  extra-cmake-modules >= %{majmin}
BuildRequires:  kf5-rpm-macros

BuildRequires:  bison
BuildRequires:  libical-devel

BuildRequires:  qt5-qtbase-devel

%if 0%{?tests}
BuildRequires: dbus-x11
BuildRequires: xorg-x11-server-Xvfb
%endif

%description
%{summary}.

%package        devel
Summary:        Development files for %{name}
Requires:       %{name}%{?_isa} = %{epoch}:%{version}-%{release}
Requires:       libical-devel
%description    devel
The %{name}-devel package contains libraries and header files for
developing applications that use %{name}.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -n %{framework}-%{version}


%build
%cmake_kf5 \
  -DBUILD_TESTING:BOOL=%{?tests:ON}%{!?tests:OFF}
%cmake_build


%install
%cmake_install

## TODO: poke upstream about failures seen on f30
#The following tests FAILED:
#         39 - testicaltimezones (Failed)
#        472 - Compat-libical3-AppleICal_1.5.ics (Failed)
#        473 - Compat-libical3-Evolution_2.8.2_timezone_test.ics (Failed)
#        475 - Compat-libical3-KOrganizer_3.1a.ics (Failed)
#        477 - Compat-libical3-MSExchange.ics (Failed)
#        478 - Compat-libical3-Mozilla_1.0.ics (Failed)
%check
%if 0%{?tests}
export CTEST_OUTPUT_ON_FAILURE=1
xvfb-run -a \
dbus-launch --exit-with-session \
make test ARGS="--output-on-failure --timeout 20" -C %{_target_platform} ||:
%endif


%ldconfig_scriptlets

%files
%license LICENSES/*.txt
%{_kf5_datadir}/qlogging-categories5/*kcalendarcore.*
%{_kf5_libdir}/libKF5CalendarCore.so.*

%files devel
%{_kf5_includedir}/kcalcore_version.h
%{_kf5_includedir}/KCalendarCore/
%{_kf5_libdir}/libKF5CalendarCore.so
%{_kf5_libdir}/cmake/KF5CalendarCore/
%{_kf5_archdatadir}/mkspecs/modules/qt_KCalendarCore.pri
%{_kf5_libdir}/pkgconfig/KF5CalendarCore.pc


%changelog
* Sun Feb 18 2024 Yaroslav Sidlovsky <zawertun@gmail.com> - 1:5.115.0-1
- 5.115.0

* Fri Jan 19 2024 Yaroslav Sidlovsky <zawertun@gmail.com> - 1:5.114.0-1
- 5.114.0

* Fri Dec 15 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 1:5.113.0-1
- 5.113.0

* Sun Nov 12 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 1:5.112.0-1
- 5.112.0

* Thu Oct 19 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 1:5.111.0-1
- 5.111.0

* Sun Sep 10 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 1:5.110.0-1
- 5.110.0

* Fri Aug 18 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 1:5.109.0-1
- 5.109.0

* Sun Jul 09 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 1:5.108.0-1
- 5.108.0

* Sat Jun 10 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 1:5.107.0-1
- 5.107.0

* Sat May 13 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 1:5.106.0-1
- 5.106.0

* Sat Apr 08 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 1:5.105.0-1
- 5.105.0

* Sat Mar 11 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 1:5.104.0-1
- 5.104.0

* Sun Feb 12 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 1:5.103.0-1
- 5.103.0

