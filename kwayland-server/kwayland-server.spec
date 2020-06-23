%global  wayland_min_version 1.3

Name:    kwayland-server
Version: 5.19.2
Release: 1%{?dist}
Summary: Wayland server components built on KDE Frameworks

License: LGPLv2+ and MIT and BSD
URL:     https://cgit.kde.org/%{name}.git

%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0: http://download.kde.org/%{stable}/plasma/%{version}/%{name}-%{version}.tar.xz

BuildRequires:  qt5-qtbase-devel

BuildRequires:  extra-cmake-modules
BuildRequires:  kf5-kwayland-devel
BuildRequires:  wayland-devel
BuildRequires:  wayland-protocols-devel

BuildRequires:  plasma-wayland-protocols-devel
BuildRequires:  kf5-kwindowsystem-devel
BuildRequires:  cmake(KF5GuiAddons)
BuildRequires:  cmake(Qt5WaylandClient)

Requires:       qt5-qtbase

%description
%{summary}.

%package        devel
Summary:        Development files for %{name}
Requires:       %{name}%{?_isa} = %{version}-%{release}

%description    devel
The %{name}-devel package contains libraries and header files for
developing applications that use %{name}.

%prep
%autosetup -p1


%build
mkdir %{_target_platform}
pushd %{_target_platform}
%{cmake_kf5} ..
popd

make %{?_smp_mflags} -C %{_target_platform}


%install
make install/fast DESTDIR=%{buildroot} -C %{_target_platform}


%ldconfig_scriptlets

%files
%license COPYING.LIB
%{_kf5_datadir}/qlogging-categories5/kwaylandserver.categories
%{_kf5_libdir}/libKWaylandServer.so.*

%files devel
%{_kf5_libdir}/libKWaylandServer.so
%{_includedir}/KWaylandServer/
%{_includedir}/kwaylandserver_version.h
%{_libdir}/cmake/KWaylandServer/

%changelog
* Tue Jun 23 2020 Yaroslav Sidlovsky <zawertun@otl.ru> - 5.19.2-1
- 5.19.2

* Tue Jun 16 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.19.1-1
- 5.19.1

* Tue Jun 9 2020 Martin Kyral <martin.kyral@gmail.com> - 5.19.0-1
- 5.19.0

* Fri May 22 2020 Martin Kyral <martin.kyral@gmail.com> - 5.18.90-1
- 5.18.90
