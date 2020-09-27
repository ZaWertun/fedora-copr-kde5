%undefine __cmake_in_source_build
%global min_qt_version 5.14.0
%global min_kf_version 5.70.0

# uncomment to enable bootstrap mode
#global bootstrap 1

Name:    kwinft
Version: 5.19.1
Release: 2%{?dist}
Summary: KWin Fast Track - Wayland compositor and X11 window manager

Provides:  kwin = %{version}
Conflicts: kwin
Conflicts: kwin-lowlatency

# all sources are effectively GPLv2+, except for:
# scripts/enforcedeco/contents/code/main.js
# KDE e.V. may determine that future GPL versions are accepted
License: GPLv2 or GPLv3
URL:     https://gitlab.com/kwinft/kwinft

%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global majmin_ver %(echo %{version} | cut -d. -f1,2).50
%global stable unstable
%else
%global majmin_ver %(echo %{version} | cut -d. -f1,2)
%global stable stable
%endif
Source0: %{url}/-/archive/%{name}@%{version}/%{name}-%{name}@%{version}.tar.bz2

## upstream patches

# Base
BuildRequires:  extra-cmake-modules
BuildRequires:  kf5-rpm-macros

# Qt
BuildRequires:  qt5-qtbase-devel         >= %{min_qt_version}
BuildRequires:  qt5-qtbase-static        >= %{min_qt_version}
# KWinQpaPlugin (and others?)
BuildRequires:  qt5-qtbase-private-devel >= %{min_qt_version}
BuildRequires:  qt5-qtsensors-devel      >= %{min_qt_version}
BuildRequires:  qt5-qtscript-devel       >= %{min_qt_version}
BuildRequires:  qt5-qttools-devel        >= %{min_qt_version}
BuildRequires:  qt5-qttools-static       >= %{min_qt_version}
BuildRequires:  qt5-qtx11extras-devel    >= %{min_qt_version}

# Wrapland
BuildRequires:  wrapland-client-devel
BuildRequires:  wrapland-server-devel

# X11/OpenGL
BuildRequires:  mesa-libGL-devel
BuildRequires:  mesa-libEGL-devel
BuildRequires:  mesa-libgbm-devel
BuildRequires:  libxkbcommon-devel
BuildRequires:  libX11-devel
BuildRequires:  libXi-devel
BuildRequires:  libxcb-devel
BuildRequires:  libICE-devel
BuildRequires:  libSM-devel
BuildRequires:  libXcursor-devel
BuildRequires:  xcb-util-wm-devel
BuildRequires:  xcb-util-image-devel
BuildRequires:  xcb-util-keysyms-devel
BuildRequires:  xcb-util-cursor-devel
BuildRequires:  xcb-util-devel
BuildRequires:  libepoxy-devel
BuildRequires:  libcap-devel

# Wayland
BuildRequires:  kf5-kwayland-devel
BuildRequires:  wayland-devel
BuildRequires:  libxkbcommon-devel >= 0.4
BuildRequires:  pkgconfig(libinput) >= 0.10
BuildRequires:  pkgconfig(libudev)
BuildRequires:  pkgconfig(wayland-eglstream)

# KF5
BuildRequires:  kf5-kcompletion-devel    >= %{min_kf_version}
BuildRequires:  kf5-kconfig-devel        >= %{min_kf_version}
BuildRequires:  kf5-kconfigwidgets-devel >= %{min_kf_version}
BuildRequires:  kf5-kcoreaddons-devel    >= %{min_kf_version}
BuildRequires:  kf5-kcrash-devel         >= %{min_kf_version}
BuildRequires:  kf5-kglobalaccel-devel   >= %{min_kf_version}
BuildRequires:  kf5-ki18n-devel          >= %{min_kf_version}
BuildRequires:  kf5-kinit-devel          >= %{min_kf_version}
BuildRequires:  kf5-kio-devel            >= %{min_kf_version}
BuildRequires:  kf5-knotifications-devel >= %{min_kf_version}
BuildRequires:  kf5-kservice-devel       >= %{min_kf_version}
BuildRequires:  kf5-plasma-devel         >= %{min_kf_version}
BuildRequires:  kf5-kwidgetsaddons-devel >= %{min_kf_version}
BuildRequires:  kf5-kwindowsystem-devel  >= %{min_kf_version}
BuildRequires:  kf5-kdoctools-devel      >= %{min_kf_version}
BuildRequires:  kf5-kcmutils-devel       >= %{min_kf_version}
BuildRequires:  kf5-knewstuff-devel      >= %{min_kf_version}
BuildRequires:  kf5-kactivities-devel    >= %{min_kf_version}
BuildRequires:  kf5-kdoctools-devel      >= %{min_kf_version}
BuildRequires:  kf5-kdeclarative-devel   >= %{min_kf_version}
BuildRequires:  kf5-kiconthemes-devel    >= %{min_kf_version}
BuildRequires:  kf5-kidletime-devel      >= %{min_kf_version}
BuildRequires:  kf5-ktextwidgets-devel   >= %{min_kf_version}
BuildRequires:  kf5-kirigami2-devel      >= %{min_kf_version}

