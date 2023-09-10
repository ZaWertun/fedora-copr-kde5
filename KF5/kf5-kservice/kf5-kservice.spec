%global framework kservice

Name:    kf5-%{framework}
Summary: KDE Frameworks 5 Tier 3 solution for advanced plugin and service introspection
Version: 5.110.0
Release: 1%{?dist}

# mixture of LGPLv2 and LGPLv2+ (mostly the latter)
License: LGPLv2
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

## downstream patches
# Fedora customizations to the menu categories
# adds the Administration menu from redhat-menus which equals System + Settings
# This prevents the stuff getting listed twice, under both System and Settings.
Patch100:  kservice-5.15.0-xdg-menu.patch

# kbuildsycoca5 always gives:
# kf5.kservice.sycoca: Parse error in  "$HOME/.config/menus/applications-merged/xdg-desktop-menu-dummy.menu" , line  1 , col  1 :  "unexpected end of file"
# hide that by default, make it qCDebug instead (of qCWarning)
Patch101:  kservice-5.17.0-vfolder_spam.patch

BuildRequires:  gnupg2
BuildRequires:  extra-cmake-modules >= %{majmin}
BuildRequires:  kf5-kconfig-devel >= %{majmin}
BuildRequires:  kf5-kcoreaddons-devel >= %{majmin}
BuildRequires:  kf5-kcrash-devel >= %{majmin}
BuildRequires:  kf5-kdbusaddons-devel >= %{majmin}
BuildRequires:  kf5-kdoctools-devel >= %{majmin}
BuildRequires:  kf5-ki18n-devel >= %{majmin}
BuildRequires:  kf5-rpm-macros
BuildRequires:  qt5-qtbase-devel

BuildRequires:  flex
BuildRequires:  bison

# for the Administration category
Recommends:       redhat-menus

%description
KDE Frameworks 5 Tier 3 solution for advanced plugin and service
introspection.

%package        devel
Summary:        Development files for %{name}
Requires:       %{name}%{?_isa} = %{version}-%{release}
Requires:       kf5-kconfig-devel >= %{majmin}
Requires:       kf5-kcoreaddons-devel >= %{majmin}
%description    devel
The %{name}-devel package contains libraries and header files for
developing applications that use %{name}.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -n %{framework}-%{version} -p1


%build
%cmake_kf5
%cmake_build


%install
%cmake_install

%find_lang %{name} --all-name --with-man

mv %{buildroot}%{_kf5_sysconfdir}/xdg/menus/applications.menu \
   %{buildroot}%{_kf5_sysconfdir}/xdg/menus/kf5-applications.menu

mkdir -p %{buildroot}%{_kf5_datadir}/kservices5
mkdir -p %{buildroot}%{_kf5_datadir}/kservicetypes5


%ldconfig_scriptlets

%files -f %{name}.lang
%doc README.md
%license LICENSES/*.txt
# this is not a config file, despite rpmlint complaining otherwise -- rex
%{_kf5_sysconfdir}/xdg/menus/kf5-applications.menu
%{_kf5_datadir}/qlogging-categories5/%{framework}.*
%{_kf5_bindir}/kbuildsycoca5
%{_kf5_libdir}/libKF5Service.so.5*
%{_kf5_datadir}/kservicetypes5/
%{_kf5_datadir}/kservices5/
%{_kf5_mandir}/man8/*.8*

%files devel
%{_kf5_includedir}/KService/
%{_kf5_libdir}/libKF5Service.so
%{_kf5_libdir}/cmake/KF5Service/
%{_kf5_archdatadir}/mkspecs/modules/qt_KService.pri


%changelog
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

