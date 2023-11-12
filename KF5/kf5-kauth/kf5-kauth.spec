%global framework kauth

Name:    kf5-%{framework}
Version: 5.112.0
Release: 1%{?dist}
Summary: KDE Frameworks 5 Tier 2 integration module to perform actions as privileged user

License: LGPLv2+
URL:     https://cgit.kde.org/%{framework}.git

%global majmin %(echo %{version} | cut -d. -f1-2)
%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0:        http://download.kde.org/%{stable}/frameworks/%{majmin}/%{framework}-%{version}.tar.xz
Source1:        http://download.kde.org/%{stable}/frameworks/%{majmin}/%{framework}-%{version}.tar.xz.sig
Source2:        gpgkey-53E6B47B45CEA3E0D5B7457758D0EE648A48B3BB.gpg

BuildRequires:  gnupg2
BuildRequires:  extra-cmake-modules >= %{majmin}
BuildRequires:  kf5-kcoreaddons-devel >= %{majmin}
BuildRequires:  kf5-rpm-macros

BuildRequires:  polkit-qt5-1-devel
BuildRequires:  qt5-qtbase-devel
BuildRequires:  qt5-qttools-devel

%description
KAuth is a framework to let applications perform actions as a privileged user.

%package        devel
Summary:        Development files for %{name}
Requires:       %{name}%{?_isa} = %{version}-%{release}
Requires:       kf5-kcoreaddons-devel >= %{majmin}
%description    devel
The %{name}-devel package contains libraries and header files for
developing applications that use %{name}.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -n %{framework}-%{version} -p1


%build
%cmake_kf5 \
  -DKDE_INSTALL_LIBEXECDIR=%{_kf5_libexecdir}

%cmake_build


%install
%cmake_install
%find_lang_kf5 kauth5_qt


%ldconfig_scriptlets

%files -f kauth5_qt.lang
%doc README.md
%license LICENSES/*.txt
%{_kf5_libdir}/libKF5Auth*.so.5*
%{_kf5_datadir}/dbus-1/system.d/org.kde.kf5auth.conf
%{_kf5_qtplugindir}/kauth/
%{_kf5_datadir}/kf5/kauth/
%{_kf5_libexecdir}/kauth/
%{_kf5_datadir}/qlogging-categories5/kauth.categories
%{_kf5_datadir}/qlogging-categories5/kauth.renamecategories

%files devel
%{_kf5_includedir}/KAuth/
%{_kf5_includedir}/KAuthCore/
%{_kf5_includedir}/KAuthWidgets/
%{_kf5_libdir}/libKF5Auth*.so
%{_kf5_libdir}/cmake/KF5Auth/
%{_kf5_archdatadir}/mkspecs/modules/qt_KAuth*.pri


%changelog
* Sun Nov 12 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.112.0-1
- 5.112.0

* Thu Oct 19 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.111.0-1
- 5.111.0

* Sun Sep 10 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.110.0-1
- 5.110.0

* Fri Aug 18 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.109.0-1
- 5.109.0

* Sun Jul 09 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.108.0-1
- 5.108.0

* Sat Jun 10 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.107.0-1
- 5.107.0

* Sat May 13 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.106.0-1
- 5.106.0

* Sat Apr 08 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.105.0-1
- 5.105.0

* Sat Mar 11 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.104.0-1
- 5.104.0

* Sun Feb 12 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.103.0-1
- 5.103.0

