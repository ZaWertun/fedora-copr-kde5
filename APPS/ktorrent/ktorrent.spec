# trim changelog included in binary rpms
%global _changelog_trimtime %(date +%s -d "1 year ago")

Name:    ktorrent
Version: 23.08.0
Release: 1%{?dist}
Summary: A BitTorrent program

License: GPLv2+
URL:     https://www.kde.org/applications/internet/ktorrent/

%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0: http://download.kde.org/stable/release-service/%{version}/src/%{name}-%{version}.tar.xz
Source1: http://download.kde.org/stable/release-service/%{version}/src/%{name}-%{version}.tar.xz.sig
Source2: gpgkey-D81C0CB38EB725EF6691C385BB463350D6EF31EF.gpg

%global majmin_ver %(echo %{version} | cut -d. -f1,2)

## upstream patches

## upstreamable patches

## downstream patches

BuildRequires: gnupg2
BuildRequires: boost-devel
BuildRequires: cmake(Qca-qt5)
BuildRequires: desktop-file-utils
BuildRequires: gettext
BuildRequires: GeoIP-devel
BuildRequires: libappstream-glib
BuildRequires: pkgconfig(phonon4qt5)
BuildRequires: pkgconfig(taglib)
# kf5 deps
BuildRequires: extra-cmake-modules
BuildRequires: kf5-rpm-macros

BuildRequires: cmake(KF5Config)
BuildRequires: cmake(KF5ConfigWidgets)
BuildRequires: cmake(KF5Crash)
BuildRequires: cmake(KF5DBusAddons)
BuildRequires: cmake(KF5I18n)
BuildRequires: cmake(KF5IconThemes)
BuildRequires: cmake(KF5KIO)
BuildRequires: cmake(KF5Notifications)
BuildRequires: cmake(KF5NotifyConfig)
BuildRequires: cmake(KF5KCMUtils)
BuildRequires: cmake(KF5Parts)
BuildRequires: cmake(KF5Service)
BuildRequires: cmake(KF5Solid)
BuildRequires: cmake(KF5WidgetsAddons)
BuildRequires: cmake(KF5WindowSystem)
BuildRequires: cmake(KF5XmlGui)
# optional (kf5) deps
BuildRequires: cmake(KF5Archive)
BuildRequires: cmake(KF5Completion)
BuildRequires: cmake(KF5DNSSD)
BuildRequires: cmake(KF5DocTools)
BuildRequires: cmake(KF5ItemViews)
BuildRequires: cmake(KF5Kross)
BuildRequires: cmake(KF5Plotting)
BuildRequires: cmake(KF5TextWidgets)

BuildRequires: cmake(KF5Syndication)
BuildRequires: cmake(LibKWorkspace)

%ifarch %{qt5_qtwebengine_arches}
BuildRequires: cmake(Qt5WebEngineWidgets)
%endif

BuildRequires: kf5-libktorrent-devel >= %{version}

# multilib, when -libs was introduced
Obsoletes: ktorrent < 3.2.3-2

Requires: %{name}-libs%{?_isa} = %{version}-%{release}

%description
KTorrent is a BitTorrent program for KDE. Its main features are native KDE
integration, download of torrent files, upload speed capping, internet
searching using various search engines, UDP Trackers and UPnP support.

%package libs
Summary: Runtime libraries for %{name}
Requires: %{name}%{?_isa} = %{version}-%{release}
Requires: kf5-libktorrent%{?_isa} >= %{version}
%description libs
%{summary}.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -n %{name}-%{version}%{?pre} -p1


%build
%cmake_kf5 \
  -DWITH_SYSTEM_GEOIP:BOOL=ON
%cmake_build


%install
%cmake_install

# Make desktop file valid
sed -i 's|SingleMainWindow=True|SingleMainWindow=true|' \
  %{buildroot}%{_kf5_datadir}/applications/org.kde.ktorrent.desktop

# ensure this exists (sometimes not, e.g. when qtwebengine support isn't available)
mkdir -p %{buildroot}%{_kf5_datadir}/ktorrent

%find_lang %{name} --with-html


%check
appstream-util validate-relax --nonet %{buildroot}%{_kf5_metainfodir}/org.kde.ktorrent.appdata.xml
desktop-file-validate %{buildroot}%{_kf5_datadir}/applications/org.kde.ktorrent.desktop


%files -f %{name}.lang
%doc ChangeLog
%license LICENSES/*
%{_kf5_bindir}/ktorrent
%{_kf5_bindir}/ktmagnetdownloader
%{_kf5_bindir}/ktupnptest
%{_kf5_metainfodir}/org.kde.ktorrent.appdata.xml
%{_kf5_datadir}/applications/org.kde.ktorrent.desktop
%{_kf5_datadir}/icons/hicolor/*/*/*
%{_kf5_datadir}/ktorrent/
%{_kf5_datadir}/knotifications5/ktorrent.notifyrc
%{_kf5_datadir}/kxmlgui5/ktorrent/

%ldconfig_scriptlets libs

%files libs
%{_kf5_libdir}/libktcore.so.*
%{_kf5_qtplugindir}/ktorrent_plugins/ktorrent_*.so


%changelog
* Sun Aug 27 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.08.0-1
- 23.08.0

* Thu Jul 06 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.04.3-1
- 23.04.3

* Thu Jun 08 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.04.2-1
- 23.04.2

* Thu May 11 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.04.1-1
- 23.04.1

* Thu Apr 20 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.04.0-1
- 23.04.0

* Thu Mar 02 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.12.3-1
- 22.12.3

* Thu Feb 02 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.12.2-1
- 22.12.2

