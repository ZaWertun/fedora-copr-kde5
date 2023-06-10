%global framework kholidays

# uncomment to enable bootstrap mode
#global bootstrap 1

%if !0%{?bootstrap}
%global tests 1
%endif

Name:    kf5-%{framework}
Epoch:   1
Version: 5.107.0
Release: 1%{?dist}
Summary: The KHolidays Library

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

BuildRequires:  gnupg2
BuildRequires:  extra-cmake-modules >= %{majmin}
BuildRequires:  kf5-rpm-macros

# qt5-qtbase-devel
BuildRequires:  pkgconfig(Qt5Core)
# qt5-qtdeclarative-devel
BuildRequires:  pkgconfig(Qt5Qml)
BuildRequires:  qt5-qttools-static
#BuildRequires:  cmake(Qt5LinguistTools)

# translations moved here
Conflicts: kde-l10n < 17.03

%description
The KHolidays library provides a C++ API that determines holiday
and other special events for a geographical region.

%package        devel
Summary:        Development files for %{name}
Requires:       %{name}%{?_isa} = %{?epoch:%{epoch}:}%{version}-%{release}
%description    devel
The %{name}-devel package contains libraries and header files for
developing applications that use %{name}.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -n %{framework}-%{version} -p1


%build
%cmake_kf5 \
  -DBUILD_TESTING:BOOL=%{?tests:ON}%{!?tests:OFF}
%cmake_build


%install
%cmake_install

%find_lang_kf5 libkholidays5_qt


%check
%if 0%{?tests}
export CTEST_OUTPUT_ON_FAILURE=1
xvfb-run -a \
dbus-launch --exit-with-session \
time \
make test ARGS="--output-on-failure --timeout 20" -C %{_target_platform} ||:
%endif


%ldconfig_scriptlets

%files -f libkholidays5_qt.lang
%license LICENSES/*.txt
%{_kf5_libdir}/libKF5Holidays.so.*
%{_kf5_qmldir}/org/kde/kholidays/
%{_kf5_datadir}/qlogging-categories5/kholidays.categories

%files devel
%{_kf5_includedir}/KHolidays/
%{_kf5_libdir}/libKF5Holidays.so
%{_kf5_libdir}/cmake/KF5Holidays/
%{_kf5_archdatadir}/mkspecs/modules/qt_KHolidays.pri


%changelog
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

