%global framework frameworkintegration

Name:    kf5-%{framework}
Version: 5.62.0
Release: 1%{?dist}
Summary: KDE Frameworks 5 Tier 4 workspace and cross-framework integration plugins
License: LGPLv2+
URL:     https://cgit.kde.org/%{framework}.git

%global versiondir %(echo %{version} | cut -d. -f1-2)
%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0:        http://download.kde.org/%{stable}/frameworks/%{versiondir}/%{framework}-%{version}.tar.xz

# filter plugin provides
%global __provides_exclude_from ^(%{_kf5_plugindir}/.*\\.so)$

BuildRequires:  extra-cmake-modules >= %{version}
BuildRequires:  kf5-kconfig-devel >= %{version}
BuildRequires:  kf5-kconfigwidgets-devel >= %{version}
BuildRequires:  kf5-ki18n-devel >= %{version}
BuildRequires:  kf5-kiconthemes-devel >= %{version}
BuildRequires:  kf5-kio-devel >= %{version}
BuildRequires:  kf5-knewstuff-devel >= %{version}
BuildRequires:  kf5-knotifications-devel >= %{version}
BuildRequires:  kf5-kpackage-devel >= %{version}
BuildRequires:  kf5-kwidgetsaddons-devel >= %{version}
BuildRequires:  kf5-rpm-macros >= %{version}

BuildRequires:  libXcursor-devel
BuildRequires:  qt5-qtbase-devel
BuildRequires:  qt5-qtx11extras-devel
%if 0%{?fedora} > 23
%global appstream 1
BuildRequires:  cmake(AppStreamQt)
BuildRequires:  cmake(packagekitqt5)
%endif

# prior to 5.11.0-3, main pkg was multilib'd (due to arch'd -devel deps) -- rex
Obsoletes:      %{name} < 5.11.0-3
Requires:       %{name}-libs%{?_isa} = %{version}-%{release}

## consider removing for f28+, since Qt5 tracks via versioned symbols now -- rex
BuildRequires: qt5-qtbase-private-devel
%{?_qt5:Requires: %{_qt5}%{?_isa} = %{_qt5_version}}

%description
Framework Integration is a set of plugins responsible for better integration of
Qt applications when running on a KDE Plasma workspace.

Applications do not need to link to this directly.

%package        libs
Summary:        Runtime libraries for %{name}.
Requires:       %{name} = %{version}-%{release}
%description    libs
%{summary}.

%package        devel
Summary:        Development files for %{name}
Requires:       %{name}-libs%{?_isa} = %{version}-%{release}
Requires:       kf5-kiconthemes-devel >= %{version}
Requires:       kf5-kconfigwidgets-devel >= %{version}

%description    devel
The %{name}-devel package contains libraries and header files for
developing applications that use %{name}.


%prep
%autosetup -p1 -n %{framework}-%{version}


%build
mkdir %{_target_platform}
pushd %{_target_platform}
%{cmake_kf5} ..
popd

%make_build -C %{_target_platform}


%install
make install/fast DESTDIR=%{buildroot} -C %{_target_platform}


%files
%doc README.md
%license COPYING.LIB
%{_kf5_datadir}/kf5/infopage/
%{_kf5_datadir}/knotifications5/plasma_workspace.notifyrc
%dir %{_kf5_libexecdir}/kpackagehandlers
%{_kf5_libexecdir}/kpackagehandlers/knshandler
# move to subpkg? -- rex
%if 0%{?appstream}
%{_kf5_libexecdir}/kpackagehandlers/appstreamhandler
%endif

%ldconfig_scriptlets libs

%files libs
%{_kf5_libdir}/libKF5Style.so.*
%{_kf5_plugindir}/FrameworkIntegrationPlugin.so

%files devel
%{_kf5_includedir}/frameworkintegration_version.h
%{_kf5_includedir}/KStyle/
%{_kf5_libdir}/libKF5Style.so
%{_kf5_libdir}/cmake/KF5FrameworkIntegration/


