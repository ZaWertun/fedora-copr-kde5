%global framework krunner

Name:    kf5-%{framework}
Version: 5.112.0
Release: 1%{?dist}
Summary: KDE Frameworks 5 Tier 3 solution with parallelized query system

License: LGPLv2+ and BSD
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

# filter qml provides
%global __provides_exclude_from ^%{_kf5_qmldir}/.*\\.so$

BuildRequires:  gnupg2
BuildRequires:  extra-cmake-modules >= %{majmin}
BuildRequires:  kf5-rpm-macros

BuildRequires:  kf5-kconfig-devel >= %{majmin}
BuildRequires:  kf5-kcoreaddons-devel >= %{majmin}
BuildRequires:  kf5-ki18n-devel >= %{majmin}
BuildRequires:  kf5-kio-devel >= %{majmin}
BuildRequires:  kf5-kservice-devel >= %{majmin}
BuildRequires:  kf5-plasma-devel >= %{majmin}
BuildRequires:  kf5-solid-devel >= %{majmin}
BuildRequires:  kf5-threadweaver-devel >= %{majmin}
BuildRequires:  cmake(KF5Activities)

BuildRequires:  qt5-qtbase-devel
BuildRequires:  qt5-qtdeclarative-devel

%description
KRunner provides a parallelized query system extendable via plugins.

%package        devel
Summary:        Development files for %{name}
# krunner template moved here
Conflicts:      kapptemplate < 16.03.80
Requires:       %{name}%{?_isa} = %{version}-%{release}
Requires:       kf5-plasma-devel >= %{majmin}
Requires:       qt5-qtbase-devel
%description    devel
The %{name}-devel package contains libraries and header files for
developing applications that use %{name}.


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
%doc README.md
%license LICENSES/*.txt
%{_kf5_datadir}/qlogging-categories5/%{framework}*
%{_kf5_libdir}/libKF5Runner.so.*
%{_kf5_qmldir}/org/kde/runnermodel/
%{_kf5_datadir}/kservicetypes5/plasma-runner.desktop

%files devel
%{_kf5_includedir}/KRunner/
%{_kf5_libdir}/libKF5Runner.so
%{_kf5_libdir}/cmake/KF5Runner/
%{_kf5_archdatadir}/mkspecs/modules/qt_KRunner.pri
%{_kf5_datadir}/dbus-1/interfaces/*
%{_kf5_datadir}/kdevappwizard/templates/runner.tar.bz2
%{_kf5_datadir}/kdevappwizard/templates/runnerpython.tar.bz2


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

