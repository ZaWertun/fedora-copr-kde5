%global framework kwallet

# uncomment to enable bootstrap mode
#global bootstrap 1

%if !0%{?bootstrap}
%global tests 1
%endif

Name:    kf5-%{framework}
Version: 5.112.0
Release: 1%{?dist}
Summary: KDE Frameworks 5 Tier 3 solution for password management

License: LGPLv2+
URL:     https://cgit.kde.org/%{framework}.git

%global majmin %(echo %{version} | cut -d. -f1-2)
%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0: http://download.kde.org/%{stable}/frameworks/%{majmin}/%{framework}-%{version}.tar.xz
Source1: http://download.kde.org/%{stable}/frameworks/%{majmin}/%{framework}-%{version}.tar.xz.sig
Source2: gpgkey-53E6B47B45CEA3E0D5B7457758D0EE648A48B3BB.gpg

## upstream patches

## upstreamable patches

BuildRequires:  gnupg2
BuildRequires:  extra-cmake-modules >= %{majmin}
BuildRequires:  kf5-kconfig-devel >= %{majmin}
BuildRequires:  kf5-kconfigwidgets-devel >= %{majmin}
BuildRequires:  kf5-kcoreaddons-devel >= %{majmin}
BuildRequires:  kf5-kdbusaddons-devel >= %{majmin}
BuildRequires:  kf5-kdoctools-devel >= %{majmin}
BuildRequires:  kf5-ki18n-devel >= %{majmin}
BuildRequires:  kf5-kiconthemes-devel >= %{majmin}
BuildRequires:  kf5-knotifications-devel >= %{majmin}
BuildRequires:  kf5-kservice-devel >= %{majmin}
BuildRequires:  kf5-kwidgetsaddons-devel >= %{majmin}
BuildRequires:  kf5-kwindowsystem-devel >= %{majmin}
BuildRequires:  kf5-rpm-macros

BuildRequires:  cmake(QGpgme)
BuildRequires:  cmake(Qca-qt5)
BuildRequires:  qt5-qtbase-devel

BuildRequires:  libgcrypt-devel

%if ! 0%{?bootstrap} && 0%{?fedora}
# optional gpgme suppot
BuildRequires:  cmake(Gpgmepp)
%endif

Requires:       %{name}-libs%{?_isa} = %{version}-%{release}

# gpg support ui
%if 0%{?fedora} < 26 && 0%{?rhel} < 8
Requires:       pinentry-gui
%else
Recommends:     pinentry-gui
%endif

%description
KWallet is a secure and unified container for user passwords.

%package        libs
Summary:        KWallet framework libraries
Requires:       %{name} = %{version}-%{release}
%description    libs
Provides API to access KWallet data from applications.

%package        devel
Summary:        Development files for %{name}
Requires:       %{name}-libs%{?_isa} = %{version}-%{release}
Requires:       qt5-qtbase-devel

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
%find_lang %{name} --all-name --with-man


%check
%if 0%{?tests}
export CTEST_OUTPUT_ON_FAILURE=1
make test ARGS="--output-on-failure --timeout 30" -C %{_target_platform} ||:
%endif


%files -f %{name}.lang
%doc README.md
%license LICENSES/*.txt
%{_kf5_datadir}/qlogging-categories5/%{framework}*
%{_kf5_datadir}/dbus-1/services/org.kde.kwalletd5.service
%{_kf5_bindir}/kwalletd5
%{_kf5_datadir}/applications/org.kde.kwalletd5.desktop
%{_kf5_datadir}/kservices5/kwalletd5.desktop
%{_kf5_datadir}/knotifications5/kwalletd5.notifyrc
%{_kf5_bindir}/kwallet-query
%{_mandir}/man1/kwallet-query.1*

%ldconfig_scriptlets libs

%files libs
%{_kf5_libdir}/libKF5Wallet.so.*
%{_kf5_libdir}/libkwalletbackend5.so.*

%files devel
%{_kf5_datadir}/dbus-1/interfaces/kf5_org.kde.KWallet.xml
%{_kf5_includedir}/KWallet/
%{_kf5_libdir}/cmake/KF5Wallet/
%{_kf5_libdir}/libKF5Wallet.so
%{_kf5_libdir}/libkwalletbackend5.so
%{_kf5_archdatadir}/mkspecs/modules/qt_KWallet.pri


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

