
## uncomment to enable bootstrap mode
#global bootstrap 1

%if !0%{?bootstrap}
%if 0%{?fedora}
%global chm 1
%global ebook 1
%endif
# uncomment to include -mobile (currently doesn't work)
# it links libokularpart.so, but fails to file/load at runtime
#global mobile 1
%endif

Name:    okular 
Summary: A document viewer
Version: 19.12.3
Release: 2%{?dist}

License: GPLv2
URL:     https://www.kde.org/applications/graphics/okular/

%global majmin_ver %(echo %{version} | cut -d. -f1,2)
%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0: http://download.kde.org/%{stable}/applications/%{version}/src/%{name}-%{version}.tar.xz

## upstream patches
Patch10: okular-CVE-2020-9359.patch

BuildRequires: desktop-file-utils
BuildRequires: libappstream-glib

BuildRequires: extra-cmake-modules
BuildRequires: kf5-rpm-macros
BuildRequires: cmake(KDEExperimentalPurpose)
BuildRequires: cmake(KF5Activities)
BuildRequires: cmake(KF5Archive)
BuildRequires: cmake(KF5Bookmarks)
BuildRequires: cmake(KF5Completion)
BuildRequires: cmake(KF5Config)
BuildRequires: cmake(KF5ConfigWidgets)
BuildRequires: cmake(KF5CoreAddons)
BuildRequires: cmake(KF5Crash)
BuildRequires: cmake(KF5DBusAddons)
BuildRequires: cmake(KF5DocTools)
BuildRequires: cmake(KF5IconThemes)
BuildRequires: cmake(KF5JS)
BuildRequires: cmake(KF5KIO)
BuildRequires: cmake(KF5Kirigami2)
BuildRequires: cmake(KF5Parts)
BuildRequires: cmake(KF5Pty)
BuildRequires: cmake(KF5ThreadWeaver)
BuildRequires: cmake(KF5Wallet)
BuildRequires: cmake(KF5KHtml)
BuildRequires: cmake(KF5WindowSystem)

BuildRequires: cmake(Qt5DBus)
BuildRequires: cmake(Qt5Test)
BuildRequires: cmake(Qt5Widgets)
BuildRequires: cmake(Qt5PrintSupport)
BuildRequires: cmake(Qt5Svg)
BuildRequires: cmake(Qt5Qml)
BuildRequires: cmake(Qt5Quick)

# okular-mobile
#BuildRequires: kf5-purpose-devel
#Requires: kf5-purpose%{?_isa}

BuildRequires: pkgconfig(phonon4qt5)
BuildRequires: cmake(Qca-qt5)

## generater/plugin deps
BuildRequires: cmake(KF5KExiv2)
BuildRequires: kdegraphics-mobipocket-devel
%if 0%{?chm}
BuildRequires: chmlib-devel
BuildRequires: pkgconfig(libzip)
%endif
%if 0%{?ebook}
BuildRequires: ebook-tools-devel
%endif
BuildRequires: libjpeg-devel
BuildRequires: libtiff-devel
BuildRequires: pkgconfig(freetype2)
BuildRequires: pkgconfig(libmarkdown)
BuildRequires: pkgconfig(libspectre)
BuildRequires: pkgconfig(poppler-qt5)
%if 0%{?fedora}
BuildRequires: pkgconfig(ddjvuapi) 
BuildRequires: pkgconfig(qca2)
BuildRequires: pkgconfig(zlib)
%endif

%if !0%{?bootstrap}
BuildRequires:  cmake(Qt5TextToSpeech)
%endif

Requires: %{name}-part%{?_isa} = %{version}-%{release}

%description
%{summary}.

%if 0%{?mobile}
%package mobile
Summary: Document viewer for plasma mobile
# included last in okular-15.12.3-1.fc23
Obsoletes: okular-active < 16.04
Requires: %{name}-part%{?_isa} = %{version}-%{release}
%description mobile
%{summary}.
%endif

