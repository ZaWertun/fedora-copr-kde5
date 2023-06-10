%global framework kconfig

# uncomment to enable bootstrap mode
#global bootstrap 1

%if !0%{?bootstrap}
# FIXME/TODO: sip currently segfaults on s390x
%if 0%{?fedora}
%ifnarch ppc64 s390x
%if 0%{?fedora} < 29
%global python_bindings 1
%endif
%endif
%endif
#endif
%global tests 1
%endif

Name:    kf5-%{framework}
Version: 5.107.0
Release: 1%{?dist}
Summary: KDE Frameworks 5 Tier 1 addon with advanced configuration system

License: GPLv2+ and LGPLv2+ and MIT
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

## upstream patches

## upstreamable patches

BuildRequires:  gnupg2
BuildRequires:  extra-cmake-modules >= %{majmin}
BuildRequires:  kf5-rpm-macros >= %{majmin}

BuildRequires:  pkgconfig(Qt5Gui)
BuildRequires:  pkgconfig(Qt5Xml)
BuildRequires:  pkgconfig(Qt5Qml)
BuildRequires:  qt5-qtbase-devel
BuildRequires:  qt5-qttools-devel

%if 0%{?python_bindings}
%if 0%{?fedora} && 0%{?fedora} < 27
BuildRequires:  bundled(python2-clang) >= 4.0.1
%else
BuildRequires:  python2-clang
%endif
BuildRequires:  clang
BuildRequires:  clang-devel
BuildRequires:  python2-PyQt5-devel
BuildRequires:  python3-PyQt5-devel
%else
Obsoletes: python2-pykf5-%{framework} < %{version}-%{release}
Obsoletes: python3-pykf5-%{framework} < %{version}-%{release}
Obsoletes: pykf5-%{framework}-devel < %{version}-%{release}
%endif

%if 0%{?tests}
BuildRequires: dbus-x11
BuildRequires: time
BuildRequires: xorg-x11-server-Xvfb
%endif

Requires:       kf5-filesystem >= %{majmin}
Requires:       %{name}-core%{?_isa} = %{version}-%{release}
Requires:       %{name}-gui%{?_isa} = %{version}-%{release}

%description
KDE Frameworks 5 Tier 1 addon with advanced configuration system made of two
parts: KConfigCore and KConfigGui.

%package        devel
Summary:        Development files for %{name}
Requires:       %{name}%{?_isa} = %{version}-%{release}
Requires:       pkgconfig(Qt5Xml)
%description    devel
The %{name}-devel package contains libraries and header files for
developing applications that use %{name}.

%package        core
Summary:        Non-GUI part of KConfig framework
Requires:       kde-settings
%description    core
KConfigCore provides access to the configuration files themselves. It features
centralized definition and lock-down (kiosk) support.

%package        gui
Summary:        GUI part of KConfig framework
Requires:       %{name}-core%{?_isa} = %{version}-%{release}
%description    gui
KConfigGui provides a way to hook widgets to the configuration so that they are
automatically initialized from the configuration and automatically propagate
their changes to their respective configuration files.

%if 0%{?python_bindings}
%package -n python2-pykf5-%{framework}
Summary: Python2 bindings for %{framework}
Requires: %{name} = %{version}-%{release}
%description -n python2-pykf5-%{framework}
%{summary}.

%package -n python3-pykf5-%{framework}
Summary: Python3 bindings for %{framework}
Requires: %{name} = %{version}-%{release}
%description -n python3-pykf5-%{framework}
%{summary}.

%package -n pykf5-%{framework}-devel
Summary: SIP files for %{framework} Python bindings
BuildArch: noarch
%description -n pykf5-%{framework}-devel
%{summary}.
%endif


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -n %{framework}-%{version} -p1


%build

%if 0%{?python_bindings:1}
PYTHONPATH=%{_datadir}/ECM/python
export PYTHONPATH
%endif

%cmake_kf5 \
  %{?tests:-DBUILD_TESTING:BOOL=ON}
%cmake_build


%install
%cmake_install
%find_lang_kf5 kconfig5_qt


%check
%if 0%{?tests}
export CTEST_OUTPUT_ON_FAILURE=1
## cant use %%ninja_test here for some reason, doesn't inherit env vars from xvfb or dbus -- rex
xvfb-run -a \
    make test -C %{_target_platform} ARGS="--output-on-failure --timeout 300" ||:
%endif


%files
%doc DESIGN README.md TODO
%license LICENSES/*.txt

%ldconfig_scriptlets core

%files core -f kconfig5_qt.lang
%{_kf5_datadir}/qlogging-categories5/%{framework}*
%{_kf5_bindir}/kreadconfig5
%{_kf5_bindir}/kwriteconfig5
%{_kf5_libdir}/libKF5ConfigCore.so.*
%{_kf5_libdir}/libKF5ConfigQml.so.*
%{_kf5_libexecdir}/kconfig_compiler_kf5
%{_kf5_libexecdir}/kconf_update

%ldconfig_scriptlets gui

%files gui
%{_kf5_libdir}/libKF5ConfigGui.so.*

%files devel
%{_kf5_includedir}/KConfig/kconfig_version.h
%{_kf5_includedir}/KConfigCore/
%{_kf5_includedir}/KConfigGui/
%{_kf5_includedir}/KConfigQml/
%{_kf5_libdir}/libKF5ConfigCore.so
%{_kf5_libdir}/libKF5ConfigGui.so
%{_kf5_libdir}/libKF5ConfigQml.so
%{_kf5_libdir}/cmake/KF5Config/
%{_kf5_archdatadir}/mkspecs/modules/qt_KConfigCore.pri
%{_kf5_archdatadir}/mkspecs/modules/qt_KConfigGui.pri

%if 0%{?python_bindings}
%files -n python2-pykf5-%{framework}
%{python2_sitearch}/PyKF5/

%files -n python3-pykf5-%{framework}
%{python3_sitearch}/PyKF5/

%files -n pykf5-%{framework}-devel
%{_datadir}/sip/PyKF5/
%endif


%changelog
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

