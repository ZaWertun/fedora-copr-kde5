Name:    amarok
Summary: Powerful music player that lets you rediscover your music
Version: 2.9.71
Release: 3%{?dist}

License: GPLv2+
URL:     https://invent.kde.org/multimedia/amarok
Source0: http://download.kde.org/unstable/%{name}/%{version}/%{name}-%{version}.tar.xz

BuildRequires: make
BuildRequires: gcc-c++
BuildRequires: extra-cmake-modules

BuildRequires: libappstream-glib
BuildRequires: desktop-file-utils
BuildRequires: kf5-kdelibs4support

BuildRequires: cmake(Qt5Core)
BuildRequires: cmake(Qt5DBus)
BuildRequires: cmake(Qt5Gui)
BuildRequires: cmake(Qt5QuickWidgets)
BuildRequires: cmake(Qt5Qml)
BuildRequires: cmake(Qt5Sql)
BuildRequires: cmake(Qt5Svg)
BuildRequires: cmake(Qt5Test)
BuildRequires: cmake(Qt5Widgets)
BuildRequires: cmake(Qt5Xml)
BuildRequires: cmake(Qt5QuickControls2)
BuildRequires: cmake(Qt5WebEngine)
BuildRequires: cmake(Qt5UiTools)

BuildRequires: cmake(Phonon4Qt5)

BuildRequires: cmake(KF5Archive)
BuildRequires: cmake(KF5Attica)
BuildRequires: cmake(KF5Codecs)
BuildRequires: cmake(KF5Config)
BuildRequires: cmake(KF5ConfigWidgets)
BuildRequires: cmake(KF5CoreAddons)
BuildRequires: cmake(KF5Crash)
BuildRequires: cmake(KF5DBusAddons)
BuildRequires: cmake(KF5Declarative)
BuildRequires: cmake(KF5DNSSD)
BuildRequires: cmake(KF5DocTools)
BuildRequires: cmake(KF5GlobalAccel)
BuildRequires: cmake(KF5GuiAddons)
BuildRequires: cmake(KF5I18n)
BuildRequires: cmake(KF5IconThemes)
BuildRequires: cmake(KF5KCMUtils)
BuildRequires: cmake(KF5KIO)
BuildRequires: cmake(KF5NewStuff)
BuildRequires: cmake(KF5Notifications)
BuildRequires: cmake(KF5NotifyConfig)
BuildRequires: cmake(KF5Package)
BuildRequires: cmake(KF5Solid)
BuildRequires: cmake(KF5TextEditor)
BuildRequires: cmake(KF5ThreadWeaver)
BuildRequires: cmake(KF5WidgetsAddons)
BuildRequires: cmake(KF5WindowSystem)
BuildRequires: cmake(KF5SyntaxHighlighting)
BuildRequires: cmake(KF5Kirigami2)
BuildRequires: cmake(KF5Wallet)

BuildRequires: fftw-devel
BuildRequires: zlib-devel
BuildRequires: glib2-devel
BuildRequires: gtkpod-devel
BuildRequires: libmtp-devel
BuildRequires: libofa-devel
BuildRequires: taglib-devel
BuildRequires: libcurl-devel
BuildRequires: libxml2-devel
BuildRequires: mariadb-devel
BuildRequires: liblastfm-devel
BuildRequires: loudmouth-devel
BuildRequires: gdk-pixbuf2-devel
# Commented - produces compilation error (header mygpo-qt5/ApiRequest.h not found)
#BuildRequires: libmygpo-qt5-devel
BuildRequires: taglib-extras-devel
BuildRequires: mariadb-embedded-devel

Requires:      %{name}-libs%{?_isa} = %{version}-%{release}
Requires:      %{name}-utils%{?_isa} = %{version}-%{release}
Recommends:    %{name}-docs = %{version}-%{release}

%description
%{summary}.

%package        libs
Summary:        Runtime files for %{name}
Requires:       %{name} = %{version}-%{release}
%description    libs
Runtime files for %{name}.

%package        utils
Summary:        Amarok standalone utilities
%description    utils
%{summary}, including amarokcollectionscanner.

%package        doc
Summary:        Documentation for %{name}
BuildArch:      noarch
%description    doc
Documentation for %{name}.

%prep
%autosetup -p1


%build
%cmake_kf5
%cmake_build


%install
%cmake_install

%find_lang amarok --with-html --without-mo && mv amarok.lang amarok-doc.lang
%find_lang amarok --all-name
%find_lang amarokcollectionscanner --with-qt

rm -fv %{buildroot}%{_libdir}/libamarok{-sqlcollection,-transcoding,core,lib,shared}.so
chmod -x %{buildroot}%{_kf5_datadir}/applications/org.kde.amarok*.desktop


