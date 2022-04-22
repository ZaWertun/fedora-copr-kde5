%global framework libktorrent

Name:    kf5-libktorrent
Summary: Library providing torrent downloading code
Version: 22.04.0
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

## upstream patches

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
* Thu Apr 21 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.04.0-1
- 22.04.0

* Thu Mar 03 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 21.12.3-1
- 21.12.3

* Thu Feb 03 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 21.12.2-1
- 21.12.2

* Thu Jan 13 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 21.12.1-1
- 21.12.1

* Mon Dec 27 2021 Rex Dieter <rdieter@fedoraproject.org> - 21.12.0-1
- 21.12.0

* Tue Nov 02 2021 Rex Dieter <rdieter@fedoraproject.org> - 21.08.3-1
- 21.08.3

* Thu Oct 21 2021 Rex Dieter <rdieter@fedoraproject.org> - 21.08.2-1
- 21.08.2

* Wed Jul 28 2021 Rex Dieter <rdieter@fedoraproject.org> - 21.04.3-1
- 21.04.3

* Thu Jul 22 2021 Fedora Release Engineering <releng@fedoraproject.org> - 21.04.2-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_35_Mass_Rebuild

* Thu Jun 10 2021 Rex Dieter <rdieter@fedoraproject.org> - 21.04.2-1
- 21.04.2

* Tue May 11 2021 Rex Dieter <rdieter@fedoraproject.org> - 21.04.1-1
- 21.04.1

* Mon Apr 19 2021 Rex Dieter <rdieter@fedoraproject.org> - 21.04.0-1
- 21.04.0

* Wed Mar 03 2021 Rex Dieter <rdieter@fedoraproject.org> - 20.12.3-1
- 20.12.3

* Sat Feb 27 2021 Rex Dieter <rdieter@fedoraproject.org> - 20.12.2-1
- part of kde releases now

* Tue Jan 26 2021 Fedora Release Engineering <releng@fedoraproject.org> - 2.2.0-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_34_Mass_Rebuild

* Tue Jul 28 2020 Fedora Release Engineering <releng@fedoraproject.org> - 2.2.0-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_33_Mass_Rebuild

* Wed Jul 01 2020 Rex Dieter <rdieter@fedoraproject.org> - 2.2.0-1
- 2.2.0

* Wed Jan 29 2020 Fedora Release Engineering <releng@fedoraproject.org> - 2.1.1-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_32_Mass_Rebuild

* Fri Jan 17 2020 Jeff Law <law@redhat.com> 2.1.1-2
- Drop reference to non-existent patch file

* Wed Sep 11 2019 Rex Dieter <rdieter@fedoraproject.org> 2.1.1-1
- 2.1.1

* Thu Jul 25 2019 Fedora Release Engineering <releng@fedoraproject.org> - 2.1-6
- Rebuilt for https://fedoraproject.org/wiki/Fedora_31_Mass_Rebuild

* Fri Feb 01 2019 Fedora Release Engineering <releng@fedoraproject.org> - 2.1-5
- Rebuilt for https://fedoraproject.org/wiki/Fedora_30_Mass_Rebuild

* Fri Jul 13 2018 Fedora Release Engineering <releng@fedoraproject.org> - 2.1-4
- Rebuilt for https://fedoraproject.org/wiki/Fedora_29_Mass_Rebuild

* Wed Feb 07 2018 Fedora Release Engineering <releng@fedoraproject.org> - 2.1-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_28_Mass_Rebuild

* Fri Jan 19 2018 Rex Dieter <rdieter@fedoraproject.org> - 2.1-2
- pull in upstream fix

* Wed Nov 29 2017 Rex Dieter <rdieter@fedoraproject.org> - 2.1-1
- 2.1

* Tue Aug 22 2017 Rex Dieter <rdieter@fedoraproject.org> - 2.0.90-1
- 2.0.90

* Thu Aug 03 2017 Fedora Release Engineering <releng@fedoraproject.org> - 2.0.1-9
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Binutils_Mass_Rebuild

* Wed Jul 26 2017 Fedora Release Engineering <releng@fedoraproject.org> - 2.0.1-8
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Mass_Rebuild

* Tue Jul 18 2017 Jonathan Wakely <jwakely@redhat.com> - 2.0.1-7
- Rebuilt for Boost 1.64

* Fri Feb 10 2017 Fedora Release Engineering <releng@fedoraproject.org> - 2.0.1-6
- Rebuilt for https://fedoraproject.org/wiki/Fedora_26_Mass_Rebuild

* Mon Dec 12 2016 Rex Dieter <rdieter@fedoraproject.org> - 2.0.1-5
- Requires: kf5-filesystem
- update URL
- %%license COPYING

* Wed Oct 19 2016 Rex Dieter <rdieter@fedoraproject.org> - 2.0.1-4
- use upstreamable gmp fix/workaround

* Wed Oct 19 2016 Rex Dieter <rdieter@fedoraproject.org> - 2.0.1-3
- more upstream fixes

* Fri Jun 03 2016 Rex Dieter <rdieter@fedoraproject.org> - 2.0.1-2
- pull in upstream fixes

* Fri Apr 22 2016 Rex Dieter <rdieter@fedoraproject.org> -  2.0.1-1
- libktorrent-2.0.1
