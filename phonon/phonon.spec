
## When bootstrapping new releases/archs, set this initially to avoid
## unresolvable dependency on phonon-backend (and friends)
#global bootstrap 1

%if !0%{?bootstrap}
## enable zeitgeist support
#global zeitgeist 1
## enable experimental qtdeclarative/qml1 plugin
## FIXME/TODO: still FTBFS if enabling this feature (unpackaged files, where to put them?)
#global declarative 1
%endif

Summary: Multimedia framework api
Name:    phonon
Version: 4.11.1
Release: 1%{?dist}
License: LGPLv2+
URL:     http://phonon.kde.org/

%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0: http://download.kde.org/%{stable}/phonon/%{version}/phonon-%{version}.tar.xz

## upstream patches

## upstreamable patches

# filter plugins
%global __provides_exclude_from ^(%{_qt4_plugindir}/.*\\.so|%{_qt5_plugindir}/.*\\.so)$

BuildRequires: cmake >= 2.8.9
BuildRequires: extra-cmake-modules
BuildRequires: kf5-rpm-macros
BuildRequires: kde-filesystem
BuildRequires: pkgconfig
BuildRequires: pkgconfig(glib-2.0)
BuildRequires: pkgconfig(libpulse-mainloop-glib) > 0.9.15
BuildRequires: pkgconfig(libxml-2.0)
# Qt5
BuildRequires: pkgconfig(Qt5DBus) pkgconfig(Qt5Designer) pkgconfig(Qt5OpenGL) pkgconfig(Qt5Widgets)
BuildRequires: pkgconfig(xcb)

## optional features
%if 0%{?declarative}
BuildRequires: pkgconfig(QtDeclarative)
BuildRequires: pkgconfig(Qt5Declarative)
%endif
%if 0%{?zeitgeist}
BuildRequires: pkgconfig(QZeitgeist)
%endif

Requires: kde-filesystem
%if 0%{?bootstrap}
Provides: phonon-backend%{?_isa} = 4.7
%else
%if 0%{?fedora} || 0%{?rhel} > 7
Recommends: phonon-backend-gstreamer%{?_isa}
%else
Requires: phonon-backend%{?_isa} => 4.7
%endif
%endif

%{?_qt4:Requires: %{_qt4}%{?_isa} >= %{_qt4_version}}

Provides:  phonon-experimental = %{version}-%{release}

%description
%{summary}.

%package devel
Summary: Developer files for %{name}
Requires: %{name}%{?_isa} = %{version}-%{release}
Provides:  phonon-experimental-devel = %{version}-%{release}
%description devel
%{summary}.

%package qt5 
Summary: phonon for Qt5
%{?_qt5:Requires: %{_qt5}%{?_isa} >= %{_qt5_version}}
%if 0%{?bootstrap}
Recommends: phonon-qt5-backend-gstreamer%{?_isa}
%endif
%if 0%{?fedora} || 0%{?rhel} > 7
Provides: phonon-qt5-backend%{?_isa} = 4.7
%else
Requires: phonon-qt5-backend%{?_isa} => 4.7
%endif
%description qt5 
%{summary}.

%package qt5-devel
Summary: Developer files for %{name}-qt5 
Requires: %{name}-qt5%{?_isa} = %{version}-%{release}
%description qt5-devel
%{summary}.


%prep
%autosetup -p1


%build
mkdir %{_target_platform}
pushd %{_target_platform}
%{cmake} .. \
  -DCMAKE_BUILD_TYPE:STRING="Release" \
  %{!?zeitgeist:-DCMAKE_DISABLE_FIND_PACKAGE_QZeitgeist:BOOL=ON} \
  -DPHONON_BUILD_DECLARATIVE_PLUGIN:BOOL=%{?declarative:ON}%{!?declarative:OFF} \
  -DPHONON_INSTALL_QT_COMPAT_HEADERS:BOOL=ON \
  -DPHONON_QT_IMPORTS_INSTALL_DIR=%{_qt4_importdir} \
  -DPHONON_QT_MKSPECS_INSTALL_DIR=%{_qt4_datadir}/mkspecs/modules \
  -DPHONON_QT_PLUGIN_INSTALL_DIR=%{_qt4_plugindir}/designer
popd

%make_build -C %{_target_platform}

