%global framework kidletime

Name:           kf5-%{framework}
Version:        5.109.0
Release:        1%{?dist}
Summary:        KDE Frameworks 5 Tier 1 integration module for idle time detection

License:        GPLv2+ and LGPLv2+
URL:            https://cgit.kde.org/%{framework}.git

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

# filter plugin provides
%global __provides_exclude_from ^(%{_kf5_plugindir}/.*\\.so)$

BuildRequires:  gnupg2
BuildRequires:  extra-cmake-modules >= %{majmin}
BuildRequires:  kf5-rpm-macros >= %{majmin}
BuildRequires:  qt5-qtbase-devel
BuildRequires:  qt5-qtx11extras-devel

BuildRequires:  pkgconfig(xext)
BuildRequires:  pkgconfig(x11-xcb)
BuildRequires:  pkgconfig(xcb)
BuildRequires:  pkgconfig(xcb-sync)

BuildRequires:  pkgconfig(xscrnsaver)

BuildRequires:  wayland-protocols-devel
BuildRequires:  cmake(Qt5WaylandClient)
BuildRequires:  qt5-qtbase-private-devel
BuildRequires:  pkgconfig(wayland-client)
BuildRequires:  cmake(PlasmaWaylandProtocols)

Requires:       kf5-filesystem >= %{majmin}

%description
KDE Frameworks 5 Tier 1 integration module for idle time detection.

%package        devel
Summary:        Development files for %{name}
Requires:       %{name}%{?_isa} = %{version}-%{release}
Requires:       qt5-qtbase-devel
%description    devel
The %{name}-devel package contains libraries and header files for
developing applications that use %{name}.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -n %{framework}-%{version}


%build
%cmake_kf5
%cmake_build


%install
%cmake_install


%ldconfig_scriptlets

%files
%doc README.md
%license LICENSES/*.txt
%{_kf5_datadir}/qlogging-categories5/%{framework}.*
%{_kf5_libdir}/libKF5IdleTime.so.*
%dir %{_kf5_plugindir}/org.kde.kidletime.platforms/
%{_kf5_plugindir}/org.kde.kidletime.platforms/KF5IdleTimeXcbPlugin0.so
%{_kf5_plugindir}/org.kde.kidletime.platforms/KF5IdleTimeXcbPlugin1.so
%{_kf5_plugindir}/org.kde.kidletime.platforms/KF5IdleTimeWaylandPlugin.so

%files devel
%{_kf5_includedir}/KIdleTime/
%{_kf5_libdir}/libKF5IdleTime.so
%{_kf5_libdir}/cmake/KF5IdleTime/
%{_kf5_archdatadir}/mkspecs/modules/qt_KIdleTime.pri


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

* Thu Feb 23 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.103.0-5
- BR: cmake(Qt5WaylandClient)

