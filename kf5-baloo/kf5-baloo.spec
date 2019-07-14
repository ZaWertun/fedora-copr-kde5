%global         framework baloo

# uncomment to enable bootstrap mode
#global bootstrap 1

%if !0%{?bootstrap}
%global tests 1
%endif

Name:    kf5-%{framework}
Summary: A Tier 3 KDE Frameworks 5 module that provides indexing and search functionality
Version: 5.60.0
Release: 1%{?dist}

# libs are LGPL, tools are GPL
# KDE e.V. may determine that future LGPL/GPL versions are accepted
License: (LGPLv2 or LGPLv3) and (GPLv2 or GPLv3)
URL:     https://community.kde.org/Baloo
#URL:     https://cgit.kde.org/%{framework}.git

%global majmin %(echo %{version} | cut -d. -f1-2)
%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0:        http://download.kde.org/%{stable}/frameworks/%{majmin}/%{framework}-%{version}.tar.xz

Source1:        97-kde-baloo-filewatch-inotify.conf

## upstreamable patches
# http://bugzilla.redhat.com/1235026
Patch100: baloo-5.45.0-baloofile_config.patch

## upstream patches

BuildRequires:  extra-cmake-modules >= %{majmin}
BuildRequires:  kf5-kconfig-devel >= %{majmin}
BuildRequires:  kf5-kcoreaddons-devel >= %{majmin}
BuildRequires:  kf5-kcrash-devel >= %{majmin}
BuildRequires:  kf5-kdbusaddons-devel >= %{majmin}
BuildRequires:  kf5-kfilemetadata-devel >= %{majmin}
BuildRequires:  kf5-ki18n-devel >= %{majmin}
BuildRequires:  kf5-kidletime-devel >= %{majmin}
BuildRequires:  kf5-kio-devel >= %{majmin}
BuildRequires:  kf5-rpm-macros >= %{majmin}
BuildRequires:  kf5-solid-devel >= %{majmin}

BuildRequires:  lmdb-devel
BuildRequires:  qt5-qtbase-devel
BuildRequires:  qt5-qtdeclarative-devel

%if 0%{?tests}
BuildRequires: dbus-x11
BuildRequires: time
BuildRequires: xorg-x11-server-Xvfb
%endif

Obsoletes:      kf5-baloo-tools < 5.5.95-1
Provides:       kf5-baloo-tools = %{version}-%{release}

%if 0%{?fedora}
Obsoletes:      baloo < 5
Provides:       baloo = %{version}-%{release}
%else
Conflicts:      baloo < 5
%endif

# main pkg accidentally multilib'd prior to 5.21.0-4
Obsoletes:      kf5-baloo < 5.21.0-4

%description
%{summary}.

%package        devel
Summary:        Development files for %{name}
# KDE e.V. may determine that future LGPL versions are accepted
License:        LGPLv2 or LGPLv3
Requires:       %{name}-libs%{?_isa} = %{version}-%{release}
Requires:       kf5-kcoreaddons-devel >= %{majmin}
Requires:       kf5-kfilemetadata-devel >= %{majmin}
Requires:       qt5-qtbase-devel

%description    devel
The %{name}-devel package contains libraries and header files for
developing applications that use %{name}.

%package        file
Summary:        File indexing and search for Baloo
# KDE e.V. may determine that future LGPL versions are accepted
License:        LGPLv2 or LGPLv3
%if 0%{?fedora}
Obsoletes:      baloo-file < 5.0.1-2
Provides:       baloo-file = %{version}-%{release}
%else
Conflicts:      baloo-file < 5
%endif
Requires:       %{name}-libs%{?_isa} = %{version}-%{release}
%description    file
%{summary}.

%package        libs
Summary:        Runtime libraries for %{name}
# KDE e.V. may determine that future LGPL versions are accepted
License:        LGPLv2 or LGPLv3
%description    libs
%{summary}.


%prep
%autosetup -n %{framework}-%{version} -p1


%build
mkdir %{_target_platform}
pushd %{_target_platform}
%{cmake_kf5} .. \
  -DBUILD_TESTING:BOOL=%{?tests:ON}%{!?tests:OFF}
popd

%make_build -C %{_target_platform}


%install
make install/fast  DESTDIR=%{buildroot} -C %{_target_platform}