%changelog
* Sun Sep 15 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.62.0-1
- 5.62.0

* Mon Aug 12 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.61.0-1
- 5.61.0

* Sat Jul 13 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.60.0-1
- 5.60.0

* Wed Jul 03 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.59.0-2
- rebuild with new qt5

* Sat Jun 08 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.59.0-1
- 5.59.0

* Tue May 14 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.58.0-1
- 5.58.0

* Sun Apr 28 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.57.0-1
- 5.57.0

* Mon Feb 04 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.55.0-1
- 5.55.0

* Fri Feb 01 2019 Fedora Release Engineering <releng@fedoraproject.org> - 5.54.0-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_30_Mass_Rebuild

* Tue Jan 08 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.54.0-1
- 5.54.0

* Mon Dec 24 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.53.0-3
- rebuild (qt5)

* Tue Dec 11 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.53.0-2
- rebuild (qt5)

* Sun Dec 09 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.53.0-1
- 5.53.0

* Sun Nov 04 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.52.0-1
- 5.52.0

* Wed Oct 10 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.51.0-1
- 5.51.0

* Fri Sep 21 2018 Jan Grulich <jgrulich@redhat.com> - 5.50.0-2
- rebuild (qt5)

* Tue Sep 04 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.50.0-1
- 5.50.0

* Tue Aug 07 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.49.0-1
- 5.49.0

* Mon Jul 09 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.48.0-1
- 5.48.0

* Wed Jun 20 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.47.0-2
- rebuild (qt5)

* Sat Jun 02 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.47.0-1
- 5.47.0

* Sun May 27 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.46.0-2
- rebuild (qt5)

* Sat May 05 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.46.0-1
- 5.46.0

* Sun Apr 08 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.45.0-1
- 5.45.0

* Sat Mar 03 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.44.0-1
- 5.44.0

* Mon Feb 19 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.43.0-4
- drop kde-filesystem dep, old cruft
- drop font deps (no longer needed or used)
- use %%ldconfig_scriptles %%make_build

* Wed Feb 14 2018 Jan Grulich <jgrulich@redhat.com> - 5.43.0-3
- rebuild (qt5)

* Wed Feb 07 2018 Fedora Release Engineering <releng@fedoraproject.org> - 5.43.0-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_28_Mass_Rebuild

* Wed Feb 07 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.43.0-1
- 5.43.0

* Mon Jan 22 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.42.0-2
- rebuild (PackageKit-Qt)

* Mon Jan 08 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.42.0-1
- 5.42.0

* Wed Dec 20 2017 Jan Grulich <jgrulich@redhat.com> - 5.41.0-3
- rebuild (qt5)

* Mon Dec 11 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.41.0-2
- rebuild

* Mon Dec 04 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.41.0-1
- 5.41.0

* Sun Nov 26 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.40.0-2
- rebuild (qt5)

* Fri Nov 10 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.40.0-1
- 5.40.0

* Mon Oct 09 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.39.0-2
- rebuild (qt5)

* Sun Oct 08 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.39.0-1
- 5.39.0

* Mon Sep 11 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.38.0-1
- 5.38.0

* Fri Aug 25 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.37.0-1
- 5.37.0

* Thu Aug 03 2017 Fedora Release Engineering <releng@fedoraproject.org> - 5.36.0-4
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Binutils_Mass_Rebuild

* Wed Jul 26 2017 Fedora Release Engineering <releng@fedoraproject.org> - 5.36.0-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Mass_Rebuild

* Wed Jul 19 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.36.0-2
- rebuild (qt5)

* Mon Jul 03 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.36.0-1
- 5.36.0

* Sun Jun 04 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.35.0-1
- 5.35.0

* Mon May 15 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.34.0-1
- 5.34.0

* Mon May 08 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.33.0-2
- rebuild (Qt 5.9), drop BR: qt5-qtdeclarative-devel

* Mon Apr 03 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.33.0-1
- 5.33.0

