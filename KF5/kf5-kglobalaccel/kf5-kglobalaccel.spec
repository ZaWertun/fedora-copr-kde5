%global framework kglobalaccel

Name:    kf5-%{framework}
Version: 5.114.0
Release: 1%{?dist}
Summary: KDE Frameworks 5 Tier 3 integration module for global shortcuts

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

## upstream

BuildRequires:  gnupg2
BuildRequires:  extra-cmake-modules >= %{majmin}
BuildRequires:  kf5-rpm-macros
BuildRequires:  kf5-kconfig-devel >= %{majmin}
BuildRequires:  kf5-kcoreaddons-devel >= %{majmin}
BuildRequires:  kf5-kcrash-devel >= %{majmin}
BuildRequires:  kf5-kdbusaddons-devel >= %{majmin}
BuildRequires:  kf5-ki18n-devel >= %{majmin}
BuildRequires:  kf5-kservice-devel >= %{majmin}
BuildRequires:  kf5-kwindowsystem-devel >= %{majmin}
BuildRequires:  systemd-rpm-macros

BuildRequires:  qt5-qtbase-devel
BuildRequires:  qt5-qttools-devel
BuildRequires:  qt5-qtx11extras-devel

BuildRequires:  xcb-util-keysyms-devel
BuildRequires:  libX11-devel
BuildRequires:  libxcb-devel


Requires:       %{name}-libs%{?_isa} = %{version}-%{release}

Conflicts: plasma-workspace < 5.2.0-8

%description
%{summary}.

%package        libs
Summary:        Runtime libraries for %{name}
Requires:       %{name} = %{version}-%{release}
%description    libs
%{summary}.

%package        devel
Summary:        Development files for %{name}
Requires:       %{name}-libs%{?_isa} = %{version}-%{release}
Requires:       qt5-qtbase-devel
%description    devel
The %{name}-devel package contains libraries and header files for
developing applications that use %{name}.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -p1 -n %{framework}-%{version}


%build
%cmake_kf5
%cmake_build


%install
%cmake_install
%find_lang_kf5 kglobalaccel5_qt


%post
%systemd_user_post plasma-%{framework}.service


%preun
%systemd_user_preun plasma-%{framework}.service


%files -f kglobalaccel5_qt.lang
%doc README.md
%license LICENSES/*.txt
%{_kf5_bindir}/kglobalaccel5
%{_kf5_datadir}/kservices5/kglobalaccel5.desktop
%{_kf5_datadir}/qlogging-categories5/kglobalaccel.categories
%{_kf5_datadir}/qlogging-categories5/kglobalaccel.renamecategories
%{_datadir}/dbus-1/services/org.kde.kglobalaccel.service
%{_userunitdir}/plasma-%{framework}.service

%files libs
%{_kf5_libdir}/libKF5GlobalAccel.so.*
%{_kf5_libdir}/libKF5GlobalAccelPrivate.so.*
%{_kf5_qtplugindir}/org.kde.kglobalaccel5.platforms/

%files devel
%{_kf5_includedir}/KGlobalAccel/
%{_kf5_libdir}/libKF5GlobalAccel.so
%{_kf5_libdir}/cmake/KF5GlobalAccel/
%{_kf5_archdatadir}/mkspecs/modules/qt_KGlobalAccel.pri
%{_kf5_datadir}/dbus-1/interfaces/*


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

