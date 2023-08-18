%global framework kdeclarative

# uncomment to enable bootstrap mode
%global bootstrap 1

%if !0%{?bootstrap}
%global tests 1
%endif

Name:    kf5-%{framework}
Version: 5.109.0
Release: 1%{?dist}
Summary: KDE Frameworks 5 Tier 3 addon for Qt declarative

License: LGPLv2+
URL:     https://cgit.kde.org/%{framework}.git/

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

# filter qml provides
%global __provides_exclude_from ^%{_kf5_qmldir}/.*\\.so$

## upstream patches

BuildRequires:  gnupg2
BuildRequires:  extra-cmake-modules >= %{majmin}
BuildRequires:  kf5-kconfig-devel >= %{majmin}
BuildRequires:  kf5-kglobalaccel-devel >= %{majmin}
BuildRequires:  kf5-kguiaddons-devel >= %{majmin}
BuildRequires:  kf5-ki18n-devel >= %{majmin}
BuildRequires:  kf5-kiconthemes-devel >= %{majmin}
BuildRequires:  kf5-kio-devel >= %{majmin}
BuildRequires:  kf5-kpackage-devel >= %{majmin}
BuildRequires:  kf5-kwidgetsaddons-devel >= %{majmin}
BuildRequires:  kf5-kwindowsystem-devel >= %{majmin}
BuildRequires:  kf5-knotifications-devel >= %{majmin}
BuildRequires:  kf5-rpm-macros
BuildRequires:  libepoxy-devel
BuildRequires:  qt5-qtbase-devel
# https://bugs.kde.org/show_bug.cgi?id=365569#c8 claims this may be needed,
# so err on the side of caution
BuildRequires:  qt5-qtbase-private-devel
BuildRequires:  qt5-qtdeclarative-devel
%if 0%{?tests}
BuildRequires: dbus-x11
BuildRequires: mesa-dri-drivers
BuildRequires: time
BuildRequires: xorg-x11-server-Xvfb
%endif

%description
KDE Frameworks 5 Tier 3 addon for Qt declarative

%package        devel
Summary:        Development files for %{name}
Requires:       %{name}%{?_isa} = %{version}-%{release}
Requires:       kf5-kconfig-devel >= %{majmin}
Requires:       kf5-kpackage-devel >= %{majmin}
Requires:       qt5-qtdeclarative-devel
%description    devel
The %{name}-devel package contains libraries and header files for
developing applications that use %{name}.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -p1 -n %{framework}-%{version}


%build
%cmake_kf5 \
  -DBUILD_TESTING:BOOL=%{?tests:ON}%{!?tests:OFF}
%cmake_build


%install
%cmake_install

%find_lang %{name} --all-name


%check
%if 0%{?tests}
export CTEST_OUTPUT_ON_FAILURE=1
xvfb-run -a \
dbus-launch --exit-with-session \
time \
make test ARGS="--output-on-failure --timeout 10 --verbose" -C %{_target_platform} ||:
%endif


%ldconfig_scriptlets

%files -f %{name}.lang
%doc README.md
%license LICENSES/*.txt
%{_kf5_bindir}/kpackagelauncherqml
%{_kf5_libdir}/libKF5Declarative.so.*
%{_kf5_libdir}/libKF5QuickAddons.so.*
%{_kf5_libdir}/libKF5CalendarEvents.so.*
%dir %{_kf5_qmldir}/org/
%dir %{_kf5_qmldir}/org/kde/
%{_kf5_qmldir}/org/kde/draganddrop/
%{_kf5_qmldir}/org/kde/kconfig/
%{_kf5_qmldir}/org/kde/kcoreaddons/
%{_kf5_qmldir}/org/kde/kquickcontrols/
%{_kf5_qmldir}/org/kde/kquickcontrolsaddons/
%dir %{_kf5_qmldir}/org/kde/private/
%{_kf5_qmldir}/org/kde/private/kquickcontrols/
%{_kf5_qmldir}/org/kde/kio/
%{_kf5_qmldir}/org/kde/kwindowsystem/
%{_kf5_qmldir}/org/kde/kcm/
%{_kf5_qmldir}/org/kde/graphicaleffects


%files devel
%{_kf5_includedir}/KDeclarative/
%{_kf5_libdir}/libKF5Declarative.so
%{_kf5_libdir}/libKF5QuickAddons.so
%{_kf5_libdir}/libKF5CalendarEvents.so
%{_kf5_libdir}/cmake/KF5Declarative/
%{_kf5_archdatadir}/mkspecs/modules/qt_KDeclarative.pri
%{_kf5_archdatadir}/mkspecs/modules/qt_QuickAddons.pri


%changelog
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

