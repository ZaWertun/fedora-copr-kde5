Name:    marble
Summary: Virtual globe and world atlas 
Epoch:   1
Version: 23.08.0
Release: 1%{?dist}

%global maj_ver %(echo %{version} | cut -d. -f1)
%global min_ver %(echo %{version} | cut -d. -f2)
%global patch_ver %(echo %{version} | cut -d. -f3)

License: LGPLv2+
URL:     http://edu.kde.org/marble/
%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0: https://download.kde.org/%{stable}/release-service/%{version}/src/%{name}-%{version}.tar.xz
Source1: https://download.kde.org/%{stable}/release-service/%{version}/src/%{name}-%{version}.tar.xz.sig
Source2: gpgkey-D81C0CB38EB725EF6691C385BB463350D6EF31EF.gpg

## upstream patches

## upstreamable patches

BuildRequires: gnupg2
BuildRequires: desktop-file-utils
BuildRequires: libappstream-glib

BuildRequires: extra-cmake-modules
BuildRequires: kf5-kconfig-devel
BuildRequires: kf5-kcoreaddons-devel
BuildRequires: kf5-kcrash-devel
BuildRequires: kf5-kdoctools-devel
BuildRequires: kf5-ki18n-devel
BuildRequires: kf5-kio-devel
BuildRequires: kf5-knewstuff-devel
BuildRequires: kf5-kparts-devel
BuildRequires: kf5-krunner-devel
BuildRequires: kf5-kservice-devel
BuildRequires: kf5-kwallet-devel
BuildRequires: kf5-rpm-macros
%if 0%{?fedora}
BuildRequires: pkgconfig(libgps)
%endif
BuildRequires: pkgconfig(phonon4qt5)
BuildRequires: pkgconfig(Qt5Core)
BuildRequires: pkgconfig(Qt5Xml)
BuildRequires: pkgconfig(Qt5Network)
BuildRequires: pkgconfig(Qt5Test)
BuildRequires: pkgconfig(Qt5Script)
BuildRequires: pkgconfig(Qt5Widgets)
BuildRequires: pkgconfig(Qt5Quick)
BuildRequires: pkgconfig(Qt5WebKitWidgets)
BuildRequires: pkgconfig(Qt5SerialPort)
BuildRequires: pkgconfig(Qt5Svg)
BuildRequires: pkgconfig(Qt5Sql)
BuildRequires: pkgconfig(Qt5Concurrent)
BuildRequires: pkgconfig(Qt5PrintSupport)
BuildRequires: pkgconfig(Qt5Location) pkgconfig(Qt5Positioning)
BuildRequires: pkgconfig(Qt5WebEngine)
BuildRequires: pkgconfig(Qt5WebEngineWidgets)
BuildRequires: cmake(Qt5LinguistTools)
BuildRequires: pkgconfig(shapelib)
BuildRequires: pkgconfig(shared-mime-info)
BuildRequires: zlib-devel

# when split occurred
Obsoletes: kdeedu-marble < 4.7.0-10
Provides:  kdeedu-marble = %{version}-%{release}
Provides:  kdeedu-marble%{?_isa} = %{version}-%{release}

# fixme, insert last build this was included -- rex
Obsoletes: python-marble < %{epoch}:%{version}-%{release}

Requires: %{name}-widget-qt5%{?_isa} = %{epoch}:%{version}-%{release}

# filter plugin provides
%global __provides_exclude_from ^(%{_libdir}/marble/plugins/.*\\.so)$

%description
Marble is a Virtual Globe and World Atlas that you can use to learn more
about Earth: You can pan and zoom around and you can look up places and
roads. A mouse click on a place label will provide the respective Wikipedia
article.

Of course it's also possible to measure distances between locations or watch
the current cloud cover. Marble offers different thematic maps: A classroom-
style topographic map, a satellite view, street map, earth at night and
temperature and precipitation maps. All maps include a custom map key, so it
can also be used as an educational tool for use in class-rooms. For
educational purposes you can also change date and time and watch how the
starry sky and the twilight zone on the map change.

In opposite to other virtual globes Marble also features multiple
projections: Choose between a Flat Map ("Plate carré"), Mercator or the Globe.

%package qt
Summary: Marble qt-only interface
Requires: %{name}-widget-qt5%{?_isa} = %{epoch}:%{version}-%{release}
Requires: %{name}-common = %{epoch}:%{version}-%{release}
%description qt
%{summary}.

%package common
Summary:  Common files of %{name}
BuildArch: noarch
%if ! 0%{?mobile}
Obsoletes: marble-mobile < %{epoch}:%{version}-%{release}
%endif
%if ! 0%{?touch}
Obsoletes: marble-touch < %{epoch}:%{version}-%{release}
%endif
%description common
{summary}.

%package astro
Summary: Marble Astro Library
Requires: %{name}-common = %{epoch}:%{version}-%{release}
%description astro
%{summary}.

%package astro-devel
Summary: Development files for Marble Astro Library
Requires: %{name}-astro%{?_isa} = %{epoch}:%{version}-%{release}
%description astro-devel
%{summary}.

%package widget-data
Summary: Marble Widget data
Requires: %{name}-common = %{epoch}:%{version}-%{release}
BuildArch: noarch
%description widget-data
%{summary}.

%package widget-qt5
Summary: Marble Widget Library
Requires: %{name}-astro%{?_isa} = %{epoch}:%{version}-%{release}
Requires: %{name}-widget-data = %{epoch}:%{version}-%{release}
%description widget-qt5
%{summary}.

