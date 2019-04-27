%global framework plasma

Name:    kf5-%{framework}
Version: 5.55.0
Release: 1%{?dist}
Summary: KDE Frameworks 5 Tier 3 framework is foundation to build a primary user interface

License: GPLv2+ and LGPLv2+ and BSD
URL:     https://cgit.kde.org/plasma-framework.git

%global majmin %(echo %{version} | cut -d. -f1-2)
%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0: http://download.kde.org/%{stable}/frameworks/%{majmin}/%{framework}-framework-%{version}.tar.xz

# hackish cache invalidation
# upstream has worked on this issue recently (.31 or .32?) so may consider dropping this -- rex
Source10: fedora-plasma-cache.sh.in

## upstream patches (using lookaside cache)

# filter qml provides
%global __provides_exclude_from ^%{_kf5_qmldir}/.*\\.so$

BuildRequires:  extra-cmake-modules >= %{majmin}
BuildRequires:  kf5-kactivities-devel >= %{majmin}
BuildRequires:  kf5-kactivities-devel >= %{majmin}
BuildRequires:  kf5-karchive-devel >= %{majmin}
BuildRequires:  kf5-kconfig-devel >= %{majmin}
BuildRequires:  kf5-kconfigwidgets-devel >= %{majmin}
BuildRequires:  kf5-kcoreaddons-devel >= %{majmin}
BuildRequires:  kf5-kdbusaddons-devel >= %{majmin}
BuildRequires:  kf5-kdeclarative-devel >= %{majmin}
BuildRequires:  kf5-kdesu-devel >= %{majmin}
BuildRequires:  kf5-kdoctools-devel >= %{majmin}
BuildRequires:  kf5-kglobalaccel-devel >= %{majmin}
BuildRequires:  kf5-kguiaddons-devel >= %{majmin}
BuildRequires:  kf5-ki18n-devel >= %{majmin}
BuildRequires:  kf5-kiconthemes-devel >= %{majmin}
BuildRequires:  kf5-kio-devel >= %{majmin}
BuildRequires:  kf5-knotifications-devel >= %{majmin}
BuildRequires:  kf5-kpackage-devel >= %{majmin}
BuildRequires:  kf5-kparts-devel >= %{majmin}
BuildRequires:  kf5-kservice-devel >= %{majmin}
BuildRequires:  kf5-kirigami2-devel >= %{majmin}
%if 0%{?fedora}
BuildRequires:  kf5-kwayland-devel >= %{majmin}
%endif
BuildRequires:  kf5-kwidgetsaddons-devel >= %{majmin}
BuildRequires:  kf5-kwindowsystem-devel >= %{majmin}
BuildRequires:  kf5-kxmlgui-devel >= %{majmin}
BuildRequires:  kf5-rpm-macros
BuildRequires:  kf5-solid-devel >= %{majmin}
BuildRequires:  libGL-devel
BuildRequires:  libSM-devel
BuildRequires:  libX11-devel
BuildRequires:  libxcb-devel
BuildRequires:  libXext-devel
BuildRequires:  libXrender-devel
BuildRequires:  libXScrnSaver-devel
BuildRequires:  openssl-devel

BuildRequires:  qt5-qtbase-devel
BuildRequires:  qt5-qtdeclarative-devel
BuildRequires:  qt5-qtquickcontrols2-devel
BuildRequires:  qt5-qtscript-devel
BuildRequires:  qt5-qtsvg-devel
BuildRequires:  qt5-qtx11extras-devel

Requires:       qt5-qtquickcontrols%{?_isa}
Requires:       qt5-qtquickcontrols2%{?_isa}

%if 0%{?fedora}
# https://bugzilla.redhat.com/1293415
Conflicts:      kdeplasma-addons < 5.5.0-3
%endif

%description
%{summary}.

