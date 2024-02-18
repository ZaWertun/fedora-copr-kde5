%global framework kjs

Name:    kf5-%{framework}
Version: 5.115.0
Release: 1%{?dist}
Summary: KDE Frameworks 5 Tier 3 functional module with JavaScript interpreter

License: GPLv2+ and BSD
URL:     https://cgit.kde.org/%{framework}.git

%global majmin %(echo %{version} | cut -d. -f1-2)
%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0:        http://download.kde.org/%{stable}/frameworks/%{majmin}/portingAids/%{framework}-%{version}.tar.xz
Source1:        http://download.kde.org/%{stable}/frameworks/%{majmin}/portingAids/%{framework}-%{version}.tar.xz.sig
Source2:        gpgkey-53E6B47B45CEA3E0D5B7457758D0EE648A48B3BB.gpg

BuildRequires:  gnupg2
BuildRequires:  extra-cmake-modules >= %{majmin}
BuildRequires:  kf5-kdoctools-devel >= %{majmin}
BuildRequires:  kf5-rpm-macros
BuildRequires:  pcre-devel
BuildRequires:  perl-interpreter
BuildRequires:  perl-generators
BuildRequires:  qt5-qtbase-devel

%description
KDE Frameworks 1 Tier 1 functional module with JavaScript interpret.

%package        devel
Summary:        Development files for %{name}
Requires:       %{name}%{?_isa} = %{version}-%{release}
Requires:       qt5-qtbase-devel
%description    devel
The %{name}-devel package contains libraries and header files for
developing applications that use %{name}.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -n %{framework}-%{version}


%build
%cmake_kf5
%cmake_build


%install
%cmake_install

chmod +x %{buildroot}%{_kf5_datadir}/kf5/kjs/create_hash_table

%find_lang %{name} --with-man --all-name


%ldconfig_scriptlets

%files -f %{name}.lang
%doc README.md
%license COPYING.LIB
%{_kf5_bindir}/kjs5
%{_kf5_libdir}/libKF5JS.so.*
%{_kf5_libdir}/libKF5JSApi.so.*
%{_mandir}/man1/kjs5.1.gz

%files devel
%dir %{_kf5_datadir}/kf5/kjs/
%{_kf5_datadir}/kf5/kjs/create_hash_table
%{_kf5_includedir}/kjs_version.h
%{_kf5_includedir}/kjs/
%{_kf5_includedir}/wtf/
%{_kf5_libdir}/libKF5JS.so
%{_kf5_libdir}/libKF5JSApi.so
%{_kf5_libdir}/cmake/KF5JS/
%{_kf5_archdatadir}/mkspecs/modules/qt_KJS.pri
%{_kf5_archdatadir}/mkspecs/modules/qt_KJSApi.pri


%changelog
* Sun Feb 18 2024 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.115.0-1
- 5.115.0

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

