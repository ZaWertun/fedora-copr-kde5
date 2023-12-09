%global framework kpkpass
%global tests 1

Name:    kf5-%{framework}
Version: 23.08.4
Release: 1%{?dist}
Summary: Library to deal with Apple Wallet pass files

License: LGPLv2+
URL:     https://cgit.kde.org/%{framework}.git

%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0:        https://download.kde.org/%{stable}/release-service/%{version}/src/%{framework}-%{version}.tar.xz
Source1:        https://download.kde.org/%{stable}/release-service/%{version}/src/%{framework}-%{version}.tar.xz.sig
Source2:        gpgkey-D81C0CB38EB725EF6691C385BB463350D6EF31EF.gpg

BuildRequires:  gnupg2
BuildRequires:  extra-cmake-modules
BuildRequires:  kf5-rpm-macros

BuildRequires:  cmake(KF5Archive)

BuildRequires:  qt5-qtbase-devel

BuildRequires:  pkgconfig(shared-mime-info)
%if "%(pkg-config --modversion shared-mime-info 2> /dev/null || echo 2.1)" < "2.2"
%global mime 1
%endif

%if 0%{?tests}
BuildRequires: dbus-x11
BuildRequires: xorg-x11-server-Xvfb
%endif

%description
%{summary}

%package        devel
Summary:        Development files for %{name}
Requires:       %{name}%{?_isa} = %{version}-%{release}
%description    devel
The %{name}-devel package contains libraries and header files for
developing applications that use %{name}.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -n %{framework}-%{version} -p1


%build
%cmake_kf5 \
  -DBUILD_TESTING:BOOL=%{?tests:ON}%{!?tests:OFF}
%cmake_build


%install
%cmake_install


%check
%if 0%{?tests}
export CTEST_OUTPUT_ON_FAILURE=1
make test/fast ARGS="--output-on-failure --timeout 10" -C %{_vpath_builddir} ||:
%endif


%ldconfig_scriptlets

%files
%license LICENSES/*.txt
%doc README.md
%{_kf5_libdir}/libKPim5PkPass.so.5*
%{_kf5_datadir}/qlogging-categories5/*.categories
%if 0%{?mime}
%{_datadir}/mime/packages/application-vnd-apple-pkpass.xml
%endif


%files devel
%{_includedir}/KPim5/KPkPass/
%{_kf5_libdir}/libKPim5PkPass.so
%{_kf5_libdir}/cmake/KPimPkPass/
%{_kf5_libdir}/cmake/KPim5PkPass/


%changelog
* Fri Dec 08 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.08.4-1
- 23.08.4

* Fri Nov 10 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.08.3-1
- 23.08.3

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

