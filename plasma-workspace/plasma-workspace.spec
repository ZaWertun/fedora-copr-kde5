# Set (as 1) to enable bootstrap when building plasma-workspace on a new
# repo or arch where there's no package that would provide plasmashell
#global bootstrap 1

%global kf5_version_min 5.50.0

Name:    plasma-workspace
Summary: Plasma workspace, applications and applets
Version: 5.19.0
Release: 1%{?dist}

License: GPLv2+
URL:     https://cgit.kde.org/%{name}.git

%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global majmin_ver %(echo %{version} | cut -d. -f1,2).50
%global stable unstable
%else
%global majmin_ver %(echo %{version} | cut -d. -f1,2)
%global stable stable
%endif
Source0: http://download.kde.org/%{stable}/plasma/%{version}/%{name}-%{version}.tar.xz

# filter qml/plugins provides
%global __provides_exclude_from ^(%{_kf5_qmldir}/.*\\.so|%{_kf5_qtplugindir}/.*\\.so)$

# This goes to PAM
# TODO: this should arguably be in kde-settings with the other pam-related configs
Source10:       kde
Source15:       fedora.desktop

# breeze fedora sddm theme components
# includes f25-based preview (better than breeze or nothing at least)
Source20:       breeze-fedora-0.2.tar.gz

## downstream Patches
Patch100:       plasma-workspace-5.12.5-konsole-in-contextmenu.patch
Patch101:       plasma-workspace-5.3.0-set-fedora-default-look-and-feel.patch
# remove stuff we don't want or need, plus a minor bit of customization --rex
#Patch102:       startkde.patch
# default to folderview (instead of desktop) containment, see also
# https://mail.kde.org/pipermail/distributions/2016-July/000133.html
# and example,
# https://github.com/notmart/artwork-lnf-netrunner-core/blob/master/usr/share/plasma/look-and-feel/org.kde.netrunner-core.desktop/contents/defaults
Patch105:       plasma-workspace-5.7.3-folderview_layout.patch
# workaround https://bugzilla.redhat.com/show_bug.cgi?id=1754395
Patch106:	plasma-workspace-5.18.4.1-filter-environment-v2.patch

## upstreamable Patches

## upstream Patches

# udev
BuildRequires:  zlib-devel
BuildRequires:  dbusmenu-qt5-devel
BuildRequires:  libGL-devel
BuildRequires:  mesa-libGLES-devel
BuildRequires:  libSM-devel
BuildRequires:  libX11-devel
BuildRequires:  libXau-devel
BuildRequires:  libXdmcp-devel
BuildRequires:  libxkbfile-devel
BuildRequires:  libXcomposite-devel
BuildRequires:  libXdamage-devel
BuildRequires:  libXrender-devel
BuildRequires:  libXfixes-devel
BuildRequires:  libXrandr-devel
BuildRequires:  libXcursor-devel
BuildRequires:  libXtst-devel
BuildRequires:  libxcb-devel
BuildRequires:  xcb-util-keysyms-devel
BuildRequires:  xcb-util-image-devel
BuildRequires:  xcb-util-renderutil-devel
BuildRequires:  xcb-util-wm-devel
BuildRequires:  xcb-util-devel
BuildRequires:  glib2-devel
BuildRequires:  fontconfig-devel
BuildRequires:  boost-devel
BuildRequires:  libusb-devel
BuildRequires:  libbsd-devel
BuildRequires:  pam-devel
BuildRequires:  lm_sensors-devel
BuildRequires:  pciutils-devel
%ifnarch s390 s390x
BuildRequires:  libraw1394-devel
%endif
BuildRequires:  gpsd-devel
BuildRequires:  libqalculate-devel
%global kf5_pim 1
BuildRequires:  kf5-kholidays-devel
BuildRequires:  kf5-prison-devel

BuildRequires:  qt5-qtbase-devel >= 5.7.0
BuildRequires:  qt5-qtx11extras-devel
BuildRequires:  qt5-qtscript-devel
BuildRequires:  qt5-qtdeclarative-devel
BuildRequires:  qt5-qtwebkit-devel
BuildRequires:  phonon-qt5-devel

BuildRequires:  kf5-rpm-macros >= %{kf5_version_min}
BuildRequires:  extra-cmake-modules
BuildRequires:  kf5-baloo-devel >= %{kf5_version_min}
BuildRequires:  kf5-kactivities-stats-devel >= %{kf5_version_min}
BuildRequires:  kf5-kcmutils-devel >= %{kf5_version_min}
BuildRequires:  kf5-kcrash-devel >= %{kf5_version_min}
BuildRequires:  kf5-kdeclarative-devel >= %{kf5_version_min}
BuildRequires:  kf5-kdelibs4support-devel >= %{kf5_version_min}
BuildRequires:  kf5-kdesu-devel >= %{kf5_version_min}
BuildRequires:  kf5-kdewebkit-devel >= %{kf5_version_min}
BuildRequires:  kf5-kdoctools-devel >= %{kf5_version_min}
BuildRequires:  kf5-kglobalaccel-devel >= %{kf5_version_min}
BuildRequires:  kf5-kidletime-devel >= %{kf5_version_min}
BuildRequires:  kf5-kinit-devel >= %{kf5_version_min}
BuildRequires:  kf5-kjsembed-devel >= %{kf5_version_min}
BuildRequires:  kf5-knewstuff-devel >= %{kf5_version_min}
BuildRequires:  kf5-knotifyconfig-devel >= %{kf5_version_min}
BuildRequires:  kf5-kpeople-devel >= %{kf5_version_min}
BuildRequires:  kf5-krunner-devel >= %{kf5_version_min}
BuildRequires:  kf5-ktexteditor-devel >= %{kf5_version_min}
BuildRequires:  kf5-ktextwidgets-devel >= %{kf5_version_min}
BuildRequires:  kf5-kwallet-devel >= %{kf5_version_min}
BuildRequires:  kf5-kxmlrpcclient-devel >= %{kf5_version_min}
BuildRequires:  kf5-networkmanager-qt-devel >= %{kf5_version_min}
BuildRequires:  kf5-plasma-devel >= %{kf5_version_min}
Requires:       kf5-plasma%{?_isa} >= %{_kf5_version}
BuildRequires:  kf5-threadweaver-devel >= %{kf5_version_min}
BuildRequires:  kf5-kded-devel >= %{kf5_version_min}

BuildRequires:  kf5-ksysguard-devel >= %{majmin_ver}
BuildRequires:  kf5-kwayland-devel >= %{kf5_version_min}
BuildRequires:  wayland-devel >= 1.3.0
BuildRequires:  libkscreen-qt5-devel >= %{majmin_ver}
BuildRequires:  kscreenlocker-devel >= %{majmin_ver}

BuildRequires:  kwin-devel >= %{majmin_ver}

BuildRequires:  chrpath
BuildRequires:  desktop-file-utils

# Optional
BuildRequires:  kf5-kactivities-devel
%if 0%{?fedora}
BuildRequires:  cmake(AppStreamQt) >= 0.10.4
BuildRequires:  cmake(KUserFeedback)
%endif

# when kded_desktopnotifier.so moved here
Conflicts:      kio-extras < 5.4.0

%if 0%{?fedora} || 0%{?rhel} > 7
Recommends:     %{name}-geolocation = %{version}-%{release}
Suggests:       imsettings-qt
%else
Requires:       %{name}-geolocation = %{version}-%{release}
%endif

Requires:       %{name}-common = %{version}-%{release}
Requires:       %{name}-libs%{?_isa} = %{version}-%{release}
Requires:       libkworkspace5%{?_isa} = %{version}-%{release}

# for libkdeinit5_*
%{?kf5_kinit_requires}
Requires:       kactivitymanagerd >= %{majmin_ver}
Requires:       khotkeys >= %{majmin_ver}
Requires:       kf5-kded
Requires:       kf5-kdoctools
Requires:       qt5-qtquickcontrols
Requires:       qt5-qtgraphicaleffects
Requires:       kf5-filesystem
Requires:       kf5-baloo
Requires:       kf5-kglobalaccel >= 5.7
Requires:       kf5-kxmlrpcclient
Requires:       kf5-kquickcharts

# systemmonitor dataengine
Requires:       ksysguardd >= %{majmin_ver}

