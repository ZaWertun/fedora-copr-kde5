%global framework plasma

Name:    kf5-%{framework}
Version: 5.107.0
Release: 1%{?dist}
Summary: KDE Frameworks 5 Tier 3 framework is foundation to build a primary user interface

License: GPLv2+ and LGPLv2+ and BSD
URL:     https://invent.kde.org/frameworks/%{framework}

%global majmin %(echo %{version} | cut -d. -f1-2)
%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0: http://download.kde.org/%{stable}/frameworks/%{majmin}/%{framework}-framework-%{version}.tar.xz
Source1: http://download.kde.org/%{stable}/frameworks/%{majmin}/%{framework}-framework-%{version}.tar.xz.sig
Source2: gpgkey-53E6B47B45CEA3E0D5B7457758D0EE648A48B3BB.gpg

# hackish cache invalidation
# upstream has worked on this issue recently (.31 or .32?) so may consider dropping this -- rex
Source10: fedora-plasma-cache.sh.in

## upstream patches

## upstreamable patches

# filter qml provides
%global __provides_exclude_from ^%{_kf5_qmldir}/.*\\.so$

BuildRequires:  gnupg2
BuildRequires:  extra-cmake-modules >= %{majmin}
BuildRequires:  kf5-kactivities-devel >= %{majmin}
BuildRequires:  kf5-karchive-devel >= %{majmin}
BuildRequires:  kf5-kconfig-devel >= %{majmin}
BuildRequires:  kf5-kconfigwidgets-devel >= %{majmin}
BuildRequires:  kf5-kcoreaddons-devel >= %{majmin}
BuildRequires:  kf5-kdbusaddons-devel >= %{majmin}
BuildRequires:  kf5-kdeclarative-devel >= %{majmin}
BuildRequires:  kf5-kdesu-devel >= %{majmin}
BuildRequires:  kf5-kdoctools-devel >= %{majmin}
BuildRequires:  kf5-kglobalaccel-devel >= %{majmin}
BuildRequires:  kf5-kguiaddons-devel >= %{majmin}
BuildRequires:  kf5-ki18n-devel >= %{majmin}
BuildRequires:  kf5-kiconthemes-devel >= %{majmin}
BuildRequires:  kf5-kio-devel >= %{majmin}
BuildRequires:  kf5-knotifications-devel >= %{majmin}
BuildRequires:  kf5-kpackage-devel >= %{majmin}
BuildRequires:  kf5-kparts-devel >= %{majmin}
BuildRequires:  kf5-kservice-devel >= %{majmin}
BuildRequires:  kf5-kirigami2-devel >= %{majmin}
%if 0%{?fedora}
BuildRequires:  kf5-kwayland-devel >= %{majmin}
%endif
BuildRequires:  kf5-kwidgetsaddons-devel >= %{majmin}
BuildRequires:  kf5-kwindowsystem-devel >= %{majmin}
BuildRequires:  kf5-kxmlgui-devel >= %{majmin}
BuildRequires:  kf5-rpm-macros
BuildRequires:  kf5-solid-devel >= %{majmin}
BuildRequires:  libGL-devel
BuildRequires:  libSM-devel
BuildRequires:  libX11-devel
BuildRequires:  libxcb-devel
BuildRequires:  libXext-devel
BuildRequires:  libXrender-devel
BuildRequires:  libXScrnSaver-devel
BuildRequires:  openssl-devel

BuildRequires:  qt5-qtbase-devel
BuildRequires:  qt5-qtdeclarative-devel
BuildRequires:  qt5-qtquickcontrols2-devel
BuildRequires:  qt5-qtscript-devel
BuildRequires:  qt5-qtsvg-devel
BuildRequires:  qt5-qtx11extras-devel

Requires:       qt5-qtquickcontrols%{?_isa}
Requires:       qt5-qtquickcontrols2%{?_isa}

