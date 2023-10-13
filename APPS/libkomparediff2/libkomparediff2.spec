# enable tests
%global tests 1

Name:    libkomparediff2
Summary: Library to compare files and strings
Version: 23.08.2
Release: 1%{?dist}

# Library: GPLv2+ (some files LGPLv2+), CMake scripts: BSD
License: GPLv2+ and BSD
URL:     https://cgit.kde.org/%{name}.git

%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0: https://download.kde.org/%{stable}/release-service/%{version}/src/%{name}-%{version}.tar.xz
Source1: https://download.kde.org/%{stable}/release-service/%{version}/src/%{name}-%{version}.tar.xz.sig
Source2: gpgkey-D81C0CB38EB725EF6691C385BB463350D6EF31EF.gpg

BuildRequires:  gnupg2
BuildRequires:  gcc-c++
BuildRequires:  kf5-rpm-macros
BuildRequires:  extra-cmake-modules
BuildRequires:  qt5-qtbase-devel
BuildRequires:  kf5-kcoreaddons-devel
BuildRequires:  kf5-kcodecs-devel
BuildRequires:  kf5-kconfig-devel
BuildRequires:  kf5-kxmlgui-devel
BuildRequires:  kf5-ki18n-devel
BuildRequires:  kf5-kio-devel
BuildRequires:  kf5-kparts-devel

# translations moved here
Conflicts: kde-l10n < 17.03

Requires:       diffutils

Obsoletes: kompare-libs < 4.11.80
Obsoletes: kdesdk-kompare-libs < 4.10.80

%description
A shared library to compare files and strings using KDE Frameworks 5 and GNU
diff, used in Kompare and KDevelop.

%package devel
Summary: Developer files for %{name}
Requires: %{name}%{?_isa} = %{version}-%{release}
Requires: cmake
Requires: qt5-qtbase-devel
Requires: kf5-kcodecs-devel
Requires: kf5-kcoreaddons-devel
Requires: kf5-ki18n-devel
Requires: kf5-kio-devel
Requires: kf5-kxmlgui-devel
# Conflict with old Kompare builds which included libkomparediff2.
Conflicts:      kompare-devel < 4.11.80
# The library was unversioned in 4.10, so conflict with main Kompare package.
Conflicts:      kdesdk-kompare < 4.10.80
%description devel
The %{name}-devel package contains libraries and header files for
developing applications that use %{name}.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup


%build
%cmake_kf5 \
  -DBUILD_TESTING:BOOL=%{?tests}%{!?tests:0}
%cmake_build


%install
%cmake_install

%find_lang %{name} --all-name


%check
%if 0%{?tests}
make test/fast -C %{__cmake_builddir}
%endif


%ldconfig_scriptlets

%files -f %{name}.lang
%license COPYING*
%{_libdir}/libkomparediff2.so.5*
%{_kf5_datadir}/qlogging-categories5/libkomparediff2.categories

%files devel
%{_includedir}/KompareDiff2/
%{_libdir}/libkomparediff2.so
%{_libdir}/cmake/LibKompareDiff2/


%changelog
* Fri Oct 13 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.08.2-1
- 23.08.2

* Thu Sep 14 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.08.1-1
- 23.08.1

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