%package devel
Summary:  Development files for %{name}
Provides: okular5-devel = %{version}-%{release}
Requires: %{name}-libs%{?_isa} = %{version}-%{release}
%description devel
%{summary}.

%package  libs 
Summary:  Runtime files for %{name} 
%if 0%{?fedora}
# use Recommends to avoid hard deps -- rex
## lpr
Recommends: cups-client
## ps2pdf,pdf2ps
Recommends: ghostscript-core
%endif
%description libs 
%{summary}.

%package part
Summary: Okular kpart plugin
Provides: okular5-part = %{version}-%{release}
Provides: okular5-part%{?_isa} = %{version}-%{release}
Requires: %{name}-libs%{?_isa} = %{version}-%{release}
# translations moved here
Conflicts: kde-l10n < 17.03
%description part
%{summary}.


%prep
%autosetup -p1

%if ! 0%{?mobile}
# disable/omit mobile, it doesn't work -- rex
sed -i -e 's|^add_subdirectory( mobile )|#add_subdirectory( mobile )|' CMakeLists.txt
%endif


%build
mkdir %{_target_platform}
pushd %{_target_platform}
%{cmake_kf5} .. \
  -DBUILD_OKULARKIRIGAMI:BOOL=OFF
popd

%make_build -C %{_target_platform}


%install
make install/fast DESTDIR=%{buildroot} -C %{_target_platform}

%find_lang all --all-name --with-html --with-man
grep -v \
  -e %{_mandir} \
  -e %{_kf5_docdir} \
  all.lang > okular-part.lang
cat all.lang okular-part.lang | sort | uniq -u > okular.lang

# unpackaged files
%if ! 0%{?mobile}
rm -fv %{buildroot}%{_kf5_datadir}/applications/org.kde.mobile.okular_*.desktop
%endif


%check
desktop-file-validate %{buildroot}%{_kf5_datadir}/applications/org.kde.okular.desktop
appstream-util validate-relax --nonet %{buildroot}%{_kf5_metainfodir}/org.kde.okular.appdata.xml
%if 0%{?mobile}
desktop-file-validate %{buildroot}%{_kf5_datadir}/applications/org.kde.mobile.okular.desktop
%endif

%files -f okular.lang
%license COPYING
%{_kf5_bindir}/okular
#{_kf5_sysconfdir}/xdg/%{name}*
%{_kf5_datadir}/applications/org.kde.okular.desktop
%{_kf5_metainfodir}/org.kde.okular.appdata.xml
%{_kf5_datadir}/applications/okularApplication_*.desktop
%{_kf5_metainfodir}/org.kde.okular-*.metainfo.xml
%{_kf5_datadir}/okular/
%{_kf5_datadir}/icons/hicolor/*/*/*
%{_kf5_datadir}/kconf_update/okular.upd
%{_kf5_datadir}/qlogging-categories5/%{name}*
%{_mandir}/man1/okular.1*

%if 0%{?mobile}
%files mobile
%{_qt5_qmldir}/org/kde/okular/
%{_kf5_datadir}/applications/org.kde.mobile.okular.desktop
%{_kf5_datadir}/applications/org.kde.mobile.okular_*.desktop
%{_kf5_datadir}/kpackage/genericqml/org.kde.mobile.okular/
%endif

%files devel
%{_includedir}/okular/
%{_libdir}/libOkular5Core.so
%{_libdir}/cmake/Okular5/

%ldconfig_scriptlets libs

%files libs
%{_libdir}/libOkular5Core.so.9*