%package        devel
Summary:        Development files for %{name}
# https://bugzilla.redhat.com/1292506
Conflicts:      kapptemplates < 15.12.0-1
Requires:       %{name}%{?_isa} = %{version}-%{release}
Requires:       kf5-kpackage-devel >= %{majmin}
Requires:       kf5-kservice-devel >= %{majmin}
Requires:       kf5-kwindowsystem-devel >= %{majmin}
Requires:       qt5-qtbase-devel
%description    devel
The %{name}-devel package contains libraries and header files for
developing applications that use %{name}.


%prep
%autosetup -n %{framework}-framework-%{version} -p1

install -m644 -p %{SOURCE10} .


%build
mkdir %{_target_platform}
pushd %{_target_platform}
%{cmake_kf5} ..
popd

%make_build -C %{_target_platform}


%install
make install/fast DESTDIR=%{buildroot} -C %{_target_platform}

%find_lang %{name} --all-name --with-man --all-name

# create/own dirs
mkdir -p %{buildroot}%{_kf5_datadir}/plasma/plasmoids
mkdir -p %{buildroot}%{_kf5_qmldir}/org/kde/private

%if 0%{?fedora}
mkdir -p %{buildroot}%{_sysconfdir}/xdg/plasma-workspace/env
sed -e "s|@@VERSION@@|%{version}|g" fedora-plasma-cache.sh.in > \
  %{buildroot}%{_sysconfdir}/xdg/plasma-workspace/env/fedora-plasma-cache.sh
%endif


%ldconfig_scriptlets

