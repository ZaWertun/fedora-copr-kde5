# uncomment to enable bootstrap mode
#global bootstrap 1

%if !0%{?bootstrap}
%global tests 1
%endif

Name:    analitza
Summary: Library of mathematical features
Version: 23.08.2
Release: 1%{?dist}

License: GPLv2+
URL:     https://cgit.kde.org/%{name}.git

%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0: https://download.kde.org/%{stable}/release-service/%{version}/src/%{name}-%{version}.tar.xz
Source1: https://download.kde.org/%{stable}/release-service/%{version}/src/%{name}-%{version}.tar.xz.sig
Source2: gpgkey-D81C0CB38EB725EF6691C385BB463350D6EF31EF.gpg

BuildRequires: gnupg2
BuildRequires: extra-cmake-modules >= 1.3
BuildRequires: kf5-rpm-macros
BuildRequires: pkgconfig(eigen3)
BuildRequires: pkgconfig(Qt5Widgets) pkgconfig(Qt5Xml) pkgconfig(Qt5Svg)
BuildRequires: pkgconfig(Qt5Test) pkgconfig(Qt5Qml) pkgconfig(Qt5Quick)
BuildRequires: pkgconfig(Qt5OpenGL)
# technically QtQuick private api, but this should be good enough -- rex
BuildRequires: qt5-qtbase-private-devel
BuildRequires: readline-devel

%if 0%{?tests}
BuildRequires: xorg-x11-server-Xvfb
%endif

Conflicts: kalgebra < 4.7.80

Obsoletes: kalgebra-libs < 4.7.80
Provides:  kalgebra-libs = %{version}-%{release}

%description
%{summary}.

%package devel
Summary: Developer files for %{name}
Requires: %{name}%{?_isa} = %{version}-%{release}
%description devel
%{summary}.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%setup -q


%build
%cmake_kf5 \
  %{?tests:-DBUILD_TESTING:BOOL=ON}
%cmake_build


%install
%cmake_install

%find_lang_kf5 analitza_qt


%check
%if 0%{?tests}
export CTEST_OUTPUT_ON_FAILURE=1
xvfb-run -a \
make test -C %{_target_platform} ARGS="--output-on-failure --timeout 300" ||:
%endif


%ldconfig_scriptlets

%files -f analitza_qt.lang
#doc TODO
%license COPYING*
%dir %{_datadir}/libanalitza/
%{_datadir}/libanalitza/plots/
%{_kf5_libdir}/libAnalitza.so.8*
%{_kf5_libdir}/libAnalitzaGui.so.8*
%{_kf5_libdir}/libAnalitzaPlot.so.8*
%{_kf5_libdir}/libAnalitzaWidgets.so.8*
%{_kf5_qmldir}/org/kde/analitza/

%files devel
%{_includedir}/Analitza5/
%{_kf5_libdir}/libAnalitza.so
%{_kf5_libdir}/libAnalitzaGui.so
%{_kf5_libdir}/libAnalitzaPlot.so
%{_kf5_libdir}/libAnalitzaWidgets.so
%{_kf5_libdir}/cmake/Analitza5/


%changelog
* Fri Oct 13 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.08.2-1
- 23.08.2

* Thu Sep 14 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.08.1-1
- 23.08.1

* Mon Aug 28 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.08.0-1
- 23.08.0

* Thu Jul 06 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.04.3-1
- 23.04.3

* Thu Jun 08 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.04.2-1
- 23.04.2

* Thu May 11 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.04.1-1
- 23.04.1

* Fri Apr 21 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.04.0-1
- 23.04.0

* Thu Mar 02 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.12.3-1
- 22.12.3

* Thu Jan 12 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.12.1-5
- rebuild