mkdir %{_target_platform}-Qt5
pushd %{_target_platform}-Qt5
%{cmake_kf5} .. \
  -DCMAKE_BUILD_TYPE:STRING="Release" \
  -DPHONON_BUILD_DECLARATIVE_PLUGIN:BOOL=%{?declarative:ON}%{!?declarative:OFF} \
  -DPHONON_BUILD_PHONON4QT5:BOOL=ON \
  -DPHONON_QT_IMPORTS_INSTALL_DIR=%{_qt5_importdir} \
  -DPHONON_QT_MKSPECS_INSTALL_DIR=%{_qt5_archdatadir}/mkspecs/modules \
  -DPHONON_QT_PLUGIN_INSTALL_DIR=%{_qt5_plugindir}/designer
popd

%make_build -C %{_target_platform}-Qt5


%install
make install/fast DESTDIR=%{buildroot} -C %{_target_platform}-Qt5
make install/fast DESTDIR=%{buildroot} -C %{_target_platform}

# own these dirs
mkdir -p %{buildroot}%{_kde4_libdir}/kde4/plugins/phonon_backend/
mkdir -p %{buildroot}%{_kde4_datadir}/kde4/services/phononbackends/
mkdir -p %{buildroot}%{_qt5_plugindir}/phonon4qt5_backend

%{find_lang} libphonon --with-qt
%{find_lang} phononsettings --with-qt


%check
export PKG_CONFIG_PATH="%{buildroot}%{_datadir}/pkgconfig:%{buildroot}%{_libdir}/pkgconfig${PKG_CONFIG_PATH:+:}${PKG_CONFIG_PATH}"
test "$(pkg-config --modversion phonon4qt5)" = "%{version}"

%ldconfig_scriptlets qt5

%files qt5 -f libphonon.lang -f phononsettings.lang
%license COPYING.LIB
%{_bindir}/phononsettings
%{_libdir}/libphonon4qt5.so.4*
%{_libdir}/libphonon4qt5experimental.so.4*
%{_qt5_plugindir}/designer/phononwidgets.so

%files qt5-devel
%{_datadir}/phonon4qt5/buildsystem/
%{_libdir}/cmake/phonon4qt5/
%{_includedir}/phonon4qt5/
%{_libdir}/libphonon4qt5.so
%{_libdir}/libphonon4qt5experimental.so
%{_libdir}/pkgconfig/phonon4qt5.pc
%{_qt5_archdatadir}/mkspecs/modules/qt_phonon4qt5.pri


%changelog
* Sat Dec 14 2019 Yaroslav Sidlovsky <zawertun@gmail.com>
- 4.11.1

* Mon Apr 22 2019 Rex Dieter <rdieter@fedoraproject.org> - 4.10.2-3
- rebuild

* Sat Feb 02 2019 Fedora Release Engineering <releng@fedoraproject.org> - 4.10.2-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_30_Mass_Rebuild

* Tue Jan 15 2019 Rex Dieter <rdieter@fedoraproject.org> - 4.10.2-1
- 4.10.2

* Mon Sep 24 2018 Owen Taylor <otaylor@redhat.com> - 4.10.1-3
- Pass Qt paths we'll use in the file list to CMake
- In %%check, augment PKG_CONFIG_PATH, not replace it

* Fri Jul 13 2018 Fedora Release Engineering <releng@fedoraproject.org> - 4.10.1-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_29_Mass_Rebuild

* Fri Apr 27 2018 Rex Dieter <rdieter@fedoraproject.org> - 4.10.1-1
- 4.10.1

* Wed Feb 28 2018 Rex Dieter <rdieter@fedoraproject.org> - 4.10.0-4
- Recommends: phonon-backend-gstreamer
- drop versioned pulseaudio

* Wed Feb 28 2018 Adam Williamson <awilliam@redhat.com> - 4.10.0-3
- Back to a non-bootstrap build

* Wed Feb 28 2018 Adam Williamson <awilliam@redhat.com> - 4.10.0-2
- Bootstrapping build (to fix bogus dependency error in gstreamer backend)

* Fri Feb 23 2018 Rex Dieter <rdieter@fedoraproject.org> - 4.10.0-1
- 4.10.0
- .spec cleanup/cosmetics

* Fri Feb 09 2018 Fedora Release Engineering <releng@fedoraproject.org> - 4.9.1-6
- Rebuilt for https://fedoraproject.org/wiki/Fedora_28_Mass_Rebuild

