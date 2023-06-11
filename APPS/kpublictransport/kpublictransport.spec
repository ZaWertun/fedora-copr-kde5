Name:           kpublictransport
Version:        23.04.2
Release:        1%{?dist}
License:        BSD and CC0-1.0 and LGPLv2+ and MIT and ODbL-1.0
Summary:        Library to assist with accessing public transport timetables and other data
Url:            https://invent.kde.org/libraries/kpublictransport
Source:         https://download.kde.org/stable/release-service/%{version}/src/kpublictransport-%{version}.tar.xz

BuildRequires: extra-cmake-modules
BuildRequires: gcc-c++
BuildRequires: kf5-rpm-macros
BuildRequires: zlib-devel

BuildRequires: cmake(Qt5Core)
BuildRequires: cmake(Qt5Quick)

BuildRequires: cmake(KF5I18n)
BuildRequires: cmake(KF5NetworkManagerQt)

%description
%{summary}.

%prep
%autosetup

%build
%cmake_kf5
%cmake_build

%install
%cmake_install

%files
%{_kf5_datadir}/qlogging-categories5/org_kde_kpublictransport.categories

%{_kf5_libdir}/libKPublicTransport.so.1
%{_kf5_libdir}/libKPublicTransport.so.%{version}
%{_kf5_libdir}/libKPublicTransportOnboard.so.1
%{_kf5_libdir}/libKPublicTransportOnboard.so.%{version}

%{_kf5_qmldir}/org/kde/kpublictransport/*
%{_kf5_datadir}/qlogging-categories5/org_kde_kpublictransport_onboard.categories

%package devel
Summary: Development files for %{name}
License: BSD and CC0-1.0 and LGPLv2+ and MIT and ODbL-1.0
Requires: %{name}%{?_isa} = %{version}-%{release}

%description devel
%{summary}.

%files devel
%{_includedir}/*

%{_kf5_libdir}/cmake/*
%{_kf5_libdir}/*.so

%changelog
* Sun Jun 11 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.04.2-1
- 23.04.2

* Sat May 13 2023 Marc Deop i Argemí <marcdeop@fedoraproject.org> - 23.04.1-1
- 23.04.1

* Fri Apr 14 2023 Marc Deop i Argemí <marcdeop@fedoraproject.org> - 23.04.0-1
- 23.04.0

* Fri Mar 31 2023 Marc Deop i Argemí <marcdeop@fedoraproject.org> - 23.03.90-1
- 23.03.90

* Mon Mar 20 2023 Marc Deop i Argemí <marcdeop@fedoraproject.org> - 23.03.80-1
- 23.03.80

* Thu Mar 02 2023 Marc Deop i Argemí <marcdeop@fedoraproject.org> - 22.12.3-1
- 22.12.3

* Tue Jan 31 2023 Marc Deop <marcdeop@fedoraproject.org> - 22.12.2-1
- 22.12.2

* Thu Jan 19 2023 Fedora Release Engineering <releng@fedoraproject.org> - 22.12.1-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_38_Mass_Rebuild

* Sun Jan 08 2023 Marc Deop <marcdeop@fedoraproject.org> - 22.12.1-1
- 22.12.1

* Thu Jul 21 2022 Fedora Release Engineering <releng@fedoraproject.org> - 22.04.3-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_37_Mass_Rebuild

* Mon Jul 18 2022 Than Ngo <than@redhat.com> - 22.04.3-1
- 22.04.3

* Mon May 16 2022 Justin Zobel <justin@1707.io> - 22.04.1-1
- Update to 22.04.1

* Thu Apr 21 2022 Justin Zobel <justin@1707.io> - 21.12.3-1
- Update to 21.12.3

* Wed Feb 09 2022 Justin Zobel <justin@1707.io> - 21.12.2-1
- Update to 21.12.2

* Wed Dec 22 2021 Justin Zobel <justin@1707.io> - 21.12-1
- Initial version of package
