# do not use webkit on rhel
%if 0%{?fedora} || 0%{?epel}
%define webkit 1
%endif

# trim changelog included in binary rpms
%global _changelog_trimtime %(date +%s -d "1 year ago")

Name:    k3b
Summary: CD/DVD/Blu-ray burning application
Epoch:   1
Version: 23.04.3
Release: 1%{?dist}

License: GPLv2+
URL:     http://www.k3b.org/
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

## downstream patches

BuildRequires: gnupg2
BuildRequires: desktop-file-utils
BuildRequires: libappstream-glib

BuildRequires: cmake(Qt5Gui)
%if 0%{?webkit}
BuildRequires: cmake(Qt5WebKitWidgets)
%endif

BuildRequires: extra-cmake-modules
BuildRequires: kf5-rpm-macros
BuildRequires: cmake(KF5Archive)
BuildRequires: cmake(KF5Config)
BuildRequires: cmake(KF5CoreAddons)
BuildRequires: cmake(KF5DocTools)
BuildRequires: cmake(KF5FileMetaData)
BuildRequires: cmake(KF5I18n)
BuildRequires: cmake(KF5IconThemes)
BuildRequires: cmake(KF5JobWidgets)
BuildRequires: cmake(KF5KCMUtils)
BuildRequires: cmake(KF5KIO)
BuildRequires: cmake(KF5Notifications)
BuildRequires: cmake(KF5NewStuff)
BuildRequires: cmake(KF5NotifyConfig)
BuildRequires: cmake(KF5Service)
BuildRequires: cmake(KF5Solid)
BuildRequires: cmake(KF5WidgetsAddons)
BuildRequires: cmake(KF5XmlGui)

BuildRequires: kf5-libkcddb-devel

BuildRequires: lame-devel
BuildRequires: libmpcdec-devel
BuildRequires: pkgconfig(dvdread)
BuildRequires: pkgconfig(flac++)

BuildRequires: pkgconfig(mad)
BuildRequires: pkgconfig(samplerate)
BuildRequires: pkgconfig(sndfile)
BuildRequires: pkgconfig(taglib)
BuildRequires: pkgconfig(vorbisenc) pkgconfig(vorbisfile)
BuildRequires: pkgconfig(taglib)
BuildRequires: pkgconfig(zlib)

Conflicts: k3b-extras-freeworld < 1:17.03

Obsoletes: k3b-common < 1:17.03
Provides:  k3b-common = %{epoch}:%{version}-%{release}

Requires: %{name}-libs%{?_isa} = %{epoch}:%{version}-%{release}

Requires: cdrdao
Requires: cdrskin
Requires: dvd+rw-tools
## BR these runtime dependencies for sanitiy (for now) -- rex
## use real packages, not virtual provides since they have
## been recently removed, https://bugzilla.redhat.com/1599009
# mkisofs
BuildRequires: genisoimage
Requires: genisoimage
# cdrecord
BuildRequires: wodim
Requires: wodim

%description
K3b provides a comfortable user interface to perform most CD/DVD
burning tasks. While the experienced user can take influence in all
steps of the burning process the beginner may find comfort in the
automatic settings and the reasonable k3b defaults which allow a quick
start.

%package libs
Summary: Runtime libraries for %{name}
Requires: %{name} = %{epoch}:%{version}-%{release}
%description libs
%{summary}.

%package devel
Summary: Files for the development of applications which will use %{name} 
Requires: %{name}-libs%{?_isa} = %{epoch}:%{version}-%{release}
%description devel
%{summary}.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -p1


%build

%cmake_kf5 \
  -DK3B_BUILD_FFMPEG_DECODER_PLUGIN:BOOL=OFF \
  -DK3B_BUILD_LAME_ENCODER_PLUGIN:BOOL=ON \
  -DK3B_BUILD_MAD_DECODER_PLUGIN:BOOL=ON
%cmake_build


%install
%cmake_install

%find_lang %{name} --all-name --with-html


%check
appstream-util validate-relax --nonet %{buildroot}%{_kf5_metainfodir}/org.kde.k3b.appdata.xml
desktop-file-validate %{buildroot}%{_kf5_datadir}/applications/org.kde.k3b.desktop

%files -f %{name}.lang
%doc README*
%license LICENSES/*.txt
%{_kf5_bindir}/k3b
%{_kf5_metainfodir}/org.kde.k3b.appdata.xml
%{_kf5_datadir}/applications/org.kde.k3b.desktop
%{_kf5_datadir}/knotifications5/k3b.*
%{_kf5_datadir}/konqsidebartng/virtual_folders/services/*.desktop
%{_kf5_datadir}/kservices5/*
%{_kf5_datadir}/kservicetypes5/*
%{_kf5_datadir}/kxmlgui5/k3b/
%{_kf5_datadir}/solid/actions/k3b*.desktop
%{_kf5_datadir}/mime/packages/x-k3b.xml
%{_kf5_datadir}/icons/hicolor/*/*/*
%{_kf5_datadir}/k3b/
%{_kf5_datadir}/knsrcfiles/%{name}theme.knsrc
%{_kf5_libexecdir}/kauth/k3bhelper
%{_datadir}/dbus-1/system-services/org.kde.k3b.service
%{_datadir}/dbus-1/system.d/org.kde.k3b.conf
%{_datadir}/polkit-1/actions/org.kde.k3b.policy
%{_datadir}/qlogging-categories5/%{name}.categories

%ldconfig_scriptlets libs

%files libs
%{_kf5_libdir}/libk3bdevice.so.*
%{_kf5_libdir}/libk3blib.so.*
%{_kf5_plugindir}/kio/videodvd.so
%{_kf5_qtplugindir}/k3b_plugins/

%files devel
%{_includedir}/k3b*.h
%{_kf5_libdir}/libk3bdevice.so
%{_kf5_libdir}/libk3blib.so


%changelog
* Tue Jul 18 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 1:23.04.3-1
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