%package widget-qt5-devel
Summary: Development files for Qt5 Marble Widget
Requires: %{name}-widget-qt5%{?_isa} = %{epoch}:%{version}-%{release}
%description widget-qt5-devel
%{summary}.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%setup -q

mv src/3rdparty/zlib src/3rdparty/zlib.UNUSED ||:


%build
%cmake_kf5 \
  -DBUILD_MARBLE_TESTS:BOOL=OFF \
  -DMARBLE_DATA_PATH:PATH="%{_datadir}/marble/data" \
  -DMARBLE_PRI_INSTALL_USE_QT_SYS_PATHS:BOOL=TRUE \
  -DWITH_DESIGNER_PLUGIN:BOOL=OFF
%cmake_build


%install
%cmake_install

%find_lang %{name} --all-name --with-html
# hack around buggy --with-qt ^^
%find_lang_kf5 marble_qt
cat marble_qt.lang >> %{name}.lang


%check
appstream-util validate-relax --nonet %{buildroot}%{_kf5_metainfodir}/org.kde.marble.appdata.xml
appstream-util validate-relax --nonet %{buildroot}%{_kf5_metainfodir}/org.kde.plasma.worldmap.appdata.xml ||:
appstream-util validate-relax --nonet %{buildroot}%{_kf5_metainfodir}/org.kde.plasma.worldclock.appdata.xml ||:
desktop-file-validate %{buildroot}%{_datadir}/applications/org.kde.marble.desktop
desktop-file-validate %{buildroot}%{_datadir}/applications/org.kde.marble-qt.desktop


%files
%{_bindir}/marble
%{_kf5_libdir}/libmarblewidget-qt5.so.%{maj_ver}*
%{_datadir}/kxmlgui5/marble/
%{_kf5_metainfodir}/org.kde.marble.appdata.xml
%{_kf5_metainfodir}/org.kde.plasma.worldmap.appdata.xml
%{_kf5_metainfodir}/org.kde.plasma.worldclock.appdata.xml
%{_datadir}/plasma/plasmoids/org.kde.plasma.worldclock/
%{_datadir}/plasma/wallpapers/org.kde.plasma.worldmap/
%{_datadir}/applications/org.kde.marble.desktop
%{_datadir}/applications/marble_geo.desktop
%{_datadir}/applications/marble_geojson.desktop
%{_datadir}/applications/marble_gpx.desktop
%{_datadir}/applications/marble_kml.desktop
%{_datadir}/applications/marble_kmz.desktop
%{_datadir}/applications/marble_shp.desktop
%{_datadir}/applications/marble_worldwind.desktop
%{_datadir}/config.kcfg/marble.kcfg
%{_datadir}/kservices5/marble_thumbnail_geojson.desktop
%{_datadir}/kservices5/marble_thumbnail_gpx.desktop
%{_datadir}/kservices5/marble_thumbnail_kml.desktop
%{_datadir}/kservices5/marble_thumbnail_kmz.desktop
%{_datadir}/kservices5/marble_thumbnail_osm.desktop
%{_datadir}/kservices5/marble_thumbnail_shp.desktop
%{_datadir}/kservices5/plasma-applet-org.kde.plasma.worldclock.desktop
%{_datadir}/kservices5/plasma-wallpaper-org.kde.plasma.worldmap.desktop

%files common -f %{name}.lang
%license LICENSE.txt
%doc CREDITS MANIFESTO.txt USECASES
%{_datadir}/icons/hicolor/*/apps/marble.*
%{_datadir}/mime/packages/geo.xml
%dir %{_datadir}/marble/

%files qt
%{_bindir}/marble-qt
%{_datadir}/applications/org.kde.marble-qt.desktop

%ldconfig_scriptlets astro

%files astro
%{_kf5_libdir}/libastro.so.1*
%{_kf5_libdir}/libastro.so.%{maj_ver}.%(echo %{min_ver} |sed 's/^0*//').%{patch_ver}

%files astro-devel
%{_includedir}/astro/
%{_kf5_libdir}/libastro.so
%dir %{_libdir}/cmake/
%{_libdir}/cmake/Astro/

%files widget-data
%{_datadir}/marble/data/

%ldconfig_scriptlets widget-qt5

%files widget-qt5
%{_libdir}/libmarblewidget-qt5.so.28
%{_libdir}/marble/plugins/
%{_qt5_plugindir}/marblethumbnail.so
%{_kf5_plugindir}/krunner/plasma_runner_marble.so
# include part here too
%{_datadir}/kservices5/marble_part.desktop
%{_qt5_plugindir}/libmarble_part.so
%{_libdir}/libmarbledeclarative.so
%{_kf5_qmldir}/org/kde/marble/

%files widget-qt5-devel
%{_includedir}/marble/
%{_libdir}/libmarblewidget-qt5.so
%dir %{_libdir}/cmake/
%{_libdir}/cmake/Marble/
%{_qt5_archdatadir}/mkspecs/modules/qt_Marble.pri


%changelog
* Sun Aug 27 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 1:23.08.0-1
- 23.08.0

* Thu Jul 06 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 1:23.04.3-1
- 23.04.3

* Thu Jun 08 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 1:23.04.2-1
- 23.04.2

* Thu May 11 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 1:23.04.1-1
- 23.04.1

* Thu Apr 20 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 1:23.04.0-1
- 23.04.0

* Thu Mar 02 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 1:22.12.3-1
- 22.12.3

* Thu Feb 02 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 1:22.12.2-1
- 22.12.2

