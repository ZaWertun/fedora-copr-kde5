%global framework kdgantt2

Name:    kf5-%{framework}
Version: 19.08.0
Release: 1%{?dist}
Summary: KDE PIM library for rendering Gantt graphs

License: GPLv2
URL:     http://quickgit.kde.org/?p=%{framework}.git


%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0:        http://download.kde.org/%{stable}/applications/%{version}/src/%{framework}-%{version}.tar.xz

BuildRequires:  extra-cmake-modules >= 5.23.0
BuildRequires:  kf5-rpm-macros >= 5.23.0

BuildRequires:  cmake(Qt5Widgets)
BuildRequires:  cmake(Qt5Test)
BuildRequires:  cmake(Qt5PrintSupport)

BuildRequires:  cmake(KF5I18n)

Requires:       kf5-filesystem

Obsoletes:      kdepim-libs < 7:16.04.0
Conflicts:      kdepim-libs < 7:16.04.0

%description
%{summary}.

%package        devel
Summary:        Development files for %{name}
Requires:       %{name}%{?_isa} = %{version}-%{release}
%description    devel
The %{name}-devel package contains libraries and header files for
developing applications that use %{name}.


%prep
%autosetup -n %{framework}-%{version}


%build
%cmake_kf5
%cmake_build


%install
%cmake_install


%post -p /sbin/ldconfig
%postun -p /sbin/ldconfig

%files
#TODO: Missing upstream
#license COPYING.LIB
%config(noreplace) %{_kf5_sysconfdir}/xdg/kdgantt2.categories
%{_kf5_libdir}/libKF5KDGantt2.so.*

%files devel
%{_kf5_libdir}/libKF5KDGantt2.so
%{_kf5_libdir}/cmake/KF5KDGantt2/
%{_kf5_includedir}/KDGantt2/
%{_kf5_includedir}/kdgantt2/
%{_kf5_includedir}/kdgantt2_version.h
%{_kf5_archdatadir}/mkspecs/modules/qt_KDGantt2.pri


%changelog
* Sat Aug 17 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 19.08.0-1
- 19.08.0

* Fri Jul 13 2018 Fedora Release Engineering <releng@fedoraproject.org> - 16.08.3-6
- Rebuilt for https://fedoraproject.org/wiki/Fedora_29_Mass_Rebuild

* Wed Feb 07 2018 Fedora Release Engineering <releng@fedoraproject.org> - 16.08.3-5
- Rebuilt for https://fedoraproject.org/wiki/Fedora_28_Mass_Rebuild

* Thu Aug 03 2017 Fedora Release Engineering <releng@fedoraproject.org> - 16.08.3-4
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Binutils_Mass_Rebuild

* Wed Jul 26 2017 Fedora Release Engineering <releng@fedoraproject.org> - 16.08.3-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Mass_Rebuild

* Fri Feb 10 2017 Fedora Release Engineering <releng@fedoraproject.org> - 16.08.3-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_26_Mass_Rebuild

* Mon Dec 05 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.08.3-1
- 16.08.3

* Thu Oct 13 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.08.2-1
- 16.08.2

* Thu Sep 08 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.08.1-1
- 16.08.1

* Sat Sep 03 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.08.0-1
- 16.08.0

* Sun Jul 10 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.04.3-1
- 16.04.3

* Sun Jun 12 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.04.2-1
- 16.04.2

* Thu May 26 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.04.1-2
- fix Conflicts/Obsoletes versioning

* Wed May 25 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.04.1-1
- 16.04.1, cosmetics, update URL, fix Conflicts/Obsoletes

* Tue May 03 2016 Daniel Vrátil <dvratil@fedoraproject.org> - 16.04.0-1
- Initial version
