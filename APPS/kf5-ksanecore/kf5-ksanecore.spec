%global framework ksanecore

Name:    kf5-%{framework}
Summary: Provides a Qt interface for the SANE library for scanner hardware
Version: 22.08.1
Release: 1%{?dist}

%global version_major %(echo %{version} |cut -d. -f1)

License: LGPLv2+
URL:     https://cgit.kde.org/%{framework}.git
%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0: https://download.kde.org/%{stable}/release-service/%{version}/src/%{framework}-%{version}.tar.xz
Source1: https://download.kde.org/%{stable}/release-service/%{version}/src/%{framework}-%{version}.tar.xz.sig
Source2: gpgkey-D81C0CB38EB725EF6691C385BB463350D6EF31EF.gpg

BuildRequires: gnupg2
BuildRequires: extra-cmake-modules

BuildRequires: cmake(Qt5Gui)
BuildRequires: cmake(Qt5Core)

BuildRequires: cmake(KF5I18n)

BuildRequires: pkgconfig(sane-backends)

%description
%{summary}.

%package devel
Summary:  Development files for %{name}
Requires: %{name}%{?_isa} = %{version}-%{release}
Requires: cmake(Qt5Widgets)
%description devel
%{summary}.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -n %{framework}-%{version}


%build
%cmake_kf5
%cmake_build


%install
%cmake_install
%find_lang %{name} --all-name --with-html 


%files -f %{name}.lang
%doc README.md
%license LICENSES/*.txt
%{_kf5_libdir}/libKSaneCore.so.1
%{_kf5_libdir}/libKSaneCore.so.%{version}

%files devel
%{_includedir}/KSaneCore/
%{_kf5_libdir}/libKSaneCore.so
%{_kf5_libdir}/cmake/KSaneCore/


%changelog
* Thu Sep 08 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.08.1-1
- 22.08.1

* Fri Aug 19 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.08.0-1
- First spec for version 22.08.0