%if 0%{?fedora}
# https://bugzilla.redhat.com/1293415
Conflicts:      kdeplasma-addons < 5.5.0-3
%endif

%description
%{summary}.

%package        devel
Summary:        Development files for %{name}
# https://bugzilla.redhat.com/1292506
Conflicts:      kapptemplates < 15.12.0-1
Requires:       %{name}%{?_isa} = %{version}-%{release}
Requires:       kf5-kpackage-devel >= %{majmin}
Requires:       kf5-kservice-devel >= %{majmin}
Requires:       kf5-kwindowsystem-devel >= %{majmin}
Requires:       qt5-qtbase-devel
%description    devel
The %{name}-devel package contains libraries and header files for
developing applications that use %{name}.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -p1 -n %{framework}-framework-%{version}
install -m644 -p %{SOURCE10} .


%build
%cmake_kf5
%cmake_build


%install
%cmake_install

%find_lang %{name} --all-name --with-man --all-name

# create/own dirs
mkdir -p %{buildroot}%{_kf5_datadir}/plasma/plasmoids
mkdir -p %{buildroot}%{_kf5_qmldir}/org/kde/private

%if 0%{?fedora}
mkdir -p %{buildroot}%{_sysconfdir}/xdg/plasma-workspace/env
sed -e "s|@@VERSION@@|%{version}|g" fedora-plasma-cache.sh.in > \
  %{buildroot}%{_sysconfdir}/xdg/plasma-workspace/env/fedora-plasma-cache.sh
%endif


%ldconfig_scriptlets

%files -f %{name}.lang
%doc README.md
%license LICENSES/*.txt
%if 0%{?fedora}
%{_sysconfdir}/xdg/plasma-workspace/env/fedora-plasma-cache.sh
%endif
%{_kf5_bindir}/plasmapkg2
%{_kf5_libdir}/libKF5Plasma.so.*
%{_kf5_libdir}/libKF5PlasmaQuick.so.*
%dir %{_kf5_qmldir}/org/
%dir %{_kf5_qmldir}/org/kde/
%dir %{_kf5_qmldir}/org/kde/private/
%{_kf5_qmldir}/org/kde/plasma/
%{_kf5_qmldir}/org/kde/kirigami.2/styles
%{_kf5_qmldir}/QtQuick/Controls/Styles/Plasma/
%{_kf5_qmldir}/QtQuick/Controls.2/Plasma/
%{_kf5_qtplugindir}/plasma/
%{_kf5_qtplugindir}/kpackage/packagestructure/*.so
%{_kf5_qtplugindir}/kf5/kirigami/KirigamiPlasmaStyle.so
%{_kf5_datadir}/plasma/
%{_kf5_datadir}/kservicetypes5/*.desktop
%{_kf5_mandir}/man1/plasmapkg2.1.*
%{_kf5_datadir}/qlogging-categories5/*categories
# can find_lang handle this too somehow? -- rex
%lang(lt) %{_datadir}/locale/lt/LC_SCRIPTS/libplasma5/

%files devel
%{_kf5_libdir}/cmake/KF5Plasma/
%{_kf5_libdir}/cmake/KF5PlasmaQuick/
%{_kf5_libdir}/libKF5Plasma.so
%{_kf5_libdir}/libKF5PlasmaQuick.so
%{_kf5_includedir}/plasma/
%{_kf5_includedir}/Plasma/
%{_kf5_includedir}/PlasmaQuick/
%{_kf5_includedir}/plasmaquick/
%dir %{_kf5_datadir}/kdevappwizard/
%{_kf5_datadir}/kdevappwizard/templates/


%changelog
* Sat Jun 10 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.107.0-1
- 5.107.0

* Sat May 13 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.106.0-1
- 5.106.0

* Sat Apr 08 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.105.0-1
- 5.105.0

* Mon Mar 20 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.104.0-5
- rebuild

* Sat Mar 11 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.104.0-1
- 5.104.0

