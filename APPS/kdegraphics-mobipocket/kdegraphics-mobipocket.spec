Name:    kdegraphics-mobipocket 
Summary: A collection of plugins to handle mobipocket files 
Version: 23.08.2
Release: 1%{?dist}

License: GPLv2+
URL:     https://www.kde.org/applications/graphics/
%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0: https://download.kde.org/%{stable}/release-service/%{version}/src/%{name}-%{version}.tar.xz
Source1: https://download.kde.org/%{stable}/release-service/%{version}/src/%{name}-%{version}.tar.xz.sig
Source2: gpgkey-D81C0CB38EB725EF6691C385BB463350D6EF31EF.gpg

BuildRequires: gnupg2
BuildRequires: extra-cmake-modules
BuildRequires: kf5-rpm-macros
BuildRequires: cmake(Qt5Gui)
BuildRequires: cmake(KF5KIO)

Requires: qmobipocket%{?_isa} = %{version}-%{release}

Obsoletes: qmobipocket < 16.12.0
Provides:  qmobipocket = %{version}-%{release}
Provides:  qmobipocket%{?_isa} = %{version}-%{release}

%description
%{summary}.

%package devel
Summary: Development files for %{name}
Requires: %{name}%{?_isa} = %{version}-%{release}
Obsoletes: qmobipocket-devel < 16.12.0
Provides:  qmobipocket-devel = %{version}-%{release}
Provides:  qmobipocket-devel%{?_isa} = %{version}-%{release}
%description devel
%{summary}.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup


%build
%cmake_kf5
%cmake_build


%install
%cmake_install


%ldconfig_scriptlets

%files
%license COPYING
%{_libdir}/libqmobipocket.so.2*

%files devel
%{_libdir}/libqmobipocket.so
%{_includedir}/QMobipocket/
%{_libdir}/cmake/QMobipocket/


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

