
%global  framework purpose

Name:    kf5-purpose
Summary: Framework for providing abstractions to get the developer's purposes fulfilled
Version: 5.60.0
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

## downstream patches
# src/quick/CMakeLists.txt calls 'cmake' directly, use 'cmake3' instead (mostly for epel7)
Patch100: purpose-5.47.0-cmake3.patch

# filter plugin provides
%global __provides_exclude_from ^(%{_kf5_qtplugindir}/.*\\.so)$

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
BuildRequires: pkgconfig(Qt5Network)
BuildRequires: pkgconfig(Qt5Qml)

%if 0%{?fedora} || 0%{?rhel} > 7
%global twitter 1
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

%if 0%{?twitter}
%package twitter
Summary: twitter plugin for %{name}
Requires: %{name}%{?_isa} = %{version}-%{release}
Supplements: (%{name} and nodejs)
%description twitter
%{summary}.
%endif


%prep
%autosetup -n %{framework}-%{version} -p1


%build
mkdir %{_target_platform}
pushd %{_target_platform}
%{cmake_kf5} ..
popd

%make_build -C %{_target_platform}


%install
make install/fast DESTDIR=%{buildroot} -C %{_target_platform}

%find_lang %{name} --all-name

## unpackaged files
# omit (unused?) conflicting icons with older kamoso (rename to "google-youtube"?)
rm -fv %{buildroot}%{_datadir}/icons/hicolor/*/actions/kipiplugin_youtube.png


%ldconfig_scriptlets

%files -f %{name}.lang
%doc README.md
%license COPYING.LIB
%{_kf5_libdir}/libKF5Purpose.so.5*
%{_kf5_libdir}/libKF5PurposeWidgets.so.5*
%{_kf5_libdir}/libPhabricatorHelpers.so.5*
%{_kf5_libdir}/libReviewboardHelpers.so.5*
%{_kf5_libexecdir}/purposeprocess
%{_kf5_datadir}/purpose/
%{_kf5_plugindir}/purpose/
%{_kf5_qmldir}/org/kde/purpose/
# this conditional may require adjusting too (e.g. wrt %%twitter)
%if 0%{?fedora}
%{_kf5_datadir}/accounts/services/kde/google-youtube.service
%{_kf5_datadir}/accounts/services/kde/nextcloud-upload.service
%{_kf5_datadir}/accounts/services/kde/twitter-microblog.service
%dir %{_kf5_datadir}/kpackage/Purpose/
%endif
%{_datadir}/icons/hicolor/*/apps/*-purpose.*
#{_datadir}/icons/hicolor/*/actions/google-youtube.*
%{_kf5_datadir}/qlogging-categories5/*.categories

%if 0%{?twitter}
%files twitter
%{_kf5_datadir}/kpackage/Purpose/Twitter/
%endif

%files devel
%{_kf5_libdir}/libKF5Purpose.so
%{_kf5_libdir}/libKF5PurposeWidgets.so
%{_kf5_includedir}/purpose/
%{_kf5_includedir}/purposewidgets/
%{_kf5_libdir}/cmake/KDEExperimentalPurpose/
%{_kf5_libdir}/cmake/KF5Purpose/


%changelog
* Sat Jul 13 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.60.0-1
- 5.60.0

* Sat Jun 08 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.59.0-1
- 5.59.0

* Tue May 14 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.58.0-1
- 5.58.0

* Sat Apr 27 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.57.0-1
- 5.57.0

* Mon Feb 04 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.55.0-1
- 5.55.0

* Fri Feb 01 2019 Fedora Release Engineering <releng@fedoraproject.org> - 5.54.0-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_30_Mass_Rebuild

* Wed Jan 09 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.54.0-1
- 5.54.0

* Sun Dec 09 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.53.0-1
- 5.53.0

* Sun Nov 04 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.52.0-1
- 5.52.0

* Wed Oct 10 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.51.0-1
- 5.51.0

* Tue Sep 04 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.50.0-1
- 5.50.0

* Tue Aug 07 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.49.0-1
- 5.49.0

* Mon Jul 09 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.48.0-1
- 5.48.0

* Sun Jun 17 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.47.0-3
- twitter subpkg, spilts out extra nodejs runtime dep (#1546510)

* Sat Jun 09 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.47.0-2
- cleanup, mark some deps optional (fedora only)

* Sat Jun 02 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.47.0-1
- 5.47.0

* Sat May 05 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.46.0-1
- 5.46.0

* Sun Apr 08 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.45.0-1
- 5.45.0

* Sat Mar 03 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.44.0-1
- 5.44.0

* Mon Feb 12 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.43.0-1
- 5.43.0 (frameworks release)
- use %%ldconfig_scriptlets

* Wed Feb 07 2018 Fedora Release Engineering <releng@fedoraproject.org> - 1.1-7
- Rebuilt for https://fedoraproject.org/wiki/Fedora_28_Mass_Rebuild

* Thu Jan 11 2018 Igor Gnatenko <ignatenkobrain@fedoraproject.org> - 1.1-6
- Remove obsolete scriptlets

* Thu Aug 03 2017 Fedora Release Engineering <releng@fedoraproject.org> - 1.1-5
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Binutils_Mass_Rebuild

* Wed Jul 26 2017 Fedora Release Engineering <releng@fedoraproject.org> - 1.1-4
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Mass_Rebuild

* Fri Feb 10 2017 Fedora Release Engineering <releng@fedoraproject.org> - 1.1-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_26_Mass_Rebuild

* Mon Jan 02 2017 Rex Dieter <rdieter@fedoraproject.org> - 1.1-2
- filter plugin provides

* Thu Jun 02 2016 Rex Dieter <rdieter@fedoraproject.org> - 1.1-1
- 1.1, update URL

* Thu Feb 18 2016 Rex Dieter <rdieter@fedoraproject.org> 1.0-5
- -BR: cmake

* Thu Feb 04 2016 Fedora Release Engineering <releng@fedoraproject.org> - 1.0-4
- Rebuilt for https://fedoraproject.org/wiki/Fedora_24_Mass_Rebuild

* Mon Jan 11 2016 Rex Dieter <rdieter@fedoraproject.org> 1.0-3
- omit unused/conflicting kipiplugin_youtube icons

* Wed Dec 30 2015 Rex Dieter <rdieter@fedoraproject.org> 1.0-2
- update URL, Source

* Mon Sep 28 2015 Rex Dieter <rdieter@fedoraproject.org>  1.0-1
- first try

