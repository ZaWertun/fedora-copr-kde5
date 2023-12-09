%global framework libksane

Name:    kf5-libksane
Summary: SANE Library interface for KDE
Version: 23.08.4
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
BuildRequires: gettext
BuildRequires: kf5-kconfig-devel
BuildRequires: kf5-ki18n-devel
BuildRequires: kf5-kwallet-devel
BuildRequires: kf5-kwidgetsaddons-devel
BuildRequires: kf5-ktextwidgets-devel
BuildRequires: kf5-rpm-macros
BuildRequires: cmake(Qt5Widgets)
BuildRequires: pkgconfig(sane-backends)
BuildRequires: cmake(KSaneCore)

# translations moved here
Conflicts: kde-l10n < 17.03

Requires: kf5-filesystem

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


%ldconfig_scriptlets

%files -f %{name}.lang
%doc AUTHORS
%license LICENSES/*.txt
%{_kf5_libdir}/libKF5Sane.so.5*
%{_kf5_libdir}/libKF5Sane.so.%{version_major}*
%{_kf5_datadir}/icons/hicolor/*/actions/*

%files devel
%{_kf5_includedir}/KSane/
%{_kf5_libdir}/libKF5Sane.so
%{_kf5_libdir}/cmake/KF5Sane/


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