%files -f %{name}.lang
%doc README.md
%license COPYING.LIB
%if 0%{?fedora}
%{_sysconfdir}/xdg/plasma-workspace/env/fedora-plasma-cache.sh
%endif
%{_kf5_bindir}/plasmapkg2
%{_kf5_libdir}/libKF5Plasma.so.*
%{_kf5_libdir}/libKF5PlasmaQuick.so.*
%dir %{_kf5_qmldir}/org/
%dir %{_kf5_qmldir}/org/kde/
%dir %{_kf5_qmldir}/org/kde/private/
%{_kf5_qmldir}/org/kde/plasma/
%{_kf5_qmldir}/org/kde/kirigami.2/styles
%{_kf5_qmldir}/QtQuick/Controls/Styles/Plasma/
%{_kf5_qmldir}/QtQuick/Controls.2/Plasma/
%{_kf5_qtplugindir}/plasma/
%{_kf5_qtplugindir}/kpackage/packagestructure/*.so
%{_kf5_datadir}/plasma/
%{_kf5_datadir}/kservices5/*.desktop
%{_kf5_datadir}/kservicetypes5/*.desktop
%{_kf5_mandir}/man1/plasmapkg2.1.*
# can find_lang handle this too somehow? -- rex
%lang(lt) %{_datadir}/locale/lt/LC_SCRIPTS/libplasma5/

%files devel
%{_kf5_libdir}/cmake/KF5Plasma/
%{_kf5_libdir}/cmake/KF5PlasmaQuick/
%{_kf5_libdir}/libKF5Plasma.so
%{_kf5_libdir}/libKF5PlasmaQuick.so
%{_kf5_includedir}/plasma_version.h
%{_kf5_includedir}/plasma/
%{_kf5_includedir}/Plasma/
%{_kf5_includedir}/PlasmaQuick/
%{_kf5_includedir}/plasmaquick/
%dir %{_kf5_datadir}/kdevappwizard/
%{_kf5_datadir}/kdevappwizard/templates/


%changelog
* Mon Feb 04 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.55.0-1
- 5.55.0

* Fri Feb 01 2019 Fedora Release Engineering <releng@fedoraproject.org> - 5.54.0-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_30_Mass_Rebuild

* Wed Jan 09 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.54.0-1
- 5.54.0

* Sun Dec 09 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.53.0-1
- 5.53.0

* Thu Nov 08 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.52.0-2
- respin

* Sun Nov 04 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.52.0-1
- 5.52.0

* Wed Oct 10 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.51.0-1
- 5.51.0

* Tue Sep 04 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.50.0-1
- 5.50.0

* Tue Aug 07 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.49.0-1
- 5.49.0

* Fri Jul 13 2018 Fedora Release Engineering <releng@fedoraproject.org> - 5.48.0-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_29_Mass_Rebuild

* Mon Jul 09 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.48.0-1
- 5.48.0

* Fri Jun 08 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.47.0-2
- cleanup, use %%majmin %%make_build

* Sat Jun 02 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.47.0-1
- 5.47.0

* Sat May 05 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.46.0-1
- 5.46.0

* Sun Apr 08 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.45.0-1
- 5.45.0

* Sat Mar 03 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.44.0-1
- 5.44.0

* Thu Feb 08 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.43.0-3
- use %%ldconfig_scriptlets, drop %%find_lang ... --with-qt

* Wed Feb 07 2018 Fedora Release Engineering <releng@fedoraproject.org> - 5.43.0-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_28_Mass_Rebuild

* Wed Feb 07 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.43.0-1
- 5.43.0

* Mon Jan 08 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.42.0-1
- 5.42.0

* Mon Dec 04 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.41.0-1
- 5.41.0

* Fri Nov 10 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.40.0-1
- 5.40.0

* Sun Oct 08 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.39.0-1
- 5.39.0

* Mon Sep 11 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.38.0-1
- 5.38.0

* Fri Aug 25 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.37.0-2
- -devel: Requires: kf5-kwindowsystem-devel, cosmetics, use %%find_lang --with-man

* Fri Aug 25 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.37.0-1
- 5.37.0

* Thu Aug 03 2017 Fedora Release Engineering <releng@fedoraproject.org> - 5.36.0-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Binutils_Mass_Rebuild

* Wed Jul 26 2017 Fedora Release Engineering <releng@fedoraproject.org> - 5.36.0-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Mass_Rebuild

* Mon Jul 03 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.36.0-1
- 5.36.0

* Tue Jun 06 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.35.0-2
- tarball respin, kwayland support optional (fedora only)

* Sun Jun 04 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.35.0-1
- 5.35.0

* Mon May 15 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.34.0-1
- 5.34.0

* Fri Apr 14 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.33.0-2
- pull in upstream fixes, fix URL

* Mon Apr 03 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.33.0-1
- 5.33.0, Requires: qt5-qtquickcontrols qt5-qtquickcontrols2

* Wed Mar 15 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.32.0-2
- pull in upstream fix for desktop containment/view regression (kde#377441)
- update URL

* Sat Mar 04 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.32.0-1
- 5.32.0

* Mon Feb 06 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.31.0-1
- 5.31.0

* Mon Jan 02 2017 Rex Dieter <rdieter@math.unl.edu> - 5.29.0-2
- filter qml provides

* Fri Dec 16 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.29.0-1
- 5.29.0

* Wed Nov 02 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.27.0-2
- backport autohide-panel fix

* Tue Oct 04 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.27.0-1
- 5.27.0

* Thu Sep 08 2016 Daniel Vrátil <dvratil@fedoraproject.org> - 5.26.0-1
- KDE Frameworks 5.26.0

* Wed Aug 24 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.25.0-3
- own some qml/plasma parent dirs

* Wed Aug 10 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.25.0-2
- respin tarball

* Mon Aug 08 2016 Daniel Vrátil <dvratil@fedoraproject.org> - 5.25.0-1
- KDE Frameworks 5.25.0

* Wed Jul 06 2016 Daniel Vrátil <dvratil@fedoraproject.org> - 5.24.0-1
- KDE Frameworks 5.24.0

* Tue Jun 07 2016 Daniel Vrátil <dvratil@fedoraproject.org> - 5.23.0-1
- KDE Frameworks 5.23.0

* Mon May 16 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.22.0-1
- KDE Frameworks 5.22.0

* Mon Apr 04 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.21.0-1
- KDE Frameworks 5.21.0

* Mon Mar 14 2016 Daniel Vrátil <dvratil@fedoraproject.org> - 5.20.0-1
- KDE Frameworks 5.20.0

* Mon Feb 22 2016 Rex Dieter <rdieter@fedoraproject.org> 5.19.0-3
- KDE Plasma crashes when the mouse pointer repeatedly hovers the TaskManager (#1310875)

* Tue Feb 16 2016 Daniel Vrátil <dvratil@fedoraproject.org> - 5.19.0-2
- pull upstream patch for BKO#359388

* Thu Feb 11 2016 Daniel Vrátil <dvratil@fedoraproject.org> - 5.19.0-1
- KDE Frameworks 5.19.0

* Thu Feb 04 2016 Fedora Release Engineering <releng@fedoraproject.org> - 5.18.0-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_24_Mass_Rebuild

* Thu Jan 14 2016 Rex Dieter <rdieter@fedoraproject.org> 5.18.0-2
- cosmetics, update URL

* Sun Jan 03 2016 Daniel Vrátil <dvratil@fedoraproject.org> - 5.18.0-1
- KDE Frameworks 5.18.0

* Mon Dec 21 2015 Rex Dieter <rdieter@fedoraproject.org> 5.17.0-3
- Conflicts: kdeplasma-5.5.0-3 (#1293415)

* Thu Dec 17 2015 Rex Dieter <rdieter@fedoraproject.org> - 5.17.0-2
- Conflicts: kapptemplates < 15.12.0 (#1292506)
- move template to -devel to be on the safe side
- own %%{_kf5_datadir}/kdevappwizard/{,templates/}

* Tue Dec 08 2015 Daniel Vrátil <dvratil@fedoraproject.org> - 5.17.0-1
- KDE Frameworks 5.17.0

* Tue Dec 08 2015 Jan Grulich <jgrulich@redhat.com> - 5.16.0-4
- Update to new network icons

* Fri Dec 04 2015 Jan Grulich <jgrulich@redhat.com> - 5.16.0-3
- Revert network icons to fix missing icons in plasma-nm

* Thu Nov 19 2015 Rex Dieter <rdieter@fedoraproject.org> - 5.16.0-2
- .spec cosmetics, use %%license
- add env script invalidate plasma cache on version upgrades

* Sun Nov 08 2015 Daniel Vrátil <dvratil@fedoraproject.org> - 5.16.0-1
- KDE Frameworks 5.16.0

* Thu Oct 08 2015 Daniel Vrátil <dvratil@redhat.com> - 5.15.0-1
- KDE Frameworks 5.15.0

* Wed Sep 16 2015 Daniel Vrátil <dvratil@redhat.com> - 5.14.0-1
- KDE Frameworks 5.14.0

* Fri Aug 28 2015 Helio Chissini de Castro <helio@kde.org> - 5.13.0-2
- Upstream fix to plasmapkg2 pluginloader location

* Wed Aug 19 2015 Daniel Vrátil <dvratil@redhat.com> - 5.13.0-1
- KDE Frameworks 5.13.0

* Wed Aug 19 2015 Daniel Vrátil <dvratil@redhat.com> - 5.13.0-1
- KDE Frameworks 5.13.0

* Tue Aug 11 2015 Daniel Vrátil <dvratil@redhat.com> - 5.13.0-0.1
- KDE Frameworks 5.13

* Thu Jul 16 2015 Rex Dieter <rdieter@fedoraproject.org> - 5.12.0-2
- pull in proposed fix for renamed LookAndFeelPackage (review#124371)
- bump/sort build deps
- update URL
- polish dir ownership

* Fri Jul 10 2015 Rex Dieter <rdieter@fedoraproject.org> - 5.12.0-1
- 5.12.0

* Wed Jun 17 2015 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 5.11.0-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_23_Mass_Rebuild

* Wed Jun 10 2015 Daniel Vrátil <dvratil@redhat.com> - 5.11.0-1
- KDE Frameworks 5.11.0

* Tue Jun 02 2015 Rex Dieter <rdieter@fedoraproject.org> 5.10.0-3
- -devel: Requires: kf5-kservice-devel

* Tue May 12 2015 Rex Dieter <rdieter@fedoraproject.org> 5.10.0-2
- Panel stays light when theme is set to 'Breeze Dark' (kde#347143)

* Mon May 11 2015 Daniel Vrátil <dvratil@redhat.com> - 5.10.0-1
- KDE Frameworks 5.10.0

* Tue Apr 07 2015 Daniel Vrátil <dvratil@redhat.com> - 5.9.0-1
- KDE Frameworks 5.9.0

* Mon Mar 16 2015 Daniel Vrátil <dvratil@redhat.com> - 5.8.0-1
- KDE Frameworks 5.8.0

* Fri Feb 27 2015 Daniel Vrátil <dvratil@redhat.com> - 5.7.0-2
- Rebuild (GCC 5)

* Mon Feb 16 2015 Daniel Vrátil <dvratil@redhat.com> - 5.7.0-1
- KDE Frameworks 5.7.0

* Mon Feb 09 2015 Daniel Vrátil <dvratil@redhat.com> - 5.7.0-1
- KDE Frameworks 5.7.0

* Thu Jan 08 2015 Daniel Vrátil <dvratil@redhat.com> - 5.6.0-1
- KDE Frameworks 5.6.0

* Mon Dec 08 2014 Daniel Vrátil <dvratil@redhat.com> - 5.5.0-1
- KDE Frameworks 5.5.0

* Mon Nov 03 2014 Daniel Vrátil <dvratil@redhat.com> - 5.4.0-1
- KDE Frameworks 5.4.0

* Thu Oct 23 2014 Daniel Vrátil <dvratil@redhat.com> - 5.3.1-1
- Plasma Framework 5.3.1 (upstream hotfix)

* Tue Oct 07 2014 Daniel Vrátil <dvratil@redhat.com> - 5.3.0-1
- KDE Frameworks 5.3.0

* Mon Sep 15 2014 Daniel Vrátil <dvratil@redhat.com> - 5.2.0-1
- KDE Frameworks 5.2.0

* Sun Aug 17 2014 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 5.1.0-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_21_22_Mass_Rebuild

* Wed Aug 06 2014 Daniel Vrátil <dvratil@redhat.com> - 5.1.0-1
- KDE Frameworks 5.1.0

* Mon Jul 14 2014 Daniel Vrátil <dvratil@redhat.com> - 5.0.0-2
- Fix plugin installation path

* Wed Jul 09 2014 Daniel Vrátil <dvratil@redhat.com> - 5.0.0-1
- KDE Frameworks 5.0.0

* Mon Jul 07 2014 Daniel Vrátil <dvratil@redhat.com> - 4.100.0-3
- Fix BR

* Mon Jul 07 2014 Daniel Vrátil <dvratil@redhat.com> - 4.100.0-2
- Fixed license
- Removed old Provides/Obsoleted

* Tue Jun 03 2014 Daniel Vrátil <dvratil@redhat.com> - 4.100.0-1
- KDE Frameworks 4.100.0

* Mon May 05 2014 Daniel Vrátil <dvratil@redhat.com> - 4.99.0
- KDE Frameworks 4.99.0

* Mon Mar 31 2014 Jan Grulich <jgrulich@redhat.com> 4.98.0-1
- Update to KDE Frameworks 5 Beta 1 (4.98.0)

* Sat Mar 15 2014 Jan Grulich <jgrulich@redhat.com 4.97.0-3.20140315git
- update git snapshot

* Wed Mar 12 2014 Jan Grulich <jgrulich@redhat.com 4.97.0-2.20140312git
- update to git snapshot

* Wed Mar 05 2014 Jan Grulich <jgrulich@redhat.com> 4.97.0-1
- Update to KDE Frameworks 5 Alpha 1 (4.97.0)

* Thu Feb 13 2014 Daniel Vrátil <dvratil@redhat.com> - 4.96.0-1
- upgrade to Tier 3 Framework kf5-plasma

* Sat Jan  4 2014 Daniel Vrátil <dvratil@redhat.com>
- initial version