BuildRequires:  kdecoration-devel   >= %{majmin_ver}
BuildRequires:  kscreenlocker-devel >= %{majmin_ver}
BuildRequires:  plasma-breeze-devel >= %{majmin_ver}

%if 0%{?tests}
BuildRequires: dbus-x11
BuildRequires: openbox
BuildRequires: xorg-x11-server-Xvfb
%endif

## Runtime deps
Requires:       %{name}-libs%{?_isa}   = %{version}-%{release}
Requires:       %{name}-common%{?_isa} = %{version}-%{release}

Requires:       kdecoration%{?_isa}   >= %{majmin_ver}
Requires:       kscreenlocker%{?_isa} >= %{majmin_ver}
Requires:       kdisplay%{?_isa}      >= %{majmin_ver}

# Runtime-only dependencies
%if ! 0%{?bootstrap}
BuildRequires:  qt5-qtmultimedia-devel
BuildRequires:  qt5-qtvirtualkeyboard
%endif
Requires:       qt5-qtmultimedia%{?_isa}
Recommends:     qt5-qtvirtualkeyboard%{?_isa}
# libkdeinit5_kwin*
%{?kf5_kinit_requires}

# Before kwin was split out from kde-workspace into a subpackage
Conflicts:      kde-workspace%{?_isa} < 4.11.14-2

Obsoletes:      kwin-gles < 5
Obsoletes:      kwin-gles-libs < 5

# http://bugzilla.redhat.com/605675
Provides: firstboot(windowmanager) = kwin_x11
# and kwin too (#1197135), until initial-setup fixed
Provides: firstboot(windowmanager) = kwin

%description
%{summary}.

%package        wayland
Summary:        KDE Window Manager with experimental Wayland support
Requires:       %{name}-libs%{?_isa} = %{version}-%{release}
Requires:       %{name}-common%{?_isa} = %{version}-%{release}
Requires:       kwayland-integration%{?_isa} >= %{majmin_ver}
%if ! 0%{?bootstrap}
BuildRequires:  xorg-x11-server-Xwayland
%endif
Requires:       xorg-x11-server-Xwayland
# KWinQpaPlugin (and others?)
%{?_qt5:Requires: %{_qt5}%{?_isa} = %{_qt5_version}}
# libkdeinit5_kwin*
%{?kf5_kinit_requires}
# kwin-lowlatency <\>
Provides:       kwin-wayland = %{version}
Conflicts:      kwin-wayland
# </>
%description    wayland
%{summary}.

%package        common
Summary:        Common files for KWin X11 and KWin Wayland
Requires:       %{name}-libs%{?_isa} = %{version}-%{release}
Requires:       kf5-kwayland%{?_isa} >= %{_kf5_version}
# kwin-lowlatency <\>
Provides:       kwin-common = %{version}
Conflicts:      kwin-common
# </>
%description    common
%{summary}.

%package        libs
Summary:        KWin runtime libraries
# Before kwin-libs was split out from kde-workspace into a subpackage
Conflicts:      kde-workspace-libs%{?_isa} < 4.11.14-2
# kwin-lowlatency <\>
Provides:       kwin-libs = %{version}
Conflicts:      kwin-libs
# </>
%description    libs
%{summary}.

%package        devel
Summary:        Development files for %{name}
Requires:       %{name}-libs%{?_isa} = %{version}-%{release}
Requires:       %{name}-common%{?_isa} = %{version}-%{release}
Requires:       kf5-kconfig-devel
Requires:       kf5-kservice-devel
Requires:       kf5-kwindowsystem-devel
Conflicts:      kde-workspace-devel < 5.0.0-1
# kwin-lowlatency <\>
Provides:       kwin-devel = %{version}
Conflicts:      kwin-devel
# </>
%description    devel
The %{name}-devel package contains libraries and header files for
developing applications that use %{name}.

%package        doc
Summary:        User manual for %{name}
Requires:       %{name} = %{version}-%{release}
BuildArch:      noarch
# kwin-lowlatency <\>
Provides:       kwin-doc = %{version}
Conflicts:      kwin-doc
# </>
%description    doc
%{summary}.


%prep
%autosetup -p1 -n %{name}-%{name}@%{version}

sed -i \
  -e 's|^find_package(Breeze ${PROJECT_VERSION} CONFIG)|find_package(Breeze 5.9 CONFIG)|' \
  CMakeLists.txt