* Thu Mar 30 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.32.0-2
- cosmetics, rebuild (Qt 5.8)

* Sat Mar 04 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.32.0-1
- 5.32.0

* Mon Feb 06 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.31.0-1
- 5.31.0

* Mon Jan 02 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.29.0-2
- filter plugin provides

* Fri Dec 16 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.29.0-1
- 5.29.0

* Thu Nov 17 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.27.0-2
- release++

* Thu Nov 17 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.27.0-1.2
- branch rebuild (qt5)

* Tue Oct 04 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.27.0-1
- 5.27.0

* Wed Sep 07 2016 Daniel Vrátil <dvratil@fedoraproject.org> - 5.26.0-1
- KDE Frameworks 5.26.0

* Mon Aug 08 2016 Daniel Vrátil <dvratil@fedoraproject.org> - 5.25.0-1
- KDE Frameworks 5.25.0

* Sun Jul 17 2016 Helio Chissini de Castro <helio@kde.org> - 5.24.0-2
- Rebuild agains Qt 5.7.0

* Wed Jul 06 2016 Daniel Vrátil <dvratil@fedoraproject.org> - 5.24.0-1
- KDE Frameworks 5.24.0

* Wed Jun 29 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.23.0-4
- rebuild (qt5)

* Fri Jun 10 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.23.0-3
- BR: qt5-qtbase-private-devel

* Fri Jun 10 2016 Jan Grulich <jgrulich@redhat.com> - 5.23.0-2
- Rebuild (qt5-qtbase)

* Tue Jun 07 2016 Daniel Vrátil <dvratil@fedoraproject.org> - 5.23.0-1
- KDE Frameworks 5.23.0

* Mon May 16 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.22.0-1
- KDE Frameworks 5.22.0

* Fri Apr 08 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.21.0-3
- Requires: google-noto-sans oxygen-mono (where Recommends are not supported)

* Wed Apr 06 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.21.0-2
- update URL

* Wed Apr 06 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.21.0-1.1
- rebuild (f23 - against qt-5.6.x)

* Mon Apr 04 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.21.0-1
- KDE Frameworks 5.21.0

* Mon Mar 21 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.20.0-2
- Add dep on qt5-qtbase used at buildtime

* Mon Mar 14 2016 Daniel Vrátil <dvratil@fedoraproject.org> - 5.20.0-1
- KDE Frameworks 5.20.0

* Thu Feb 11 2016 Daniel Vrátil <dvratil@fedoraproject.org> - 5.19.0-1
- KDE Frameworks 5.19.0

* Thu Feb 04 2016 Fedora Release Engineering <releng@fedoraproject.org> - 5.18.0-4
- Rebuilt for https://fedoraproject.org/wiki/Fedora_24_Mass_Rebuild

* Thu Jan 14 2016 Rex Dieter <rdieter@fedoraproject.org> 5.18.0-3
- -BR: cmake

* Fri Jan 08 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.18.0-2
- omit kconf_update/frameworksintegration-5.16-font scripts
- move plugins to -libs

* Sun Jan 03 2016 Daniel Vrátil <dvratil@fedoraproject.org> - 5.18.0-1
- KDE Frameworks 5.18.0

* Wed Dec 16 2015 Rex Dieter <rdieter@fedoraproject.org> - 5.17.0-2
- drop qt56/moc hack (qtbase should be fixed)
- .spec cosmetics, update URL, use %%license

* Tue Dec 08 2015 Daniel Vrátil <dvratil@fedoraproject.org> - 5.17.0-1
- KDE Frameworks 5.17.0

* Sun Nov 08 2015 Daniel Vrátil <dvratil@fedoraproject.org> - 5.16.0-1
- KDE Frameworks 5.16.0

* Thu Oct 08 2015 Daniel Vrátil <dvratil@redhat.com> - 5.15.0-1
- KDE Frameworks 5.15.0

* Wed Sep 16 2015 Daniel Vrátil <dvratil@redhat.com> - 5.14.0-1
- KDE Frameworks 5.14.0

