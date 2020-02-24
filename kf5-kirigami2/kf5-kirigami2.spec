%global framework kirigami2

# uncomment to enable bootstrap mode
#global bootstrap 1

%if !0%{?bootstrap}
%global tests 1
%endif

Name:    kf5-%{framework}
Version: 5.67.0
Release: 1%{?dist}
Summary: QtQuick plugins to build user interfaces based on the KDE UX guidelines

# All LGPLv2+ except for src/desktopicons.h (GPLv2+)
License: GPLv2+
URL:     https://techbase.kde.org/Kirigami
%global majmin %(echo %{version} | cut -d. -f1-2)
%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0:        http://download.kde.org/%{stable}/frameworks/%{majmin}/%{framework}-%{version}.tar.xz

## upstream paches

# filter qml provides
%global __provides_exclude_from ^%{_kf5_qmldir}/.*\\.so$

BuildRequires: extra-cmake-modules >= %{majmin}
BuildRequires: kf5-rpm-macros

BuildRequires: qt5-linguist
BuildRequires: qt5-qtbase-devel
BuildRequires: qt5-qtdeclarative-devel
BuildRequires: qt5-qtquickcontrols2-devel
BuildRequires: qt5-qtsvg-devel

%if 0%{?tests}
%if 0%{?fedora}
BuildRequires: appstream
%endif
BuildRequires: xorg-x11-server-Xvfb
%endif

# workaround https://bugs.kde.org/show_bug.cgi?id=395156
%if 0%{?rhel}==7
BuildRequires: devtoolset-7-toolchain
BuildRequires: devtoolset-7-gcc-c++
%endif

Requires:      qt5-qtquickcontrols%{?_isa}
Requires:      qt5-qtquickcontrols2%{?_isa}

%description
%{summary}.

%package        devel
Summary:        Development files for %{name}
# strictly not required, but some consumers may assume/expect runtime bits to be present too
Requires:       %{name}%{?_isa} = %{version}-%{release}
%description    devel
The %{name}-devel package contains libraries and header files for
developing applications that use %{name}.


%prep
%autosetup -n %{framework}-%{version} -p1


%build
%if 0%{?rhel}==7
. /opt/rh/devtoolset-7/enable
%endif
mkdir %{_target_platform}
pushd %{_target_platform}
%{cmake_kf5} .. \
  -DBUILD_TESTING:BOOL=%{?tests:ON}%{!?tests:OFF}
popd

%make_build -C %{_target_platform}


%install
make install/fast DESTDIR=%{buildroot} -C %{_target_platform}

%find_lang_kf5 libkirigami2plugin_qt


%check
%if 0%{?tests}
## known failure(s), not sure if possible to enable opengl/glx using
## virtualized server (QT_XCB_FORCE_SOFTWARE_OPENGL doesn't seem to help)
#2/2 Test #2: qmltests .........................***Exception: Other  0.19 sec
#Could not initialize GLX
export QT_XCB_FORCE_SOFTWARE_OPENGL=1
export CTEST_OUTPUT_ON_FAILURE=1
xvfb-run -a \
make test ARGS="--output-on-failure --timeout 30" -C %{_target_platform} ||:
%endif


%files -f libkirigami2plugin_qt.lang
# README is currently only build instructions, omit for now
#doc README.md
%license LICENSE*
%{_kf5_libdir}/libKF5Kirigami2.so.5*
%dir %{_kf5_qmldir}/org/
%dir %{_kf5_qmldir}/org/kde/
%{_kf5_qmldir}/org/kde/kirigami.2/

%files devel
%{_kf5_libdir}/libKF5Kirigami2.so
%{_kf5_includedir}/Kirigami2/
%{_kf5_archdatadir}/mkspecs/modules/qt_Kirigami2.pri
%{_kf5_libdir}/cmake/KF5Kirigami2/
%{_kf5_datadir}/kdevappwizard/
%{_kf5_datadir}/kdevappwizard/templates/
%{_kf5_datadir}/kdevappwizard/templates/kirigami.tar.bz2


%changelog
* Sun Feb 09 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.67.0-1
- 5.67.0

* Sat Jan 11 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.66.0-1
- 5.66.0

* Sat Dec 14 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.65.0-1
- 5.65.0

* Fri Nov 29 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.64.0-2
- patch for #414003

* Mon Nov 11 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.64.0-1
- 5.64.0

* Sun Oct 13 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.63.0-1
- 5.63.0

* Sun Sep 15 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.62.0-1
- 5.62.0

* Mon Aug 12 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.61.0-1
- 5.61.0

* Sat Jul 13 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.60.0-1
- 5.60.0

* Sat Jun 08 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.59.0-1
- 5.59.0

* Tue May 14 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.58.0-1
- 5.58.0

* Sat Apr 27 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.57.0-1
- 5.57.0

* Mon Feb 04 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.55.0-1
- 5.55.0

* Fri Feb 01 2019 Fedora Release Engineering <releng@fedoraproject.org> - 5.54.0-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_30_Mass_Rebuild

* Tue Jan 08 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.54.0-1
- 5.54.0

* Sun Dec 09 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.53.0-1
- 5.53.0

* Sun Nov 04 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.52.0-1
- 5.52.0

* Wed Oct 10 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.51.0-1
- 5.51.0

* Tue Sep 04 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.50.0-1
- 5.50.0

* Tue Aug 07 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.49.0-1
- 5.49.0

* Fri Jul 13 2018 Fedora Release Engineering <releng@fedoraproject.org> - 5.48.0-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_29_Mass_Rebuild

* Mon Jul 09 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.48.0-1
- 5.48.0

* Sat Jun 02 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.47.0-1
- 5.47.0

* Sat May 05 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.46.0-1
- 5.46.0

* Sun Apr 08 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.45.0-1
- 5.45.0

* Sat Mar 03 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.44.0-1
- 5.44.0

* Wed Feb 07 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.43.0-1
- 5.43.0

* Mon Jan 08 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.42.0-1
- 5.42.0

* Mon Dec 04 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.41.0-1
- 5.41.0

* Fri Nov 10 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.40.0-1
- 5.40.0

* Wed Oct 25 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.39.0-1
- 5.39.0 (included in kde frameworks since 5.39 release)

* Thu Oct 12 2017 Rex Dieter <rdieter@fedoraproject.org> - 2.2.0-1
- 2.2.0

* Thu Aug 03 2017 Fedora Release Engineering <releng@fedoraproject.org> - 2.1.0-4
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Binutils_Mass_Rebuild

* Wed Jul 26 2017 Fedora Release Engineering <releng@fedoraproject.org> - 2.1.0-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Mass_Rebuild

* Thu Jun 01 2017 Rex Dieter <rdieter@fedoraproject.org> - 2.1.0-2
- pull in upstream fixes

* Mon May 01 2017 Rex Dieter <rdieter@fedoraproject.org> - 2.1.0-1
- 2.1.0

* Fri Mar 03 2017 Rex Dieter <rdieter@fedoraproject.org> - 2.0.0-2
- Requires: qt5-qtquickcontrols, License: GPLv2+

* Wed Mar 01 2017 Rex Dieter <rdieter@fedoraproject.org> -  2.0.0-1
- kf5-kirigami-2.0.0 first try
