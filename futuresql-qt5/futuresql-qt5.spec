%global tests 1
%global project futuresql

Name:           futuresql-qt5
Version:        0.1.1
Release:        1%{?dist}
Summary:        Non-blocking Qt database framework

License:        BSD-2-Clause and LGPLv2 and LGPLv3
URL:            https://invent.kde.org/libraries/%{project}
Source0:        https://download.kde.org/stable/%{project}/%{project}-%{version}.tar.xz
Source1:        https://download.kde.org/stable/%{project}/%{project}-%{version}.tar.xz.sig
Source2:        gpgkey-C3D7CAFBF442353F95F69F4AA81E075ABEC80A7E.gpg

BuildRequires:  gnupg2
BuildRequires:  gcc-c++
BuildRequires:  extra-cmake-modules >= 5.93.0
BuildRequires:  cmake(Qt5Core) >= 5.15.2
BuildRequires:  cmake(Qt5Sql)
BuildRequires:  cmake(QCoro5)

Requires:       cmake(QCoro5)

%description
%{summary}.

%package   devel
Summary:   Development files for %{name}
Requires:  %{name}%{?_isa} = %{version}-%{release}
%description devel
The %{name}-devel package contains libraries and header files for
developing applications that use %{name}.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -p1 -n %{project}-%{version}


%build
%cmake_kf5 -DBUILD_TESTING:BOOL=%{?tests:ON}%{!?tests:OFF}
%cmake_build


%install
%cmake_install


%check
%if 0%{?tests}
%ctest --output-on-failure --timeout 30
%endif


%files
%license LICENSES/*.txt
%doc README.md
%{_libdir}/libfuturesql5.so.0


%files devel
%{_includedir}/FutureSQL5/
%{_libdir}/cmake/FutureSQL5/
%{_libdir}/libfuturesql5.so


%changelog
* Mon Aug 28 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 0.1.1-1
- first spec for version 0.1.1
