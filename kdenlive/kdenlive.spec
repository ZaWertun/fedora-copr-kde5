
Name:    kdenlive
Summary: Non-linear video editor
Version: 18.12.3
Release: 3%{?dist}

License: GPLv2+
URL:     http://www.kdenlive.org
%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0: http://download.kde.org/%{stable}/applications/%{version}/src/kdenlive-%{version}.tar.xz
Source100: kdenlive-find-lang.sh

## backport from master branch
Patch100: kdenlive-18.12.3-mlt_melt.patch

# Add support for finding html files with find-lang.sh --with-html on epel
# https://github.com/rpm-software-management/rpm/commit/0c42871ff407a3eeb1e8b8c1de9395f35659c987
%if 0%{?rhel}
# copied from openshot
# Redirect find_lang to our patched version
%global find_lang %{_sourcedir}/kdenlive-find-lang.sh %{buildroot}
%endif

BuildRequires: desktop-file-utils
BuildRequires: extra-cmake-modules
BuildRequires: gettext
BuildRequires: cmake(KF5Archive)
BuildRequires: cmake(KF5Bookmarks)
BuildRequires: cmake(KF5Config)
BuildRequires: cmake(KF5ConfigWidgets)
BuildRequires: cmake(KF5CoreAddons)
BuildRequires: cmake(KF5DocTools)
BuildRequires: cmake(KF5DBusAddons)
BuildRequires: cmake(KF5GuiAddons)
BuildRequires: cmake(KF5I18n)
BuildRequires: cmake(KF5IconThemes)
BuildRequires: cmake(KF5ItemViews)
BuildRequires: cmake(KF5KIO)
BuildRequires: cmake(KF5JobWidgets)
BuildRequires: cmake(KF5NewStuff)
BuildRequires: cmake(KF5Notifications)
BuildRequires: cmake(KF5NotifyConfig)
BuildRequires: cmake(KF5Plotting)
BuildRequires: cmake(KF5Purpose)
BuildRequires: cmake(KF5TextWidgets)
BuildRequires: cmake(KF5XmlGui)
BuildRequires: cmake(KF5Crash)
BuildRequires: cmake(KF5FileMetaData)
BuildRequires: libappstream-glib

BuildRequires: pkgconfig(libv4l2)
BuildRequires: pkgconfig(mlt++) >= 6.6.0
%global mlt_version %(pkg-config --modversion mlt++ 2>/dev/null || echo 6.6.0)

BuildRequires: pkgconfig(Qt5Concurrent)
BuildRequires: pkgconfig(Qt5DBus)
BuildRequires: pkgconfig(Qt5OpenGL)
BuildRequires: pkgconfig(Qt5Script)
BuildRequires: pkgconfig(Qt5Svg)
BuildRequires: pkgconfig(Qt5Qml)
BuildRequires: pkgconfig(Qt5Quick)
BuildRequires: pkgconfig(Qt5Widgets)
BuildRequires: pkgconfig(Qt5WebKitWidgets)

## workaround for missing dependency in kf5-kio, can remove
## once kf5-kio-5.24.0-2 (or newer is available)
BuildRequires: kf5-kinit-devel
%{?kf5_kinit_requires}
Requires: dvdauthor
Requires: dvgrab
Requires: ffmpeg
%if 0%{?fedora} >= 25 || 0%{?rhel} >= 7
Requires: mlt-freeworld%{?_isa} >= %{mlt_version}
%else
Requires: mlt%{?_isa} >= %{mlt_version}
%endif
Requires: qt5-qtquickcontrols

%description
Kdenlive is an intuitive and powerful multi-track video editor, including most
recent video technologies.


%prep
%autosetup -p1


%build
mkdir %{_target_platform}
pushd %{_target_platform}
%{cmake_kf5} .. \
  -DKDE_INSTALL_USE_QT_SYS_PATHS:BOOL=ON
popd

%make_build -C %{_target_platform}


%install
make install/fast DESTDIR=%{buildroot} -C %{_target_platform}

