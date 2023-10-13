%global framework libkexiv2

Name:    kf5-libkexiv2
Summary: A wrapper around Exiv2 library
Version: 23.08.2
Release: 1%{?dist}

License: GPLv2+
URL:     https://cgit.kde.org/%{framework}
%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0: https://download.kde.org/%{stable}/release-service/%{version}/src/%{framework}-%{version}.tar.xz
Source1: https://download.kde.org/%{stable}/release-service/%{version}/src/%{framework}-%{version}.tar.xz.sig
Source2: gpgkey-D81C0CB38EB725EF6691C385BB463350D6EF31EF.gpg

## upstream patches (master branch)

BuildRequires: gnupg2
BuildRequires: extra-cmake-modules
BuildRequires: kf5-rpm-macros
BuildRequires: cmake(Qt5Gui)
BuildRequires: pkgconfig(exiv2)

Requires: kf5-filesystem

%description
Libkexiv2 is a wrapper around Exiv2 library to manipulate pictures metadata
as EXIF IPTC and XMP.

%package devel
Summary:  Development files for %{name}
Requires: %{name}%{?_isa} = %{version}-%{release}
Requires: cmake(Qt5Gui)
%description devel
%{summary}.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -n %{framework}-%{version} -p1


%build
%cmake_kf5
%cmake_build


%install
%cmake_install


%ldconfig_scriptlets

%files
%doc AUTHORS README
%license LICENSES/*.txt
%{_kf5_libdir}/libKF5KExiv2.so.*

%files devel
%{_kf5_libdir}/libKF5KExiv2.so
%{_kf5_includedir}/KExiv2/
%{_kf5_libdir}/cmake/KF5KExiv2/
%{_kf5_datadir}/qlogging-categories5/libkexiv2.categories


%changelog
* Fri Oct 13 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.08.2-1
- 23.08.2

* Thu Sep 14 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.08.1-1
- 23.08.1

* Sun Aug 27 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.08.0-1
- 23.08.0

* Thu Jul 06 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.04.3-1
- 23.04.3

* Wed Jun 14 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.04.2-5
- rebuild

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