# The new volume control for PulseAudio
%if 0%{?fedora} || 0%{?rhel} > 7
Requires:       plasma-pa
%endif

# Without the platformtheme plugins we get broken fonts
Requires:       kf5-frameworkintegration

# For krunner
Requires:       plasma-milou >= %{majmin_ver}

# powerdevil has a versioned dep on libkworkspace5, so (may?)
# need to avoid this dep when bootstrapping
%if ! 0%{?bootstrap}
# Power management
Requires:       powerdevil >= %{majmin_ver}
%endif

# startkde
Requires:       coreutils
Requires:       dbus-x11
Requires:       socat
Requires:       xmessage
Requires:       qt5-qttools

Requires:       xorg-x11-utils
Requires:       xorg-x11-server-utils

Requires:       kde-settings-plasma

# Default look-and-feel theme
%if 0%{?fedora}
%global default_lookandfeel org.fedoraproject.fedora.desktop
Requires:       plasma-lookandfeel-fedora = %{version}-%{release}
%endif
%if ! 0%{?default_lookandfeel:1}
Requires:       desktop-backgrounds-compat
%endif

Requires:       systemd

# Oxygen
# TODO: review if oxygen-fonts, oxygen-icon-theme are still needed (I suspect not) -- rex
#Requires:       oxygen-icon-theme
Requires:       oxygen-sound-theme >= %{majmin_ver}
#Requires:       oxygen-fonts

# PolicyKit authentication agent
Requires:        polkit-kde >= %{majmin_ver}

# Require any plasmashell (plasma-desktop provides plasmashell(desktop))
%if 0%{?bootstrap}
Provides:       plasmashell = %{version}
%else
# Note: We should require >= %%{version}, but that creates a circular dependency
# at build time of plasma-desktop, because it provides the needed dependency, but
# also needs plasma-workspace to build. So for now the dependency is unversioned.
Requires:       plasmashell >= %{majmin_ver}
%endif

# when -common, libkworkspace5 was split out
Obsoletes:      plasma-workspace < 5.4.2-2

# plasmashell provides dbus service org.freedesktop.Notifications
Provides: desktop-notification-daemon

# upgrade path, when sddm-breeze was split out
Obsoletes: plasma-workspace < 5.3.2-8

# digitalclock applet
%if ! 0%{?bootstrap}
BuildRequires: pkgconfig(iso-codes)
%endif
Requires: iso-codes

%description
Plasma 5 libraries and runtime components

%package common
Summary: Common files for %{name}
%description common
%{name}.

%package -n libkworkspace5
Summary: Runtime libkworkspace5 library
# when spilt occurred
Obsoletes: plasma-workspace < 5.4.2-2
Requires:  %{name}-common = %{version}-%{release}
%description -n libkworkspace5
%{summary}.

%package libs
Summary: Runtime libraries for %{name}
# when split out
Obsoletes: plasma-workspace < 5.4.2-2
## omit dep on main pkg for now, means we can avoid pulling in a
## huge amount of deps (including kde4) into buildroot -- rex
#Requires:  %%{name}%%{?_isa} = %%{version}-%%{release}
Requires:  %{name}-common = %{version}-%{release}
# consider splitting out plasma_packagestructure content later
Provides: plasma-packagestructure = %{version}-%{release}
%description libs
%{summary}.

%package        devel
Summary:        Development files for %{name}
Requires:       %{name}-libs%{?_isa} = %{version}-%{release}
Requires:       libkworkspace5%{?_isa} = %{version}-%{release}
%description    devel
The %{name}-devel package contains libraries and header files for
developing applications that use %{name}.

%package        doc
Summary:        Documentation and user manuals for %{name}
License:        GFDL
# switch to noarch
Obsoletes:      plasma-workspace-doc < 5.3.1-2
Requires:       %{name}-common = %{version}-%{release}
BuildArch: noarch
%description    doc
Documentation and user manuals for %{name}.

%package geolocation
Summary: Plasma5 geolocation components
# when split out
Obsoletes: plasma-workspace < 5.4.2-2
Requires: %{name}-geolocation-libs%{?_isa} = %{version}-%{release}
%description geolocation
%{summary}.

%package geolocation-libs
Summary: Plasma5 geolocation runtime libraries
Requires: %{name}-common = %{version}-%{release}
Requires: %{name}-geolocation = %{version}-%{release}
%description geolocation-libs
%{summary}.

%package -n sddm-breeze
Summary:        SDDM breeze theme
# upgrade path, when sddm-breeze was split out
Obsoletes: plasma-workspace < 5.3.2-8
Requires:       kf5-plasma >= %{_kf5_version}
# Background.qml:import QtQuick
Requires:       qt5-qtquickcontrols
# on-screen keyboard
Recommends:     qt5-qtvirtualkeyboard
# QML imports:
# org.kde.plasma.workspace.components
# org.kde.plasma.workspace.keyboardlayout
Requires:       %{name} = %{version}-%{release}
# /usr/share/backgrounds/default.png
%if 0%{?fedora}
BuildRequires:  desktop-backgrounds-compat
Requires:       desktop-backgrounds-compat
%endif
%if 0%{?rhel}
Requires:       system-logos
%endif
BuildArch: noarch
%description -n sddm-breeze
%{summary}.

%package wayland
Summary:        Wayland support for Plasma
Requires:       %{name} = %{version}-%{release}
Requires:       kwin-wayland >= %{majmin_ver}
Requires:       kwayland-integration%{?_isa} >= %{majmin_ver}
Requires:       xorg-x11-server-Xwayland
Requires:       qt5-qtwayland%{?_isa}
# startplasmacompositor deps
Requires:       qt5-qttools
%description wayland
%{summary}.

%package -n plasma-lookandfeel-fedora
Summary:  Fedora look-and-feel for Plasma
Requires: %{name} = %{version}-%{release}
# when switched to noarch
Obsoletes: plasma-lookandfeel-fedora < 5.8.0-5
# https://bugzilla.redhat.com/show_bug.cgi?id=1356890
Obsoletes: f22-kde-theme < 22.4
Obsoletes: f23-kde-theme < 23.1
Obsoletes: f24-kde-theme < 24.6
Obsoletes: f24-kde-theme-core < 5.10.5-2
BuildArch: noarch
%description -n plasma-lookandfeel-fedora
%{summary}.


%prep
%setup -q -a 20

## upstream patches
%patch50 -p1

%patch100 -p1 -b .konsole-in-contextmenu
# FIXME/TODO:  it is unclear whether this is needed or even a good idea anymore -- rex
%if 0%{?default_lookandfeel:1}
%patch101 -p1 -b .set-fedora-default-look-and-feel
sed -i -e "s|@DEFAULT_LOOKANDFEEL@|%{?default_lookandfeel}%{!?default_lookandfeel:org.kde.breeze.desktop}|g" \
  shell/packageplugins/lookandfeel/lookandfeel.cpp
%endif
#%patch102 -p1 -b .startkde
%patch105 -p1
%patch106 -p1 -b .bz1754395

%if 0%{?fedora}
cp -a lookandfeel lookandfeel-fedora
install -m 0644 %{SOURCE15} lookandfeel-fedora/metadata.desktop
cat >> CMakeLists.txt <<EOL
plasma_install_package(lookandfeel-fedora org.fedoraproject.fedora.desktop look-and-feel lookandfeel)
EOL
%endif


# highlight the use of wayland
sed -i.plasmawayland -e "s|Plasma|Plasma (Wayland)|g" login-sessions/plasmawayland.desktop.cmake


%build
mkdir %{_target_platform}
pushd %{_target_platform}
%{cmake_kf5} ..
popd

%make_build -C %{_target_platform}


%install
make install/fast DESTDIR=%{buildroot} -C %{_target_platform}

chrpath --delete %{buildroot}%{_kf5_qtplugindir}/phonon_platform/kde.so

# compat symlink
ln -s startplasma-x11 %{buildroot}%{_kf5_bindir}/startkde

%if 0%{?fedora}
# remove/replace items to be customized
# not sure of (sym)links are safe yet or not -- rex
install -m644 -p \
  %{_datadir}/backgrounds/default.png \
  %{buildroot}%{_datadir}/plasma/look-and-feel/org.fedoraproject.fedora.desktop/contents/components/artwork/background.png