%files part -f okular-part.lang
%if 0%{?fedora}
%{_qt5_plugindir}/kio_msits.so
%{_kf5_datadir}/kservices5/ms-its.protocol
%endif
%{_kf5_datadir}/config.kcfg/*.kcfg
%{_kf5_datadir}/kservices5/okular[A-Z]*.desktop
%{_kf5_datadir}/kservices5/okular_part.desktop
%{_kf5_datadir}/kservicetypes5/okularGenerator.desktop
%{_kf5_datadir}/kxmlgui5/okular/
%dir %{_qt5_plugindir}/okular/
%dir %{_qt5_plugindir}/okular/generators/
%{_qt5_plugindir}/okular/generators/okularGenerator_*.so
%{_qt5_plugindir}/okularpart.so


%changelog
* Sun Mar 22 2020 Rex Dieter <rdieter@fedoraproject.org> - 19.12.3-2
- Security fix for CVE-2020-9359 (#1815651,1815652)

* Fri Mar 06 2020 Rex Dieter <rdieter@fedoraproject.org> - 19.12.3-1
- 19.12.3

* Tue Feb 04 2020 Rex Dieter <rdieter@fedoraproject.org> - 19.12.2-1
- 19.12.2

* Thu Jan 30 2020 Rex Dieter <rdieter@fedoraproject.org> - 19.12.1-1
- 19.12.1

* Wed Jan 29 2020 Fedora Release Engineering <releng@fedoraproject.org> - 19.08.3-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_32_Mass_Rebuild

* Fri Jan 17 2020 Marek Kasik <mkasik@redhat.com> - 19.08.3-2
- Rebuild for poppler-0.84.0

* Tue Nov 12 2019 Rex Dieter <rdieter@fedoraproject.org> - 19.08.3-1
- 19.08.3

* Thu Oct 17 2019 Rex Dieter <rdieter@fedoraproject.org> - 19.08.2-1
- 19.08.2

* Mon Sep 30 2019 Rex Dieter <rdieter@fedoraproject.org> - 19.08.1-1
- 19.08.1

* Thu Jul 25 2019 Fedora Release Engineering <releng@fedoraproject.org> - 19.04.3-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_31_Mass_Rebuild

* Fri Jul 12 2019 Rex Dieter <rdieter@fedoraproject.org> - 19.04.3-1
- 19.04.3

* Tue Jun 04 2019 Rex Dieter <rdieter@fedoraproject.org> - 19.04.2-1
- 19.04.2

* Fri Mar 08 2019 Rex Dieter <rdieter@fedoraproject.org> - 18.12.3-1
- 18.12.3

* Tue Feb 05 2019 Rex Dieter <rdieter@fedoraproject.org> - 18.12.2-1
- 18.12.2

* Fri Feb 01 2019 Fedora Release Engineering <releng@fedoraproject.org> - 18.12.1-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_30_Mass_Rebuild

* Tue Jan 08 2019 Rex Dieter <rdieter@fedoraproject.org> - 18.12.1-1
- 18.12.1

* Sun Dec 16 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.12.0-1
- 18.12.0

* Tue Nov 06 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.08.3-1
- 18.08.3

* Wed Oct 10 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.08.2-1
- 18.08.2

* Fri Sep 07 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.08.1-1
- 18.08.1
- -DBUILD_OKULARKIRIGAMI:BOOL=OFF for now

* Fri Aug 10 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.04.3-1
- 18.04.3

* Fri Jul 13 2018 Fedora Release Engineering <releng@fedoraproject.org> - 18.03.90-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_29_Mass_Rebuild

* Sun Apr 08 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.03.90-1
- 18.03.90, support kcrash
- enable discount/libmarkdown support

* Fri Mar 23 2018 Marek Kasik <mkasik@redhat.com> - 17.12.3-2
- Rebuild for poppler-0.63.0

* Tue Mar 06 2018 Rex Dieter <rdieter@fedoraproject.org> - 17.12.3-1
- 17.12.3

* Thu Feb 08 2018 Rex Dieter <rdieter@fedoraproject.org> - 17.12.2-1
- 17.12.2

* Thu Feb 08 2018 Fedora Release Engineering <releng@fedoraproject.org> - 17.12.1-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_28_Mass_Rebuild

* Thu Jan 18 2018 Igor Gnatenko <ignatenkobrain@fedoraproject.org> - 17.12.1-2
- Remove obsolete scriptlets

* Thu Jan 11 2018 Rex Dieter <rdieter@fedoraproject.org> - 17.12.1-1
- 17.12.1

* Thu Dec 28 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.12.0-1
- 17.12.0

* Tue Nov 21 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.08.3-3
- BR: Qt5TextToSpeech, support %%bootstrap

* Wed Nov 08 2017 David Tardon <dtardon@redhat.com> - 17.08.3-2
- rebuild for poppler 0.61.0

* Wed Nov 08 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.08.3-1
- 17.08.3

* Mon Nov 06 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.08.2-2
- rebuild (ecm)

* Wed Oct 11 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.08.2-1
- 17.08.2

* Fri Oct 06 2017 David Tardon <dtardon@redhat.com> - 17.08.1-2
- rebuild for poppler 0.60.1

* Thu Sep 28 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.08.1-1
- 17.08.1

* Fri Sep 08 2017 David Tardon <dtardon@redhat.com> - 17.04.3-2
- rebuild for poppler 0.59.0

* Thu Aug 03 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.04.3-1
- 17.04.3

* Thu Aug 03 2017 David Tardon <dtardon@redhat.com> - 17.04.2-4
- rebuild for poppler 0.57.0

* Thu Aug 03 2017 Fedora Release Engineering <releng@fedoraproject.org> - 17.04.2-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Binutils_Mass_Rebuild

* Thu Jul 27 2017 Fedora Release Engineering <releng@fedoraproject.org> - 17.04.2-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Mass_Rebuild

* Thu Jun 15 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.04.2-1
- 17.04.2

* Thu May 11 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.04.1-1
- 17.04.1

* Fri May 05 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.04.0-1
- 17.04.0

* Tue Mar 28 2017 David Tardon <dtardon@redhat.com> - 16.12.3-2
- rebuild for poppler 0.53.0

* Thu Mar 09 2017 Rex Dieter <rdieter@fedoraproject.org> - 16.12.3-1
- 16.12.3

* Thu Feb 09 2017 Rex Dieter <rdieter@fedoraproject.org> - 16.12.2-1
- 16.12.2

* Thu Jan 12 2017 Rex Dieter <rdieter@fedoraproject.org> - 16.12.1-1
- 16.12.1, kf5-ize

* Wed Nov 30 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.08.3-1
- 16.08.3, disable tests
- -devel: Provides: okular4-devel
- -part: Provides: okular4-part

* Thu Oct 13 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.08.2-1
- 16.08.2

* Wed Sep 07 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.08.1-1
- 16.08.1

* Sat Aug 13 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.08.0-1
- 16.08.0

* Sat Aug 06 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.07.90-1
- 16.07.90

* Sat Jul 30 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.07.80-1
- 16.07.80

* Sun Jul 10 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.04.3-1
- 16.04.3

* Sun Jun 12 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.04.2-1
- 16.04.2

* Sun May 08 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.04.1-1
- 16.04.1

* Wed Apr 20 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.04.0-2
- rebuild (qt)

* Mon Apr 18 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.04.0-1
- 16.04.0

* Thu Apr 14 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.03.80-1
- 16.03.80

* Tue Mar 15 2016 Rex Dieter <rdieter@fedoraproject.org> - 15.12.3-1
- 15.12.3, retire -active on f24+

* Mon Feb 15 2016 Rex Dieter <rdieter@fedoraproject.org> - 15.12.2-1
- 15.12.2

* Fri Feb 05 2016 Rex Dieter <rdieter@fedoraproject.org> 15.12.1-3
- cleanup, use %%license, -libs: Recommends: cups-client ghostscript-core

* Thu Feb 04 2016 Fedora Release Engineering <releng@fedoraproject.org> - 15.12.1-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_24_Mass_Rebuild

* Sat Jan 30 2016 Rex Dieter <rdieter@fedoraproject.org> - 15.12.1-1
- 15.12.1

* Sat Dec 05 2015 Rex Dieter <rdieter@fedoraproject.org> - 15.08.3-1
- 15.08.3

* Sat Oct 03 2015 Rex Dieter <rdieter@fedoraproject.org> - 15.08.1-1
- 15.08.1

* Thu Aug 20 2015 Than Ngo <than@redhat.com> - 15.08.0-1
- 15.08.0

* Wed Jun 17 2015 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 15.04.2-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_23_Mass_Rebuild

* Wed Jun 10 2015 Rex Dieter <rdieter@fedoraproject.org> - 15.04.2-1
- 15.04.2

* Tue May 26 2015 Rex Dieter <rdieter@fedoraproject.org> - 15.04.1-1
- 15.04.1

* Thu Apr 23 2015 Rex Dieter <rdieter@fedoraproject.org> 15.04.0-2
- make (kde4) lib build dependencies libkipi, qmobipocket unversioned

* Tue Apr 14 2015 Rex Dieter <rdieter@fedoraproject.org> - 15.04.0-1
- 15.04.0

* Sun Mar 01 2015 Rex Dieter <rdieter@fedoraproject.org> - 14.12.3-1
- 14.12.3

* Tue Feb 24 2015 Than Ngo <than@redhat.com> - 14.12.2-1
- 14.12.2

* Sat Jan 17 2015 Rex Dieter <rdieter@fedoraproject.org> - 14.12.1-1
- 14.12.1

* Fri Jan 16 2015 Rex Dieter <rdieter@fedoraproject.org> 14.11.97-3
- -libs: Requires: fix typo (#1183110), but omit (for now)

* Tue Dec 30 2014 Rex Dieter <rdieter@fedoraproject.org> 14.11.97-2
- -libs: Requires: cups(lpr) ps2pdf pdf2ps

* Mon Dec 08 2014 Rex Dieter <rdieter@fedoraproject.org> - 14.11.97-1
- 14.11.97

* Sat Nov 08 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.14.3-1
- 4.14.3

* Fri Oct 31 2014 Rex Dieter <rdieter@fedoraproject.org> 4.14.2-2
- BR: pkgconfig(kscreen)

* Sat Oct 11 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.14.2-1
- 4.14.2

* Mon Sep 15 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.14.1-1
- 4.14.1

* Sun Aug 17 2014 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 4.14.0-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_21_22_Mass_Rebuild

* Thu Aug 14 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.14.0-1
- 4.14.0

* Tue Aug 05 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.13.97-1
- 4.13.97

* Mon Jul 14 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.13.3-1
- 4.13.3

* Mon Jun 09 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.13.2-1
- 4.13.2

* Sat Jun 07 2014 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 4.13.1-4
- Rebuilt for https://fedoraproject.org/wiki/Fedora_21_Mass_Rebuild

* Thu May 29 2014 Kevin Kofler <Kevin@tigcc.ticalc.org> - 4.13.1-3
- restore -active support (the plasma-mobile FTBFS is long fixed)

* Mon May 12 2014 Rex Dieter <rdieter@fedoraproject.org> 4.13.1-2
- drop -active support (until plasma-mobile FTBFS is fixed)

* Sun May 11 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.13.1-1
- 4.13.1

* Sat Apr 12 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.13.0-1
- 4.13.0

* Fri Apr 04 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.12.97-1
- 4.12.97

* Sat Mar 22 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.12.95-1
- 4.12.95

* Wed Mar 19 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.12.90-1
- 4.12.90

* Fri Mar 07 2014 Rex Dieter <rdieter@fedoraproject.org> 4.12.3-2
- drop BR: libkipi-devel (only needs libkexiv2)

* Sat Mar 01 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.12.3-1
- 4.12.3

* Fri Jan 31 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.12.2-1
- 4.12.2

* Fri Jan 10 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.12.1-1
- 4.12.1

* Fri Jan 03 2014 Rex Dieter <rdieter@fedoraproject.org> 4.12.0-4
- %%check: verify kimgio/mobi support

* Wed Jan 01 2014 Rex Dieter <rdieter@fedoraproject.org> 4.12.0-3
- KDE4_BUILD_TESTS:BOOL=ON

* Mon Dec 30 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.12.0-2
- rebuild against fixed qmobipocket
- BR: libkexiv2-devel

* Thu Dec 19 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.12.0-1
- 4.12.0

* Sun Dec 01 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.11.97-1
- 4.11.97

* Thu Nov 21 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.11.95-1
- 4.11.95

* Sat Nov 16 2013 Rex Dieter <rdieter@fedoraproject.org> 4.11.90-2
- +mobipocket support

* Sat Nov 16 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.11.90-1
- 4.11.90

* Sat Nov 02 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.11.3-1
- 4.11.3

* Sat Sep 28 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.11.2-1
- 4.11.2

* Tue Sep 03 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.11.1-1
- 4.11.1

* Thu Aug 08 2013 Than Ngo <than@redhat.com> - 4.11.0-1
- 4.11.0

* Thu Jul 25 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.10.97-1
- 4.10.97

* Tue Jul 23 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.10.95-1
- 4.10.95

* Thu Jun 27 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.10.90-1
- 4.10.90

* Sat Jun 01 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.10.4-1
- 4.10.4

* Mon May 06 2013 Than Ngo <than@redhat.com> - 4.10.3-1
- 4.10.3

* Tue Apr 09 2013 Than Ngo <than@redhat.com> - 4.10.2-3
- don't build component if ACTIVEAPP_FOUND FALSE

* Fri Apr 05 2013 Than Ngo <than@redhat.com> - 4.10.2-2
- BR on plasma-mobile-devel only in fedora

* Sun Mar 31 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.10.2-1
- 4.10.2

* Sun Mar 03 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.10.1-1
- 4.10.1

* Thu Jan 31 2013 Than Ngo <than@redhat.com> - 4.10.0-1
- 4.10.0
- get rid of gcc overflow warning

* Tue Jan 22 2013 Rex Dieter <rdieter@fedoraproject.org> 4.9.98-2
- filename encoding fix (#747976, kde#313700)

* Sun Jan 20 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.9.98-1
- 4.9.98

* Fri Jan 04 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.9.97-1
- 4.9.97

* Thu Dec 20 2012 Rex Dieter <rdieter@fedoraproject.org> - 4.9.95-1
- 4.9.95

* Tue Dec 04 2012 Rex Dieter <rdieter@fedoraproject.org> - 4.9.90-1
- 4.9.90

* Mon Dec 03 2012 Than Ngo <than@redhat.com> - 4.9.4-1
- 4.9.4

* Sat Nov 03 2012 Rex Dieter <rdieter@fedoraproject.org> - 4.9.3-1
- 4.9.3

* Wed Oct 24 2012 Rex Dieter <rdieter@fedoraproject.org> 4.9.2-2
- rebuild (libjpeg-turbo v8)

* Fri Sep 28 2012 Rex Dieter <rdieter@fedoraproject.org> - 4.9.2-1
- 4.9.2

* Mon Sep 03 2012 Than Ngo <than@redhat.com> -  7 :4.9.1-1
- 4.9.1

* Sat Aug 18 2012 Rex Dieter <rdieter@fedoraproject.org> 4.9.0-3
- followup fix for KXMLGUIClient (hang on close)

* Sun Aug 12 2012 Rex Dieter <rdieter@fedoraproject.org> 4.9.0-2
- KXMLGUIClient memory corruption warning (kde#261538)

* Thu Jul 26 2012 Lukas Tinkl <ltinkl@redhat.com> - 4.9.0-1
- 4.9.0

* Fri Jul 20 2012 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 4.8.97-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_18_Mass_Rebuild

* Wed Jul 11 2012 Rex Dieter <rdieter@fedoraproject.org> - 4.8.97-1
- 4.8.97

* Wed Jun 27 2012 Rex Dieter <rdieter@fedoraproject.org> - 4.8.95-1
- 4.8.95

* Sat Jun 09 2012 Rex Dieter <rdieter@fedoraproject.org> - 4.8.90-1
- 4.8.90

* Sat May 26 2012 Jaroslav Reznik <jreznik@redhat.com> - 4.8.80-1
- 4.8.80

* Wed May 16 2012 Marek Kasik <mkasik@redhat.com> - 4.8.3-4
- Rebuild (poppler-0.20.0)

* Wed May 09 2012 Than Ngo <than@redhat.com> - 4.8.3-3
- add fedora/rhel condition

* Tue May 08 2012 Rex Dieter <rdieter@fedoraproject.org> 4.8.3-2
- rebuild (libtiff)

* Mon Apr 30 2012 Jaroslav Reznik <jreznik@redhat.com> - 4.8.3-1
- 4.8.3

* Fri Mar 30 2012 Rex Dieter <rdieter@fedoraproject.org> - 4.8.2-1
- 4.8.2

* Wed Mar 07 2012 Rex Dieter <rdieter@fedoraproject.org> 4.8.1-2
- s/kdebase-runtime/kde-runtime/

* Mon Mar 05 2012 Jaroslav Reznik <jreznik@redhat.com> - 4.8.1-1
- 4.8.1

* Sun Jan 22 2012 Rex Dieter <rdieter@fedoraproject.org> - 4.8.0-1
- 4.8.0

* Wed Jan 04 2012 Radek Novacek <rnovacek@redhat.com> - 4.7.97-1
- 4.7.97

* Wed Dec 21 2011 Radek Novacek <rnovacek@redhat.com> - 4.7.95-1
- 4.7.95

* Sun Dec 04 2011 Rex Dieter <rdieter@fedoraproject.org> - 4.7.90-1
- 4.7.90

* Fri Nov 25 2011 Jaroslav Reznik <jreznik@redhat.com> 4.7.80-1
- 4.7.80 (beta 1)

* Tue Nov 15 2011 Rex Dieter <rdieter@fedoraproject.org> 4.7.3-2
- okular-part subpkg
- BR: libjpeg-devel

* Sat Oct 29 2011 Rex Dieter <rdieter@fedoraproject.org> 4.7.3-1
- 4.7.3
- more pkgconfig-style deps

* Wed Oct 05 2011 Rex Dieter <rdieter@fedoraproject.org> 4.7.2-1
- 4.7.2

* Tue Sep 06 2011 Than Ngo <than@redhat.com> - 4.7.1-1
- 4.7.1

* Tue Jul 26 2011 Jaroslav Reznik <jreznik@redhat.com> 4.7.0-1
- 4.7.0

* Mon Jul 18 2011 Rex Dieter <rdieter@fedoraproject.org> 4.6.95-4
- %%postun: +update-desktop-database

* Mon Jul 18 2011 Rex Dieter <rdieter@fedoraproject.org> 4.6.95-3
- BR: pkgconfig(qca2)

* Fri Jul 15 2011 Rex Dieter <rdieter@fedoraproject.org> 4.6.95-2
- bump release

* Mon Jul 11 2011 Rex Dieter <rdieter@fedoraproject.org> 4.6.95-1
- 4.6.95
- fix URL

* Wed Jul 06 2011 Rex Dieter <rdieter@fedoraproject.org> 4.6.90-3
- fix Source URL
- Conflicts: kdegraphics < 7:4.6.90-10

* Tue Jul 05 2011 Rex Dieter <rdieter@fedoraproject.org>  4.6.90-2
- first try

