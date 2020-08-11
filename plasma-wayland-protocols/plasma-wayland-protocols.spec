%global  wayland_min_version 1.3
%global debug_package %{nil}

Name:    plasma-wayland-protocols
Version: 1.1.1
Release: 1%{?dist}
Summary: Plasma Specific Protocols for Wayland

License: LGPLv2+ and MIT and BSD
URL:     https://invent.kde.org/libraries/%{name}.git

#%global revision %(echo %{version} | cut -d. -f3)
#%if %{revision} >= 50
#%global stable unstable
#%else
#%global stable stable
#%endif
Source0: https://download.kde.org/stable/%{name}/%{version}/%{name}-%{version}.tar.xz

BuildRequires:  extra-cmake-modules
BuildRequires:  qt5-qtbase-devel

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

%make_build -C %{_target_platform}


%install
make install/fast DESTDIR=%{buildroot} -C %{_target_platform}

%files
%license COPYING.LIB
%{_kf5_datadir}/plasma-wayland-protocols/

%files devel
%{_kf5_libdir}/cmake/PlasmaWaylandProtocols/


%changelog
* Mon Aug 10 2020 Yaroslav Sidlovsky <zawertun@otl.ru> - 1.1.1-1
- 1.1.1

* Fri May 22 2020 Martin Kyral <martin.kyral@gmail.com> - 1.0-1
- 1.0
