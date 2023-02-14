
%global wayland_min_version 1.3
%global debug_package %{nil}

Name:    plasma-wayland-protocols
Version: 1.10.0
Release: 1%{?dist}
Summary: Plasma Specific Protocols for Wayland

License: LGPLv2+ and MIT and BSD
URL:     https://invent.kde.org/libraries/%{name}

Source0: https://download.kde.org/stable/%{name}/%{name}-%{version}.tar.xz

## upstream patches (lookaside cache)

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
%cmake_kf5

%cmake_build


%install
%cmake_install


%files
%license COPYING.LIB
%{_kf5_datadir}/plasma-wayland-protocols/

%files devel
%{_kf5_libdir}/cmake/PlasmaWaylandProtocols/


%changelog
* Tue Feb 14 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 1.10.0-1
- version 1.10.0

* Sat May 14 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 1.7.0-1
- version 1.7.0

* Tue Feb 08 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 1.6.0-1
- version 1.6.0

* Tue Feb 08 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 1.5.0-1
- version 1.5.0

* Sun Sep 12 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 1.4.0-1
- version 1.4.0

* Sat May 08 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 1.3.0-1
- v1.3.0

* Mon Apr 05 2021 Rex Dieter <rdieter@fedoraproject.org> - 1.2.1-1
- v1.2.1

* Tue Mar 30 2021 Rex Dieter <rdieter@fedoraproject.org> - 1.2.0-2
- pull in upstream fix so internal version is consistent

* Sat Mar 27 2021 Rex Dieter <rdieter@fedoraproject.org> - 1.2.0-1
- 1.2.0

* Wed Jan 27 2021 Fedora Release Engineering <releng@fedoraproject.org> - 1.1.1-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_34_Mass_Rebuild

* Fri Jul 31 2020 Jan Grulich <jgrulich@redhat.com> - 1.1.1-1
- 1.1.1

* Sat Jul 25 2020 Rex Dieter <rdieter@fedoraproject.org> - 1.1.0-1
- 1.1.0

* Tue Jun 9 2020 Martin Kyral <martin.kyral@gmail.com> - 5.19.0-1
- 5.19.0

* Fri May 22 2020 Martin Kyral <martin.kyral@gmail.com> - 1.0-1
- 1.0
