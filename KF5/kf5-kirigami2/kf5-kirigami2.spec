%global framework kirigami2

# uncomment to enable bootstrap mode
#global bootstrap 1

%if !0%{?bootstrap}
%global tests 1
%endif

Name:    kf5-%{framework}
Version: 5.112.0
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
Source0: http://download.kde.org/%{stable}/frameworks/%{majmin}/%{framework}-%{version}.tar.xz
Source1: http://download.kde.org/%{stable}/frameworks/%{majmin}/%{framework}-%{version}.tar.xz.sig
Source2: gpgkey-53E6B47B45CEA3E0D5B7457758D0EE648A48B3BB.gpg

## upstream paches

# filter qml provides
%global __provides_exclude_from ^%{_kf5_qmldir}/.*\\.so$

BuildRequires: gnupg2
BuildRequires: extra-cmake-modules >= %{majmin}
BuildRequires: kf5-rpm-macros

BuildRequires: qt5-linguist
BuildRequires: qt5-qtbase-devel
BuildRequires: qt5-qtdeclarative-devel
BuildRequires: qt5-qtquickcontrols2-devel
BuildRequires: qt5-qtsvg-devel
BuildRequires: qt5-qtbase-private-devel

%if 0%{?tests}
%if 0%{?fedora}
BuildRequires: appstream
%endif
BuildRequires: xorg-x11-server-Xvfb
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
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -n %{framework}-%{version} -p1


%build
%cmake_kf5 \
  -DBUILD_TESTING:BOOL=%{?tests:ON}%{!?tests:OFF}
%cmake_build


%install
%cmake_install
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
%dir %{_kf5_datadir}/kdevappwizard/
%dir %{_kf5_datadir}/kdevappwizard/templates/
%{_kf5_datadir}/kdevappwizard/templates/kirigami.tar.bz2


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

