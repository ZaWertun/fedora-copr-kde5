# uncomment to enable bootstrap mode
#global bootstrap 1

# Control wayland by default
%if (0%{?fedora} && 0%{?fedora} < 34) || (0%{?rhel} && 0%{?rhel} < 9)
%bcond_with wayland_default
%else
%bcond_without wayland_default
%endif

Name:    kwin-lowlatency
Version: 5.20.5
Release: 1%{?dist}
Summary: KDE Window manager with stutter and latency reductions

Provides:  kwin = %{version}
Conflicts: kwin
Conflicts: kwinft

# all sources are effectively GPLv2+, except for:
# scripts/enforcedeco/contents/code/main.js
# KDE e.V. may determine that future GPL versions are accepted
License: GPLv2 or GPLv3
URL:     https://github.com/tildearrow/kwin-lowlatency

%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global majmin_ver %(echo %{version} | cut -d. -f1,2).50
%global stable unstable
%else
%global majmin_ver %(echo %{version} | cut -d. -f1,2)
%global stable stable
%endif
Source0: http://download.kde.org/%{stable}/plasma/%(echo %{version} |cut -d. -f1-3)/kwin-%{version}.tar.xz

Patch0:  kwin-lowlatency-%(echo %{version} |cut -d. -f1-3).patch

## upstream patches

# Base
BuildRequires:  extra-cmake-modules
BuildRequires:  kf5-rpm-macros

# Qt
BuildRequires:  qt5-qtbase-devel
BuildRequires:  qt5-qtbase-static
# KWinQpaPlugin (and others?)
BuildRequires:  qt5-qtbase-private-devel
BuildRequires:  qt5-qtsensors-devel
BuildRequires:  qt5-qtscript-devel
BuildRequires:  qt5-qttools-devel
BuildRequires:  qt5-qttools-static
BuildRequires:  qt5-qtx11extras-devel

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
BuildRequires:  pkgconfig(libpipewire-0.3)
BuildRequires:  qt5-qtwayland-devel
BuildRequires:  wayland-protocols-devel

# Systemd
BuildRequires:  systemd

# KF5
BuildRequires:  kf5-kcompletion-devel
BuildRequires:  kf5-kconfig-devel
BuildRequires:  kf5-kconfigwidgets-devel
BuildRequires:  kf5-kcoreaddons-devel
BuildRequires:  kf5-kcrash-devel
BuildRequires:  kf5-kglobalaccel-devel
BuildRequires:  kf5-ki18n-devel
BuildRequires:  kf5-kinit-devel >= 5.10.0-3
BuildRequires:  kf5-kio-devel
BuildRequires:  kf5-knotifications-devel
BuildRequires:  kf5-kservice-devel
BuildRequires:  kf5-plasma-devel
BuildRequires:  kf5-kwidgetsaddons-devel
BuildRequires:  kf5-kwindowsystem-devel
BuildRequires:  kf5-kdoctools-devel
BuildRequires:  kf5-kcmutils-devel
BuildRequires:  kf5-knewstuff-devel
BuildRequires:  kf5-kactivities-devel
BuildRequires:  kf5-kdoctools-devel
BuildRequires:  kf5-kdeclarative-devel
BuildRequires:  kf5-kiconthemes-devel
BuildRequires:  kf5-kidletime-devel
BuildRequires:  kf5-ktextwidgets-devel
BuildRequires:  kf5-kirigami2-devel

BuildRequires:  kdecoration-devel >= %{majmin_ver}
BuildRequires:  kscreenlocker-devel >= %{majmin_ver}
BuildRequires:  plasma-breeze-devel >= %{majmin_ver}
BuildRequires:  kwayland-server-devel >= %{majmin_ver}

%if 0%{?tests}
BuildRequires: dbus-x11
BuildRequires: openbox
BuildRequires: xorg-x11-server-Xvfb
%endif

## Runtime deps
Requires:       %{name}-libs%{?_isa} = %{version}-%{release}
Requires:       %{name}-common%{?_isa} = %{version}-%{release}
Requires:       kdecoration%{?_isa} >= %{majmin_ver}
Requires:       kscreenlocker%{?_isa} >= %{majmin_ver}

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
# until initial-setup is fixed... (#1197135)
Provides: firstboot(windowmanager) = kwin

