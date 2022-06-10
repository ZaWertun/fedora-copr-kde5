# trim changelog included in binary rpms
%global _changelog_trimtime %(date +%s -d "1 year ago")

Name:    ktorrent
Version: 22.04.2
Release: 1%{?dist}
Summary: A BitTorrent program

License: GPLv2+
URL:     https://www.kde.org/applications/internet/ktorrent/

%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0: http://download.kde.org/stable/release-service/%{version}/src/%{name}-%{version}.tar.xz

%global majmin_ver %(echo %{version} | cut -d. -f1,2)

## upstream patches

## upstreamable patches

## downstream patches
# Fix build for Fedora 34 (libQt5Qml.so.5: error adding symbols: DSO missing from command line)
Patch1:  fix_fedora34_build.patch

BuildRequires: boost-devel
BuildRequires: cmake(Qca-qt5)
BuildRequires: desktop-file-utils
BuildRequires: gettext
BuildRequires: GeoIP-devel
BuildRequires: libappstream-glib
BuildRequires: pkgconfig(phonon4qt5)
BuildRequires: pkgconfig(taglib)
# kf5 deps
BuildRequires: extra-cmake-modules
BuildRequires: kf5-rpm-macros

BuildRequires: cmake(KF5Config)
BuildRequires: cmake(KF5ConfigWidgets)
BuildRequires: cmake(KF5Crash)
BuildRequires: cmake(KF5DBusAddons)
BuildRequires: cmake(KF5I18n)
BuildRequires: cmake(KF5IconThemes)
BuildRequires: cmake(KF5KIO)
BuildRequires: cmake(KF5Notifications)
BuildRequires: cmake(KF5NotifyConfig)
BuildRequires: cmake(KF5KCMUtils)
BuildRequires: cmake(KF5Parts)
BuildRequires: cmake(KF5Service)
BuildRequires: cmake(KF5Solid)
BuildRequires: cmake(KF5WidgetsAddons)
BuildRequires: cmake(KF5WindowSystem)
BuildRequires: cmake(KF5XmlGui)
# optional (kf5) deps
BuildRequires: cmake(KF5Archive)
BuildRequires: cmake(KF5Completion)
BuildRequires: cmake(KF5DNSSD)
BuildRequires: cmake(KF5DocTools)
BuildRequires: cmake(KF5ItemViews)
BuildRequires: cmake(KF5Kross)
BuildRequires: cmake(KF5Plotting)
BuildRequires: cmake(KF5TextWidgets)

BuildRequires: cmake(KF5Syndication)
BuildRequires: cmake(LibKWorkspace)

%ifarch %{qt5_qtwebengine_arches}
BuildRequires: cmake(Qt5WebEngineWidgets)
%endif

BuildRequires: kf5-libktorrent-devel >= %{version}

# multilib, when -libs was introduced
Obsoletes: ktorrent < 3.2.3-2

Requires: %{name}-libs%{?_isa} = %{version}-%{release}

%description
KTorrent is a BitTorrent program for KDE. Its main features are native KDE
integration, download of torrent files, upload speed capping, internet
searching using various search engines, UDP Trackers and UPnP support.

%package libs
Summary: Runtime libraries for %{name}
Requires: %{name}%{?_isa} = %{version}-%{release}
Requires: kf5-libktorrent%{?_isa} >= %{version}
%description libs
%{summary}.


%prep
%autosetup -n %{name}-%{version}%{?pre} -p1


%build
%cmake_kf5 \
  -DWITH_SYSTEM_GEOIP:BOOL=ON
%cmake_build


%install
%cmake_install

# ensure this exists (sometimes not, e.g. when qtwebengine support isn't available)
mkdir -p %{buildroot}%{_kf5_datadir}/ktorrent

%find_lang %{name} --with-html


%check
appstream-util validate-relax --nonet %{buildroot}%{_kf5_metainfodir}/org.kde.ktorrent.appdata.xml
desktop-file-validate %{buildroot}%{_kf5_datadir}/applications/org.kde.ktorrent.desktop