%check
desktop-file-validate %{buildroot}%{_kf5_datadir}/applications/org.kde.amarok*.desktop
# Result ignored due to validation errors:
#  ? tag-invalid           : stock icon is not valid [media-album-cover-manager-amarok]
#  ? tag-invalid           : stock icon is not valid [view-media-analyzer-amarok]
#  ? tag-invalid           : stock icon is not valid [amarok]
#  ? tag-invalid           : stock icon is not valid [current-track-amarok]
#  ? tag-invalid           : stock icon is not valid [photos-amarok]
#  ? tag-invalid           : stock icon is not valid [amarok-wikipedia]
appstream-util validate-relax --nonet %{buildroot}%{_kf5_metainfodir}/org.kde.amarok.*.appdata.xml ||:


%files -f %{name}.lang
%{_sysconfdir}/xdg/amarok_homerc
%{_bindir}/amarok
%{_bindir}/amarok_afttagger
%{_bindir}/amarokpkg
%{_kf5_datadir}/amarok/data/
%{_kf5_datadir}/amarok/icons/hicolor/
%{_kf5_datadir}/amarok/images/
%{_kf5_datadir}/amarok/scriptconsole/
%{_kf5_datadir}/applications/org.kde.amarok*.desktop
%{_kf5_datadir}/config.kcfg/amarokconfig.kcfg
%{_kf5_datadir}/dbus-1/interfaces/org.kde.amarok.*.xml
%{_kf5_datadir}/icons/hicolor/*/apps/amarok.png
%{_kf5_datadir}/kconf_update/amarok-2.4.1-tokens_syntax_update.pl
%{_kf5_datadir}/kconf_update/amarok.upd
%{_kf5_datadir}/knotifications5/amarok.notifyrc
%{_kf5_datadir}/knsrcfiles/amarok.knsrc
%{_kf5_datadir}/kpackage/amarok/org.kde.amarok.*/
%{_kf5_datadir}/kpackage/genericqml/org.kde.amarok.context/
%{_kf5_datadir}/kservices5/ServiceMenus/amarok_append.desktop
%{_kf5_datadir}/kservices5/amarok.protocol
%{_kf5_datadir}/kservices5/amarokitpc.protocol
%{_kf5_datadir}/kservices5/amarok_*.desktop
%{_kf5_datadir}/kservicetypes5/amarok*.desktop
%{_kf5_datadir}/solid/actions/amarok-play-audiocd.desktop
%{_kf5_metainfodir}/org.kde.amarok.albums.appdata.xml
%{_kf5_metainfodir}/org.kde.amarok.analyzer.appdata.xml
%{_kf5_metainfodir}/org.kde.amarok.appdata.xml
%{_kf5_metainfodir}/org.kde.amarok.context.appdata.xml
%{_kf5_metainfodir}/org.kde.amarok.currenttrack.appdata.xml
%{_kf5_metainfodir}/org.kde.amarok.lyrics.appdata.xml
%{_kf5_metainfodir}/org.kde.amarok.photos.appdata.xml
%{_kf5_metainfodir}/org.kde.amarok.wikipedia.appdata.xml


%files libs
%{_libdir}/libamarokpud.so
%{_libdir}/libampache_account_login.so
%{_libdir}/libamarok-sqlcollection.so*
%{_libdir}/libamarok-transcoding.so*
%{_libdir}/libamarokcore.so*
%{_libdir}/libamaroklib.so*
%{_libdir}/libamarokshared.so*
%{_kf5_qtplugindir}/amarok_*.so
%{_kf5_qtplugindir}/kcm_amarok_service_ampache.so
%{_kf5_qtplugindir}/kcm_amarok_service_magnatunestore.so
%{_kf5_libdir}/qt5/qml/org/kde/amarok/albums/
%{_kf5_libdir}/qt5/qml/org/kde/amarok/analyzer/
%{_kf5_libdir}/qt5/qml/org/kde/amarok/currenttrack/
%{_kf5_libdir}/qt5/qml/org/kde/amarok/lyrics/
%{_kf5_libdir}/qt5/qml/org/kde/amarok/photos/
%{_kf5_libdir}/qt5/qml/org/kde/amarok/qml/
%{_kf5_libdir}/qt5/qml/org/kde/amarok/wikipedia/


%files utils -f amarokcollectionscanner.lang
%{_bindir}/amarokcollectionscanner


%files doc -f amarok-doc.lang
%{_kf5_datadir}/doc/HTML/*/amarok/


%changelog
* Fri Mar 05 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 2.9.71-3
- Moved amarokcollectionscanner to -utils package

* Thu Mar 04 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 2.9.71-2
- More optional deps added

* Thu Mar 04 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 2.9.71-1
- First spec for version 2.9.71