%build
%{cmake_kf5}

%cmake_build


%install
%cmake_install

%find_lang kwin --with-html --all-name
grep "%{_kf5_docdir}" kwin.lang > %{name}-doc.lang

# temporary(?) hack to allow initial-setup to use /usr/bin/kwin too
ln -s kwin_x11 %{buildroot}%{_bindir}/kwin


%check
%if 0%{?tests}
# using low timeout to avoid extending buildtimes too much for now -- rex
export CTEST_OUTPUT_ON_FAILURE=1
xvfb-run -a \
dbus-launch --exit-with-session \
make test ARGS="--output-on-failure --timeout 10" -C %{_target_platform} ||:
%endif

%files
%doc README.md
%license COPYING
%{_bindir}/kwin
%{_bindir}/kwin_x11
%{_kf5_datadir}/kconf_update/

%files common
%{_datadir}/kwin
%{_kf5_qtplugindir}/*.so
%{_kf5_qtplugindir}/kwin/
%{_kf5_qtplugindir}/kcms/
%{_kf5_qtplugindir}/kf5/
%{_kf5_qtplugindir}/org.kde.kdecoration2/*.so
%{_kf5_qtplugindir}/org.kde.kwin.platforms/
%{_kf5_qtplugindir}/kpackage/packagestructure/kwin_packagestructure*.so
%{_kf5_qtplugindir}/org.kde.kwin.scenes/*.so
%{_qt5_qmldir}/org/kde/kwin
%{_kf5_libdir}/kconf_update_bin/kwin5_update_default_rules
%{_libexecdir}/kwin_killer_helper
%{_libexecdir}/kwin_rules_dialog
%{_datadir}/kconf_update/kwin.upd
%{_datadir}/kconf_update/kwin-5.18-move-animspeed.py
%{_kf5_datadir}/kservices5/*.desktop
%{_kf5_datadir}/kservices5/kwin
%{_kf5_datadir}/kservicetypes5/*.desktop
%{_kf5_datadir}/kpackage/kcms/kcm_*
%{_kf5_datadir}/knotifications5/kwin.notifyrc
%{_kf5_datadir}/config.kcfg/*.kcfg
%{_datadir}/icons/hicolor/*/apps/kwin.*
%{_datadir}/knsrcfiles/*.knsrc

%files wayland
%{_kf5_bindir}/kwin_wayland
%{_kf5_qtplugindir}/platforms/KWinQpaPlugin.so
%{_kf5_qtplugindir}/org.kde.kglobalaccel5.platforms/KF5GlobalAccelPrivateKWin.so
%{_kf5_qtplugindir}/org.kde.kwin.waylandbackends/KWinWaylandDrmBackend.so
%{_kf5_qtplugindir}/org.kde.kwin.waylandbackends/KWinWaylandFbdevBackend.so
%{_kf5_qtplugindir}/org.kde.kwin.waylandbackends/KWinWaylandWaylandBackend.so
%{_kf5_qtplugindir}/org.kde.kwin.waylandbackends/KWinWaylandX11Backend.so
%{_kf5_qtplugindir}/org.kde.kwin.waylandbackends/KWinWaylandVirtualBackend.so
%{_kf5_plugindir}/org.kde.kidletime.platforms/KF5IdleTimeKWinWaylandPrivatePlugin.so

%ldconfig_scriptlets libs

%files libs
%{_libdir}/libkwin.so.*
%{_libdir}/libkwinxrenderutils.so.*
%{_libdir}/libkwineffects.so.*
%{_libdir}/libkwinglutils.so.*
%{_libdir}/libkwin4_effect_builtins.so.*
%{_libdir}/libkcmkwincommon.so.*
%{_qt5_plugindir}/kcms/kcm_kwin_virtualdesktops.so
%{_kf5_datadir}/qlogging-categories5/*categories

%files devel
%{_datadir}/dbus-1/interfaces/*.xml
%{_libdir}/cmake/KWinDBusInterface
%{_libdir}/libkwinxrenderutils.so
%{_libdir}/libkwineffects.so
%{_libdir}/libkwinglutils.so
%{_libdir}/libkwin4_effect_builtins.so
%{_includedir}/kwin*.h

%files doc -f %{name}-doc.lang


%changelog
* Sun Sep 27 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.19.1-2
- rebuild

* Tue Jun 16 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.19.1-1
- 5.19.1

* Sun Jun 14 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.18.1-3
- rebuild

* Fri Jun 12 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.18.1-2
- rebuild

* Wed May 06 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.18.1-1
- 5.18.1

* Fri Apr 17 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.18.0-1
- spec copied from kwin-lowlatency, changelog trimmed