## unpackaged files
rm -rfv  %{buildroot}%{_datadir}/doc/Kdenlive/

%find_lang %{name} --with-html --all-name


%check
appstream-util validate-relax --nonet %{buildroot}%{_kf5_metainfodir}/org.kde.%{name}.appdata.xml ||:
desktop-file-validate %{buildroot}%{_kf5_datadir}/applications/org.kde.%{name}.desktop


%if 0%{?rhel} && 0%{?rhel} < 8
%post
/usr/bin/update-desktop-database &> /dev/null || :
/bin/touch --no-create %{_kf5_datadir}/icons/hicolor &> /dev/null || :
/bin/touch --no-create %{_kf5_datadir}/mime/packages &> /dev/null || :

%postun
if [ $1 -eq 0 ] ; then
  /bin/touch --no-create %{_kf5_datadir}/icons/hicolor &>/dev/null
  /usr/bin/gtk-update-icon-cache %{_kf5_datadir}/icons/hicolor &>/dev/null || :
  /bin/touch --no-create %{_kf5_datadir}/mime/packages &> /dev/null || :
  /usr/bin/update-mime-database %{_kf5_datadir}/mime &> /dev/null || :
  /usr/bin/update-desktop-database &> /dev/null || :
fi

%posttrans
/usr/bin/gtk-update-icon-cache %{_kf5_datadir}/icons/hicolor &> /dev/null || :
/usr/bin/update-mime-database %{?fedora:-n} %{_kf5_datadir}/mime &> /dev/null || :
%endif

%files -f %{name}.lang
%doc AUTHORS README
%license COPYING
%{_kf5_bindir}/kdenlive_render
%{_kf5_bindir}/%{name}
%{_kf5_datadir}/applications/org.kde.%{name}.desktop
%{_kf5_metainfodir}/org.kde.%{name}.appdata.xml
%{_kf5_datadir}/kdenlive/
%{_kf5_datadir}/mime/packages/org.kde.kdenlive.xml
%{_kf5_datadir}/mime/packages/westley.xml
%{_kf5_datadir}/icons/hicolor/*/*/*
%{_kf5_datadir}/config.kcfg/kdenlivesettings.kcfg
%{_kf5_datadir}/knotifications5/kdenlive.notifyrc
%{_kf5_datadir}/kservices5/mltpreview.desktop
%{_kf5_datadir}/kxmlgui5/kdenlive/
%{_kf5_sysconfdir}/xdg/kdenlive_renderprofiles.knsrc
%{_kf5_sysconfdir}/xdg/kdenlive_titles.knsrc
%{_kf5_sysconfdir}/xdg/kdenlive_wipes.knsrc
%{_kf5_sysconfdir}/xdg/kdenlive.categories
%{_kf5_mandir}/man1/kdenlive.1*
%{_kf5_mandir}/man1/kdenlive_render.1*
# consider subpkg for multilib
%{_kf5_qtplugindir}/mltpreview.so


%changelog
* Fri Mar 29 2019 Rex Dieter <rdieter@fedoraproject.org> - 18.12.3-3
- respin melt.patch for real this time

* Fri Mar 29 2019 Rex Dieter <rdieter@fedoraproject.org> - 18.12.3-2
- respin melt patch to give mlt-melt priority over melt

