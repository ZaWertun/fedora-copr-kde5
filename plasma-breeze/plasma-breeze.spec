%undefine __cmake_in_source_build

## bootstrap, omit problematic optional build deps)
#global bootstrap 1

%global         base_name   breeze

# disable kde4 build on RHEL8+
%global         build_kde4  1
%if 0%{?rhel} && 0%{?rhel} > 7
%global         build_kde4  0
%endif

Name:    plasma-breeze
Version: 5.19.5
Release: 1%{?dist}
Summary: Artwork, styles and assets for the Breeze visual style for the Plasma Desktop

License: GPLv2+
URL:     https://cgit.kde.org/%{base_name}.git

%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global majmin_ver %(echo %{version} | cut -d. -f1,2).50
%global stable unstable
%else
%global majmin_ver %(echo %{version} | cut -d. -f1,2)
%global stable stable
%endif
Source0: http://download.kde.org/%{stable}/plasma/%{version}/%{base_name}-%{version}.tar.xz

# filter plugin provides
%global __provides_exclude_from ^(%{_kde4_libdir}/kde4/.*\\.so|%{_kf5_qtplugindir}/.*\\.so)$

BuildRequires:  gettext
BuildRequires:  kdecoration-devel >= %{majmin_ver}

BuildRequires:  extra-cmake-modules
BuildRequires:  kf5-kauth-devel
BuildRequires:  kf5-frameworkintegration-devel
BuildRequires:  kf5-kcmutils-devel
BuildRequires:  kf5-kcompletion-devel
BuildRequires:  kf5-kconfig-devel
BuildRequires:  kf5-kcoreaddons-devel
BuildRequires:  kf5-kguiaddons-devel
BuildRequires:  kf5-ki18n-devel
BuildRequires:  kf5-kpackage-devel
BuildRequires:  kf5-kservice-devel
BuildRequires:  kf5-kwayland-devel
BuildRequires:  kf5-kwindowsystem-devel
BuildRequires:  kf5-plasma-devel
BuildRequires:  kf5-rpm-macros

BuildRequires:  libxcb-devel
BuildRequires:  fftw-devel
%if 0
# required kpackage plugins
BuildRequires:  plasma-packagestructure
%endif

BuildRequires:  cmake(Qt5DBus)
BuildRequires:  cmake(Qt5Quick)
BuildRequires:  cmake(Qt5Widgets)
BuildRequires:  cmake(Qt5X11Extras)

Requires:       %{name}-common = %{version}-%{release}

# since we provide a cmake dev-like file
Provides:       %{name}-devel = %{version}-%{release}

%description
%{summary}.

%package        common
Summary:        Common files shared between KDE 4 and Plasma 5 versions of the Breeze style
BuildArch:      noarch
%description    common
%{summary}.

%package -n     breeze-cursor-theme
Summary:        Breeze cursor theme
BuildArch:      noarch
Obsoletes:      breeze-icon-theme < 5.17.0
%if 0%{?fedora} < 24
# let's be paranoid on upgrade path
Requires:       breeze-icon-theme >= 5.17.0
%endif
Provides:       breeze-cursor-themes = %{version}-%{release}
%description -n breeze-cursor-theme
%{summary}.

%if 0%{?build_kde4}
%package -n     kde-style-breeze
Summary:        KDE 4 version of Plasma 5 artwork, style and assets
BuildRequires:  kdelibs4-devel
BuildRequires:  libxcb-devel
## currently mostly plasma5-specific resources, not needed or useful here really
Obsoletes:      plasma-breeze-kde4 < 5.1.95
Provides:       plasma-breeze-kde4%{?_isa} = %{version}-%{release}
%if 0
Supplements: (kde-runtime and plasma-workspace)
%endif
%description -n kde-style-breeze
%{summary}.
%endif


%prep
%autosetup -n %{base_name}-%{version} -p1


%build
%{cmake_kf5}
%cmake_build


%if 0%{?build_kde4}
%global _vpath_builddir %{_target_platform}-kde4
%{cmake_kde4} -DUSE_KDE4=TRUE -B %{_vpath_builddir}
%cmake_build
%undefine _vpath_builddir
%endif