* Thu Aug 03 2017 Fedora Release Engineering <releng@fedoraproject.org> - 4.9.1-5
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Binutils_Mass_Rebuild

* Thu Jul 27 2017 Fedora Release Engineering <releng@fedoraproject.org> - 4.9.1-4
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Mass_Rebuild

* Mon Feb 20 2017 Rex Dieter <rdieter@fedoraproject.org> - 4.9.1-3
- rebuild (cmake.prov)

* Sat Feb 11 2017 Fedora Release Engineering <releng@fedoraproject.org> - 4.9.1-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_26_Mass_Rebuild

* Wed Jan 25 2017 Rex Dieter <rdieter@fedoraproject.org> - 4.9.1-1
- phonon-4.9.1
- better handle optional (default off) features: declarative, zeitgeist

* Mon Jan 02 2017 Rex Dieter <rdieter@math.unl.edu> - 4.9.0-4
- filter plugin provides

* Thu May 05 2016 Rex Dieter <rdieter@fedoraproject.org> - 4.9.0-3
- drop revert, fix in other components instead (knotifications, knotifyconfig)

* Fri Apr 29 2016 Rex Dieter <rdieter@fedoraproject.org> - 4.9.0-2
- revert upstream commit causing regression (kde#337276)

* Thu Apr 21 2016 Rex Dieter <rdieter@fedoraproject.org> - 4.9.0-1
- phonon-4.9.0, disable qzeitgeist support

* Wed Apr 20 2016 Rex Dieter <rdieter@fedoraproject.org> - 4.8.3-12
- rebuild (qt5)

* Mon Apr 18 2016 Rex Dieter <rdieter@fedoraproject.org> 4.8.3-11
- rebuild (qt)

* Sun Apr 03 2016 Rex Dieter <rdieter@fedoraproject.org> - 4.8.3-10
- FindPhononInternal.cmake: fix _BSD_SOURCE is deprecated warnings
- FindPhononInternal.cmake: do proper includes, fix FTBFS using Qt-5.6

* Thu Feb 04 2016 Fedora Release Engineering <releng@fedoraproject.org> - 4.8.3-9
- Rebuilt for https://fedoraproject.org/wiki/Fedora_24_Mass_Rebuild

* Mon Dec 21 2015 Rex Dieter <rdieter@fedoraproject.org> 4.8.3-8
- move xml dbus interface files to -devel, use %%license

* Tue Sep 15 2015 Rex Dieter <rdieter@fedoraproject.org> 4.8.3-7
- -devel: add lua scriptlet workaround for symlink->dir (#1223956)

* Mon Jul 06 2015 Rex Dieter <rdieter@fedoraproject.org> 4.8.3-6
- backport upstream fixes, mostly for Qt5-fPIC-related FTBFS (#1239789)

* Thu Jun 18 2015 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 4.8.3-5
- Rebuilt for https://fedoraproject.org/wiki/Fedora_23_Mass_Rebuild

* Thu May 07 2015 Rex Dieter <rdieter@fedoraproject.org> 4.8.3-4
- %%build: -DPHONON_INSTALL_QT_COMPAT_HEADERS (instead of our own symlink hack)

* Thu May 07 2015 Rex Dieter <rdieter@fedoraproject.org> 4.8.3-3
- %%build: -DCMAKE_BUILD_TYPE="Release" (sets -DNDEBUG -DQT_NO_DEBUG)

* Sat May 02 2015 Kalev Lember <kalevlember@gmail.com> - 4.8.3-2
- Rebuilt for GCC 5 C++11 ABI change

* Fri Dec 12 2014 Rex Dieter <rdieter@fedoraproject.org> 4.8.3-1
- phonon-4.8.3

* Tue Nov 04 2014 Rex Dieter <rdieter@fedoraproject.org> 4.8.2-1
- phonon-4.8.2

* Sun Oct 19 2014 Rex Dieter <rdieter@fedoraproject.org> 4.8.1-2
- remove qt5-qttools workaround

* Fri Oct 17 2014 Rex Dieter <rdieter@fedoraproject.org> 4.8.1-1
- phonon-4.8.1

* Mon Sep 08 2014 Rex Dieter <rdieter@fedoraproject.org> 4.8.0-2
- 4.8.0

* Sun Aug 17 2014 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 4.7.80-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_21_22_Mass_Rebuild

* Thu Aug 14 2014 Rex Dieter <rdieter@fedoraproject.org> 4.7.80-1
- 4.7.80

* Thu Jul 17 2014 Rex Dieter <rdieter@fedoraproject.org> 4.7.2-1.1
- rebuild (for pulseaudio, bug #1117683)

* Mon Jun 23 2014 Rex Dieter <rdieter@fedoraproject.org> 4.7.2-1
- phonon-4.7.2

* Sat Jun 07 2014 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 4.7.1-4
- Rebuilt for https://fedoraproject.org/wiki/Fedora_21_Mass_Rebuild

* Wed May 28 2014 Rex Dieter <rdieter@fedoraproject.org> 4.7.1-3
- use better upstream fix for rootDir issue

* Wed Mar 26 2014 Rex Dieter <rdieter@fedoraproject.org> 4.7.1-2
- backport some upstream fixes, one that fixes building with cmake-3 particularly

* Fri Dec 06 2013 Rex Dieter <rdieter@fedoraproject.org> 4.7.1-1
- phonon-4.7.1

* Fri Nov 15 2013 Rex Dieter <rdieter@fedoraproject.org> 4.7.0-8
- more upstream fixes, upstreamable rpath_use_link_path handling

* Mon Nov 11 2013 Rex Dieter <rdieter@fedoraproject.org> 4.7.0-7
- workaround rootDir bogosity

* Mon Nov 11 2013 Rex Dieter <rdieter@fedoraproject.org> 4.7.0-6
- Ensure-the-PulseAudio-envrionment-is-set-up (kde#327279)

* Mon Nov 11 2013 Rex Dieter <rdieter@fedoraproject.org> 4.7.0-5
- rebuild (qt5 qreal/arm)

* Sun Nov 10 2013 Kevin Kofler <Kevin@tigcc.ticalc.org> 4.7.0-4
- really fix rpath handling (the upstream version of the patch is incomplete)

* Wed Nov 06 2013 Rex Dieter <rdieter@fedoraproject.org> 4.7.0-3
- disable bootstrap

* Tue Nov 05 2013 Rex Dieter <rdieter@fedoraproject.org> 4.7.0-2
- use upstream version of rpath patch

* Tue Nov 05 2013 Rex Dieter <rdieter@fedoraproject.org> 4.7.0-1
- phonon-4.7.0, Qt5 support

* Wed Oct 30 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.6.0-9
- pull in upstream fixes
- PhononConfig.cmake: fix/workaround regression'y cmake behavior

* Sun Aug 04 2013 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 4.6.0-8
- Rebuilt for https://fedoraproject.org/wiki/Fedora_20_Mass_Rebuild

* Thu Feb 14 2013 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 4.6.0-7
- Rebuilt for https://fedoraproject.org/wiki/Fedora_19_Mass_Rebuild

* Tue Jul 31 2012 Than Ngo <than@redhat.com> - 4.6.0-6
- add rhel/fedora condition

* Fri Jul 20 2012 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 4.6.0-5
- Rebuilt for https://fedoraproject.org/wiki/Fedora_18_Mass_Rebuild

* Sun May 20 2012 Rex Dieter <rdieter@fedoraproject.org> 4.6.0-4
- refresh rpath patch

* Wed Mar 28 2012 Than Ngo <than@redhat.com> - 4.6.0-3
- fix syntax in *.pri file

* Sat Jan 14 2012 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 4.6.0-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_17_Mass_Rebuild

* Tue Dec 20 2011 Rex Dieter <rdieter@fedoraproject.org> 4.6.0-1
- 4.6.0

* Wed Dec 07 2011 Rex Dieter <rdieter@fedoraproject.org> 4.5.57-4.20111031
- fix plugindir usage (#760039)

* Wed Nov 02 2011 Rex Dieter <rdieter@fedoraproject.org> 4.5.57-3.20111031
- fix release

* Mon Oct 31 2011 Rex Dieter <rdieter@fedoraproject.org> 4.5.57-1.20111031
- 20111031 snapshot

* Mon Oct 31 2011 Rex Dieter <rdieter@fedoraproject.org> 4.5.57-2.20110914
- rebuild (qzeitgeist)

* Fri Sep 23 2011 Rex Dieter <rdieter@fedoraproject.org> 4.5.57-1.20110914
- 4.5.57 20110914 snapshot
- pkgconfig-style deps

* Tue May 24 2011 Rex Dieter <rdieter@fedoraproject.org> 4.5.0-3
- BR: libqzeitgeist-devel

* Fri Apr 08 2011 Rex Dieter <rdieter@fedoraproject.org> 4.5.0-2
- avoid Conflicts with judicious (Build)Requires: qt4(-devel) instead

* Fri Mar 25 2011 Rex Dieter <rdieter@fedoraproject.org> 4.5.0-1
- phonon-4.5.0
- qt-designer-plugin-phonon moved here (from qt)

* Wed Feb 09 2011 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 4.4.4-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_15_Mass_Rebuild

* Fri Jan 21 2011 Rex Dieter <rdieter@fedoraproject.org> 4.4.4-2
- re-instate allow-stop-empty-source match from mdv

* Fri Jan 21 2011 Rex Dieter <rdieter@fedoraproject.org> 4.4.4-1
- phonon-4.4.4

* Wed Jan 05 2011 Rex Dieter <rdieter@fedoraproject.org> - 4.4.4-0.1.20110104
- Requires: phonon-backend

* Wed Jan 05 2011 Rex Dieter <rdieter@fedoraproject.org> - 4.4.4-0.0.20110104
- phonon-4.4.4 snapshot (sans backends)
- bootstrap without Requires: phonon-backend

* Tue Nov 30 2010 Rex Dieter <rdieter@fedoraproject.org> - 4.4.3-2
- recognize audio/flac in gstreamer backend (kde#257488)

* Wed Nov 24 2010 Rex Dieter <rdieter@fedoraproject.org> - 4.4.3-1
- phonon-4.4.3

* Mon Nov 22 2010 Rex Dieter <rdieter@fedoraproject.org> - 4.4.3-0.4.20101122
- phonon-4.4.3 20101122 snapshot

* Fri Nov 12 2010 Rex Dieter <rdieter@fedoraproject.org> - 4.4.3-0.3.20101112
- phonon-4.4.3 20101112 snapshot

* Tue Oct 19 2010 Rex Dieter <rdieter@fedoraproject.org> - 4.4.3-0.2.20100909
- Requires: kde-filesystem (#644571)
- own %%{_kde4_libdir}/kde4 (<f15) (#644571)

* Thu Sep 09 2010 Rex Dieter <rdieter@fedoraproject.org> - 4.4.3-0.1.20100909
- phonon-4.4.3 20100909 snapshot

* Tue Jun 08 2010 Rex Dieter <rdieter@fedoraproject.org> - 4.4.2-1
- phonon-4.4.2

* Sat Apr 24 2010 Rex Dieter <rdieter@fedoraproject.org> - 4.4.1-2
- phonon-backend-xine-4.4.1 (with pulseaudio) = no audio (kde#235193)

* Thu Apr 22 2010 Rex Dieter <rdieter@fedoraproject.org> - 4.4.1-1
- phonon-4.4.1

* Thu Apr 01 2010 Rex Dieter <rdieter@fedoraproject.org> - 4.4.0-3
- add minimal pulseaudio runtime dep

* Wed Mar 17 2010 Rex Dieter <rdieter@fedoraproject.org> - 4.4.0-2
- pa glib/qt eventloop patch (kde#228324)

* Tue Mar 16 2010 Rex Dieter <rdieter@fedoraproject.org> - 4.4.0-1
- phonon-4.4.0 final

* Fri Mar 12 2010 Rex Dieter <rdieter@fedoraproject.org> - 4.4.0-0.3
- phonon-4.3.80-pulse-devicemove-rejig.patch (from mdv)

* Wed Feb 24 2010 Rex Dieter <rdieter@fedoraproject.org> - 4.4.0-0.2
- preliminary phonon-4.4.0 tarball

* Fri Jan 22 2010 Rex Dieter <rdieter@fedoraproject.org> - 4.3.80-6
- sync w/mdv patches

* Fri Jan 22 2010 Rex Dieter <rdieter@fedoraproject.org> - 4.3.80-5.2
- F11: patch/modularize pa device-manager bits 

* Fri Jan 22 2010 Kevin Kofler <Kevin@tigcc.ticalc.org> - 4.3.80-5.1
- F11: port the old PA device priorities patch as we don't have PA integration

* Thu Jan 21 2010 Rex Dieter <rdieter@fedoraproject.org> - 4.3.80-5
- no sound with phonon-xine/pulseaudio (kde#223662, rh#553945)

* Thu Jan 21 2010 Rex Dieter <rdieter@fedoraproject.org> - 4.3.80-4
- snarf mdv patches

* Mon Jan 18 2010 Than Ngo <than@redhat.com> - 4.3.80-3
- backport GStreamer backend bugfixes, fix random disappearing sound under KDE

* Thu Dec 03 2009 Rex Dieter <rdieter@fedoraproject.org> - 4.3.80-2
- phonon-4.3.80 (upstream tarball, yes getting ridiculous now)

* Thu Dec 03 2009 Rex Dieter <rdieter@fedoraproject.org> - 4.3.80-1.20091203
- phonon-4.3.80 (20091203 snapshot)

* Thu Dec 03 2009 Rex Dieter <rdieter@fedoraproject.org> - 4.3.50-6.20091203
- phonon-4.3.50 (20091203 snapshot)

* Mon Nov 30 2009 Rex Dieter <rdieter@fedoraproject.org> - 4.3.50-5.20091124
- backend-gstreamer: Requires: gstreamer-plugins-good

* Fri Nov 27 2009 Rex Dieter <rdieter@fedoraproject.org> - 4.3.50-4.20091124
- ln -s ../KDE/Phonon %%_includedir/phonon/Phonon (qt/phonon compat)

* Wed Nov 18 2009 Rex Dieter <rdieter@fedoraproject.org> - 4.3.50-3.20091124
- phonon-4.3.50 (20091124 snapshot)
- enable pulseaudio integration (F-12+)

* Wed Nov 18 2009 Rex Dieter <rdieter@fedoraproject.org> - 4.3.50-2.20091118
- phonon-4.3.50 (20091118 snapshot)

* Mon Oct 19 2009 Rex Dieter <rdieter@fedoraproject.org> - 4.3.50-1.20091019
- phonon-4.3.50 (20091019 snapshot)

* Sat Oct 03 2009 Rex Dieter <rdieter@fedoraproject.org> - 4.3.1-102
- Requires: qt4 >= 4.5.2-21 (f12+)

* Tue Sep 29 2009 Rex Dieter <rdieter@fedoraproject.org> - 4.3.1-101
- revert to kde/phonon
- inflate to Release: 101
- -backend-gstreamer: Epoch: 2

* Sun Jul 26 2009 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 4.3.1-12
- Rebuilt for https://fedoraproject.org/wiki/Fedora_12_Mass_Rebuild

* Thu Jun 18 2009 Rex Dieter <rdieter@fedoraproject.org> - 4.3.1-11
- fix for '#' in filenames

* Tue Jun 09 2009 Than Ngo <than@redhat.com> - 4.3.1-10
- make InitialPreference=9

* Sun Jun 07 2009 Rex Dieter <rdieter@fedoraproject.org> - 4.3.1-9
- optimize scriptlets
- Req: phonon >= %%phonon_version_major

* Fri Jun 05 2009 Kevin Kofler <Kevin@tigcc.ticalc.org> - 4.3.1-8
- restore patches to the xine backend

* Fri Jun 05 2009 Than Ngo <than@redhat.com> - 4.3.1-7
- only xine-backend

* Wed May 20 2009 Rex Dieter <rdieter@fedoraproject.org> - 4.3.1-6
- phonon-backend-gstreamer multilib conflict (#501816)

* Wed May 20 2009 Rex Dieter <rdieter@fedoraproject.org> - 4.3.1-5
- s/ImageMagick/GraphicsMagick/, avail on more secondary archs, is faster,
  yields better results.

* Mon May 04 2009 Rex Dieter <rdieter@fedoraproject.org> - 4.3.1-4
- fix Source0 Url
- xine backend will not play files with non-ascii names (kdebug#172242)

* Sat Apr 11 2009 Rex Dieter <rdieter@fedoraproject.org> - 4.3.1-3
- optimize scriptlets
- Provides/Requires: phonon-backend%%{_isa} ...

* Tue Mar  3 2009 Lukáš Tinkl <ltinkl@redhat.com> - 4.3.1-2
- backport GStreamer backend bugfixes (UTF-8 file handling, volume
fader)

* Fri Feb 27 2009 Than Ngo <than@redhat.com> - 4.3.1-1
- 4.3.1

* Thu Feb 26 2009 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 4.3.0-6
- Rebuilt for https://fedoraproject.org/wiki/Fedora_11_Mass_Rebuild

* Sun Feb 01 2009 Rex Dieter <rdieter@fedoraproject.org> - 4.3.0-5
- put icons in the right subpkg

* Thu Jan 29 2009 Rex Dieter <rdieter@fedoraproject.org> - 4.3.0-4
- Requires: phonon-backend >= %%version
- move icons to hicolor theme and into -backend subpkgs
- BR: libxcb-devel
- move phonon-gstreamer.svg to sources

* Mon Jan 26 2009 Rex Dieter <rdieter@fedoraproject.org> - 4.3.0-3
- BR: automoc4 >= 0.9.86

* Fri Jan 23 2009 Kevin Kofler <Kevin@tigcc.ticalc.org> - 4.3.0-2
- fix typo in postun scriptlet (introduced in 4.2.96-3)

* Thu Jan 22 2009 Than Ngo <than@redhat.com> - 4.3.0-1
- 4.3.0

* Thu Jan 08 2009 Rex Dieter <rdieter@fedoraproject.org> - 4.2.96-3
- new tarball
- put icons/scriptlets into main pkg
- Requires: phonon-backend

* Thu Jan 08 2009 Lorenzo Villani <lvillani@binaryhelix.net> - 4.2.96-2
- add gstreaer-logo.svg

* Thu Jan 08 2009 Lorenzo Villani <lvillani@binaryhelix.net> - 4.2.96-1
- 4.2.96
- rebase phonon-4.2.0-pulseaudio.patch (-> phonon-4.2.96-pulseaudio.patch)
- rebase phonon-4.2.70-xine-pulseaudio.patch 
  (-> phonon-4.2.96-xine-pulseaudio.patch)

* Fri Dec 12 2008 Rex Dieter <rdieter@fedoraproject.org> 4.2.80-3
- rebuild for pkgconfig deps

* Tue Nov 25 2008 Kevin Kofler <Kevin@tigcc.ticalc.org> 4.2.80-2
- phonon-backend-xine: don't Obsolete/Provide itself, Provides: phonon-backend

* Tue Nov 25 2008 Than Ngo <than@redhat.com> 4.2.80-1
- 4.2.80

* Fri Nov 21 2008 Lorenzo Villani <lvillani@binaryhelix.net> - 4.2.80-0.1.20081121svn887051
- Use subversion (4.2.80) snapshot
- phonon-backend-xine subpkg
- make VERBOSE=1
- make install/fast
- Xine backend is in phonon now, add xine-lib-devel as BR
- BR cmake >= 2.6.0
- forward-port xine pulseaudio patch

* Tue Sep 30 2008 Than Ngo <than@redhat.com> 4.2.0-7
- fix tranparent issue by convert

* Tue Sep 30 2008 Than Ngo <than@redhat.com> 4.2.0-6
- add missing icon

* Wed Sep 17 2008 Rex Dieter <rdieter@fedoraproject.org> 4.2.0-5
- Requires: phonon-backend-xine

* Sun Aug 10 2008 Kevin Kofler <Kevin@tigcc.ticalc.org> 4.2.0-4
- rename -backend-gst back to -backend-gstreamer (longer name as -backend-xine)
  The GStreamer backend isn't ready to be the default, and KDE 4.1 also defaults
  to the Xine backend when both are installed.
- fix PulseAudio not being the default in the Xine backend (4.2 regression)

* Sat Aug 02 2008 Rex Dieter <rdieter@fedoraproject.org> 4.2.0-3
- -backend-gst: Obsoletes: -backend-gstreamer < 4.2.0-2

* Thu Jul 24 2008 Rex Dieter <rdieter@fedoraproject.org> 4.2.0-2
- rename -backend-gstreamer -> backend-gst

* Wed Jul 23 2008 Rex Dieter <rdieter@fedoraproject.org> 4.2.0-1
- phonon-4.2.0

* Mon Jul 14 2008 Rex Dieter <rdieter@fedoraproject.org> 4.2-0.4.beta2
- BR: automoc4
- -backend-gstreamer subpkg

* Tue Jul 01 2008 Kevin Kofler <Kevin@tigcc.ticalc.org> 4.2-0.3.beta2
- drop automoc libsuffix patch, no longer needed

* Fri Jun 20 2008 Rex Dieter <rdieter@fedoraproject.org> 4.2-0.2.beta2
- phonon 4.2beta2 (aka 4.1.83)

* Sat Jun 14 2008 Rex Dieter <rdieter@fedoraproject.org> 4.2-0.1.20080614svn820634
- first try