* Thu Mar 28 2019 Rex Dieter <rdieter@fedoraproject.org> - 18.12.3-1
- 18.12.3
- backport upstream fix for mlt-melt (kde#405564)

* Mon Mar 04 2019 RPM Fusion Release Engineering <leigh123linux@gmail.com> - 18.12.2-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_30_Mass_Rebuild

* Thu Feb 28 2019 Rex Dieter <rdieter@fedoraproject.org> - 18.12.2-1
- 18.12.2

* Tue Oct 23 2018 Leigh Scott <leigh123linux@googlemail.com> - 18.08.2-1
- Update kdenlive to 18.08.2

* Mon Sep 17 2018 Sérgio Basto <sergio@serjux.com> - 18.08.1-1
- Update kdenlive to 18.08.1

* Sun Aug 19 2018 Leigh Scott <leigh123linux@googlemail.com> - 18.04.3-2
- Rebuilt for Fedora 29 Mass Rebuild binutils issue

* Fri Jul 27 2018 Sérgio Basto <sergio@serjux.com> - 18.04.3-1
- Update kdenlive to 18.04.3

* Thu Jul 26 2018 RPM Fusion Release Engineering <leigh123linux@gmail.com> - 18.04.1-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_29_Mass_Rebuild

* Sun May 13 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.04.1-1
- 18.04.1
- .spec cosmetics
- omit scriptlets on fedora (all handled via system triggers now)

* Sun May 13 2018 Rex Dieter <rdieter@fedoraproject.org> - 17.12.3-4
- rebuild (mlt)

* Wed Mar 21 2018 Sérgio Basto <sergio@serjux.com> - 17.12.3-3
- recordmydesktop is not used since 0.9.something

* Mon Mar 19 2018 Sérgio Basto <sergio@serjux.com> - 17.12.3-2
- Add a fix to build in el7

* Sun Mar 11 2018 Sérgio Basto <sergio@serjux.com> - 17.12.3-1
- Update kdenlive to 17.12.3

* Thu Feb 22 2018 Sérgio Basto <sergio@serjux.com> - 17.12.2-1
- Update kdenlive to 17.12.2

* Tue Jan 30 2018 Sérgio Basto <sergio@serjux.com> - 17.12.1-1
- Update kdenlive to 17.12.1

* Tue Jan 02 2018 Sérgio Basto <sergio@serjux.com> - 17.12.0-2
- Use _kf5_metainfodir to fix appdata directory issue

* Fri Dec 29 2017 Sérgio Basto <sergio@serjux.com> - 17.12.0-1
- Update kdenlive to 17.12.0

* Thu Nov 02 2017 Sérgio Basto <sergio@serjux.com> - 17.08.2-1
- Update kdenlive to 17.08.2

* Sun Oct 08 2017 Sérgio Basto <sergio@serjux.com> - 17.08.1-1
- Update to 17.08.1

* Thu Aug 31 2017 RPM Fusion Release Engineering <kwizart@rpmfusion.org> - 17.04.2-4
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Mass_Rebuild

* Fri Jun 16 2017 Sérgio Basto <sergio@serjux.com> - 17.04.2-3
- yet another fix for gcc7

* Sun Jun 11 2017 Sérgio Basto <sergio@serjux.com> - 17.04.2-2
- Add a new gcc7.patch
- Add find_lang macro

* Sun Jun 11 2017 Sérgio Basto <sergio@serjux.com> - 17.04.2-1
- Update to 17.04.2

* Sat Apr 01 2017 Sérgio Basto <sergio@serjux.com> - 16.12.3-1
- Update to 16.12.3

* Sun Mar 19 2017 RPM Fusion Release Engineering <kwizart@rpmfusion.org> - 16.12.2-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_26_Mass_Rebuild

* Wed Feb 22 2017 Leigh Scott <leigh123linux@googlemail.com> - 16.12.2-2
- Add build fix for gcc-7 changes

* Tue Feb 21 2017 Sérgio Basto <sergio@serjux.com> - 16.12.2-1
- Update kdenlive to 16.12.2 following Fedora KDE applications

* Tue Jan 31 2017 Sérgio Basto <sergio@serjux.com> - 16.08.3-2
- Requires mlt-freeworld on el7

* Tue Jan 31 2017 Sérgio Basto <sergio@serjux.com> - 16.08.3-1
- Update kdenlive to 16.08.3 like kde-baseapps

* Fri Jan 06 2017 Leigh Scott <leigh123linux@googlemail.com> - 16.12.0-1
- Update to 16.12.0

* Thu Oct 13 2016 Sérgio Basto <sergio@serjux.com> - 16.08.2-1
- Update to 16.08.2
- Scriplets fixed by upstream

* Fri Sep 16 2016 Sérgio Basto <sergio@serjux.com> - 16.08.1-1
- Update to 16.08.1
- Requires mlt-freeworld on F25+

* Tue Aug 23 2016 Sérgio Basto <sergio@serjux.com> - 16.08.0-1
- Kdenlive 16.08.0 is here

* Thu Jul 28 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.04.3-1
- 16.04.3, add missing 'touch' to %%postun, document kinit workaround

* Fri Jul 08 2016 Leigh Scott <leigh123linux@googlemail.com> - 16.04.2-2
- Use marcos for files
- Fix scriptlets

* Wed Jun 15 2016 Sérgio Basto <sergio@serjux.com> - 16.04.2-1
- Update kdenlive to 16.04.2

* Fri May 27 2016 David Vásquez <davidjeremias82 AT gmail DOT com> 16.04.1-2
- Added missing dependencies
- Disabled, build testing
- Automatic qt system path

* Fri May 13 2016 Sérgio Basto <sergio@serjux.com> - 16.04.1-1
- Update to 16.04.1

* Thu Mar 24 2016 Sérgio Basto <sergio@serjux.com> - 15.12.2-2
- Fix rfbz #4015

* Wed Feb 17 2016 Rex Dieter <rdieter@fedoraproject.org> 15.12.2-1
- 15.12.2

* Mon Nov 09 2015 Rex Dieter <rdieter@fedoraproject.org> 15.08.2-1
- 15.08.2

* Mon Dec 22 2014 Rex Dieter <rdieter@fedoraproject.org> 0.9.10-1
- 0.9.10

* Wed Aug 06 2014 Rex Dieter <rdieter@fedoraproject.org> 0.9.8-2
- optimize mime scriptlets

* Wed May 14 2014 Rex Dieter <rdieter@fedoraproject.org> 0.9.8-1
- 0.9.8

* Mon Apr 07 2014 Nicolas Chauvet <kwizart@gmail.com> - 0.9.6-4
- Rebuilt for rfbz#3209

* Wed Oct 09 2013 Nicolas Chauvet <kwizart@gmail.com> - 0.9.6-3
- rebuilt for mlt

* Sun May 26 2013 Nicolas Chauvet <kwizart@gmail.com> - 0.9.6-2
- Rebuilt for x264/FFmpeg

* Sun Apr 07 2013 Rex Dieter <rdieter@fedoraproject.org> 0.9.6-1
- 0.9.6

* Sun Mar 03 2013 Nicolas Chauvet <kwizart@gmail.com> - 0.9.4-2
- Mass rebuilt for Fedora 19 Features

* Tue Jan 29 2013 Rex Dieter <rdieter@fedoraproject.org> 0.9.4-1
- 0.9.4

* Tue Jun 19 2012 Richard Shaw <hobbes1069@gmail.com> - 0.9.2-2
- Rebuild for updated mlt.

* Thu May 31 2012 Rex Dieter <rdieter@fedoraproject.org> 0.9.2-1
- 0.9.2

* Tue May 15 2012 Rex Dieter <rdieter@fedoraproject.org> 0.9-1
- 0.9
- pkgconfig-style deps

* Fri Mar 02 2012 Nicolas Chauvet <kwizart@gmail.com> - 0.8.2.1-3
- Rebuilt for c++ ABI breakage

* Wed Feb 08 2012 Nicolas Chauvet <kwizart@gmail.com> - 0.8.2.1-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_17_Mass_Rebuild

* Sat Dec 10 2011 Rex Dieter <rdieter@fedoraproject.org> 0.8.2.1-1
- 0.8.2.1

* Tue Nov 15 2011 Rex Dieter <rdieter@fedoraproject.org> 0.8.2-2
- rebuild

* Fri Nov 11 2011 Rex Dieter <rdieter@fedoraproject.org> 0.8.2-1
- 0.8.2
- tighten mlt deps

* Thu Jul 21 2011 Ryan Rix <ry@n.rix.si> 0.8-1
- New version
- Add patch to fix FTBFS

* Fri Apr 15 2011 Rex Dieter <rdieter@fedoraproject.org> 0.7.8-2
- update scriptlets, %%_kde4_... macros/best-practices
- +Requires: kdebase-runtime (versioned)
- fix ftbfs

* Thu Apr 07 2011 Ryan Rix <ry@n.rix.si> - 0.7.8-1
- new version

* Mon Mar 01 2010 Zarko <zarko.pintar@gmail.com> - 0.7.7.1-1
- new version

* Thu Feb 18 2010 Zarko <zarko.pintar@gmail.com> - 0.7.7-1
- new version

* Mon Sep 07 2009 Zarko <zarko.pintar@gmail.com> - 0.7.5-1
- new version

* Sat May 30 2009 Zarko <zarko.pintar@gmail.com> - 0.7.4-2
- added updating of mime database
- changed dir of .desktop file

* Fri May 22 2009 Zarko <zarko.pintar@gmail.com> - 0.7.4-1
- new release
- spec cleaning

* Thu Apr 16 2009 Zarko <zarko.pintar@gmail.com> - 0.7.3-2
- some clearing
- added doc files

* Wed Apr 15 2009 Zarko <zarko.pintar@gmail.com> - 0.7.3-1
- new release

* Sun Apr 12 2009 Zarko <zarko.pintar@gmail.com> - 0.7.2.1-2
- spec convert to kde4 macros

* Mon Mar 16 2009 Zarko <zarko.pintar@gmail.com> - 0.7.2.1-1
- update to 0.7.2.1
- spec cleaned
- Resolve RPATHs

* Sun Nov 16 2008 Arkady L. Shane <ashejn@yandex-team.ru> - 0.7-1
- update to 0.7

* Wed Nov  5 2008 Arkady L. Shane <ashejn@yandex-team.ru> - 0.7-0.1.20081104svn2622
- update to last svn revision

* Tue Nov  4 2008 Arkady L. Shane <ashejn@yandex-team.ru> - 0.7-0.beta1
- clean up spec

* Fri Oct 17 2008 jeff <moe@blagblagblag.org> - 0.7-1.beta1
- Add URL
- Full URL for Source:
- Remove all Requires:
- Update BuildRoot
- Remove Packager: Brixton Linux Action Group
- Add BuildRequires: ffmpeg-devel kdebindings-devel soprano-devel
- Update %%files
- %%doc with only effects/README
- GPLv2+
- Add lang files

* Tue Jul  8 2008 jeff <moe@blagblagblag.org> - 0.6-1.svn2298.0blag.f9
- Update to KDE4 branch
  https://kdenlive.svn.sourceforge.net/svnroot/kdenlive/branches/KDE4

* Tue Jul  8 2008 jeff <moe@blagblagblag.org> - 0.6-1.svn2298.0blag.f9
- Update to svn r2298
- New Requires
- kdenlive-svn-r2298-renderer-CMakeLists.patch 

* Sun Nov 11 2007 jeff <moe@blagblagblag.org> - 0.5-1blag.f7
- Update to 0.5 final

* Tue Apr 17 2007 jeff <moe@blagblagblag.org> - 0.5-0svn20070417.0blag.fc6
- svn to 20070417

* Fri Apr  6 2007 jeff <moe@blagblagblag.org> - 0.5-0svn20070406.0blag.fc6
- svn to 20070406

* Tue Apr  3 2007 jeff <moe@blagblagblag.org> - 0.5-0svn20070403.0blag.fc6
- svn to 20070403

* Thu Mar 22 2007 jeff <moe@blagblagblag.org> - 0.5-0svn20070322.0blag.fc6
- svn to 20070322

* Thu Mar 15 2007 jeff <moe@blagblagblag.org> - 0.5-0svn20070316.0blag.fc6
- BLAG'd

* Sun Apr 27 2003 Jason Wood <jasonwood@blueyonder.co.uk> 0.2.2-1mdk
- First stab at an RPM package.
- This is taken from kdenlive-0.2.2 source package.