%install
%cmake_install

%find_lang breeze --all-name

%if 0%{?build_kde4}
%global _vpath_builddir %{_target_platform}-kde4
%cmake_install
%undefine _vpath_builddir
%endif

# omit/rename kde4breeze.upd, seems to be causing problems for
# (at least) new users, lame workaround for
# http://bugzilla.redhat.com/1283348
mv %{buildroot}%{_kf5_datadir}/kconf_update/kde4breeze.upd \
   %{buildroot}%{_kf5_datadir}/kconf_update/kde4breeze.upd.BAK


%ldconfig_scriptlets

%files
%license COPYING
%{_kf5_qtplugindir}/org.kde.kdecoration2/breezedecoration.so
%{_kf5_qtplugindir}/styles/breeze.so
%{_kf5_datadir}/kstyle/themes/breeze.themerc
%{_kf5_qtplugindir}/kstyle_breeze_config.so
%{_kf5_datadir}/kconf_update/kde4breeze.upd*
%{_kf5_libdir}/kconf_update_bin/kde4breeze
# used by breezedecoration
%{_libdir}/libbreezecommon5.so.5*
%{_bindir}/breeze-settings5
%{_datadir}/icons/hicolor/*/apps/breeze-settings.*
%{_kf5_datadir}/kservices5/breezedecorationconfig.desktop
%{_kf5_datadir}/kservices5/breezestyleconfig.desktop
%{_kf5_datadir}/plasma/look-and-feel/org.kde.breezedark.desktop/
%{_kf5_metainfodir}/org.kde.breezedark.desktop.appdata.xml
# fedora does autodep on cmake-filesystem, others?
%if ! 0%{?fedora}
%dir %{_libdir}/cmake/
%endif
%{_libdir}/cmake/Breeze/

%files common -f breeze.lang
%{_datadir}/color-schemes/*.colors
%dir %{_datadir}/QtCurve/
%{_datadir}/QtCurve/Breeze.qtcurve
%{_datadir}/wallpapers/Next/

%if 0%{?build_kde4}
%ldconfig_scriptlets -n kde-style-breeze

%files -n kde-style-breeze
%{_kde4_appsdir}/kstyle/themes/breeze.themerc
%{_datadir}/kde4/apps/QtCurve/Breeze.qtcurve
%{_datadir}/kde4/apps/color-schemes/*.colors
%{_datadir}/kde4/apps/plasma/look-and-feel/org.kde.breezedark.desktop/*
%endif

%files -n breeze-cursor-theme
%doc cursors/Breeze/README
%dir %{_kf5_datadir}/icons/Breeze_Snow/
%{_kf5_datadir}/icons/Breeze_Snow/cursors/
%{_kf5_datadir}/icons/Breeze_Snow/index.theme
%dir %{_kf5_datadir}/icons/breeze_cursors/
%{_kf5_datadir}/icons/breeze_cursors/cursors/
%{_kf5_datadir}/icons/breeze_cursors/index.theme


%changelog
* Tue Sep 01 2020 Jan Grulich <jgrulich@redhat.com> - 5.19.5-1
- 5.19.5

* Tue Jul 28 2020 Fedora Release Engineering <releng@fedoraproject.org> - 5.19.4-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_33_Mass_Rebuild

* Tue Jul 28 2020 Jan Grulich <jgrulich@redhat.com> - 5.19.4-1
- 5.19.4

* Tue Jul 07 2020 Jan Grulich <jgrulich@redhat.com> - 5.19.3-1
- 5.19.3

* Tue Jun 23 2020 Jan Grulich <jgrulich@redhat.com> - 5.19.2-1
- 5.19.2

* Wed Jun 17 2020 Martin Kyral <martin.kyral@gmail.com> - 5.19.1-1
- 5.19.1

* Tue Jun 9 2020 Martin Kyral <martin.kyral@gmail.com> - 5.19.0-1
- 5.19.0

* Fri May 15 2020 Martin Kyral <martin.kyral@gmail.com> - 5.18.90-1
- 5.18.90

* Tue May 05 2020 Jan Grulich <jgrulich@redhat.com> - 5.18.5-1
- 5.18.5

* Sat Apr 04 2020 Rex Dieter <rdieter@fedoraproject.org> - 5.18.4.1-1
- 5.18.4.1

* Tue Mar 31 2020 Jan Grulich <jgrulich@redhat.com> - 5.18.4-1
- 5.18.4

* Tue Mar 10 2020 Jan Grulich <jgrulich@redhat.com> - 5.18.3-1
- 5.18.3

* Tue Feb 25 2020 Jan Grulich <jgrulich@redhat.com> - 5.18.2-1
- 5.18.2

* Tue Feb 18 2020 Jan Grulich <jgrulich@redhat.com> - 5.18.1-1
- 5.18.1

* Tue Feb 11 2020 Jan Grulich <jgrulich@redhat.com> - 5.18.0-1
- 5.18.0

* Thu Jan 30 2020 Fedora Release Engineering <releng@fedoraproject.org> - 5.17.90-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_32_Mass_Rebuild

* Thu Jan 16 2020 Jan Grulich <jgrulich@redhat.com> - 5.17.90-1
- 5.17.90

* Wed Jan 08 2020 Jan Grulich <jgrulich@redhat.com> - 5.17.5-1
- 5.17.5

* Thu Dec 05 2019 Jan Grulich <jgrulich@redhat.com> - 5.17.4-1
- 5.17.4

* Wed Nov 13 2019 Martin Kyral <martin.kyral@gmail.com> - 5.17.3-1
- 5.17.3

* Wed Oct 30 2019 Jan Grulich <jgrulich@redhat.com> - 5.17.2-1
- 5.17.2

* Wed Oct 23 2019 Jan Grulich <jgrulich@redhat.com> - 5.17.1-1
- 5.17.1

* Thu Oct 10 2019 Jan Grulich <jgrulich@redhat.com> - 5.17.0-1
- 5.17.0

* Fri Sep 20 2019 Martin Kyral <martin.kyral@gmail.com> - 5.16.90-1
- 5.16.90

* Fri Sep 06 2019 Martin Kyral <martin.kyral@gmail.com> - 5.16.5-1
- 5.16.5

* Tue Jul 30 2019 Martin Kyral <martin.kyral@gmail.com> - 5.16.4-1
- 5.16.4

* Fri Jul 26 2019 Fedora Release Engineering <releng@fedoraproject.org> - 5.16.3-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_31_Mass_Rebuild

* Wed Jul 10 2019 Martin Kyral <martin.kyral@gmail.com> - 5.16.3-1
- 5.16.3

* Wed Jun 26 2019 Martin Kyral <martin.kyral@gmail.com> - 5.16.2-1
- 5.16.2

* Tue Jun 18 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.16.1-1
- 5.16.1

* Tue Jun 11 2019 Martin Kyral <martin.kyral@gmail.com> - 5.16.0-1
- 5.16.0

* Thu May 16 2019 Martin Kyral <martin.kyral@gmail.com> - 5.15.90-1
- 5.15.90

* Thu May 09 2019 Martin Kyral <martin.kyral@gmail.com> - 5.15.5-1
- 5.15.5

* Wed Apr 03 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.15.4.1-1
- 5.15.4.1

* Tue Apr 02 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.15.3-2
- re-enable kde4 support

* Tue Mar 12 2019 Martin Kyral <martin.kyral@gmail.com> - 5.15.3-1
- 5.15.3

* Tue Feb 26 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.15.2-1
- 5.15.2

* Tue Feb 19 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.15.1-1
- 5.15.1

* Wed Feb 13 2019 Martin Kyral <martin.kyral@gmail.com> - 5.15.0-1
- 5.15.0
- omit kde-style-breeze on f30+, workaround FTBFS

* Sat Feb 02 2019 Fedora Release Engineering <releng@fedoraproject.org> - 5.14.90-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_30_Mass_Rebuild

* Wed Jan 23 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.14.90-2
- disable bootstrap

* Sun Jan 20 2019 Martin Kyral <martin.kyral@gmail.com> - 5.14.90-1
- 5.14.90
- enable bootstrap

* Tue Nov 27 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.14.4-1
- 5.14.4

* Thu Nov 08 2018 Martin Kyral <martin.kyral@gmail.com> - 5.14.3-1
- 5.14.3

* Wed Oct 24 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.14.2-1
- 5.14.2

* Fri Oct 19 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.14.1-2
- plasma-breeze force qt4 dependencies (#1640944)
- use %%make_build %%ldconfig_scriptlets
- drop extraneous kf5-filesystem dep

* Tue Oct 16 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.14.1-1
- 5.14.1

* Fri Oct 05 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.14.0-1
- 5.14.0

* Fri Sep 14 2018 Martin Kyral <martin.kyral@gmail.com> - 5.13.90-1
- 5.13.90

* Tue Sep 04 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.13.5-1
- 5.13.5

* Thu Aug 02 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.13.4-1
- 5.13.4

* Fri Jul 13 2018 Fedora Release Engineering <releng@fedoraproject.org> - 5.13.3-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_29_Mass_Rebuild

* Wed Jul 11 2018 Martin Kyral <martin.kyral@gmail.com> - 5.13.3-1
- 5.13.3

* Mon Jul 09 2018 Martin Kyral <martin.kyral@gmail.com> - 5.13.2-1
- 5.13.2

* Tue Jun 19 2018 Martin Kyral <martin.kyral@gmail.com> - 5.13.1-1
- 5.13.1

* Sat Jun 09 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.13.0-1
- 5.13.0

* Fri May 18 2018 Martin Kyral <martin.kyral@gmail.com> - 5.12.90-1
- 5.12.90

* Tue May 01 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.12.5-1
- 5.12.5

* Tue Mar 27 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.12.4-1
- 5.12.4

* Tue Mar 06 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.12.3-1
- 5.12.3

* Wed Feb 21 2018 Jan Grulich <jgrulich@redhat.com> - 5.12.2-1
- 5.12.2

* Tue Feb 13 2018 Jan Grulich <jgrulich@redhat.com> - 5.12.1-1
- 5.12.1

* Fri Feb 09 2018 Fedora Release Engineering <releng@fedoraproject.org> - 5.12.0-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_28_Mass_Rebuild

* Fri Feb 02 2018 Jan Grulich <jgrulich@redhat.com> - 5.12.0-1
- 5.12.0

* Thu Jan 18 2018 Igor Gnatenko <ignatenkobrain@fedoraproject.org> - 5.11.95-2
- Remove obsolete scriptlets

* Mon Jan 15 2018 Jan Grulich <jgrulich@redhat.com> - 5.11.95-1
- 5.11.95

* Tue Jan 02 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.11.5-1
- 5.11.5

* Thu Nov 30 2017 Martin Kyral <martin.kyral@gmail.com> - 5.11.4-1
- 5.11.4

* Wed Nov 08 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.11.3-1
- 5.11.3

* Wed Oct 25 2017 Martin Kyral <martin.kyral@gmail.com> - 5.11.2-1
- 5.11.2

* Tue Oct 17 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.11.1-1
- 5.11.1

* Wed Oct 11 2017 Martin Kyral <martin.kyral@gmail.com> - 5.11.0-1
- 5.11.0

* Mon Aug 28 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.10.5-2
- cleanup dir ownership (fedora uses cmake-filesystem automatically now)

* Thu Aug 24 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.10.5-1
- 5.10.5

* Thu Aug 03 2017 Fedora Release Engineering <releng@fedoraproject.org> - 5.10.4-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Binutils_Mass_Rebuild

* Thu Jul 27 2017 Fedora Release Engineering <releng@fedoraproject.org> - 5.10.4-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Mass_Rebuild

* Fri Jul 21 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.10.4-1
- 5.10.4

* Tue Jun 27 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.10.3-1
- 5.10.3

* Thu Jun 15 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.10.2-1
- 5.10.2

* Tue Jun 06 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.10.1-1
- 5.10.1

* Wed May 31 2017 Jan Grulich <jgrulich@redhat.com> - 5.10.0-1
- 5.10.0

* Thu May 11 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.9.5-2
- rebuild (qt5)

* Wed Apr 26 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.9.5-1
- 5.9.5

* Thu Mar 23 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.9.4-1
- 5.9.4

* Sat Mar 04 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.9.3-2
- rebuild

* Wed Mar 01 2017 Jan Grulich <jgrulich@redhat.com> - 5.9.3-1
- 5.9.3, update URL/BR

* Tue Feb 21 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.8.6-1
- 5.8.6

* Sat Feb 11 2017 Fedora Release Engineering <releng@fedoraproject.org> - 5.8.5-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_26_Mass_Rebuild

* Mon Jan 02 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.8.5-2
- filter plugin provides

* Wed Dec 28 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.8.5-1
- 5.8.5

* Tue Nov 22 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.8.4-1
- 5.8.4

* Tue Nov 01 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.8.3-1
- 5.8.3

* Tue Oct 18 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.8.2-1
- 5.8.2

* Tue Oct 11 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.8.1-1
- 5.8.1

* Fri Sep 30 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.8.0-2
- don't use plasma-packagestructure yet

* Thu Sep 29 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.8.0-1
- 5.8.0

* Fri Sep 23 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.7.95-2
- de-bootstrap

* Thu Sep 22 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.7.95-1
- 5.7.95

* Tue Sep 13 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.7.5-1
- 5.7.5

* Sun Sep 11 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.7.4-3
- BR: plasma-workspace-libs (kpackage plugins, kde#367923)

* Sun Sep 11 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.7.4-2
- sort/simplify build deps (+BR: kf5-kwayland)
- make note of appdata/metainfo generation failure(s) in koji

* Tue Aug 23 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.7.4-1
- 5.7.4

* Tue Aug 02 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.7.3-1
- 5.7.3

* Tue Jul 19 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.7.2-1
- 5.7.2

* Thu Jul 14 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.7.1-2
- Provides: plasma-breeze-devel

* Tue Jul 12 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.7.1-1
- 5.7.1

* Thu Jun 30 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.7.0-1
- 5.7.0

* Sat Jun 25 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.6.95-1
- 5.6.95

* Tue Jun 14 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.6.5-1
- 5.6.5

* Sat May 14 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.6.4-1
- 5.6.4

* Wed Apr 20 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.6.3-2
- rebuild (qt)

* Tue Apr 19 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.6.3-1
- 5.6.3

* Mon Apr 18 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.6.2-5
- rebuild (qt)

* Thu Apr 14 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.6.2-3
- breeze-cursor-theme: Obsoletes/Requires: breeze-icon-theme (upgrade path)

* Mon Apr 11 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.6.2-2
- kde-style-breeze: drop use of Supplements, workaround bug #1325471

* Sat Apr 09 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.6.2-1
- 5.6.2

* Fri Apr 08 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.6.1-1
- 5.6.1

* Mon Mar 14 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.5.5-3
- kde-style-breeze: drop Requires: -common, Supplements for f23 too
- drop BR: cmake

* Fri Mar 11 2016 Rex Dieter <rdieter@fedoraproject.org> 5.5.5-2
- kde-style-breeze: Supplements: (kde-runtime and plasma-workspace), f24+

* Tue Mar 01 2016 Daniel Vrátil <dvratil@fedoraproject.org> - 5.5.5-1
- Plasma 5.5.5

* Thu Feb 04 2016 Fedora Release Engineering <releng@fedoraproject.org> - 5.5.4-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_24_Mass_Rebuild

* Wed Jan 27 2016 Daniel Vrátil <dvratil@fedoraproject.org> - 5.5.4-1
- Plasma 5.5.4

* Fri Jan 08 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.5.3-2
- .spec cosmetics
- drop icon-related deps
- breeze-cursor-theme: tighten %%files, don't use %%ghost, drop scriptlets
- avoid kde4breeze.upd, causes problems for new users (#1283348)

* Thu Jan 07 2016 Daniel Vrátil <dvratil@fedoraproject.org> - 5.5.3-1
- Plasma 5.5.3

* Thu Dec 31 2015 Rex Dieter <rdieter@fedoraproject.org> - 5.5.2-1
- 5.5.2

* Fri Dec 18 2015 Daniel Vrátil <dvratil@fedoraproject.org> - 5.5.1-1
- Plasma 5.5.1

* Thu Dec 03 2015 Daniel Vrátil <dvratil@fedoraproject.org> - 5.5.0-1
- Plasma 5.5.0

* Wed Nov 25 2015 Daniel Vrátil <dvratil@fedoraproject.org> - 5.4.95-2
- Plasma 5.4.95

* Sun Nov 15 2015 Rex Dieter <rdieter@fedoraproject.org> 5.4.3-5
- icon-theme/cursor theme: drop Requires: -common, add versioned Conflicts instead

* Sun Nov 15 2015 Rex Dieter <rdieter@fedoraproject.org> 5.4.3-4
- breeze-cursor-theme pkg (#1282203)

* Fri Nov 06 2015 Daniel Vrátil <dvraitl@fedoraproject.org> - 5.4.3-2
- tarball respin

* Thu Nov 05 2015 Daniel Vrátil <dvratil@fedoraproject.org> - 5.4.3-1
- Plasma 5.4.3

* Tue Oct 13 2015 Jan Grulich <jgrulich@redhat.com> - 5.4.2-2
- Fix breeze-dark icons inheritance

* Thu Oct 01 2015 Rex Dieter <rdieter@fedoraproject.org> - 5.4.2-1
- 5.4.2

* Wed Sep 16 2015 Rex Dieter <rdieter@fedoraproject.org> 5.4.1-2
- breeze-icon-theme: optimizegraphics,hardlink optimizations

* Wed Sep 09 2015 Rex Dieter <rdieter@fedoraproject.org> - 5.4.1-1
- 5.4.1

* Fri Aug 21 2015 Daniel Vrátil <dvratil@redhat.com> - 5.4.0-1
- Plasma 5.4.0

* Thu Aug 13 2015 Daniel Vrátil <dvratil@redhat.com> - 5.3.95-1
- Plasma 5.3.95

* Thu Jun 25 2015 Daniel Vrátil <dvratil@redhat.com> - 5.3.2-1
- Plasma 5.3.2

* Thu Jun 18 2015 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 5.3.1-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_23_Mass_Rebuild

* Tue May 26 2015 Daniel Vrátil <dvratil@redhat.com> - 5.3.1-1
- Plasma 5.3.1

* Mon Apr 27 2015 Daniel Vrátil <dvratil@redhat.com> - 5.3.0-1
- Plasma 5.3.0

* Wed Apr 22 2015 Daniel Vrátil <dvratil@redhat.com> - 5.2.95-1
- Plasma 5.2.95

* Fri Mar 20 2015 Daniel Vrátil <dvratil@redhat.com> - 5.2.2-1
- Plasma 5.2.2

* Tue Mar 10 2015 Rex Dieter <rdieter@fedoraproject.org> - 5.2.1-3
- backport upstream fixes (mostly crashers)
- .spec cosmetics

* Fri Feb 27 2015 Daniel Vrátil <dvratil@redhat.com> - 5.2.1-2
- Rebuild (GCC 5)

* Tue Feb 24 2015 Daniel Vrátil <dvratil@redhat.com> - 5.2.1-1
- Plasma 5.2.1

* Mon Jan 26 2015 Daniel Vrátil <dvratil@redhat.com> - 5.2.0-1
- Plasma 5.2.0

* Mon Jan 12 2015 Daniel Vrátil <dvratil@redhat.com> - 5.1.95-1.beta
- Plasma 5.1.95 Beta

* Mon Jan 05 2015 Jan Grulich <jgrulich@redhat.com> - 5.1.1-2
- better URL
  breeze-kde4 renamed to kde-style-breeze
  created breeze-icon-theme subpackage
  used make install instead of make_install macro

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

* Sun Aug 10 2014 Daniel Vrátil <dvratil@redhat.com> - 5.0.1-1
- Plasma 5.0.1

* Wed Jul 16 2014 Daniel Vrátil <dvratil@redhat.com> - 5.0.0-1
- Plasma 5.0.0

* Wed May 14 2014 Daniel Vrátil <dvratil@redhat.com> - 4.90.1-1.20140514git73a19ea
- Update to latest upstream

* Fri May 02 2014 Jan Grulich <jgrulich@redhat.com> 4.90.1-0.1.20140502git
- Initial version