# Split of X11 variant into subpackage
Obsoletes: kwin-lowlatency < 5.20.0

%if ! %{with wayland_default}
Recommends: %{name}-wayland = %{version}-%{release}
Requires:   %{name}-x11 = %{version}-%{release}
%else
Requires:   %{name}-wayland = %{version}-%{release}
Recommends: %{name}-x11 = %{version}-%{release}
%endif

%description
%{summary}.

%package        wayland
Summary:        KDE Window Manager with Wayland support
Requires:       %{name}-libs%{?_isa} = %{version}-%{release}
Requires:       %{name}-common%{?_isa} = %{version}-%{release}
Requires:       kwayland-integration%{?_isa} >= %{majmin_ver}
%if ! 0%{?bootstrap}
BuildRequires:  xorg-x11-server-Xwayland
%endif
Requires:       xorg-x11-server-Xwayland
# http://bugzilla.redhat.com/605675
Provides:       firstboot(windowmanager) = kwin_wayland
# KWinQpaPlugin (and others?)
# libkdeinit5_kwin*
%{?kf5_kinit_requires}
# kwin-lowlatency <\>
Provides:       kwin-wayland = %{version}
Conflicts:      kwin-wayland
# </>
# Obsolete kwin-wayland-nvidia package as this is now done automatically
# by kwin-wayland
Obsoletes:      %{name}-wayland-nvidia <= 5.20.3
Provides:       %{name}-wayland-nvidia = %{version}-%{release}
%description    wayland
%{summary}.

%package        x11
Summary:        KDE Window Manager with X11 support
Requires:       %{name}-libs%{?_isa} = %{version}-%{release}
Requires:       %{name}-common%{?_isa} = %{version}-%{release}
%if ! 0%{?bootstrap}
BuildRequires:  xorg-x11-server-Xorg
%endif
Requires:       xorg-x11-server-Xorg
# http://bugzilla.redhat.com/605675
Provides:       firstboot(windowmanager) = kwin_x11
# KWinX11Platform (and others?)
# libkdeinit5_kwin*
%{?kf5_kinit_requires}
%description    x11
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
%autosetup -p1 -n kwin-%{version}

sed -i \
  -e 's|^find_package(Breeze ${PROJECT_VERSION} CONFIG)|find_package(Breeze 5.9 CONFIG)|' \
  CMakeLists.txt


%build
%cmake_kf5
%cmake_build


%install
%cmake_install

%find_lang kwin --with-html --all-name
grep "%{_kf5_docdir}" kwin.lang > %{name}-doc.lang
cat kwin.lang %{name}-doc.lang | sort | uniq -u > kwin5.lang
echo >> kwin5.lang

%if ! %{with wayland_default}
# temporary(?) hack to allow initial-setup to use /usr/bin/kwin too
ln -s kwin_x11 %{buildroot}%{_bindir}/kwin
%else
# temporary(?) hack to allow initial-setup to use /usr/bin/kwin too
ln -s kwin_wayland %{buildroot}%{_bindir}/kwin
%endif


%check
%if 0%{?tests}
# using low timeout to avoid extending buildtimes too much for now -- rex
export CTEST_OUTPUT_ON_FAILURE=1
xvfb-run -a \
dbus-launch --exit-with-session \
make test ARGS="--output-on-failure --timeout 10" -C %{_target_platform} ||:
%endif

%files
%{_bindir}/kwin