* Wed Aug 26 2015 Daniel Vrátil <dvratil@redhat.com> - 5.15.0-2
- Workaround GCC bug with strict-aliasing on F22 (RHBZ#1255902)

* Wed Aug 19 2015 Daniel Vrátil <dvratil@redhat.com> - 5.13.0-1
- KDE Frameworks 5.13.0

* Wed Aug 19 2015 Daniel Vrátil <dvratil@redhat.com> - 5.13.0-1
- KDE Frameworks 5.13.0

* Tue Aug 11 2015 Daniel Vrátil <dvratil@redhat.com> - 5.13.0-0.1
- KDE Frameworks 5.13

* Thu Jul 09 2015 Rex Dieter <rdieter@fedoraproject.org> - 5.12.0-1
- 5.12.0

* Thu Jul 09 2015 Rex Dieter <rdieter@fedoraproject.org> 5.11.0-3
- update url, own %%{_kf5_datadir}/kf5/infopage, .spec cleanup, fix main/-libs deps

* Wed Jun 17 2015 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 5.11.0-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_23_Mass_Rebuild

* Wed Jun 10 2015 Daniel Vrátil <dvratil@redhat.com> - 5.11.0-1
- KDE Frameworks 5.11.0

* Mon May 11 2015 Daniel Vrátil <dvratil@redhat.com> - 5.10.0-1
- KDE Frameworks 5.10.0

* Mon Apr 20 2015 Daniel Vrátil <dvratil@redhat.com> - 5.9.0-2
- pull upstream patch for native QDialogs

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

* Sun Oct 26 2014 Kevin Kofler <Kevin@tigcc.ticalc.org> - 5.3.0-3
- -libs: add versioned Conflicts to force main package to get upgraded alongside

* Sun Oct 26 2014 Daniel Vrátil <dvratil@redhat.com> - 5.3.0-2
- Move libKF5Style.so into -libs subpkg to simplify dependency chain for themes (RHBZ#1156687)

* Tue Oct 07 2014 Daniel Vrátil <dvratil@redhat.com> - 5.3.0-1
- KDE Frameworks 5.3.0

* Mon Sep 15 2014 Daniel Vrátil <dvratil@redhat.com> - 5.2.0-1
- KDE Frameworks 5.2.0

* Sat Aug 16 2014 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 5.1.0-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_21_22_Mass_Rebuild

* Wed Aug 06 2014 Daniel Vrátil <dvratil@redhat.com> - 5.1.0-1
- KDE Frameworks 5.1.0

* Wed Jul 09 2014 Daniel Vrátil <dvratil@redhat.com> - 5.0.0-1
- KDE Frameworks 5.0.0

* Tue Jun 03 2014 Daniel Vrátil <dvratil@redhat.com> - 4.100.0-1
- KDE Frameworks 4.100.0

* Mon May 05 2014 Daniel Vrátil <dvratil@redhat.com> - 4.99.0
- KDE Frameworks 4.99.0

* Mon Mar 31 2014 Jan Grulich <jgrulich@redhat.com> 4.98.0-1
- Update to KDE Frameworks 5 Beta 1 (4.98.0)

* Wed Mar 05 2014 Jan Grulich <jgrulich@redhat.com> 4.97.0-1
- Update to KDE Frameworks 5 Alpha 1 (4.97.0)

* Wed Feb 12 2014 Daniel Vrátil <dvratil@redhat.com> 4.96.0-1
- Update to KDE Frameworks 5 Alpha 1 (4.96.0)

* Thu Feb 06 2014 Daniel Vrátil <dvratil@redhat.com> 4.96.0-0.1.20140206git
- Update to pre-relase snapshot of 4.96.0

* Thu Jan 09 2014 Daniel Vrátil <dvratil@redhat.com> 4.95.0-1
- Update to KDE Frameworks 5 TP1 (4.9.95)

* Wed Jan 8 2014 Lukáš Tinkl <ltinkl@redhat.com>
- initial version