%endif

# make fedora-breeze sddm theme variant.
cp -alf %{buildroot}%{_datadir}/sddm/themes/breeze/ \
        %{buildroot}%{_datadir}/sddm/themes/01-breeze-fedora
# replace items
ln -sf  %{_datadir}/backgrounds/default.png \
        %{buildroot}%{_datadir}/sddm/themes/01-breeze-fedora/components/artwork/background.png
install -m644 -p breeze-fedora/* \
        %{buildroot}%{_datadir}/sddm/themes/01-breeze-fedora/

%if 0%{?fedora} > 30
## customize plasma-lookandfeel-fedora defaults
# from [Wallpaper] Image=Next to Image=Fedora
sed -i -e 's|^Image=.*$|Image=Fedora|g' \
  %{buildroot}%{_kf5_datadir}/plasma/look-and-feel/org.fedoraproject.fedora.desktop/contents/defaults
%endif

# Make kcheckpass work
install -m644 -p -D %{SOURCE10} %{buildroot}%{_sysconfdir}/pam.d/kde

%find_lang all --with-html --all-name

grep "%{_kf5_docdir}" all.lang > %{name}-doc.lang
grep libkworkspace.mo all.lang > libkworkspace5.lang
# any translations not used elsewhere, include in main pkg
cat *.lang | sort | uniq -u > %{name}.lang


%check
desktop-file-validate %{buildroot}%{_kf5_datadir}/applications/plasma-windowed.desktop
desktop-file-validate %{buildroot}%{_kf5_datadir}/applications/org.kde.{klipper,plasmashell,systemmonitor}.desktop


%files common
%license COPYING
%license COPYING.DOC
%license COPYING.LIB

%files -f %{name}.lang
%{_kf5_bindir}/gmenudbusmenuproxy
%{_kf5_bindir}/kcminit
%{_kf5_bindir}/kcminit_startup
%{_kf5_bindir}/klipper
%{_kf5_bindir}/krunner
%{_kf5_bindir}/ksmserver
%{_kf5_bindir}/ksplashqml
%{_kf5_bindir}/plasmashell
%{_kf5_bindir}/plasmawindowed
%{_kf5_bindir}/plasma_session
%{_kf5_bindir}/plasma_waitforname
%{_kf5_bindir}/startkde
%{_kf5_bindir}/startplasma-x11
%{_kf5_bindir}/systemmonitor
%{_kf5_bindir}/xembedsniproxy
%{_kf5_libdir}/libkdeinit5_*.so
%{_kf5_qmldir}/org/kde/*
%{_libexecdir}/baloorunner
%{_libexecdir}/ksmserver-logout-greeter
%{_libexecdir}/ksyncdbusenv
%{_kf5_datadir}/ksplash/
%{_kf5_datadir}/plasma/plasmoids/
%{_kf5_datadir}/plasma/services/
%{_kf5_datadir}/plasma/wallpapers/
%dir %{_kf5_datadir}/plasma/look-and-feel/
%{_kf5_datadir}/plasma/look-and-feel/org.kde.breeze.desktop/
%{_kf5_datadir}/solid/
%{_kf5_datadir}/kstyle/
%{_sysconfdir}/xdg/autostart/*.desktop
%{_datadir}/desktop-directories/*.directory
%{_datadir}/dbus-1/services/*.service
%{_datadir}/knsrcfiles/*.knsrc
%{_kf5_datadir}/kservices5/*.desktop
%{_kf5_datadir}/kservices5/*.protocol
%{_kf5_datadir}/kservicetypes5/*.desktop
%{_kf5_datadir}/knotifications5/*.notifyrc
%{_kf5_datadir}/config.kcfg/*
%{_kf5_datadir}/kio_desktop/
%{_kf5_datadir}/kconf_update/krunnerplugins.upd
%{_kf5_libdir}/kconf_update_bin/krunnerplugins
%{_kf5_metainfodir}/*.xml
%{_kf5_datadir}/applications/org.kde.klipper.desktop
%{_kf5_datadir}/applications/org.kde.plasmashell.desktop
%{_kf5_datadir}/applications/plasma-windowed.desktop
%{_kf5_datadir}/applications/org.kde.systemmonitor.desktop
%{_datadir}/xsessions/plasma.desktop
%{_kf5_bindir}/plasma_waitforname
%{_kf5_datadir}/qlogging-categories5/*.categories
%{_sysconfdir}/xdg/plasmanotifyrc
%{_kf5_datadir}/kpackage/kcms/kcm_translations/*
# PAM
%config(noreplace) %{_sysconfdir}/pam.d/kde
%exclude %{_kf5_datadir}/kservices5/plasma-dataengine-geolocation.desktop
%exclude %{_kf5_datadir}/kservices5/plasma-geolocation-gps.desktop
%exclude %{_kf5_datadir}/kservices5/plasma-geolocation-ip.desktop
%exclude %{_kf5_datadir}/kservicetypes5/plasma-geolocationprovider.desktop
# KUserFeedback
%{_kf5_datadir}/kpackage/kcms/kcm_feedback/*

%files doc -f %{name}-doc.lang

%ldconfig_scriptlets -n libkworkspace5

%files -n libkworkspace5 -f libkworkspace5.lang
%{_libdir}/libkworkspace5.so.5*

%ldconfig_scriptlets libs

%files libs
%{_sysconfdir}/xdg/taskmanagerrulesrc
%{_libdir}/libcolorcorrect.so.*
%{_libdir}/libtaskmanager.so.*
%{_libdir}/libweather_ion.so.*
%{_libdir}/libnotificationmanager.*
# multilib'able plugins
%{_kf5_qtplugindir}/plasma/applets/
%{_kf5_qtplugindir}/plasma/dataengine/
%if 0%{?kf5_pim}
%{_kf5_qtplugindir}/plasmacalendarplugins/
%endif
%{_kf5_qtplugindir}/*.so
%exclude %{_kf5_qtplugindir}/plasma-geolocation-gps.so
%exclude %{_kf5_qtplugindir}/plasma-geolocation-ip.so
%exclude %{_kf5_qtplugindir}/plasma/dataengine/plasma_engine_geolocation.so
%dir %{_kf5_qtplugindir}/phonon_platform/
%{_kf5_qtplugindir}/phonon_platform/kde.so
%{_kf5_qtplugindir}/kpackage/packagestructure/*.so
%{_kf5_plugindir}/kio/*.so
%{_kf5_plugindir}/kded/*.so
%{_qt5_plugindir}/kcms/kcm_translations.so
%{_qt5_plugindir}/kcms/kcm_feedback.so
%{_libdir}/kconf_update_bin/krunnerglobalshortcuts
%{_kf5_qtplugindir}/plasma/containmentactions/plasma_containmentactions_applauncher.so
%{_kf5_qtplugindir}/plasma/containmentactions/plasma_containmentactions_contextmenu.so
%{_kf5_qtplugindir}/plasma/containmentactions/plasma_containmentactions_paste.so
%{_kf5_qtplugindir}/plasma/containmentactions/plasma_containmentactions_switchdesktop.so
%{_kf5_qtplugindir}/plasma/containmentactions/plasma_containmentactions_switchwindow.so
%{_libexecdir}/plasma-sourceenv.sh
%{_libexecdir}/startplasma-waylandsession
%{_datadir}/kconf_update/krunnerglobalshortcuts.upd
%{_datadir}/kglobalaccel/krunner.desktop

%files geolocation
%{_kf5_qtplugindir}/plasma-geolocation-gps.so
%{_kf5_qtplugindir}/plasma-geolocation-ip.so
%{_kf5_qtplugindir}/plasma/dataengine/plasma_engine_geolocation.so
%{_kf5_datadir}/kservices5/plasma-dataengine-geolocation.desktop
%{_kf5_datadir}/kservices5/plasma-geolocation-gps.desktop
%{_kf5_datadir}/kservices5/plasma-geolocation-ip.desktop
%{_kf5_datadir}/kservicetypes5/plasma-geolocationprovider.desktop

%ldconfig_scriptlets geolocation-libs

%files geolocation-libs
%{_libdir}/libplasma-geolocation-interface.so.5*

%files devel
%{_libdir}/libcolorcorrect.so
%{_libdir}/libweather_ion.so
%{_libdir}/libtaskmanager.so
%{_libdir}/libplasma-geolocation-interface.so
%{_libdir}/libkworkspace5.so
%dir %{_includedir}/plasma/
%{_includedir}/colorcorrect/
%{_includedir}/plasma/weather/
%{_includedir}/kworkspace5/
%{_includedir}/plasma/geolocation/
%{_includedir}/taskmanager/
%{_includedir}/notificationmanager/
%{_libdir}/cmake/KRunnerAppDBusInterface/
%{_libdir}/cmake/KSMServerDBusInterface/
%{_libdir}/cmake/LibColorCorrect
%{_libdir}/cmake/LibKWorkspace/
%{_libdir}/cmake/LibTaskManager/
%{_libdir}/cmake/LibNotificationManager/
%{_datadir}/dbus-1/interfaces/*.xml
%{_datadir}/kdevappwizard/templates/ion-dataengine.tar.bz2

%files -n sddm-breeze
%{_datadir}/sddm/themes/breeze/
%{_datadir}/sddm/themes/01-breeze-fedora/
#%config(noreplace) %{_datadir}/sddm/themes/01-breeze-fedora/theme.conf.user

%files wayland
%{_kf5_bindir}/startplasma-wayland
%{_datadir}/wayland-sessions/plasmawayland.desktop

%if 0%{?fedora}
%files -n plasma-lookandfeel-fedora
%{_kf5_datadir}/plasma/look-and-feel/org.fedoraproject.fedora.desktop/
%endif


%changelog
* Tue Jun 09 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.19.0-1
- 5.19.0

* Mon Jun 01 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.18.5-3
- kcm_feedback reenabled

* Mon May 18 2020 Rex Dieter <rdieter@fedoraproject.org> - 5.18.5-2
- (branch) 5.19 backport "Stop multiplying duration values"

* Tue May 05 2020 Jan Grulich <jgrulich@redhat.com> - 5.18.5-1
- 5.18.5

* Thu Apr 09 2020 Rex Dieter <rdieter@fedoraproject.org> - 5.18.4.1-2
- update patch "Qt applications lose system theme if launched via dbus activation" (#1754395)

* Sat Apr 04 2020 Rex Dieter <rdieter@fedoraproject.org> - 5.18.4.1-1
- 5.18.4.1

* Fri Apr 03 2020 Rex Dieter <rdieter@fedoraproject.org> - 5.18.4-2
- patch to workaround "Qt applications lose system theme if launched via dbus activation" (#1754395)

* Tue Mar 31 2020 Jan Grulich <jgrulich@redhat.com> - 5.18.4-1
- 5.18.4

* Thu Mar 19 2020 Rex Dieter <rdieter@fedoraproject.org> - 5.18.3-2
- f31+ plasma-lookandfeel-fedora: default to 'Fedora' wallpaper (#1812293)

* Tue Mar 10 2020 Jan Grulich <jgrulich@redhat.com> - 5.18.3-1
- 5.18.3

* Sun Mar 08 2020 Mukundan Ragavan <nonamedotc@gmail.com> - 5.18.2-2
- rebuild for libqalculate

* Tue Feb 25 2020 Jan Grulich <jgrulich@redhat.com> - 5.18.2-1
- 5.18.2

* Tue Feb 18 2020 Jan Grulich <jgrulich@redhat.com> - 5.18.1-1
- 5.18.1

* Tue Feb 11 2020 Jan Grulich <jgrulich@redhat.com> - 5.18.0-1
- 5.18.0

* Thu Jan 30 2020 Fedora Release Engineering <releng@fedoraproject.org> - 5.17.90-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_32_Mass_Rebuild

* Thu Jan 16 2020 Jan Grulich <jgrulich@redhat.com> - 5.17.90-1
- 5.17.90

* Wed Jan 08 2020 Jan Grulich <jgrulich@redhat.com> - 5.17.5-1
- 5.17.5

* Mon Dec 23 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.17.4-2
- provide compat /usr/bin/startkde symlink (#1785826, #1785973)

* Thu Dec 05 2019 Jan Grulich <jgrulich@redhat.com> - 5.17.4-1
- 5.17.4

* Fri Nov 29 2019 Mukundan Ragavan <nonamedotc@gmail.com> - 5.17.3-2
- rebuild for libqalculate

* Wed Nov 13 2019 Martin Kyral <martin.kyral@gmail.com> - 5.17.3-1
- 5.17.3

* Wed Oct 30 2019 Jan Grulich <jgrulich@redhat.com> - 5.17.2-1
- 5.17.2

* Wed Oct 23 2019 Jan Grulich <jgrulich@redhat.com> - 5.17.1-1
- 5.17.1

* Thu Oct 10 2019 Jan Grulich <jgrulich@redhat.com> - 5.17.0-1
- 5.17.0

* Fri Sep 20 2019 Martin Kyral <martin.kyral@gmail.com> - 5.16.90-1
- 5.16.90

* Fri Sep 06 2019 Martin Kyral <martin.kyral@gmail.com> - 5.16.5-1
- 5.16.5

* Tue Aug 27 2019 Mukundan Ragavan <nonamedotc@gmail.com> - 5.16.4-2
- rebuild for libqalculate

* Tue Jul 30 2019 Martin Kyral <martin.kyral@gmail.com> - 5.16.4-1
- 5.16.4

* Fri Jul 26 2019 Fedora Release Engineering <releng@fedoraproject.org> - 5.16.3-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_31_Mass_Rebuild

* Wed Jul 10 2019 Martin Kyral <martin.kyral@gmail.com> - 5.16.3-1
- 5.16.3

* Wed Jul 03 2019 Björn Esser <besser82@fedoraproject.org> - 5.16.2-2
- Rebuild (gpsd)

* Wed Jun 26 2019 Martin Kyral <martin.kyral@gmail.com> - 5.16.2-1
- 5.16.2

* Tue Jun 18 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.16.1-1
- 5.16.1

* Tue Jun 11 2019 Martin Kyral <martin.kyral@gmail.com> - 5.16.0-1
- 5.16.0

* Sun May 19 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.15.90-3
- de-bootstrap

* Sun May 19 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.15.90-2
- bootstrap build

* Thu May 16 2019 Martin Kyral <martin.kyral@gmail.com> - 5.15.90-1
- 5.15.90

* Thu May 09 2019 Martin Kyral <martin.kyral@gmail.com> - 5.15.5-1
- 5.15.5

* Sun Apr 21 2019 Mukundan Ragavan <nonamedotc@gmail.com> - 5.15.4-2
- rebuild for libqalculate

* Wed Apr 03 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.15.4-1
- 5.15.4

* Sat Mar 23 2019 Mukundan Ragavan <nonamedotc@gmail.com> - 5.15.3-2
- rebuild for libqalculate

* Tue Mar 12 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.15.3-1
- 5.15.3

* Thu Feb 28 2019 Pete Walter <pwalter@fedoraproject.org> - 5.15.2-2
- Update wayland deps

* Tue Feb 26 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.15.2-1
- 5.15.2

* Tue Feb 19 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.15.1-1
- 5.15.1

* Wed Feb 13 2019 Martin Kyral <martin.kyral@gmail.com> - 5.15.0-1
- 5.15.0

* Tue Feb 05 2019 Martin Kyral <martin.kyral@gmail.com> - 5.14.90-5
- fix startkde.patch

* Sat Feb 02 2019 Fedora Release Engineering <releng@fedoraproject.org> - 5.14.90-4
- Rebuilt for https://fedoraproject.org/wiki/Fedora_30_Mass_Rebuild

* Wed Jan 23 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.14.90-3
- de-bootstrap

* Sun Jan 20 2019 Martin Kyral <martin.kyral@gmail.com> - 5.14.90-1
- 5.14.90
- enable boostrap

* Tue Nov 27 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.14.4-1
- 5.14.4

* Thu Nov 08 2018 Martin Kyral <martin.kyral@gmail.com> - 5.14.3-1
- 5.14.3

* Wed Oct 24 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.14.2-2
- move systemmonitor.desktop to main, bump kf5 dep

* Wed Oct 24 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.14.2-1
- 5.14.2

* Tue Oct 16 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.14.1-1
- 5.14.1

* Thu Oct 11 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.14.0-3
- rebuild (gpsd)

* Wed Oct 10 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.14.0-2
- Fix build with gpsd 3.18 (#1638110)

* Sat Oct 06 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.14.0-1
- 5.14.0

* Tue Oct 02 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.13.90-2
- Provides: desktop-notification-daemon (#1628758)

* Fri Sep 14 2018 Martin Kyral <martin.kyral@gmail.com> - 5.13.90-1
- 5.13.90

* Tue Sep 04 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.13.5-1
- 5.13.5

* Tue Aug 21 2018 Mukundan Ragavan <nonamedotc@gmail.com> - 5.13.4-2
- rebuild for libqalculate.so.19()

* Thu Aug 02 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.13.4-1
- 5.13.4

* Wed Jul 18 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.13.3-4
- avoid versioned runtime powerdevil dep when bootstrapping

* Fri Jul 13 2018 Fedora Release Engineering <releng@fedoraproject.org> - 5.13.3-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_29_Mass_Rebuild

* Wed Jul 11 2018 Martin Kyral <martin.kyral@gmail.com> - 5.13.3-1
- 5.13.3

* Mon Jul 09 2018 Martin Kyral <martin.kyral@gmail.com> - 5.13.2-1
- 5.13.2

* Fri Jun 22 2018 Mukundan Ragavan <nonamedotc@gmail.com> - 5.13.1-2
- rebuild for libqalculate.so.18()

* Tue Jun 19 2018 Martin Kyral <martin.kyral@gmail.com> - 5.13.1-1
- 5.13.1

* Tue Jun 12 2018 Martin Kyral <martin.kyral@gmail.com> - 5.13.0-1
- 5.13.0

* Fri May 18 2018 Martin Kyral <martin.kyral@gmail.com> - 5.12.90-1
- 5.12.90

* Fri May 18 2018 Mukundan Ragavan <nonamedotc@gmail.com> - 5.12.5-4
- rebuild for libqalculate.so.17()

* Wed May 09 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.12.5-3
- sddm-breeze: Recommends: qt5-qtvirtualkeyboard

* Sun May 06 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.12.5-2
- refresh startkde.patch
- .spec cleanup

* Tue May 01 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.12.5-1
- 5.12.5

* Wed Apr 11 2018 Mukundan Ragavan <nonamedotc@gmail.com> - 5.12.4-2
- rebuild for libqalculate.so.16()

* Fri Mar 30 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.12.4-1
- 5.12.4

* Sat Mar 10 2018 Mukundan Ragavan <nonamedotc@gmail.com> - 5.12.3-2
- rebuild for libqalculate.so.14()

* Tue Mar 06 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.12.3-1
- 5.13.3
- plasmawayland session: drop explcitly running dbus_launch
- use %%make_build %%ldconfig_scriptlets

* Wed Feb 21 2018 Jan Grulich <jgrulich@redhat.com> - 5.12.2-1
- 5.12.2

* Tue Feb 13 2018 Jan Grulich <jgrulich@redhat.com> - 5.12.1-1
- 5.12.1

* Fri Feb 09 2018 Fedora Release Engineering <releng@fedoraproject.org> - 5.12.0-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_28_Mass_Rebuild

* Mon Feb 05 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.12.0-2
- respin

* Fri Feb 02 2018 Jan Grulich <jgrulich@redhat.com> - 5.12.0-1
- 5.12.0

* Mon Jan 15 2018 Jan Grulich <jgrulich@redhat.com> - 5.11.95-1
- 5.11.95

* Mon Jan 15 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.11.4-4
- use upstreamed version of previous commit/patch

* Mon Jan 15 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.11.4-3
- include candidate crash fix for xembedsniproxy (#1497829,kde#359664)

* Tue Dec 19 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.11.4-2
- refresh/fix startkde.patch

* Thu Nov 30 2017 Martin Kyral <martin.kyral@gmail.com> - 5.11.4-1
- 5.11.4

* Tue Nov 21 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.11.3-2
- .spec cruft, BR: kf5-prison, bump min qt5/kf5 deps

* Wed Nov 08 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.11.3-1
- 5.11.3

* Wed Oct 25 2017 Martin Kyral <martin.kyral@gmail.com> - 5.11.2-1
- 5.11.2

* Tue Oct 17 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.11.1-1
- 5.11.1

* Wed Oct 11 2017 Martin Kyral <martin.kyral@gmail.com> - 5.11.0-1
- 5.11.0

* Mon Oct 02 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.10.5-5
- Requires: ksysguardd (#1497831)

* Wed Sep 20 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.10.5-4
- rebuild (libqalculate)

* Tue Aug 29 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.10.5-3
- Restore: Requires: plasma-lookandfeel-fedora

* Thu Aug 24 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.10.5-2
- drop old stuff
- RPM Bundling Fedora look and feel themes (#1356890)
- BR: cmake(AppstreamQt)

* Thu Aug 24 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.10.5-1
- 5.10.5

* Thu Aug 03 2017 Fedora Release Engineering <releng@fedoraproject.org> - 5.10.4-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Binutils_Mass_Rebuild

* Thu Jul 27 2017 Fedora Release Engineering <releng@fedoraproject.org> - 5.10.4-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Mass_Rebuild

* Thu Jul 20 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.10.4-1
- 5.10.4

* Tue Jun 27 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.10.3-1
- 5.10.3

* Thu Jun 15 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.10.2-1
- 5.10.2

* Tue Jun 06 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.10.1-1
- 5.10.1

* Wed May 31 2017 Jan Grulich <jgrulich@redhat.com> - 5.10.0-2
- Disable bootstrap

* Wed May 31 2017 Jan Grulich <jgrulich@redhat.com> - 5.10.0-1
- 5.10.0

* Mon May 15 2017 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 5.9.5.1-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_26_27_Mass_Rebuild

* Fri Apr 28 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.9.5.1-1
- 5.9.5.1

* Thu Apr 27 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.9.5-3
- pull in upstream 5.9 branch fixes

* Wed Apr 26 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.9.5-2
- -doc: use %%find_lang --with-html

* Wed Apr 26 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.9.5-1
- 5.9.5

* Thu Mar 23 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.9.4-1
- 5.9.4

* Sat Mar 04 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.9.3-3
- rebuild

* Fri Mar 03 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.9.3-2
- fix sddm-breeze (01-breeze-fedora theme)
- bump kf5 dep

* Wed Mar 01 2017 Jan Grulich <jgrulich@redhat.com> - 5.9.3-1
- 5.9.3

* Sat Feb 25 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.8.6-3
- Requires: kf5-plasma >= %%_kf5_version

* Thu Feb 23 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.8.6-2
- avoid fedora theme crasher (kde#376847)

* Tue Feb 21 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.8.6-1
- 5.8.6

* Sat Feb 11 2017 Fedora Release Engineering <releng@fedoraproject.org> - 5.8.5-6
- Rebuilt for https://fedoraproject.org/wiki/Fedora_26_Mass_Rebuild

* Sat Jan 28 2017 Mukundan Ragavan <nonamedotc@gmail.com> - 5.8.5-5
- rebuild for libqalculate.so.6

* Mon Jan 23 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.8.5-4
- Obsoletes: kde-runtime-drkonqi (#1415360)

* Thu Jan 19 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.8.5-3
- pull in 5.8 branch fixes

* Mon Jan 02 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.8.5-2
- filter qml/plugin provides

* Wed Dec 28 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.8.5-1
- 5.8.5

* Wed Dec 14 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.8.4-2
- rebuild (libqalculate)

* Tue Nov 22 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.8.4-1
- 5.8.4

* Tue Nov 01 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.8.3-1
- 5.8.3

* Sun Oct 23 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.8.2-4
- sddm-breeze: Requires: qt5-qtquickcontrols instead (kde#371493)

* Sat Oct 22 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.8.2-3
- sddm-breeze: Requires: qt5-qtgraphicaleffects (kde#371493)

* Fri Oct 21 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.8.2-2
- backport candidate systray-icon cpu fix from master (kde#356479)

* Tue Oct 18 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.8.2-1
- 5.8.2

* Tue Oct 11 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.8.1-1
- 5.8.1

* Mon Oct 10 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.8.0-5
- f24-kde-theme/plasma-lookandfeel-fedora noarch
- continue to produce f24-kde-theme on f25+ builds

* Sat Oct 08 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.8.0-4
- plasma-lookandfeel-fedora (f25+)

* Fri Oct 07 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.8.0-3
- sddm-breeze: make 01-breeze-fedora theme use backgrounds/default.png

* Fri Oct 07 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.8.0-2
- pull in upstream branch fixes
- re-order patches so upstream applied first, then downstream

* Thu Sep 29 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.8.0-1
- 5.8.0

* Fri Sep 23 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.7.95-2
- -libs: Provides: plasma-packagestructure

* Thu Sep 22 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.7.95-1
- 5.7.95

* Fri Sep 16 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.7.5-2
- restore fedora.twenty.two theme support (#1376102)

* Tue Sep 13 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.7.5-1
- 5.7.5

* Fri Sep 09 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.7.4-2
- drop support for f22 (plasma theme)

* Tue Aug 23 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.7.4-1
- 5.7.4

* Tue Aug 02 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.7.3-2
- adapt to upstream looknfeel/default-layout changes
- BR: iso-codes (technically only runtime dep, but can't hurt)

* Tue Aug 02 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.7.3-1
- 5.7.3

* Sat Jul 30 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.7.2-3
- klipper autostart: OnlyShowIn=KDE (#1361765,kde#366277)

* Mon Jul 25 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.7.2-2
- -drkonqi: -Requires: kdialog
- remove BR: qt5-qtbase-private-devel until we can properly document why it is needed

* Tue Jul 19 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.7.2-1
- 5.7.2

* Tue Jul 19 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.7.1-3
- BR: qt5-qtbase-private-devel

* Thu Jul 14 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.7.1-2
- BR: kf5-kholidays-devel
- revert recent upstream systray icon resize (kde#365570)

* Tue Jul 12 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.7.1-1
- 5.7.1

* Thu Jun 30 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.7.0-1
- 5.7.0

* Mon Jun 27 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.6.95-3
- kwaylad-integration is part of plasma, not kf5

* Sun Jun 26 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.6.95-2
- bump Qt5 dep

* Sat Jun 25 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.6.95-1
- 5.6.95

* Fri Jun 24 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.6.5-2
- Suggests: imsettings-qt (#1349743)

* Tue Jun 14 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.6.5-1
- 5.6.5

* Sun Jun 05 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.6.4-3
- -drkonqi: support 'dnf debuginfo-install' (f24+)
- -drkonqi: Requires: kdialog konsole5 dnf-command(debuginfo-install) (f24+)

* Thu May 26 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.6.4-2
- backport 5.6 branch fixes

* Sat May 14 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.6.4-1
- 5.6.4

* Thu May 12 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.6.3-5
- /etc/pam.d/kde is executable (#1335500)

* Sun May 01 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.6.3-4
- -libs: omit geolocation plugins

* Sat Apr 30 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.6.3-3
- own phonon_platform plugin dir
- -libs: move multilib'able plugins here

* Wed Apr 27 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.6.3-2
- Some processes (kuiserver) are left running after exiting KDE (#348123)

* Tue Apr 19 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.6.3-1
- 5.6.3

* Sat Apr 09 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.6.2-1
- 5.6.2

* Fri Apr 08 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.6.1-2
- de-bootstrap

* Fri Apr 08 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.6.1-1
- Plasma-5.6.1 (bootstrap)

* Wed Mar 30 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.5.5-10
- f24-kde-theme-core: fix conflict with f24-kde-theme
- f24-kde-theme-core: add dep to/from plasma-workspace
- -wayland: s/plasma-workspace/%%name/

* Wed Mar 30 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.5.5-9
- enable f24-kde-theme default looknfeel (f24+)

* Mon Mar 28 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.5.5-8
- f24-kde-theme-core subpkg (readying for f24-kde-theme)

* Mon Mar 21 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.5.5-7
- Provides: f23-kde-theme-core

* Mon Mar 21 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.5.5-6
- generic theming for f24+

* Mon Mar 21 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.5.5-5
- drop Requires: sddm-breeze for f23+ (workaround for bug #1261034)

* Fri Mar 11 2016 Rex Dieter <rdieter@fedoraproject.org> 5.5.5-4
- f23+: -Requires: sni-qt kde-platform-plugin (use rich/soft deps elsewhere)

* Mon Mar 07 2016 Rex Dieter <rdieter@fedoraproject.org> 5.5.5-3
- backport "Avoid blocking DBus calls in SNI startup" (kde#359611)

* Thu Mar 03 2016 Daniel Vrátil <dvratil@fedoraproject.org> - 5.5.5-2
- Upstream respun tarball

* Wed Mar 02 2016 Daniel Vrátil <dvratil@fedoraproject.org> - 5.5.5-1
- Plasma 5.5.5

* Mon Feb 29 2016 Rex Dieter <rdieter@fedoraproject.org> 5.5.4-6
- Requires: iso-codes (digitalclock applet)

* Mon Feb 29 2016 Rex Dieter <rdieter@fedoraproject.org> 5.5.4-5
- pull in some 5.5 branch fixes

* Mon Feb 22 2016 Rex Dieter <rdieter@fedoraproject.org> 5.5.4-4
- -wayland: Requires: xorg-x11-server-Xwayland

* Tue Feb 09 2016 Rex Dieter <rdieter@fedoraproject.org> 5.5.4-3
- backport xembedsniproxy fixes

* Thu Feb 04 2016 Rex Dieter <rdieter@fedoraproject.org> 5.5.4-2
- backport systray applets not shown workaround (kde#352055)

* Wed Jan 27 2016 Daniel Vrátil <dvratil@fedoraproject.org> - 5.5.4-1
- Plasma 5.5.4

* Mon Jan 25 2016 Rex Dieter <rdieter@fedoraproject.org> 5.5.3-6
- pull in upstream fixes (notifications/xembedsniproxy)

* Mon Jan 11 2016 Rex Dieter <rdieter@fedoraproject.org> 5.5.3-5
- -wayland: Requires: qt5-qtools (for qdbus-qt5)

* Mon Jan 11 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.5.3-4
- startplasmacompositor.patch (#1297418)
- disable bootstrap

* Sun Jan 10 2016 Rex Dieter <rdieter@fedoraproject.org> 5.5.3-3
- drop hacked klipper/prison support (until we have kf5-prison available properly)

* Sat Jan 09 2016 Rex Dieter <rdieter@fedoraproject.org> 5.5.3-2
- pull in upstream fixes (notifications,xembedsniproxy)

* Thu Jan 07 2016 Daniel Vrátil <dvratil@fedoraproject.org> - 5.5.3-1
- Plasma 5.5.3

* Thu Dec 31 2015 Rex Dieter <rdieter@fedoraproject.org> - 5.5.2-2
- use %%majmin_ver for most plasma-related deps
- tighten plugin deps using %%_isa
- update URL

* Thu Dec 31 2015 Rex Dieter <rdieter@fedoraproject.org> - 5.5.2-1
- 5.5.2

* Fri Dec 18 2015 Daniel Vrátil <dvratil@fedoraproject.org> - 5.5.1-1
- Plasma 5.5.1

* Tue Dec 15 2015 Than Ngo <than@redhat.com> - 5.5.0-5
- enable bootstrap for secondary arch

* Mon Dec 14 2015 Daniel Vrátil <dvratil@fedoraproject.org> - 5.5.0-4
- proper upstream fix for #356415 (review #126331)

* Sun Dec 13 2015 Rex Dieter <rdieter@fedoraproject.org> - 5.5.0-3
- latest upstream fixes (#1291100)
- revert commit causing regression'ish kde #356415
- drop kwayland-integration from main pkg (only in -wayland subpkg)

* Sat Dec 05 2015 Daniel Vrátil <dvraitl@fedoraproject.org> - 5.5.0-2
- remove version dependency on oxygen-fonts, because it's not being released anymore

* Thu Dec 03 2015 Daniel Vrátil <dvratil@fedoraproject.org> - 5.5.0-1
- Plasma 5.5.0

* Wed Nov 25 2015 Daniel Vrátil <dvratil@fedoraproject.org> - 5.4.95-1
- Plasma 5.4.95

* Tue Nov 17 2015 Rex Dieter <rdieter@fedoraproject.org> 5.4.3-4
- Unhelpful summary/description for drkonqi packages (#1282810)

* Mon Nov 16 2015 Jan Grulich <jgrulich@redhat.com> - 5.4.3-3
- Fix changing of visibility for system tray entries
  Resolves: kdebz#355404

* Wed Nov 11 2015 Rex Dieter <rdieter@fedoraproject.org> 5.4.3-2
- refresh xembedsniproxy support (#1280457)

* Thu Nov 05 2015 Daniel Vrátil <dvratil@fedoraproject.org> - 5.4.3-1
- Plasma 5.4.3

* Tue Nov 03 2015 Rex Dieter <rdieter@fedoraproject.org> - 5.4.2-8
- make klipper/prison support f24+ only (for now)
- backport xembed-sni-proxy

* Tue Oct 20 2015 Rex Dieter <rdieter@fedoraproject.org> 5.4.2-7
- klipper: prison (qrcode) support

* Wed Oct 14 2015 Rex Dieter <rdieter@fedoraproject.org> 5.4.2-6
- rev startkde.patch drop dbus launch (kde#352251)

* Mon Oct 12 2015 Rex Dieter <rdieter@fedoraproject.org> 5.4.2-5
- Obsoletes: kde-runtime-kuiserver (#1249157), Provides: kuiserver

* Mon Oct 05 2015 Rex Dieter <rdieter@fedoraproject.org> 5.4.2-4
- startkde: don't try to source things in a subshell, don't munge XDG_DATA_DIRS needlessly

* Sun Oct 04 2015 Rex Dieter <rdieter@fedoraproject.org> 5.4.2-3
- consistently use %%{majmin_ver} macro for other plasma5-related deps

* Sat Oct 03 2015 Rex Dieter <rdieter@fedoraproject.org> - 5.4.2-2
- .spec cosmetics, use %%license
- -common, -drkonqi, -libs, libkworkspace5 subpkgs
- -geolocation subpkg (#1222097)
- -drkonqi: include installdbgsymbols.sh

* Thu Oct 01 2015 Rex Dieter <rdieter@fedoraproject.org> - 5.4.2-1
- 5.4.2

* Thu Oct 01 2015 Rex Dieter <rdieter@fedoraproject.org> 5.4.1-6
- try tightened plasmashell dep (loosened in plasma-desktop)

* Fri Sep 25 2015 Rex Dieter <rdieter@fedoraproject.org> 5.4.1-5
- relax kf5-kxmlrpcclient dep (and drop related hacks), tighten khotkeys

* Tue Sep 15 2015 Rex Dieter <rdieter@fedoraproject.org> 5.4.1-4
- Requires: sddm-breeze unconditionally (#1260394)

* Sat Sep 12 2015 Rex Dieter <rdieter@fedoraproject.org> 5.4.1-3
- tighten build deps

* Sat Sep 12 2015 Rex Dieter <rdieter@fedoraproject.org> 5.4.1-2
- Requires: sddm-breeze, (hopefully) temporary workaround for dnf Obsoletes bug (#1260394, f22)

* Fri Sep 11 2015 Rex Dieter <rdieter@fedoraproject.org> 5.4.1-1
- de-bootstrap

* Wed Sep 09 2015 Rex Dieter <rdieter@fedoraproject.org> - 5.4.1-0.1
- 5.4.1, enable bootstrap

* Fri Sep 04 2015 Rex Dieter <rdieter@fedoraproject.org> 5.4.0-7
- Conflicts: kio-extras < 5.4.0

* Wed Sep 02 2015 Rex Dieter <rdieter@fedoraproject.org> 5.4.0-6.1
- make plasma-pa f23+ only

* Tue Sep 01 2015 Daniel Vrátil <dvratil@redhat.com> - 5.4.0-6
- Try rebuilding against new baloo

* Wed Aug 26 2015 Rex Dieter <rdieter@fedoraproject.org> 5.4.0-5
- versioned kf5-related build deps

* Tue Aug 25 2015 Daniel Vrátil <dvratil@redhat.com> - 5.4.0-4
- Disable bootstrap

* Tue Aug 25 2015 Daniel Vrátil <dvratil@redhat.com> - 5.4.0-3
- Re-enable plasma-pa and kwayland-integration dependencies

* Sat Aug 22 2015 Daniel Vrátil <dvratil@redhat.com> - 5.4.0-2
- Temporarily disable plasma-pa and kwayland-integration until the packages are reviewed

* Fri Aug 21 2015 Daniel Vrátil <dvratil@redhat.com> - 5.4.0-1
- Plasma 5.4.0

* Thu Aug 20 2015 Daniel Vrátil <dvratil@redhat.com> - 5.3.95-4
- use patch for startkde.cmake, remove redundant prison dependency

* Thu Aug 13 2015 Daniel Vrátil <dvratil@redhat.com> - 5.3.95-1
- Plasma 5.3.95

* Tue Aug 11 2015 Rex Dieter <rdieter@fedoraproject.org> - 5.3.2-11
- Provides: f23-kde-theme-core (and f22-kde-theme-core)
- default_lookandfeel org.fedoraproject.fedora.twenty.three (f23+)

* Thu Aug 06 2015 Rex Dieter <rdieter@fedoraproject.org> 5.3.2-10
- prep fedora.twenty.three plasma theme

* Thu Aug 06 2015 Rex Dieter <rdieter@fedoraproject.org> 5.3.2-9
- make sddm-breeze noarch (#1250204)

* Thu Aug 06 2015 Rex Dieter <rdieter@fedoraproject.org> 5.3.2-8
- sddm-breeze subpkg, userlist variant for bz #1250204

* Wed Aug 05 2015 Jonathan Wakely <jwakely@redhat.com> 5.3.2-7
- Rebuilt for Boost 1.58

* Fri Jul 31 2015 Rex Dieter <rdieter@fedoraproject.org> 5.3.2-6
- Requires: kde-platform-plugin

* Wed Jul 29 2015 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 5.3.2-5
- Rebuilt for https://fedoraproject.org/wiki/Changes/F23Boost159

* Wed Jul 22 2015 David Tardon <dtardon@redhat.com> - 5.3.2-4
- rebuild for Boost 1.58

* Thu Jul 09 2015 Rex Dieter <rdieter@fedoraproject.org> - 5.3.2-3
- .spec cosmetics
- port selinux/drkonqi scriptlet (from kde-runtime)
- own /usr/share/drkonqi/
- %%config(noreplace) pam

* Fri Jun 26 2015 Daniel Vrátil <dvratil@redhat.com> - 5.3.2-2
- Make the Requires: plasmashell unversioned to break circular dependency during update

* Thu Jun 25 2015 Daniel Vrátil <dvratil@redhat.com> - 5.3.2-1
- Plasma 5.3.2

* Sat Jun 20 2015 Rex Dieter <rdieter@fedoraproject.org> 5.3.1-5
- shutdown scripts are not executed (#1234059)

* Thu Jun 18 2015 Rex Dieter <rdieter@fedoraproject.org> 5.3.1-4
- startkde.cmake: sync ScaleFactor changes, drop QT_PLUGIN_PATH munging (#1233298)

* Thu Jun 18 2015 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 5.3.1-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_23_Mass_Rebuild

* Tue Jun 02 2015 Rex Dieter <rdieter@fedoraproject.org> - 5.3.1-2
- use %%{?kf5_kinit_requires}
- Requires: kf5-kactivities
- doc: make noarch, %%lang'ify

* Tue May 26 2015 Daniel Vrátil <dvratil@redhat.com> - 5.3.1-1
- Plasma 5.3.1

* Wed May 20 2015 Jan Grulich <jgrulich@redhat.com> - 5.3.0-8
- apply the new patch for update scripts execution

* Wed May 20 2015 Jan Grulich <jgrulich@redhat.com> - 5.3.0-7
- process update scripts after first initialization

* Tue May 19 2015 Jan Grulich <jgrulich@redhat.com> - 5.3.0-6
- copy Breeze look-and-feel package also as Fedora Twenty Two look-and-feel package

* Mon May 18 2015 Jan Grulich <jgrulich@redhat.com> - 5.3.0-5
- set default look and feel theme to Fedora Twenty Two

* Tue May 05 2015 Daniel Vrátil <dvratil@redhat.com> - 5.3.0-4
- backport patch form kde-workspace to add Konsole into shell context menu
- re-enable fix-update-scripts.patch

* Wed Apr 29 2015 Daniel Vrátil <dvratil@redhat.com> - 5.3.0-3
- Disable bootstrap

* Wed Apr 29 2015 Daniel Vrátil <dvratil@redhat.com> - 5.3.0-2
- Requires plasmashell (virtual provides for packages that provide Plasma shells, like plasma-desktop)

* Mon Apr 27 2015 Daniel Vrátil <dvratil@redhat.com> - 5.3.0-1
- Plasma 5.3.0

* Wed Apr 22 2015 Daniel Vrátil <dvratil@redhat.com> - 5.2.95-1
- Plasma 5.2.95

* Wed Apr 15 2015 Rex Dieter <rdieter@fedoraproject.org> 5.2.2-6
- Requires: kde-settings-plasma (#1197709)

* Sat Apr 04 2015 Rex Dieter <rdieter@fedoraproject.org> 5.2.2-5
- conflicts with kf5-kxmlrpcclient (#1208947)

* Tue Mar 31 2015 Rex Dieter <rdieter@fedoraproject.org> 5.2.2-4
- Requires: khotkeys (#1207079)

* Mon Mar 30 2015 Rex Dieter <rdieter@fedoraproject.org> 5.2.2-3
- backport fix for update scripts

* Wed Mar 25 2015 Rex Dieter <rdieter@fedoraproject.org> 5.2.2-2
- Lockscreen: Password field does not have focus (kde#344823)

* Fri Mar 20 2015 Daniel Vrátil <dvratil@redhat.com> - 5.2.2-1
- Plasma 5.2.2

* Mon Mar 16 2015 Rex Dieter <rdieter@fedoraproject.org> - 5.2.1-6
- revert Requires: plasma-desktop (dep should be the other way around)
- drop Obsoletes: kde-workspace (leave for plasma-desktop)
- Requires: polkit-kde

* Sun Mar 15 2015 Rex Dieter <rdieter@fedoraproject.org> 5.2.1-5
- Requires: -sddm (#1201034), +plasma-desktop

* Fri Mar 06 2015 Rex Dieter <rdieter@fedoraproject.org> 5.2.1-4
- rebuild (gpsd)

* Tue Mar 03 2015 Rex Dieter <rdieter@fedoraproject.org> 5.2.1-3
- use our own startkde.cmake

* Fri Feb 27 2015 Daniel Vrátil <dvratil@redhat.com> - 5.2.1-2
- Rebuild (GCC 5)

* Tue Feb 24 2015 Daniel Vrátil <dvratil@redhat.com> - 5.2.1-1
- Plasma 5.2.1

* Wed Feb 18 2015 Rex Dieter <rdieter@fedoraproject.org> - 5.2.0-8
- (Build)Requires: kf5-kglobalaccel(-devel) >= 5.7
- drop ksyncdbusenv.patch workaround
- .spec cosmetics

* Wed Feb 11 2015 Rex Dieter <rdieter@fedoraproject.org> 5.2.0-7
- "Could not sync environment to dbus." (startkde) (#1191171)

* Mon Feb 09 2015 Daniel Vrátil <dvratil@redhat.com> - 5.2.0-6
- Revert the previous change

* Mon Feb 09 2015 Daniel Vrátil <dvratil@redhat.com> - 5.2.0-5
- Provides/Obsoletes: kdeclassic-cursor-theme

* Sun Feb 08 2015 Daniel Vrátil <dvratil@redhat.com> - 5.2.0-4
- Requires: powerdevil, oxygen-sound-theme

* Thu Jan 29 2015 Daniel Vrátil <dvratil@redhat.com> - 5.2.0-3
- Requires: plasma-milou (for krunner)

* Thu Jan 29 2015 Dan Horák <dan[at]danny.cz> - 5.2.0-2
- no FireWire on s390(x)

* Mon Jan 26 2015 Daniel Vrátil <dvratil@redhat.com> - 5.2.0-1
- Plasma 5.2.0

* Wed Jan 14 2015 Daniel Vrátil <dvratil@redhat.com> - 5.1.95-3.beta
- Requires: kf5-frameworkintegration (provides platformtheme plugin)

* Wed Jan 14 2015 Daniel Vrátil <dvratil@redhat.com> - 5.1.95-2.beta
- BR: kf5-kscreen-devel (renamed)

* Tue Jan 13 2015 Daniel Vrátil <dvratil@redhat.com> - 5.1.95-1.beta
- Plasma 5.1.95 Beta

* Mon Jan 12 2015 Daniel Vrátil <dvratil@redhat.com> - 5.1.2-5
- Add upstream patch to make ksyncdbusenv work with dbus-1.8.14

* Fri Jan 09 2015 Daniel Vrátil <dvratil@redhat.com> - 5.1.2-4
- Requires: qt5-qttools (for dbus-qt5)

* Wed Jan 07 2015 Jan Grulich <jgrulich@redhat.com> - 5.1.2-3
- Omit "5" from pkg summary
  Drop config macro for files installed to /etc/xdg
  Move /usr/share/dbus-1/interfaces/*.xml stuff to main package
  Validate .desktop files
  look for qdbus-qt5 in startkde instead of qdbus

* Mon Jan 05 2015 Daniel Vrátil <dvratil@redhat.com> - 5.1.2-2
- add upstream patch to fix black screen on start

* Wed Dec 17 2014 Daniel Vrátil <dvratil@redhat.com> - 5.1.2-1
- Plasma 5.1.2

* Fri Nov 28 2014 Daniel Vrátil <dvratil@redhat.com> - 5.1.1-2
- Apply upstream patch to build against new version of KScreen

* Fri Nov 07 2014 Daniel Vrátil <dvratil@redhat.com> - 5.1.1-1
- Plasma 5.1.1

* Tue Oct 14 2014 Daniel Vrátil <dvratil@redhat.com> - 5.1.0.1-1
- Plasma 5.1.0.1

* Thu Oct 09 2014 Daniel Vrátil <dvratil@redhat.com> - 5.1.0-1
- Plasma 5.1.0

* Tue Sep 16 2014 Daniel Vrátil <dvratil@redhat.com> - 5.0.2-1
- Plasma 5.0.2

* Tue Sep 02 2014 Daniel Vrátil <dvratil@redhat.com> - 5.0.1-3
- Make sure we get oxygen-icon-theme and oxyge-icons installed

* Fri Aug 29 2014 Daniel Vrátil <dvratil@redhat.com> - 5.0.1-2
- Add upstream patch to fix generated path in plasma.desktop

* Sun Aug 10 2014 Daniel Vrátil <dvratil@redhat.com> - 5.0.1-1
- Plasma 5.0.1

* Wed Aug 06 2014 Daniel Vrátil <dvratil@redhat.com> - 5.0.0-7
- Add more Obsoletes to make upgrade from KDE 4 smooth
- Add sni-qt to Requires so that Qt 4 apps are working with Plasma 5 systray
- Requires kde-settings

* Thu Jul 24 2014 Daniel Vrátil <dvratil@redhat.com> - 5.0.0-4
- Add patch to fix build-time generated paths

* Thu Jul 24 2014 Daniel Vrátil <dvratil@redhat.com> - 5.0.0-3
- Use relative BIN_INSTALL_DIR so that built-in paths are correctly generated

* Thu Jul 24 2014 Daniel Vrátil <dvratil@redhat.com> - 5.0.0-2
- Fix /usr//usr/ in generated files

* Wed Jul 16 2014 Daniel Vrátil <dvratil@redhat.com> - 5.0.0-1
- Plasma 5.0.0

* Tue May 20 2014 Daniel Vrátil <dvratil@redhat.com> - 4.96.0-6.20140519gita85f5bc
- Add LIBEXEC_PATH to kde5 profile to fix drkonqi lookup
- Fix install

* Mon May 19 2014 Daniel Vrátil <dvratil@redhat.com> - 4.96.0-3.20140519gita85f5bc
- Update to latest git snapshot
- Add PAM file
- Add profile.d entry

* Fri Apr 25 2014 Daniel Vrátil <dvratil@redhat.com> - 4.95.0-1.20140425git7c97c92
- Initial version of kde5-plasma-workspace
