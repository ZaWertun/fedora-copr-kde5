%global kf5_min_version 5.98.0

Name:    kpipewire
Version: 5.26.2
Release: 1%{?dist}
Summary: Components relating to Flatpak 'pipewire' use in Plasma

License: LGPLv2+
URL:     https://invent.kde.org/plasma/%{name}

%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0: http://download.kde.org/%{stable}/plasma/%{version}/%{name}-%{version}.tar.xz
Source1: http://download.kde.org/%{stable}/plasma/%{version}/%{name}-%{version}.tar.xz.sig
Source2: https://jriddell.org/esk-riddell.gpg

BuildRequires: gnupg2
BuildRequires: cmake
BuildRequires: gcc-c++
BuildRequires: gettext
BuildRequires: kf5-rpm-macros
BuildRequires: extra-cmake-modules

BuildRequires: cmake(Qt5DBus)
BuildRequires: cmake(Qt5Quick)
BuildRequires: cmake(Qt5WaylandClient)
BuildRequires: qt5-qtbase-private-devel

BuildRequires: cmake(KF5Wayland)    >= %{kf5_min_version}
BuildRequires: cmake(KF5I18n)       >= %{kf5_min_version}
BuildRequires: cmake(KF5CoreAddons) >= %{kf5_min_version}

BuildRequires: pkgconfig(wayland-client)
BuildRequires: pkgconfig(wayland-scanner)
BuildRequires: pkgconfig(libpipewire-0.3)
BuildRequires: cmake(PlasmaWaylandProtocols)

# FFmpeg
BuildRequires: pkgconfig(libavcodec)
BuildRequires: pkgconfig(libavdevice)
BuildRequires: pkgconfig(libavfilter)
BuildRequires: pkgconfig(libavformat)
BuildRequires: pkgconfig(libavutil)
BuildRequires: pkgconfig(libpostproc)
BuildRequires: pkgconfig(libswresample)
BuildRequires: pkgconfig(libswscale)

BuildRequires: pkgconfig(gbm)
BuildRequires: pkgconfig(epoxy) >= 1.3
BuildRequires: pkgconfig(libdrm) >= 2.4.62

%description
%{summary}.

%package   devel
Summary:   Development files for %{name}
Requires:  %{name}%{?_isa} = %{version}-%{release}
Provides:  kpipewire-devel = %{version}-%{release}
Provides:  kpipewire-devel%{?_isa} = %{version}-%{release}
Obsoletes: kpipewire-devel <= 1:5.2.0
%description devel
The %{name}-devel package contains libraries and header files for
developing applications that use %{name}.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup


%build
%cmake_kf5 -DBUILD_TESTING:BOOL=ON
%cmake_build


%install
%cmake_install
%find_lang %{name} --with-qt --all-name


%check
%ctest


%ldconfig_scriptlets


%files -f %{name}.lang
%doc README.md
%license LICENSES/*.txt
%{_kf5_qmldir}/org/kde/pipewire/
%{_kf5_libdir}/libKPipeWire.so.5*
%{_kf5_libdir}/libKPipeWireRecord.so.5*
%{_kf5_datadir}/qlogging-categories5/kpipewire.categories
%{_kf5_datadir}/qlogging-categories5/kpipewirerecord.categories


%files devel
%{_kf5_libdir}/libKPipeWire.so
%{_kf5_libdir}/libKPipeWireRecord.so
%{_kf5_libdir}/cmake/KPipeWire/KPipeWire*.cmake
%{_includedir}/KPipeWire/


%changelog
* Wed Oct 26 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.26.2-1
- 5.26.2

* Tue Oct 18 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.26.1-1
- 5.26.1

* Wed Oct 12 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.26.0-1
- 5.26.0

* Mon Sep 19 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.25.90-1
- first spec for version 5.25.90

