%global framework bluez-qt

Name:           kf5-%{framework}
Summary:        A Qt wrapper for Bluez
Version: 5.112.0
Release: 1%{?dist}

License:        LGPLv2+
URL:            https://cgit.kde.org/%{framework}.git

%global versiondir %(echo %{version} | cut -d. -f1-2)
%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0:        http://download.kde.org/%{stable}/frameworks/%{versiondir}/%{framework}-%{version}.tar.xz
Source1:        http://download.kde.org/%{stable}/frameworks/%{versiondir}/%{framework}-%{version}.tar.xz.sig
Source2:        gpgkey-53E6B47B45CEA3E0D5B7457758D0EE648A48B3BB.gpg

BuildRequires:  gnupg2
BuildRequires:  extra-cmake-modules >= %{version}
BuildRequires:  kf5-rpm-macros >= %{version}
BuildRequires:  qt5-qtbase-devel
BuildRequires:  qt5-qtdeclarative-devel

# For %%{_udevrulesdir}
BuildRequires:  systemd

Requires:       kf5-filesystem >= %{version}
%if 0%{?fedora} > 22
Recommends:     bluez >= 5
%else
Requires:       bluez >= 5
%endif

%if 0%{?fedora} >= 22
## libbluedevil 5.2.2 was the last release
Obsoletes:      libbluedevil < 5.2.90
%endif

%description
BluezQt is Qt-based library written handle all Bluetooth functionality.

%package        devel
Summary:        Development files for %{name}
%if 0%{?fedora} >= 22
## libbluedevil 5.2.2 was the last release
Obsoletes:      libbluedevil-devel < 5.2.90
%endif
Requires:       %{name}%{?_isa} = %{version}-%{release}
Requires:       qt5-qtbase-devel
%description    devel
Development files for %{name}.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -n %{framework}-%{version}


%build
%cmake_kf5 \
  -DUDEV_RULES_INSTALL_DIR:PATH="%{_udevrulesdir}"
%cmake_build


%install
%cmake_install


%ldconfig_scriptlets

%files
%doc README.md
%license LICENSES/*.txt
%{_kf5_datadir}/qlogging-categories5/*categories
%{_libdir}/libKF5BluezQt.so.*
%{_kf5_qmldir}/org/kde/bluezqt/
%{_udevrulesdir}/61-kde-bluetooth-rfkill.rules

%files devel
%{_kf5_includedir}/BluezQt/
%{_kf5_libdir}/libKF5BluezQt.so
%{_kf5_libdir}/cmake/KF5BluezQt/
%{_kf5_libdir}/pkgconfig/KF5BluezQt.pc
%{_qt5_archdatadir}/mkspecs/modules/qt_BluezQt.pri


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