%files -f %{name}.lang
%doc ChangeLog
%license LICENSES/*
%{_kf5_bindir}/ktorrent
%{_kf5_bindir}/ktmagnetdownloader
%{_kf5_bindir}/ktupnptest
%{_kf5_metainfodir}/org.kde.ktorrent.appdata.xml
%{_kf5_datadir}/applications/org.kde.ktorrent.desktop
%{_kf5_datadir}/icons/hicolor/*/*/*
%{_kf5_datadir}/ktorrent/
%{_kf5_datadir}/knotifications5/ktorrent.notifyrc
%{_kf5_datadir}/kxmlgui5/ktorrent/

%ldconfig_scriptlets libs

%files libs
%{_kf5_libdir}/libktcore.so.*
%{_kf5_qtplugindir}/ktorrent_plugins/ktorrent_*.so


%changelog
* Thu Jun 09 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.04.2-1
- 22.04.2

* Thu May 12 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.04.1-1
- 22.04.1

* Thu Apr 21 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.04.0-1
- 22.04.0

* Thu Mar 03 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 21.12.3-1
- 21.12.3

* Thu Feb 03 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 21.12.2-1
- 21.12.2

* Thu Jan 13 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 21.12.1-1
- 21.12.1

* Mon Dec 27 2021 Rex Dieter <rdieter@fedoraproject.org> - 21.12.0-1
- 21.12.0

* Tue Nov 02 2021 Rex Dieter <rdieter@fedoraproject.org> - 21.08.3-1
- 21.08.3

* Thu Oct 21 2021 Rex Dieter <rdieter@fedoraproject.org> - 21.08.2-1
- 21.08.2

* Wed Jul 28 2021 Rex Dieter <rdieter@fedoraproject.org> - 21.04.3-1
- 21.04.3

* Thu Jul 22 2021 Fedora Release Engineering <releng@fedoraproject.org> - 21.04.2-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_35_Mass_Rebuild

* Thu Jun 10 2021 Rex Dieter <rdieter@fedoraproject.org> - 21.04.2-1
- 21.04.2

* Tue May 11 2021 Rex Dieter <rdieter@fedoraproject.org> - 21.04.1-1
- 21.04.1

* Mon Apr 19 2021 Rex Dieter <rdieter@fedoraproject.org> - 21.04.0-1
- 21.04.0

* Wed Mar 03 2021 Rex Dieter <rdieter@fedoraproject.org> - 20.12.3-1
- 20.12.3

* Sat Feb 27 2021 Rex Dieter <rdieter@fedoraproject.org> -  20.12.2-1
- part of kde releases

* Tue Jan 26 2021 Fedora Release Engineering <releng@fedoraproject.org> - 5.2.0-6
- Rebuilt for https://fedoraproject.org/wiki/Fedora_34_Mass_Rebuild

* Fri Oct 02 2020 Rex Dieter <rdieter@fedoraproject.org> - 5.2.0-5
- pull in upstream build fix (taglib)

* Sat Aug 01 2020 Fedora Release Engineering <releng@fedoraproject.org> - 5.2.0-4
- Second attempt - Rebuilt for
  https://fedoraproject.org/wiki/Fedora_33_Mass_Rebuild

* Tue Jul 28 2020 Fedora Release Engineering <releng@fedoraproject.org> - 5.2.0-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_33_Mass_Rebuild

* Thu Jul 02 2020 Rex Dieter <rdieter@fedoraproject.org> - 5.2.0-2
- BR: Qt5WebEngineWidgets

* Wed Jul 01 2020 Rex Dieter <rdieter@fedoraproject.org> - 5.2.0-1
- 5.2.0

* Wed Jan 29 2020 Fedora Release Engineering <releng@fedoraproject.org> - 5.1.2-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_32_Mass_Rebuild

* Wed Sep 11 2019 Rex Dieter <rdieter@fedoraproject.org> 5.1.2-1
- 5.1.2

* Thu Jul 25 2019 Fedora Release Engineering <releng@fedoraproject.org> - 5.1.1-6
- Rebuilt for https://fedoraproject.org/wiki/Fedora_31_Mass_Rebuild

* Fri Feb 01 2019 Fedora Release Engineering <releng@fedoraproject.org> - 5.1.1-5
- Rebuilt for https://fedoraproject.org/wiki/Fedora_30_Mass_Rebuild

* Thu Nov 29 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.1.1-4
- pull in upstream memory corruption fix

* Thu Nov 29 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.1.1-3
- rebuild

* Mon Oct 01 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.1.1-2
- rebuild (kf5-syndication)

* Fri Aug 31 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.1.1-1
- 5.1.1

* Fri Jul 13 2018 Fedora Release Engineering <releng@fedoraproject.org> - 5.1.0-6
- Rebuilt for https://fedoraproject.org/wiki/Fedora_29_Mass_Rebuild

* Sun May 27 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.1.0-5
- pull in upstream fixes
- use %%make_build %%ldconfig_scriptlets
- drop deprecated scriptlets

* Wed Feb 07 2018 Fedora Release Engineering <releng@fedoraproject.org> - 5.1.0-4
- Rebuilt for https://fedoraproject.org/wiki/Fedora_28_Mass_Rebuild

* Fri Jan 19 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.1.0-3
- ktorrent reports wrong client ID (#1536284)

* Wed Nov 29 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.1.0-2
- update URL

* Wed Nov 29 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.1-1
- 5.1

* Tue Aug 22 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.0.90-1
- 5.0.90

* Thu Aug 03 2017 Fedora Release Engineering <releng@fedoraproject.org> - 5.0.1-5
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Binutils_Mass_Rebuild

* Wed Jul 26 2017 Fedora Release Engineering <releng@fedoraproject.org> - 5.0.1-4
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Mass_Rebuild

* Fri Feb 10 2017 Fedora Release Engineering <releng@fedoraproject.org> - 5.0.1-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_26_Mass_Rebuild

* Sat Dec 24 2016 Rex Dieter <rdieter@math.unl.edu> - 5.0.1-2
- -libs: remove reference to old libktorrent

* Fri Dec 23 2016 Rex Dieter <rdieter@math.unl.edu> - 5.0.1-1
- (kf5-based) ktorrent-5.0.1

* Thu Sep 01 2016 Rex Dieter <rdieter@fedoraproject.org> 4.3.1-25
- update URL (#1325258)

* Fri Feb 19 2016 Rex Dieter <rdieter@fedoraproject.org> 4.3.1-24
- ktorrent: FTBFS in rawhide (#1307704)

* Thu Feb 04 2016 Fedora Release Engineering <releng@fedoraproject.org> - 4.3.1-23
- Rebuilt for https://fedoraproject.org/wiki/Fedora_24_Mass_Rebuild

* Sun Dec 20 2015 Rex Dieter <rdieter@fedoraproject.org> 4.3.1-22
- (re)enable shutdown plugin

* Thu Dec 17 2015 Rex Dieter <rdieter@fedoraproject.org> 4.3.1-21
- unversioned kde-runtime dep (#1292259)

* Thu Oct 22 2015 Rex Dieter <rdieter@fedoraproject.org> 4.3.1-20
- pull in latest upstream fixes, use %%kde_runtime_requires macro, update URL

* Wed Jul 29 2015 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 4.3.1-19
- Rebuilt for https://fedoraproject.org/wiki/Changes/F23Boost159

* Wed Jul 22 2015 David Tardon <dtardon@redhat.com> - 4.3.1-18
- rebuild for Boost 1.58

* Fri Jun 19 2015 Rex Dieter <rdieter@fedoraproject.org> 4.3.1-17
- omit kde-plasma-ktorrent on f22+

* Wed Jun 17 2015 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 4.3.1-16
- Rebuilt for https://fedoraproject.org/wiki/Fedora_23_Mass_Rebuild

* Sat May 02 2015 Kalev Lember <kalevlember@gmail.com> - 4.3.1-15
- Rebuilt for GCC 5 C++11 ABI change

* Tue Jan 27 2015 Petr Machata <pmachata@redhat.com> - 4.3.1-14
- Rebuild for boost 1.57.0

* Tue Jan 27 2015 Petr Machata <pmachata@redhat.com> - 4.3.1-13
- Rebuild for boost 1.57.0

* Sun Aug 17 2014 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 4.3.1-12
- Rebuilt for https://fedoraproject.org/wiki/Fedora_21_22_Mass_Rebuild

* Sun Jul 13 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.3.1-11
- pull in some upstream fixes... in particular,
- ktorrent can't parse ip block list (#1101122)
- s/kdebase-/kde-/ in -runtime,-workspace deps

* Thu Jun 19 2014 Rex Dieter <rdieter@fedoraproject.org> 4.3.1-10
- BR: kdelibs4-webkit-devel

* Sun Jun 08 2014 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 4.3.1-9
- Rebuilt for https://fedoraproject.org/wiki/Fedora_21_Mass_Rebuild

* Fri May 23 2014 Petr Machata <pmachata@redhat.com> - 4.3.1-8
- Rebuild for boost 1.55.0

* Sat Apr 19 2014 Rex Dieter <rdieter@fedoraproject.org> 4.3.1-7
- plasma-dataengine-depextractor support

* Fri Jan 31 2014 Rex Dieter <rdieter@fedoraproject.org> 4.3.1-6
- update URL's

* Wed Sep 25 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.3.1-5
- drop unused BR: avahi-devel
- trim changelog

* Sat Aug 03 2013 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 4.3.1-4
- Rebuilt for https://fedoraproject.org/wiki/Fedora_20_Mass_Rebuild

* Sun Apr 07 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.3.1-3
- .spec cleanup
- BR: boost-devel

* Thu Feb 14 2013 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 4.3.1-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_19_Mass_Rebuild

* Tue Jan 15 2013 Alexey Kurov <nucleo@fedoraproject.org> - 4.3.1-1
- ktorrent-4.3.1

* Sat Sep  1 2012 Alexey Kurov <nucleo@fedoraproject.org> - 4.3.0-1
- ktorrent-4.3.0

* Wed Aug 15 2012 Alexey Kurov <nucleo@fedoraproject.org> - 4.3-0.1.rc1
- ktorrent-4.3rc1

* Thu Jul 19 2012 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 4.2.1-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_18_Mass_Rebuild

* Tue Jun 12 2012 Alexey Kurov <nucleo@fedoraproject.org> - 4.2.1-1
- ktorrent-4.2.1

* Wed May 30 2012 Rex Dieter <rdieter@fedoraproject.org> 4.2.0-5
- -libs: Requires: libktorrent >= ...

* Mon May 28 2012 Rex Dieter <rdieter@fedoraproject.org>
- 4.2.0-4
- drop BR: gmp-devel (not used here, libktorrent does)
- kde-plasma-ktorrent: +Requires: %%name-libs
- libs: +Requires: libktorrent
- drop deprecated/unused -DWITH_BUILTIN_COUNTRY_FLAGS option

* Thu Mar 29 2012 Rex Dieter <rdieter@fedoraproject.org> 4.2.0-3
- omit magnet.protocol
- support/use MimeTypes=x-scheme-handler/magnet; instead

* Thu Mar 29 2012 Rex Dieter <rdieter@fedoraproject.org> 4.2.0-2
- drop ENABLE_KIO_MAGNET, let main app handle it

* Mon Mar  5 2012 Alexey Kurov <nucleo@fedoraproject.org> - 4.2.0-1
- ktorrent-4.2.0
- set ENABLE_KIO_MAGNET=1

* Sat Jan  7 2012 Alexey Kurov <nucleo@fedoraproject.org> - 4.2-0.1.rc1
- ktorrent-4.2rc1

* Tue Nov 22 2011 Alexey Kurov <nucleo@fedoraproject.org> - 4.1.3-1
- ktorrent-4.1.3

* Tue Aug 30 2011 Alexey Kurov <nucleo@fedoraproject.org> - 4.1.2-1
- ktorrent-4.1.2

* Thu Apr 28 2011 Alexey Kurov <nucleo@fedoraproject.org> - 4.1.1-1
- ktorrent-4.1.1

* Tue Mar 15 2011 Alexey Kurov <nucleo@fedoraproject.org> - 4.1.0-1
- ktorrent-4.1.0

* Mon Feb 07 2011 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 4.1-0.3.rc1
- Rebuilt for https://fedoraproject.org/wiki/Fedora_15_Mass_Rebuild

* Sun Feb  6 2011 Alexey Kurov <nucleo@fedoraproject.org> - 4.1-0.2.rc1
- ktorrent-4.1rc1

* Thu Dec 30 2010 Alexey Kurov <nucleo@fedoraproject.org> - 4.1-0.1.beta1
- ktorrent-4.1beta1
- libktupnp code moved to libktorrent

* Mon Oct 18 2010 Rex Dieter <rdieter@fedoraproject.org> - 4.0.4-1
- ktorrent-4.0.4

* Tue Aug 31 2010 Alexey Kurov <nucleo@fedoraproject.org> - 4.0.3-1
- ktorrent-4.0.3

* Wed Aug 11 2010 David Malcolm <dmalcolm@redhat.com> - 4.0.2-2
- recompiling .py files against Python 2.7 (rhbz#623329)

* Thu Jul  8 2010 Alexey Kurov <nucleo@fedoraproject.org> - 4.0.2-1
- ktorrent-4.0.2
- drop DSO linking patch

* Tue Jun 15 2010 Alexey Kurov <nucleo@fedoraproject.org> - 4.0.1-1
- ktorrent-4.0.1
- fix DSO linking

* Mon May 24 2010 Rex Dieter <rdieter@fedoraproject.org> - 4.0.0-1
- ktorrent-4.0.0

* Thu May 06 2010 Rex Dieter <rdieter@fedoraproject.org> - 4.0-0.4.rc1
- ktorrent-4.0rc1

* Mon Apr 05 2010 Rex Dieter <rdieter@fedoraproject.org> - 4.0-0.3.beta2
- ktorrent-4.0beta2

* Thu Jan 28 2010 Rex Dieter <rdieter@fedoraproject.org> - 4.0-0.2.beta1
- -libs: use %%{_kde4_version}

* Mon Dec 21 2009 Rex Dieter <rdieter@fedoraproject.org> - 4.0-0.1.beta1
- ktorrent-4.0beta1

* Thu Dec 17 2009 Rex Dieter <rdieter@fedoraproject.org> - 3.3.2-1
- ktorrent-3.3.2

* Mon Nov 23 2009 Roland Wolters <wolters.liste@gmx.net> - 3.3.1-1
- ktorrent-3.3.1

* Mon Nov 09 2009 Rex Dieter <rdieter@fedoraproject.org> - 3.3-1
- ktorrent-3.3
- -libs: add/fix scriptlets, move kdelibs4 dep here

* Sat Oct 24 2009 Rex Dieter <rdieter@fedoraproject.org> - 3.3-0.1.rc1
- ktorrent-3.3rc1

* Sun Sep 27 2009 Rex Dieter <rdieter@fedoraproject.org> - 3.2.4-1
- ktorrent-3.2.4

* Mon Aug 24 2009 Rex Dieter <rdieter@fedoraproject.org> - 3.2.3-3
- upstream tarball respin

* Sun Aug 23 2009 Rex Dieter <rdieter@fedoraproject.org> - 3.2.3-2
- kde-plasma-ktorrent pkg (so main pkg doesn't pull in kdebase-workspace)
- -libs to make multilib friendly

* Wed Aug 12 2009 Roland Wolters <wolters.liste@gmx.net> - 3.2.3-1
- ktorrent-3.2.3

* Fri Jul 24 2009 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 3.2.2-4
- Rebuilt for https://fedoraproject.org/wiki/Fedora_12_Mass_Rebuild

* Tue Jul 07 2009 Rex Dieter <rdieter@fedoraproject.org> - 3.2.2-3
- don't use internal flags (prefer those provided by kdebase-runtime-flags)

* Wed Jun 03 2009 Roland Wolters <wolters.liste@gmx.net> - 3.2.2-2
- ktorrent-3.2.2

* Tue May 05 2009 Rex Dieter <rdieter@fedoraproject.org> - 3.2.1-3
- crash adjusting speed from systray (kdebug#188447, rhbz#499147)

* Wed Apr 29 2009 Rex Dieter <rdieter@fedoraproject.org> - 3.2.1-2
- -DWITH_SYSTEM_GEOIP=1

* Mon Apr 06 2009 Rex Dieter <rdieter@fedoraproject.org> - 3.2.1-1
- ktorrent-3.2.1
- optimize scriptlets

* Tue Feb 26 2009 Roland Wolters <wolters.liste@gmx.net> - 3.2-8
- Some spec file dependency fixes.

* Wed Feb 25 2009 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 3.2-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_11_Mass_Rebuild

* Tue Feb 17 2009 Roland Wolters <wolters.liste@gmx.net> - 3.2-1
- Update to new version ktorrent 3.2
 
* Tue Feb 01 2009 Roland Wolters <wolters.liste@gmx.net> - 3.1.6-4
- ktorrent-3.1.6-4

* Mon Nov 17 2008 Rex Dieter <rdieter@fedoraproject.org> - 3.1.5-1
- ktorrent-3.1.5 (#469870)

* Thu Oct 23 2008 Rex Dieter <rdieter@fedoraproject.org> - 3.1.4-1
- ktorrent-3.1.4 (#468233)

* Tue Oct 14 2008 Rex Dieter <rdieter@fedoraproject.org> - 3.1.3-4
- KDEDInit could not launch .../ktorrent (#451559, kde#157853)

* Mon Oct 13 2008 Roland Wolters <wolters.liste@gmx.net> - 3.1.3-3
- Update to upstream version 3.1.3

* Fri Aug 08 2008 Rex Dieter <rdieter@fedoraproject.org> - 3.1.2-1
- ktorrent-3.1.2

* Sun Jul 13 2008 Roland Wolters <wolters.liste@gmx.net> - 3.1-5
- Update to version 3.1

* Wed May 14 2008 Roland Wolters <wolters.liste@gmx.net> - 3.0.2-3
- bugfix update to version 3.0.2
- some spec file fixes due to an update error

* Mon Apr 28 2008 Rex Dieter <rdieter@fedoraproject.org> - 3.0.1-4
- %%postun: remove extraneous scriplets
- -devel: own %%{_kde4_includedir}/libbtcore/ (and subdirs)
- -devel: Requires: kdelibs4-devel
- drop: Requires: oxygen-icon-theme (kde4 runtime already does)
- Requires(post,postun): xdg-utils

* Thu Apr 17 2008 Roland Wolters <wolters.liste@gmx.net> - 3.0.1-3
- bugfix update to version 3.0.1

* Tue Feb 19 2008 Roland Wolters <wolters.liste@gmx.net> - 3.0.0-7
- further spec file improvements for the 3.0.0 version

* Tue Feb 19 2008 Fedora Release Engineering <rel-eng@fedoraproject.org> - 3.0.0-2
- Autorebuild for GCC 4.3

* Mon Feb 18 2008 Roland Wolters <wolters.liste@gmx.net> - 3.0.0-1
- first KDE 4 build

* Sun Jan 27 2008 Roland Wolters <wolters.liste@gmx.net> - 2.2.5-5
- updated to bugfix version 2.2.5
- fixed build-system-Qt problem in spec file

* Sat Dec 01 2007 Roland Wolters <wolters.liste@gmx.net> - 2.2.4-2
- changed build require from kdelibs-devel to kdelibs3-devel

* Thu Nov 21 2007 Roland Wolters <wolters.liste@gmx.net> - 2.2.4-1
- bugfix update to version 2.2.4

* Fri Nov 16 2007 Roland Wolters <wolters.liste@gmx.net> - 2.2.3-2
- fixed version number for libktorrent file (2.2.2 for now)

* Thu Nov 15 2007 Roland Wolters <wolters.liste@gmx.net> - 2.2.3-1
- features and bugfix update to version 2.2.3

* Wed Aug 19 2007 Roland Wolters <wolters.liste@gmx.net> - 2.2.2-1
- bugfix update to version 2.2.2

* Thu Aug 16 2007 Roland Wolters <wolters.liste@gmx.net> - 2.2.1-3
- licence tag corrected

* Wed Jul 25 2007 Jesse Keating <jkeating@redhat.com> - 2.2.1-2
- Rebuild for RH #249435

* Tue Jul 24 2007 Roland Wolters <wolters.liste@gmx.net> 2.2.1-1
- update to bugfix upstream 2.2.1

* Fri Jul 06 2007 Roland Wolters <wolters.liste@gmx.net> 2.2-2
- spec-file fixes re-included

* Tue Jul 03 2007 Roland Wolters <wolters.liste@gmx.net> 2.2-1
- update to upstream 2.2:
	- cleaner UI
	- - New file selection dialog
	- Statistics plugin with pretty graphs
	- Possibility to open as many tabs as you want
	- Diskspace monitoring and stopping of downloads when the diskspace
	  drops below a certain value
	- Individual torrent speed limits
	- Full disk preallocation to avoid fragmentation

* Wed Jun 13 2007 Roland Wolters <wolters.liste@gmx.net> 2.2rc1-1
- update to upstream 2.2rc1

* Tue Apr 03 2007 Rex Dieter <rdieter[AT]fedoraproject.org> 2.1.3-1
- ktorrent-2.1.3 (#235014)
- optimize %%configure

* Fri Mar 09 2007 Roland Wolters <wolters.liste@gmx.net> 2.1.2-2
- update to upstream 2.1.2

* Mon Mar 05 2007 Roland Wolters <wolters.liste@gmx.net> 2.1.1-2
- update to upstream 2.1.1

* Wed Feb 07 2007 Roland Wolters <wolters.liste@gmx.net> 2.1-7
- spec-file adjustments

* Wed Feb 07 2007 Roland Wolters <wolters.liste@gmx.net> 2.1-6
- fixed plugins bug
- fixed configure warnings

* Tue Feb 06 2007 Roland Wolters <wolters.liste@gmx.net> 2.1-4
- added avahi-devel dependecy

* Mon Feb 05 2007 Roland Wolters <wolters.liste@gmx.net> 2.1-3
- Update to upstream version 2.1

* Fri Oct 13 2006 Roland Wolters <wolters.liste@gmx.net> 2.0.3-4
- Update to upstream version 2.0.3
- added znow patch for ppc

* Thu Sep 07 2006 Roland Wolters <wolters.liste@gmx.net> 2.0.2-3
- mass rebuild

* Tue Aug 30 2006 Roland Wolters <wolters.liste@gmx.net> 2.0.2-1
- minor bugfix update

* Tue Aug 29 2006 Roland Wolters <wolters.liste@gmx.net> 2.0.1-3
- mass rebuild

* Mon Aug 21 2006 Roland Wolters <wolters.liste@gmx.net> 2.0.1-1
- update to version 2.0.1

* Sun Aug 20 2006 Roland Wolters <wolters.liste@gmx.net> 2.0-6
- increased minor version to avoid broken update path

* Mon Aug 14 2006 Roland Wolters <wolters.liste@gmx.net> 2.0-3
- fixed small errors in spec file

* Wed Aug 09 2006 Roland Wolters <wolters.liste@gmx.net> 2.0-1
- update to version 2.0

* Fri Jun 23 2006 Roland Wolters <wolters.liste@gmx.net> 1.2-6
- fixed doublication error in rpm spec
- spec file polishing

* Fri Jun 23 2006 Roland Wolters <wolters.liste@gmx.net> 1.2-5
- added %%{_datadir}/apps/ktorrent
- removed redundant KTorrent in summary

* Wed Jun 21 2006 Roland Wolters <wolters.liste@gmx.net> 1.2-4
- changed e-mail address to correct packager address

* Wed Apr  5 2006 Roland Wolters <rolandwolters@web.de> 1.2-3
- corrected *.desktop files
- changed icon scriplets

* Wed Apr  5 2006 Roland Wolters <rolandwolters@web.de> 1.2-2
- set vendor string to ""
- added gtk-update-icon-cache scriplets to post and postun
- added desktop-database scriplet because of MimeType in ktorrent.desktop
- moved %%{_libdir}/kde3/* to main package
- moved %%{_libdir}/libktorrent.so to main package
- configure with disable-static and enable-shared
- added %%exclude %%{_libdir}/lib*.la

* Mon Apr  3 2006 Roland Wolters <rolandwolters@web.de> 1.2-1
- initial packaging
