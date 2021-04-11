%undefine __cmake_in_source_build

%global  framework purpose

Name:    kf5-purpose
Summary: Framework for providing abstractions to get the developer's purposes fulfilled
Version: 5.81.0
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

Obsoletes:  kf5-purpose-twitter < 5.68.0

## downstream patches
# src/quick/CMakeLists.txt calls 'cmake' directly, use 'cmake3' instead (mostly for epel7)
Patch100: purpose-5.79.0-cmake3.patch

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
%autosetup -n %{framework}-%{version} -p1


%build
%{cmake_kf5}

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
* Sun Apr 11 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.81.0-1
- 5.81.0

* Sat Mar 13 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.80.0-1
- 5.80.0

* Sat Feb 13 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.79.0-1
- 5.79.0

* Sat Jan  9 16:30:34 MSK 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.78.0-1
- 5.78.0

* Mon Dec 14 16:50:26 MSK 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.77.0-1
- 5.77.0

* Sun Nov 15 22:13:56 MSK 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.76.0-1
- 5.76.0

* Sat Oct 10 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.75.0-1
- 5.75.0

* Thu Sep 17 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.74.0-1
- 5.74.0

* Mon Aug 10 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.73.0-1
- 5.73.0

* Mon Jul 13 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.72.0-1
- 5.72.0

* Thu Jul 09 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.71.0-2
- rebuild

* Sun Jun 14 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.71.0-1
- 5.71.0

* Thu May 28 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.70.0-2
- rebuild

* Mon May 04 2020 Rex Dieter <rdieter@fedoraproject.org> - 5.70.0-1
- 5.70.0

* Tue Apr 21 2020 Rex Dieter <rdieter@fedoraproject.org> - 5.69.0-1
- 5.69.0

* Fri Mar 20 2020 Rex Dieter <rdieter@fedoraproject.org> - 5.68.0-1
- 5.68.0
- drop -twitter subpkg

* Mon Feb 03 2020 Rex Dieter <rdieter@fedoraproject.org> - 5.67.0-1
- 5.67.0

* Wed Jan 29 2020 Fedora Release Engineering <releng@fedoraproject.org> - 5.66.0-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_32_Mass_Rebuild

* Tue Jan 07 2020 Rex Dieter <rdieter@fedoraproject.org> - 5.66.0-1
- 5.66.0

* Tue Dec 17 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.65.0-1
- 5.65.0

* Fri Nov 08 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.64.0-1
- 5.64.0

* Tue Oct 22 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.63.0-1
- 5.63.0

* Mon Sep 16 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.62.0-1
- 5.62.0

* Wed Aug 07 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.61.0-1
- 5.61.0

* Thu Jul 25 2019 Fedora Release Engineering <releng@fedoraproject.org> - 5.60.0-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_31_Mass_Rebuild

* Sat Jul 13 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.60.0-1
- 5.60.0

* Fri Jun 21 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.59.0-2
- add builddeps for sharefileitem plugin (#1721808)

* Thu Jun 06 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.59.0-1
- 5.59.0

* Tue May 07 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.58.0-1
- 5.58.0

* Tue Apr 09 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.57.0-1
- 5.57.0

* Tue Mar 05 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.56.0-1
- 5.56.0

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

