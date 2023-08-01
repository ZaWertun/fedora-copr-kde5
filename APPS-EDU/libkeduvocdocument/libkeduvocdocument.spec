# uncomment to enable bootstrap mode
#global bootstrap 1

%if !0%{?bootstrap}
%global tests 1
%endif

Name:    libkeduvocdocument
Summary: Library to parse, convert, and manipulate KVTML files
Version: 23.04.3
Release: 1%{?dist}

License: GPLv2+
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

## upstream patches
# master branch

BuildRequires:  gnupg2
BuildRequires:  extra-cmake-modules
BuildRequires:  kf5-karchive-devel
BuildRequires:  kf5-ki18n-devel
BuildRequires:  kf5-kio-devel
BuildRequires:  kf5-rpm-macros
BuildRequires:  pkgconfig(Qt5Xml)

%if 0%{?tests}
BuildRequires: xorg-x11-server-Xvfb
%endif

%global majmin_ver %(echo %{version} | cut -d. -f1,2)
Requires:       kdeedu-data >= %{majmin_ver}


%description
A Library to parse, convert, and manipulate KVTML files (and older formats
including kvtml1, csv, etc.).

%package devel
Summary:  Development files for %{name}
License:  GPLv2+ and LGPLv2 and BSD
Requires: %{name}%{?_isa} = %{version}-%{release}
%description devel
%{summary}.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -p1


%build
%cmake_kf5 \
  %{?tests:-DBUILD_TESTING:BOOL=ON}
%cmake_build


%install
%cmake_install

%find_lang %{name} --all-name --with-html


%check
%if 0%{?tests}
export CTEST_OUTPUT_ON_FAILURE=1
xvfb-run -a \
make test -C %{_target_platform} ARGS="--output-on-failure --timeout 300" ||:
%endif


%ldconfig_scriptlets

%files -f %{name}.lang
%doc AUTHORS README 
%license LICENSES/*.txt
%{_kf5_libdir}/libKEduVocDocument.so.5*

%files devel
%{_includedir}/libkeduvocdocument/
%{_kf5_libdir}/libKEduVocDocument.so
%license COPYING-CMAKE-SCRIPTS 
%{_kf5_libdir}/cmake/libkeduvocdocument/


%changelog
* Thu Jul 06 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.04.3-1
- 23.04.3

* Thu Jun 08 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.04.2-1
- 23.04.2

* Thu May 11 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.04.1-1
- 23.04.1

* Fri Apr 21 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.04.0-1
- 23.04.0

* Thu Mar 02 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.12.3-1
- 22.12.3

* Thu Jan 05 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.12.1-1
- 22.12.1