# baloodb not installed unless BUILD_EXPERIMENTAL is enabled, so omit translations
rm -fv %{buildroot}%{_datadir}/locale/*/LC_MESSAGES/baloodb5.*

install -p -m644 -D %{SOURCE1} %{buildroot}%{_prefix}/lib/sysctl.d/97-kde-baloo-filewatch-inotify.conf

%find_lang kio5_baloosearch
%find_lang kio5_tags
%find_lang kio5_timeline
%find_lang balooctl5
#find_lang baloodb5
%find_lang balooengine5
%find_lang baloosearch5
%find_lang balooshow5
%find_lang baloo_file5
%find_lang baloo_file_extractor5
%find_lang baloomonitorplugin

cat kio5_tags.lang kio5_baloosearch.lang kio5_timeline.lang \
    balooctl5.lang balooengine5.lang baloosearch5.lang \
    balooshow5.lang baloomonitorplugin.lang \
    > %{name}.lang

cat baloo_file5.lang baloo_file_extractor5.lang \
    > %{name}-file.lang


%check
%if 0%{?tests}
export CTEST_OUTPUT_ON_FAILURE=1
xvfb-run -a \
dbus-launch --exit-with-session \
time \
make test ARGS="--output-on-failure --timeout 300" -C %{_target_platform} ||:
%endif


%files -f %{name}.lang
%license COPYING
#{_kf5_bindir}/baloodb
%{_kf5_bindir}/baloosearch
%{_kf5_bindir}/balooshow
%{_kf5_bindir}/balooctl
%{_kf5_datadir}/qlogging-categories5/baloo.categories

%files file -f %{name}-file.lang
%{_prefix}/lib/sysctl.d/97-kde-baloo-filewatch-inotify.conf
%{_kf5_bindir}/baloo_file
%{_kf5_bindir}/baloo_file_extractor
%{_kf5_sysconfdir}/xdg/autostart/baloo_file.desktop

%ldconfig_scriptlets libs

%files libs
%license COPYING.LIB
%{_kf5_libdir}/libKF5Baloo.so.*
%{_kf5_libdir}/libKF5BalooEngine.so.*
# multilib'd plugins and friends
%{_kf5_plugindir}/kio/baloosearch.so
%{_kf5_plugindir}/kio/tags.so
%{_kf5_plugindir}/kio/timeline.so
%{_kf5_plugindir}/kded/baloosearchmodule.so
%{_kf5_qmldir}/org/kde/baloo
%{_kf5_datadir}/kservices5/baloosearch.protocol
%{_kf5_datadir}/kservices5/tags.protocol
%{_kf5_datadir}/kservices5/timeline.protocol
# track icon size too, since it may conflict with baloo-4.x
%{_kf5_datadir}/icons/hicolor/128x128/apps/baloo.png

%files devel
%{_kf5_libdir}/libKF5Baloo.so
%{_kf5_libdir}/cmake/KF5Baloo/
%{_kf5_libdir}/pkgconfig/Baloo.pc
%{_kf5_includedir}/Baloo/
%{_kf5_includedir}/baloo_version.h
%{_kf5_archdatadir}/mkspecs/modules/qt_Baloo.pri
%{_kf5_datadir}/dbus-1/interfaces/org.kde.baloo.*.xml
%{_kf5_datadir}/dbus-1/interfaces/org.kde.Baloo*.xml


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

* Tue Jan 08 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.54.0-1
- 5.54.0

* Sun Dec 09 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.53.0-1
- 5.53.0

* Mon Nov 05 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.52.0-2
- rebuild

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

* Sat Jun 02 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.47.0-1
- 5.47.0

* Sat May 05 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.46.0-1
- 5.46.0

* Sun Apr 08 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.45.0-1
- 5.45.0

* Sat Mar 03 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.44.0-1
- 5.44.0

* Wed Feb 07 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.43.0-1
- 5.43.0

* Wed Feb 07 2018 Fedora Release Engineering <releng@fedoraproject.org> - 5.42.0-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_28_Mass_Rebuild

* Mon Jan 15 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.42.0-2
- .spec cleanup

* Mon Jan 08 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.42.0-1
- 5.42.0

* Mon Dec 11 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.41.0-2
- rebuild

* Mon Dec 04 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.41.0-1
- 5.41.0

* Fri Nov 10 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.40.0-1
- 5.40.0

* Sun Oct 08 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.39.0-1
- 5.39.0

* Mon Sep 11 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.38.0-1
- 5.38.0

* Fri Aug 25 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.37.0-1
- 5.37.0

* Thu Aug 03 2017 Fedora Release Engineering <releng@fedoraproject.org> - 5.36.0-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Binutils_Mass_Rebuild

* Wed Jul 26 2017 Fedora Release Engineering <releng@fedoraproject.org> - 5.36.0-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Mass_Rebuild

* Mon Jul 03 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.36.0-1
- 5.36.0

* Sun Jun 04 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.35.0-1
- 5.35.0

* Mon May 15 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.34.0-1
- 5.34.0

* Mon Apr 03 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.33.0-1
- 5.33.0

* Mon Apr 03 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.32.0-2
- .spec cosmetics, update URL

* Sat Mar 04 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.32.0-1
- 5.32.0

* Mon Feb 06 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.31.0-1
- 5.31.0

* Fri Dec 16 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.29.0-1
- 5.29.0

* Tue Oct 04 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.27.0-1
- 5.27.0

* Thu Sep 08 2016 Daniel Vrátil <dvratil@fedoraproject.org> - 5.26.0-1
- KDE Frameworks 5.26.0

* Thu Aug 18 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.25.0-2
- validate baloo.file.desktop (#1367207)

* Mon Aug 08 2016 Daniel Vrátil <dvratil@fedoraproject.org> - 5.25.0-1
- KDE Frameworks 5.25.0

* Wed Jul 06 2016 Daniel Vrátil <dvratil@fedoraproject.org> - 5.24.0-1
- KDE Frameworks 5.24.0

* Tue Jun 07 2016 Daniel Vrátil <dvratil@fedoraproject.org> - 5.23.0-1
- KDE Frameworks 5.23.0

* Mon May 16 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.22.0-1
- KDE Frameworks 5.22.0

* Sat Apr 30 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.21.0-6
- %%check: use standardized BRs, 'make test'

* Fri Apr 29 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.21.0-5
- -file: Requires: %%name-libs

* Fri Apr 29 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.21.0-4
- fix URL
- -devel: depend (only) on -libs
- -libs: move plugins here
- Provides: kf5-baloo-tools

* Wed Apr 27 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.21.0-3
- support bootstrap, %%check: enable tests (advisory)

* Wed Apr 27 2016 Orion Poplawski <orion@cora.nwra.com> - 5.21.0-2
- Do not obsolete/provide baloo{-file} in EPEL, use Conflicts (#1329899)
- update URL

* Mon Apr 04 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.21.0-1
- KDE Frameworks 5.21.0

* Mon Mar 14 2016 Daniel Vrátil <dvratil@fedoraproject.org> - 5.20.0-1
- KDE Frameworks 5.20.0

* Thu Feb 18 2016 Rex Dieter <rdieter@fedoraproject.org> 5.19.0-2
- cleanup

* Thu Feb 11 2016 Daniel Vrátil <dvratil@fedoraproject.org> - 5.19.0-1
- KDE Frameworks 5.19.0

* Thu Feb 04 2016 Fedora Release Engineering <releng@fedoraproject.org> - 5.18.0-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_24_Mass_Rebuild

* Sun Jan 03 2016 Daniel Vrátil <dvratil@fedoraproject.org> - 5.18.0-1
- KDE Frameworks 5.18.0

* Tue Dec 08 2015 Daniel Vrátil <dvratil@fedoraproject.org> - 5.17.0-1
- KDE Frameworks 5.17.0

* Sun Nov 08 2015 Daniel Vrátil <dvratil@fedoraproject.org> - 5.16.0-1
- KDE Frameworks 5.16.0

* Thu Oct 08 2015 Daniel Vrátil <dvratil@redhat.com> - 5.15.0-1
- KDE Frameworks 5.15.0

* Sat Oct 03 2015 Rex Dieter <rdieter@fedoraproject.org> - 5.14.0-2
- index only well-known document-centric dirs by default (#1235026)
- .spec cosmetics
- polish licensing
- -devel: drop xapian dep

* Wed Sep 16 2015 Daniel Vrátil <dvratil@redhat.com> - 5.14.0-1
- KDE Frameworks 5.14.0

* Wed Aug 19 2015 Daniel Vrátil <dvratil@redhat.com> - 5.13.0-1
- KDE Frameworks 5.13.0

* Wed Aug 19 2015 Daniel Vrátil <dvratil@redhat.com> - 5.13.0-1
- KDE Frameworks 5.13.0

* Tue Aug 11 2015 Daniel Vrátil <dvratil@redhat.com> - 5.13.0-0.1
- KDE Frameworks 5.13 (Baloo moved from Plasma 5 to KF5)

* Wed Apr 22 2015 Daniel Vrátil <dvratil@redhat.com> - 5.6.95-1
- Plasma 5.2.95

* Fri Mar 20 2015 Daniel Vrátil <dvratil@redhat.com> - 5.6.2-1
- Plasma 5.2.2

* Fri Feb 27 2015 Daniel Vrátil <dvratil@redhat.com> - 5.6.1-2
- Rebuild (GCC 5)

* Tue Feb 24 2015 Daniel Vrátil <dvratil@redhat.com> - 5.6.1-1
- Plasma 5.2.1

* Sun Feb 08 2015 Daniel Vrátil <dvratil@redhat.com> - 5.6.0-3
- kf5-baloo-file provides baloo-file

* Sat Feb 07 2015 Rex Dieter <rdieter@fedoraproject.org> 5.6.0-2
- port 97-kde-baloo-filewatch-inotify.conf from Obsoletes'd baloo pkg

* Mon Jan 26 2015 Daniel Vrátil <dvratil@redhat.com> - 5.6.0-1
- Plasma 5.2.0

* Mon Jan 12 2015 Daniel Vrátil <dvratil@redhat.com> - 5.5.95-1
- Plasma 5.1.95 (Plasma 5.2 beta) (baloo 5.5.95 to follow KF5)
- create -libs subpkg

* Wed Jan 07 2015 Jan Grulich <jgrulich@redhat.com> - 5.1.2-3
- Drop -tools subpkg
-  Add icon cache scriptlets
-  Remove deprecated Group: tag
-  Move org.kde.baloo.file.indexer.xml to -file subpkg

* Wed Dec 17 2014 Daniel Vrátil <dvratil@redhat.com> - 5.1.2-2
- Plasma 5.1.2

* Fri Nov 07 2014 Daniel Vrátil <dvratil@redhat.com> - 5.1.1-1
- Plasma 5.1.1

* Tue Oct 14 2014 Daniel Vrátil <dvratil@redhat.com> - 5.1.0.1-1
- Plasma 5.1.0.1

* Thu Oct 09 2014 Daniel Vrátil <dvratil@redhat.com> - 5.1.0-1
- Plasma 5.1.0

* Tue Sep 16 2014 Daniel Vrátil <dvratil@redhat.com> - 5.0.2-1
- Plasma 5.0.2

* Mon Aug 18 2014 Daniel Vrátil <dvratil@redhat.com> - 5.0.1-2
- Fix coinstallability with updated baloo package

* Sun Aug 10 2014 Daniel Vrátil <dvratil@redhat.com> - 5.0.1-1
- Plasma 5.0.1

* Tue Jul 22 2014 Daniel Vrátil <dvratil@redhat.com> - 5.0.0-4
- -devel Requires xapian-core-devel

* Tue Jul 22 2014 Daniel Vrátil <dvratil@redhat.com> - 5.0.0-3
- split bin tools to -tools subpackage

* Tue Jul 22 2014 Daniel Vrátil <dvratil@redhat.com> - 5.0.0-2
- -devel Requires kf5-kfilemetadata-devel
- does not obsolete baloo < 5.0.0 (coinstallability)

* Wed Jul 16 2014 Daniel Vrátil <dvratil@redhat.com> - 5.0.0-1
- Plasma 5.0.0

* Wed Jun 11 2014 Daniel Vrátil <dvratil@redhat.com> - 4.98.0-3.20140611git84bc23c
- Update to latest git snapshot

* Wed May 14 2014 Daniel Vrátil <dvratil@redhat.com> - 4.90.0-2.20140611git46e3ea7
- KF5 Baloo 4.90.0 (git snapshot built from common kdepimlibs/frameworks repo)