%files common -f kwin5.lang
%{_datadir}/kwin
%{_kf5_qtplugindir}/*.so
%{_kf5_qtplugindir}/kwin/
%{_kf5_qtplugindir}/kcms/
%{_kf5_qtplugindir}/kf5/
%{_kf5_qtplugindir}/org.kde.kdecoration2/*.so
%{_kf5_qtplugindir}/kpackage/packagestructure/kwin_packagestructure*.so
%{_kf5_qtplugindir}/org.kde.kwin.scenes/*.so
%{_qt5_qmldir}/org/kde/kwin
%{_kf5_libdir}/kconf_update_bin/kwin5_update_default_rules
%{_libexecdir}/kwin_killer_helper
%{_libexecdir}/kwin_rules_dialog
%{_datadir}/kconf_update/kwin.upd
%{_datadir}/kconf_update/kwin-5.16-auto-bordersize.sh
%{_datadir}/kconf_update/kwin-5.18-move-animspeed.py
%{_kf5_datadir}/kservices5/*.desktop
%{_kf5_datadir}/kservices5/kwin
%{_kf5_datadir}/kservicetypes5/*.desktop
%{_kf5_datadir}/kpackage/kcms/kcm_*
%{_kf5_datadir}/knotifications5/kwin.notifyrc
%{_kf5_datadir}/config.kcfg/kwin.kcfg
%{_kf5_datadir}/config.kcfg/kwindecorationsettings.kcfg
%{_kf5_datadir}/config.kcfg/virtualdesktopssettings.kcfg
%{_kf5_datadir}/config.kcfg/kwin_colorcorrect.kcfg
%{_kf5_datadir}/kconf_update/kwinrules-5.19-placement.pl
%{_kf5_datadir}/kconf_update/kwinrules.upd
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
%{_userunitdir}/plasma-kwin_wayland.service

%files x11
%{_kf5_bindir}/kwin_x11
%{_kf5_qtplugindir}/org.kde.kwin.platforms/KWinX11Platform.so
%{_userunitdir}/plasma-kwin_x11.service

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
%license LICENSES/*.txt


%changelog
* Tue Jan  5 22:11:42 MSK 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.20.5-1
- 5.20.5

* Sat Dec  5 13:32:37 MSK 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.20.4-2
- rebuild

* Tue Dec  1 22:31:39 MSK 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.20.4-1
- 5.20.4

* Wed Nov 11 12:32:27 MSK 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.20.3-1
- 5.20.3

* Wed Oct 21 01:59:07 MSK 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.20.1-1
- 5.20.1

* Thu Oct 15 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.20.0-2
- kwin_x11 moved to separate package

* Tue Oct 13 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.20.0-1
- 5.20.0 (patch from tag v5.20.0)

* Sun Sep 27 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.19.5-2
- rebuild

* Tue Sep 01 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.19.5-1
- 5.19.5

* Tue Jul 28 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.19.4-1
- 5.19.4

* Tue Jul 07 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.19.3-1
- 5.19.3

* Wed Jun 24 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.19.2-1
- 5.19.2

* Fri Jun 19 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.19.1-1
- 5.19.1

* Tue Jun 16 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.19.0-1
- 5.19.0

* Sun Jun 14 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.18.5-3
- rebuild

* Fri Jun 12 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.18.5-2
- rebuild

* Wed May 06 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.18.5-1
- 5.18.5

* Wed Apr 08 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.18.4.1-1
- version 5.18.4.1

* Wed Apr 08 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.18.4-2
- patch 5.18.4-2 without april fools joke

* Wed Apr 01 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.18.4-1
- 5.18.4

* Wed Mar 11 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.18.3-1
- 5.18.3

* Wed Feb 26 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.18.2-1
- 5.18.2

* Wed Feb 19 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.18.1-1
- 5.18.1

* Wed Feb 12 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.18.0-1
- 5.18.0

* Thu Jan 09 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.17.5-1
- 5.17.5

* Sat Dec 21 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.17.4-2
- rebuild

* Tue Dec 03 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.17.4-1
- 5.17.4

* Mon Nov 25 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.17.3-3
- rebuild

* Wed Nov 13 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.17.3-2
- GTK_FRAME_EXTENTS.patch returned

* Tue Nov 12 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.17.3-1
- 5.17.3

* Wed Oct 30 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.17.2-1
- 5.17.2

* Wed Oct 23 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.17.1-1
- 5.17.1

* Tue Oct 15 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.17.0-1
- 5.17.0

* Sun Oct 06 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.16.5-2
- rebuild with new qt5

* Wed Sep 04 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.16.5-1
- 5.16.5

* Wed Jul 31 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.16.4-1
- 5.16.4

* Wed Jul 17 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.16.3-2
- applied patch to support _GTK_FRAME_EXTENTS

* Wed Jul 03 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.16.2-2
- rebuild with new qt5

* Tue Jun 25 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.16.2-1
- 5.16.2

* Tue Jun 18 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.16.1-1
- 5.16.1

* Thu Jun 13 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.16.0-1
- 5.16.0

* Tue May 07 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.15.5-1
- 5.15.5

* Sat Apr 27 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.15.4-1
- 5.15.4

* Tue Feb 19 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.14.5-1
- 5.14.5

* Wed Dec 12 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.14.4-2
- rebuild (qt5)

* Tue Nov 27 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.14.4-1
- 5.14.4

* Thu Nov 08 2018 Martin Kyral <martin.kyral@gmail.com> - 5.14.3-1
- 5.14.3

* Wed Oct 24 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.14.2-1
- 5.14.2

* Tue Oct 16 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.14.1-1
- 5.14.1

* Fri Oct 05 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.14.0-1
- 5.14.0

* Fri Sep 21 2018 Jan Grulich <jgrulich@redhat.com> - 5.13.90-2
- rebuild (qt5)

* Fri Sep 14 2018 Martin Kyral <martin.kyral@gmail.com> - 5.13.90-1
- 5.13.90

* Tue Sep 04 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.13.5-1
- 5.13.5

* Fri Aug 24 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.13.4-2
- rebuild

* Thu Aug 02 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.13.4-1
- 5.13.4

* Fri Jul 20 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.13.3-4
- use %%_qt5_qmldir (#1604528)

* Fri Jul 13 2018 Fedora Release Engineering <releng@fedoraproject.org> - 5.13.3-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_29_Mass_Rebuild

* Wed Jul 11 2018 Martin Kyral <martin.kyral@gmail.com> - 5.13.3-1
- 5.13.3

* Mon Jul 09 2018 Martin Kyral <martin.kyral@gmail.com> - 5.13.2-1
- 5.13.2

* Thu Jun 21 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.13.1-2
- rebuild (qt5)

* Tue Jun 19 2018 Martin Kyral <martin.kyral@gmail.com> - 5.13.1-1
- 5.13.1

* Sat Jun 09 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.13.0-1
- 5.13.0

* Sun May 27 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.12.90-2
- rebuild (qt5)

* Fri May 18 2018 Martin Kyral <martin.kyral@gmail.com> - 5.12.90-1
- 5.12.90

* Tue May 01 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.12.5-1
- 5.12.5

* Tue Mar 27 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.12.4-1
- 5.12.4

* Thu Mar 15 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.12.3-2
- -common: add versioned dep on kf5-kwayland (no longer optional)
- use %%make_build %%ldconfig_scriptlets
- BR: libcap-devel

* Tue Mar 06 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.12.3-1
- 5.12.3

* Wed Feb 21 2018 Jan Grulich <jgrulich@redhat.com> - 5.12.2-1
- 5.12.2

* Tue Feb 13 2018 Jan Grulich <jgrulich@redhat.com> - 5.12.1-1
- 5.12.1

* Wed Feb 07 2018 Fedora Release Engineering <releng@fedoraproject.org> - 5.12.0-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_28_Mass_Rebuild

* Sun Feb 04 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.12.0-2
- respin

* Fri Feb 02 2018 Jan Grulich <jgrulich@redhat.com> - 5.12.0-1
- 5.12.0

* Thu Jan 18 2018 Igor Gnatenko <ignatenkobrain@fedoraproject.org> - 5.11.95-2
- Remove obsolete scriptlets

* Mon Jan 15 2018 Jan Grulich <jgrulich@redhat.com> - 5.11.95-1
- 5.11.95

* Tue Jan 02 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.11.5-1
- 5.11.5

* Wed Dec 20 2017 Jan Grulich <jgrulich@redhat.com> - 5.11.4-2
- rebuild (qt5)

* Thu Nov 30 2017 Martin Kyral <martin.kyral@gmail.com> - 5.11.4-1
- 5.11.4

* Mon Nov 27 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.11.3-2
- rebuild (qt5)

* Wed Nov 08 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.11.3-1
- 5.11.3

* Wed Oct 25 2017 Martin Kyral <martin.kyral@gmail.com> - 5.11.2-1
- 5.11.2

* Tue Oct 17 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.11.1-1
- 5.11.1

* Wed Oct 11 2017 Martin Kyral <martin.kyral@gmail.com> - 5.11.0-1
- 5.11.0

* Wed Oct 11 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.10.5-3
- confirmed only -wayland uses private api

* Tue Oct 10 2017 Jan Grulich <jgrulich@redhat.com> - 5.10.5-2
- rebuild (qt5)

* Thu Aug 24 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.10.5-1
- 5.10.5

* Thu Aug 03 2017 Fedora Release Engineering <releng@fedoraproject.org> - 5.10.4-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Binutils_Mass_Rebuild

* Wed Jul 26 2017 Fedora Release Engineering <releng@fedoraproject.org> - 5.10.4-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Mass_Rebuild

* Thu Jul 20 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.10.4-1
- 5.10.4

* Wed Jul 19 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.10.3.1-2
- rebuild (qt5)

* Mon Jul 03 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.10.3.1-1
- kwin-5.10.3.1

* Mon Jul 03 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.10.3-3
- respin

* Sun Jul 02 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.10.3-2
- enable tests, support %%bootstrap, update URL

* Tue Jun 27 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.10.3-1
- 5.10.3

* Thu Jun 15 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.10.2-1
- 5.10.2

* Tue Jun 06 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.10.1-1
- 5.10.1

* Wed May 31 2017 Jan Grulich <jgrulich@redhat.com> - 5.10.0-1
- 5.10.0

* Thu May 11 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.9.5-3
- rebuild (qt5)

* Wed Apr 26 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.9.5-2
- -doc: use %%find_lang --with-html

* Wed Apr 26 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.9.5-1
- 5.9.5

* Fri Mar 31 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.9.4-2
- rebuild (qt5), update URL

* Thu Mar 23 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.9.4-1
- 5.9.4

* Sat Mar 04 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.9.3-2
- rebuild

* Wed Mar 01 2017 Jan Grulich <jgrulich@redhat.com> - 5.9.3-1
- 5.9.3

* Tue Feb 21 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.8.6-1
- 5.8.6

* Fri Feb 10 2017 Fedora Release Engineering <releng@fedoraproject.org> - 5.8.5-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_26_Mass_Rebuild

* Wed Dec 28 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.8.5-1
- 5.8.5

* Thu Dec 15 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.8.4-3
- rebuild (qt5)

* Sat Dec 03 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.8.4-2
- rebuild (qt5)

* Tue Nov 22 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.8.4-1
- 5.8.4

* Thu Nov 17 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.8.3-2
- release++

* Thu Nov 17 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.8.3-1.2
- branch rebuild (qt5)

* Tue Nov 01 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.8.3-1
- 5.8.3

* Mon Oct 31 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.8.2-2
- pull in upstream fixes

* Tue Oct 18 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.8.2-1
- 5.8.2

* Tue Oct 11 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.8.1-1
- 5.8.1

* Thu Sep 29 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.8.0-1
- 5.8.0

* Thu Sep 22 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.7.95-1
- 5.7.95

* Tue Sep 13 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.7.5-1
- 5.7.5

* Tue Aug 23 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.7.4-1
- 5.7.4

* Tue Aug 02 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.7.3-2
- patch-n-relax breeze verision

* Tue Aug 02 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.7.3-1
- 5.7.3

* Fri Jul 22 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.7.2-2
- BR: plasma-breeze-devel

* Tue Jul 19 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.7.2-1
- 5.7.2

* Tue Jul 19 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.7.1-5
- rebuild (qt5)

* Fri Jul 15 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.7.1-4
- add versioned qt5 dep in main pkg too

* Thu Jul 14 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.7.1-3
- -wayland: Requires: xorg-x11-server-Xwayland

* Thu Jul 14 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.7.1-2
- -wayland: KWinQpaPlugin uses private Qt5 apis, BR: qt5-qtbase-private-devel

* Tue Jul 12 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.7.1-1
- 5.7.1

* Thu Jun 30 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.7.0-1
- 5.7.0

* Sat Jun 25 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.6.95-1
- 5.6.95

* Tue Jun 14 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.6.5-1
- 5.6.5

* Sat May 14 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.6.4-1
- 5.6.4

* Wed Apr 20 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.6.3-2
- tighten kscreenlocker, kdecoration runtime deps (#1328942)
- -wayland: relax kwayland-integration runtime dep

* Tue Apr 19 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.6.3-1
- 5.6.3

* Sat Apr 09 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.6.2-1
- 5.6.2

* Fri Apr 08 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.6.1-1
- 5.6.1

* Tue Mar 01 2016 Daniel Vrátil <dvratil@fedoraproject.org> - 5.5.5-1
- Plasma 5.5.5

* Thu Feb 04 2016 Fedora Release Engineering <releng@fedoraproject.org> - 5.5.4-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_24_Mass_Rebuild

* Wed Jan 27 2016 Daniel Vrátil <dvratil@fedoraproject.org> - 5.5.4-1
- Plasma 5.5.4

* Thu Jan 14 2016 Rex Dieter <rdieter@fedoraproject.org> 5.5.3-2
- -BR: cmake, use %%license

* Thu Jan 07 2016 Daniel Vrátil <dvratil@fedoraproject.org> - 5.5.3-1
- Plasma 5.5.3

* Thu Dec 31 2015 Rex Dieter <rdieter@fedoraproject.org> 5.5.2-2
- update URL, use %%majmin_ver for more plasma-related deps

* Thu Dec 31 2015 Rex Dieter <rdieter@fedoraproject.org> - 5.5.2-1
- 5.5.2

* Fri Dec 18 2015 Daniel Vrátil <dvratil@fedoraproject.org> - 5.5.1-1
- Plasma 5.5.1

* Thu Dec 03 2015 Daniel Vrátil <dvratil@fedoraproject.org> - 5.5.0-1
- Plasma 5.5.0

* Wed Nov 25 2015 Daniel Vrátil <dvratil@fedoraproject.org> - 5.4.95-1
- Plasma 5.4.95

* Thu Nov 05 2015 Daniel Vrátil <dvratil@fedoraproject.org> - 5.4.3-1
- Plasma 5.4.3

* Sat Oct 24 2015 Rex Dieter <rdieter@fedoraproject.org> 5.4.2-4
- respin (rawhide)

* Fri Oct 23 2015 Rex Dieter <rdieter@fedoraproject.org> 5.4.2-3
- latest batch of upstream fixes (kde#344278,kde#354164,kde#351763,kde#354090)

* Tue Oct 20 2015 Rex Dieter <rdieter@fedoraproject.org> 5.4.2-2
- .spec cosmetics, backport kwin/aurorae crasher fix (kde#346857)

* Thu Oct 01 2015 Rex Dieter <rdieter@fedoraproject.org> - 5.4.2-1
- 5.4.2

* Thu Oct 01 2015 Rex Dieter <rdieter@fedoraproject.org> 5.4.1-3
- tigthen kdecorration-devel dep

* Thu Oct 01 2015 Rex Dieter <rdieter@fedoraproject.org> 5.4.1-2
- -devel: move dbus xml interface files here

* Wed Sep 09 2015 Rex Dieter <rdieter@fedoraproject.org> - 5.4.1-1
- 5.4.1

* Wed Sep 02 2015 Rex Dieter <rdieter@fedoraproject.org> 5.4.0-4
- versioned kf5-kwayland dep too
- make kwayland-integration dep arch'd

* Wed Sep 02 2015 Rex Dieter <rdieter@fedoraproject.org> 5.4.0-3
- add versioned Requires: kwin-libs dep to main pkg

* Tue Aug 25 2015 Daniel Vrátil <dvratil@redhat.com> - 5.4.0-2
- add upstream patch to fix crash
- make sure kwayland-integration is installed for kwin-wayland

* Fri Aug 21 2015 Daniel Vrátil <dvratil@redhat.com> - 5.4.0-1
- Plasma 5.4.0

* Thu Aug 13 2015 Daniel Vrátil <dvratil@redhat.com> - 5.3.95-1
- Plasma 5.3.95

* Thu Jun 25 2015 Daniel Vrátil <dvratil@redhat.com> - 5.3.2-1
- Plasma 5.3.2

* Wed Jun 17 2015 Rex Dieter <rdieter@fedoraproject.org> 5.3.1-4
- BR: kf5-kcompletion-devel kf5-kiconthemes-devel kf5-kio-devel

* Wed Jun 17 2015 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 5.3.1-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_23_Mass_Rebuild

* Tue May 26 2015 Daniel Vrátil <dvratil@redhat.com> - 5.3.1-1
- Plasma 5.3.1

* Tue May 19 2015 Rex Dieter <rdieter@fedoraproject.org> 5.3.0-5
- move dbus xml files to -libs (so present for -devel)

* Sun May 17 2015 Rex Dieter <rdieter@fedoraproject.org> - 5.3.0-4
- followup SM fix, discard support (kde#341930)
- note qt5-qtmultimedia dep is runtime-only

* Thu May 14 2015 Rex Dieter <rdieter@fedoraproject.org> - 5.3.0-3
- test candidate SM fixes (reviewboard#123580,kde#341930)
- move libkdeinit bits out of -libs
- move dbus interface xml to runtime pkg
- drop %%config from knsrc files
- enable wayland support (f21+)
- .spec cosmetics

* Wed Apr 29 2015 Jan Grulich <jgrulich@redhat.com> - 5.3.0-2
- BR xcb-util-cursor-devel

* Mon Apr 27 2015 Daniel Vrátil <dvratil@redhat.com> - 5.3.0-1
- Plasma 5.3.0

* Wed Apr 22 2015 Daniel Vrátil <dvratil@redhat.com> - 5.2.95-1
- Plasma 5.2.95

* Tue Apr 07 2015 Rex Dieter <rdieter@fedoraproject.org> 5.2.2-2
- tarball respin

* Fri Mar 20 2015 Daniel Vrátil <dvratil@redhat.com> - 5.2.2-1
- Plasma 5.2.2

* Fri Feb 27 2015 Daniel Vrátil <dvratil@redhat.com> - 5.2.1-4
- Rebuild (GCC 5)

* Fri Feb 27 2015 Rex Dieter <rdieter@fedoraproject.org> - 5.2.1-3
- Provide /usr/bin/kwin too (#1197135)
- bump plasma_version macro

* Fri Feb 27 2015 Rex Dieter <rdieter@fedoraproject.org> 5.2.1-2
- Provides: firstboot(windowmanager) = kwin_x11  (#605675)

* Tue Feb 24 2015 Daniel Vrátil <dvratil@redhat.com> - 5.2.1-1
- Plasma 5.2.1

* Sun Feb 08 2015 Daniel Vrátil <dvratli@redhat.com> - 5.2.0.1-2
- Obsoletes: kwin-gles, kwin-gles-libs

* Wed Jan 28 2015 Daniel Vrátil <dvratil@redhat.com> - 5.2.0.1-1
- Update to upstream hotfix release 5.2.0.1 (kwindeco KCM bugfix)

* Wed Jan 28 2015 Daniel Vrátil <dvratil@redhat.com> - 5.2.0-3
- add upstream patch for bug #341971 - fixes Window decorations KCM

* Tue Jan 27 2015 Daniel Vrátil <dvratil@redhat.com> - 5.2.0-2
- -doc: Don't require arch-specific kwin in noarch package

* Mon Jan 26 2015 Daniel Vrátil <dvratil@redhat.com> - 5.2.0-1
- Plasma 5.2.0

* Mon Jan 12 2015 Daniel Vrátil <dvratil@redhat.com> - 5.1.95-1.beta
- Plasma 5.1.95 Beta

* Wed Dec 17 2014 Daniel Vrátil <dvratil@redhat.com> - 5.1.2-2
- Plasma 5.1.2

* Tue Nov 18 2014 Daniel Vrátil <dvratil@redhat.com> - 5.1.1-3
- Fixed license
- Fixed scriptlets
- Fixed Conflicts in -devel
- -docs is noarch

* Wed Nov 12 2014 Daniel Vrátil <dvratil@redhat.com> - 5.1.1-2
- added optional Wayland support

* Fri Nov 07 2014 Daniel Vrátil <dvratil@redhat.com> - 5.1.1-1
- Plasma 5.1.1

* Tue Oct 14 2014 Daniel Vrátil <dvratil@redhat.com> - 5.1.0.1-1
- Plasma 5.1.0.1

* Thu Oct 09 2014 Daniel Vrátil <dvratil@redhat.com> - 5.1.0-1
- Plasma 5.1.0

* Tue Sep 16 2014 Daniel Vrátil <dvratil@redhat.com> - 5.0.2-1
- Plasma 5.0.2

* Sun Aug 10 2014 Daniel Vrátil <dvratil@redhat.com> - 5.0.1-1
- Plasma 5.0.1

* Wed Jul 16 2014 Daniel Vrátil <dvratil@redhat.com> 5.0.0-1
- Plasma 5.0.0

* Wed May 14 2014 Daniel Vrátil <dvratil@redhat.com> 4.96.0-1.20140514git61c631c
- Update to latest upstream git snapshot

* Fri Apr 25 2014 Daniel Vrátil <dvratil@redhat.com> 4.95.0-1.20140425gitb92f4a6
- Initial package
