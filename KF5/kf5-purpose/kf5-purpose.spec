%global  framework purpose

Name:    kf5-purpose
Summary: Framework for providing abstractions to get the developer's purposes fulfilled
Version: 5.109.0
Release: 1%{?dist}

# KDE e.V. may determine that future GPL versions are accepted
# most files LGPLv2+, configuration.cpp is KDE e.V. GPL variant
License: GPLv2 or GPLv3
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

Obsoletes:  kf5-purpose-twitter < 5.68.0

## downstream patches
# src/quick/CMakeLists.txt calls 'cmake' directly, use 'cmake3' instead (mostly for epel7)
%if 0%{?rhel} && 0%{?rhel} < 8
Patch100: purpose-5.79.0-cmake3.patch
%endif

# filter plugin provides
%global __provides_exclude_from ^(%{_kf5_qtplugindir}/.*\\.so)$

BuildRequires: gnupg2
BuildRequires: extra-cmake-modules >= %{majmin}
BuildRequires: gettext
BuildRequires: intltool

BuildRequires: kf5-rpm-macros
BuildRequires: kf5-kconfig-devel >= %{majmin}
BuildRequires: kf5-kcoreaddons-devel >= %{majmin}
BuildRequires: kf5-kdeclarative-devel >= %{majmin}
BuildRequires: kf5-ki18n-devel >= %{majmin}
BuildRequires: kf5-kio-devel >= %{majmin}
BuildRequires: kf5-kirigami2-devel >= %{majmin}

BuildRequires: cmake(KF5Kirigami2)

# optional sharefile plugin
BuildRequires: cmake(KF5KIO)
BuildRequires: cmake(KF5Notifications)

BuildRequires: pkgconfig(Qt5Network)
BuildRequires: pkgconfig(Qt5Qml)

%if 0%{?fedora} || 0%{?rhel} > 7
BuildRequires: kaccounts-integration-devel
# runtime dep?
BuildRequires: kde-connect
BuildRequires: pkgconfig(accounts-qt5)
BuildRequires: pkgconfig(libaccounts-glib)
%endif

%description
Purpose offers the possibility to create integrate services and actions on
any application without having to implement them specifically. Purpose will
offer them mechanisms to list the different alternatives to execute given the
requested action type and will facilitate components so that all the plugins
can receive all the information they need.

%package  devel
Summary:  Development files for %{name}
Requires: %{name}%{?_isa} = %{version}-%{release}
Requires: cmake(KF5CoreAddons)
%description devel
%{summary}.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -n %{framework}-%{version} -p1


%build
%cmake_kf5
%cmake_build


%install
%cmake_install

%find_lang %{name} --all-name

## unpackaged files
# omit (unused?) conflicting icons with older kamoso (rename to "google-youtube"?)
rm -fv %{buildroot}%{_datadir}/icons/hicolor/*/actions/kipiplugin_youtube.png


%ldconfig_scriptlets

%files -f %{name}.lang
%doc README.md
%license LICENSES/*.txt
%{_kf5_datadir}/qlogging-categories5/%{framework}.*
%{_kf5_libdir}/libKF5Purpose.so.5*
%{_kf5_libdir}/libKF5PurposeWidgets.so.5*
%{_kf5_libdir}/libPhabricatorHelpers.so.5*
%{_kf5_libdir}/libReviewboardHelpers.so.5*
%{_kf5_libexecdir}/purposeprocess
%{_kf5_datadir}/purpose/
%{_kf5_plugindir}/purpose/
%dir %{_kf5_plugindir}/kfileitemaction/
%{_kf5_plugindir}/kfileitemaction/sharefileitemaction.so
%{_kf5_qmldir}/org/kde/purpose/
# this conditional may require adjusting too (e.g. wrt %%twitter)
%if 0%{?fedora} || 0%{?rhel} > 7
%{_kf5_datadir}/accounts/services/kde/google-youtube.service
%{_kf5_datadir}/accounts/services/kde/nextcloud-upload.service
%endif
%{_datadir}/icons/hicolor/*/apps/*-purpose.*
#{_datadir}/icons/hicolor/*/actions/google-youtube.*

%files devel
%{_kf5_libdir}/libKF5Purpose.so
%{_kf5_libdir}/libKF5PurposeWidgets.so
%{_kf5_includedir}/purpose/
%{_kf5_includedir}/purposewidgets/
%{_kf5_libdir}/cmake/KDEExperimentalPurpose/
%{_kf5_libdir}/cmake/KF5Purpose/


%changelog
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

