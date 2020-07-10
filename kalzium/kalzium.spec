# workaround for bz#1546230
# ocaml doesn't support relocation
%undefine _hardened_build

%if ! 0%{?bootstrap}
# The Kalzium solver needs OCaml with native compilation support (ocamlopt) and
# ocaml-facile.
%ifarch %{?ocaml_native_compiler}
%global with_facile 1
%endif

# pending fix for https://bugzilla.redhat.com/1544510
%global avogadro 1
%endif

Name:    kalzium
Summary: Periodic Table of Elements
Version: 20.04.3
Release: 1%{?dist}

License: GPLv2+
URL:     https://edu.kde.org/kalzium/

%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0: https://download.kde.org/%{stable}/release-service/%{version}/src/%{name}-%{version}.tar.xz

## upstream patches

%if ! 0%{?bootstrap}
BuildRequires: libappstream-glib
BuildRequires: chemical-mime-data
BuildRequires: desktop-file-utils
%endif

# kf5
BuildRequires: extra-cmake-modules
BuildRequires: kf5-rpm-macros
BuildRequires: cmake(KF5Archive)
BuildRequires: cmake(KF5Config)
BuildRequires: cmake(KF5CoreAddons)
BuildRequires: cmake(KF5DocTools)
BuildRequires: cmake(KF5I18n)
BuildRequires: cmake(KF5KDELibs4Support)
BuildRequires: cmake(KF5KHtml)
BuildRequires: cmake(KF5NewStuff)
BuildRequires: cmake(KF5Parts)
BuildRequires: cmake(KF5Plotting)
BuildRequires: cmake(KF5Solid)
BuildRequires: cmake(KF5UnitConversion)
BuildRequires: cmake(KF5WidgetsAddons)

BuildRequires: cmake(Qt5Widgets)
BuildRequires: cmake(Qt5Script)
BuildRequires: cmake(Qt5Quick)
BuildRequires: cmake(Qt5Gui)
BuildRequires: cmake(Qt5OpenGL)
BuildRequires: cmake(Qt5Svg)

BuildRequires: pkgconfig(openbabel-2.0)
%if 0%{?avogadro}
# Eigen is only used for the Avogadro-based compound viewer.
BuildRequires: pkgconfig(eigen3)
BuildRequires: cmake(AvogadroLibs)
# workaround missing dep in avogadro2-libs-devel for now
BuildRequires: glew-devel
BuildRequires: spglib-devel
%endif
%if 0%{?with_facile}
# OCaml is only used with the Facile library, in the equation balancer.
BuildRequires: ocaml(compiler)
BuildRequires: ocaml-facile-devel
%endif

Requires: %{name}-libs%{?_isa} = %{version}-%{release}

# build.log wants this one, but it is really only used at runtime:
Requires: chemical-mime-data

%description
%{summary}.

%package  libs
Summary:  Runtime files for %{name}
Requires: %{name} = %{version}-%{release}
%description libs
%{summary}.

%package devel
Summary:  Development files for %{name}
Requires: %{name}-libs%{?_isa} = %{version}-%{release}
%description devel
%{summary}.


%prep
%autosetup -p1


%build
mkdir %{_target_platform}
pushd %{_target_platform}
%{cmake_kf5} ..
popd

make %{?_smp_mflags} -C %{_target_platform}


%install
make install/fast DESTDIR=%{buildroot} -C %{_target_platform}

%find_lang %{name} --all-name --with-html --with-man


%check
%if !0%{?bootstrap}
appstream-util validate-relax --nonet %{buildroot}%{_kf5_metainfodir}/org.kde.kalzium.appdata.xml
desktop-file-validate %{buildroot}%{_kf5_datadir}/applications/org.kde.kalzium.desktop
desktop-file-validate %{buildroot}%{_kf5_datadir}/applications/org.kde.kalzium_cml.desktop
%endif


%files -f %{name}.lang
%license COPYING*
%{_kf5_bindir}/kalzium
%{_kf5_sysconfdir}/xdg/kalzium.knsrc
%{_kf5_datadir}/kalzium/
%{_kf5_metainfodir}/org.kde.kalzium.appdata.xml
%{_kf5_datadir}/applications/org.kde.kalzium.desktop
%{_kf5_datadir}/applications/org.kde.kalzium_cml.desktop
%{_kf5_datadir}/config.kcfg/kalzium.kcfg
%{_kf5_datadir}/kxmlgui5/kalzium/
%{_kf5_datadir}/icons/hicolor/*/*/kalzium.*
%{_mandir}/man1/kalzium.*
%dir %{_kf5_datadir}/libkdeedu/
%{_kf5_datadir}/libkdeedu/data/

%{?ldconfig_scriptlets:%ldconfig_scriptlets libs}

%files libs
%if 0%{?avogadro}
%{_kf5_libdir}/libcompoundviewer.so.5*
%endif
%{_kf5_libdir}/libscience.so.5*

%files devel
%dir %{_includedir}/libkdeedu/
%{_includedir}/libkdeedu/*.h
%if 0%{?avogadro}
%{_kf5_libdir}/libcompoundviewer.so
%endif
%{_kf5_libdir}/libscience.so


%changelog
* Fri Jul 10 2020 Yaroslav Sidlovsky <zawertun@otl.ru> - 20.04.3-1
- 20.04.3

* Sat Jun 13 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.04.2-1
- 20.04.2

* Thu May 21 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.04.1-1
- 20.04.1

* Mon Apr 27 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.04.0-1
- 20.04.0

* Fri Jan 31 2020 Petr Viktorin <pviktori@redhat.com> - 19.12.1-3
- Remove BuildRequires on python2

* Wed Jan 29 2020 Fedora Release Engineering <releng@fedoraproject.org> - 19.12.1-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_32_Mass_Rebuild

* Thu Jan 16 2020 Rex Dieter <rdieter@fedoraproject.org> - 19.12.1-1
- 19.12.1

* Tue Nov 12 2019 Rex Dieter <rdieter@fedoraproject.org> - 19.08.3-1
- 19.08.3

* Thu Oct 17 2019 Rex Dieter <rdieter@fedoraproject.org> - 19.08.2-1
- 19.08.2

* Sun Sep 29 2019 Rex Dieter <rdieter@fedoraproject.org> - 19.08.1-1
- 19.08.1

* Tue Aug 20 2019 Rex Dieter <rdieter@fedoraproject.org> - 19.08.0-1
- 19.08.0

* Thu Jul 25 2019 Fedora Release Engineering <releng@fedoraproject.org> - 19.04.3-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_31_Mass_Rebuild

* Thu Jul 11 2019 Rex Dieter <rdieter@fedoraproject.org> - 19.04.3-1
- 19.04.3

* Tue Jun 04 2019 Rex Dieter <rdieter@fedoraproject.org> - 19.04.2-1
- 19.04.2

* Fri May 10 2019 Rex Dieter <rdieter@fedoraproject.org> - 19.04.1-1
- 19.04.1

* Fri Mar 08 2019 Rex Dieter <rdieter@fedoraproject.org> - 18.12.3-1
- 18.12.3

* Tue Feb 05 2019 Rex Dieter <rdieter@fedoraproject.org> - 18.12.2-1
- 18.12.2

* Fri Feb 01 2019 Fedora Release Engineering <releng@fedoraproject.org> - 18.12.1-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_30_Mass_Rebuild

* Tue Jan 08 2019 Rex Dieter <rdieter@fedoraproject.org> - 18.12.1-1
- 18.12.1

* Sat Dec 15 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.12.0-1
- 18.12.0

* Tue Nov 06 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.08.3-1
- 18.08.3

* Wed Oct 10 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.08.2-1
- 18.08.2

* Sun Sep 16 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.08.1-1
- 18.08.1

* Tue Aug 28 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.04.3-3
- rebuild for avogadro2

* Thu Aug 23 2018 Nicolas Chauvet <kwizart@gmail.com> - 18.04.3-2
- Rebuilt for glew 2.1.0

* Thu Jul 12 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.04.3-1
- 18.04.3

* Wed Jun 06 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.04.2-1
- 18.04.2

* Tue May 08 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.04.1-1
- 18.04.1

* Fri Apr 13 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.04.0-1
- 18.04.0

* Mon Apr 09 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.03.90-1
- 18.03.90

* Fri Mar 30 2018 Rex Dieter <rdieter@fedoraproject.org> - 17.12.3-1
- 17.12.3

* Thu Mar 15 2018 Iryna Shcherbina <ishcherb@redhat.com> - 17.12.2-6
- Update Python 2 dependency declarations to new packaging standards
  (See https://fedoraproject.org/wiki/FinalizingFedoraSwitchtoPython3)

* Fri Feb 16 2018 Than Ngo <than@redhat.com> - 17.12.2-5
- fixed FTBS

* Tue Feb 13 2018 Rex Dieter <rdieter@fedoraproject.org> - 17.12.2-4
- enable avogadro/compoundviewer support (#1544514)

* Mon Feb 12 2018 Rex Dieter <rdieter@fedoraproject.org> - 17.12.2-3
- fix kalzium support (pending fix for avogadro2-libs bug 1544510)
- more bootstrapping
- use %%ldconfig_scriptlets

* Wed Feb 07 2018 Fedora Release Engineering <releng@fedoraproject.org> - 17.12.2-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_28_Mass_Rebuild

* Tue Feb 06 2018 Rex Dieter <rdieter@fedoraproject.org> - 17.12.2-1
- 17.12.2

* Thu Jan 11 2018 Rex Dieter <rdieter@fedoraproject.org> - 17.12.1-1
- 17.12.1

* Thu Jan 11 2018 Igor Gnatenko <ignatenkobrain@fedoraproject.org> - 17.12.0-2
- Remove obsolete scriptlets

* Thu Dec 28 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.12.0-1
- 17.12.0

* Wed Nov 08 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.08.3-1
- 17.08.3

* Wed Oct 11 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.08.2-1
- 17.08.2

* Wed Sep 06 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.08.1-1
- 17.08.1

* Thu Aug 03 2017 Fedora Release Engineering <releng@fedoraproject.org> - 16.12.3-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Binutils_Mass_Rebuild

* Wed Jul 26 2017 Fedora Release Engineering <releng@fedoraproject.org> - 16.12.3-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Mass_Rebuild

* Fri Mar 10 2017 Rex Dieter <rdieter@fedoraproject.org> - 16.12.3-1
- 16.12.3

* Mon Feb 27 2017 Than Ngo <than@redhat.com> - 16.12.2-2
- fixed bz#1423092 - kalzium: FTBFS in rawhide

* Thu Feb 09 2017 Rex Dieter <rdieter@fedoraproject.org> - 16.12.2-1
- 16.12.2

* Wed Jan 11 2017 Rex Dieter <rdieter@fedoraproject.org> - 16.12.1-1
- 16.12.1, kf5-ize, disable avogadro support (for now)

* Thu Dec 22 2016 Sandro Mani <manisandro@gmail.com> - 16.08.3-2
- Build against eigen2 until avogadro supports eigen3 >= 3.3.0

* Mon Dec 05 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.08.3-1
- 16.08.3

* Fri Oct 14 2016 Dominik Mierzejewski <dominik@greysector.net> - 16.08.2-3
- fix appdata installation path
- use correct desktop and appdata filenames

* Fri Oct 14 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.08.2-3
- use appstream-util validate-relax
- drop unused majmin_ver macro
- support %%bootstrap

* Fri Oct 14 2016 Dominik Mierzejewski <dominik@greysector.net> - 16.08.2-2
- rebuild for openbabel-2.4.1

* Thu Oct 13 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.08.2-1
- 16.08.2

* Wed Sep 07 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.08.1-1
- 16.08.1

* Fri Aug 12 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.08.0-1
- 16.08.0

* Sat Aug 06 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.07.90-1
- 16.07.90

* Sat Jul 30 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.07.80-2
- -libs: drop verisioned libkdeedu reference

* Sat Jul 30 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.07.80-1
- 16.07.80

* Sat Jul 09 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.04.3-1
- 16.04.3

* Sun Jun 12 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.04.2-1
- 16.04.2

* Sun May 08 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.04.1-1
- 16.04.1

* Fri Apr 22 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.04.0-1
- 16.04.0

* Tue Mar 15 2016 Rex Dieter <rdieter@fedoraproject.org> - 15.12.3-1
- 15.12.3

* Sat Feb 20 2016 Dominik Mierzejewski <rpm@greysector.net> - 15.12.2-2
- Rebuild for openbabel

* Sun Feb 14 2016 Rex Dieter <rdieter@fedoraproject.org> - 15.12.2-1
- 15.12.2

* Thu Feb 04 2016 Fedora Release Engineering <releng@fedoraproject.org> - 15.12.1-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_24_Mass_Rebuild

* Mon Jan 25 2016 Rex Dieter <rdieter@fedoraproject.org> - 15.12.1-1
- 15.12.1

* Mon Dec 28 2015 Rex Dieter <rdieter@fedoraproject.org> 15.12.0-1
- 15.12.0

* Mon Nov 30 2015 Rex Dieter <rdieter@fedoraproject.org> - 15.08.3-1
- 15.08.3

* Mon Nov 30 2015 Rex Dieter <rdieter@fedoraproject.org> 15.08.0-2
- loosen build deps

* Thu Aug 20 2015 Than Ngo <than@redhat.com> - 15.08.0-1
- 15.08.0

* Wed Jun 17 2015 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 15.04.2-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_23_Mass_Rebuild

* Wed Jun 10 2015 Rex Dieter <rdieter@fedoraproject.org> - 15.04.2-1
- 15.04.2

* Thu May 28 2015 Rex Dieter <rdieter@fedoraproject.org> - 15.04.1-1
- 15.04.1

* Thu Apr 16 2015 Rex Dieter <rdieter@fedoraproject.org> - 15.04.0-1
- 15.04.0

* Sun Mar 29 2015 Kevin Kofler <Kevin@tigcc.ticalc.org> - 14.12.3-2
- Enable avogadro support unconditionally again, F21 avogadro was fixed on ARM
- Drop the BR pkgconfig(eigen2) on F20, avogadro now uses Eigen3 (everywhere)

* Sun Mar 01 2015 Rex Dieter <rdieter@fedoraproject.org> - 14.12.3-1
- 14.12.3

* Tue Feb 24 2015 Than Ngo <than@redhat.com> - 14.12.2-1
- 14.12.2

* Sat Jan 17 2015 Rex Dieter <rdieter@fedoraproject.org> - 14.12.1-1
- 14.12.1

* Sat Nov 08 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.14.3-1
- 4.14.3

* Sun Oct 12 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.14.2-1
- 4.14.2

* Tue Sep 16 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.14.1-1
- 4.14.1

* Sat Aug 16 2014 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 4.14.0-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_21_22_Mass_Rebuild

* Fri Aug 15 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.14.0-1
- 4.14.0

* Tue Aug 05 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.13.97-1
- 4.13.97

* Tue Jul 15 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.13.3-1
- 4.13.3

* Mon Jun 09 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.13.2-1
- 4.13.2

* Sun Jun 08 2014 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 4.13.1-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_21_Mass_Rebuild

* Sun May 11 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.13.1-1
- 4.13.1

* Sat Apr 12 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.13.0-1
- 4.13.0

* Fri Apr 04 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.12.97-1
- 4.12.97

* Sun Mar 23 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.12.95-1
- 4.12.95

* Wed Mar 19 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.12.90-1
- 4.12.90

* Sun Mar 02 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.12.3-1
- 4.12.3

* Fri Jan 31 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.12.2-1
- 4.12.2

* Sat Jan 25 2014 Kevin Kofler <Kevin@tigcc.ticalc.org> - 4.12.1-4
- Requires: chemical-mime-data

* Sat Jan 25 2014 Kevin Kofler <Kevin@tigcc.ticalc.org> - 4.12.1-3
- Disable avogadro support on ARM on F21+, drop ExcludeArch added in -2
- Enable facile support on aarch64 on F21+

* Sat Jan 25 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.12.1-2
- rebuild (avogadro)

* Fri Jan 10 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.12.1-1
- 4.12.1

* Thu Dec 19 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.12.0-1
- 4.12.0

* Sun Dec 01 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.11.97-1
- 4.11.97

* Thu Nov 21 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.11.95-1
- 4.11.95

* Mon Nov 18 2013 Dave Airlie <airlied@redhat.com> - 4.11.90-2
- rebuilt for GLEW 1.10

* Sat Nov 16 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.11.90-1
- 4.11.90

* Sat Nov 02 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.11.3-1
- 4.11.3

* Sat Sep 28 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.11.2-1
- 4.11.2

* Sun Sep 15 2013 Kevin Kofler <Kevin@tigcc.ticalc.org> - 4.11.1-2
- Rebuild for OCaml 4.01.0
- Drop Fedora < 18 conditional

* Wed Sep 04 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.11.1-1
- 4.11.1

* Thu Aug 08 2013 Than Ngo <than@redhat.com> - 4.11.0-1
- 4.11.0

* Sun Jul 28 2013 Petr Machata <pmachata@redhat.com> - 4.10.97-2
- Rebuild for boost 1.54.0

* Thu Jul 25 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.10.97-1
- 4.10.97

* Tue Jul 23 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.10.95-1
- 4.10.95

* Fri Jun 28 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.10.90-1
- 4.10.90

* Sat Jun 01 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.10.4-1
- 4.10.4

* Mon May 06 2013 Than Ngo <than@redhat.com> - 4.10.3-1
- 4.10.3

* Sun Mar 31 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.10.2-1
- 4.10.2

* Sat Mar 02 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.10.1-1
- 4.10.1

* Sun Feb 10 2013 Denis Arnaud <denis.arnaud_fedora@m4x.org> - 4.10.0-3
- Rebuild for Boost-1.53.0

* Sat Feb 09 2013 Denis Arnaud <denis.arnaud_fedora@m4x.org> - 4.10.0-2
- Rebuild for Boost-1.53.0

* Fri Feb 01 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.10.0-1
- 4.10.0

* Tue Jan 22 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.9.98-1
- 4.9.98

* Fri Jan 04 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.9.97-1
- 4.9.97

* Thu Dec 20 2012 Rex Dieter <rdieter@fedoraproject.org> - 4.9.95-1
- 4.9.95

* Thu Dec 13 2012 Adam Jackson <ajax@redhat.com> - 4.9.90-2
- Rebuild for glew 1.9.0

* Tue Dec 04 2012 Rex Dieter <rdieter@fedoraproject.org> - 4.9.90-1
- 4.9.90

* Mon Dec 03 2012 Than Ngo <than@redhat.com> - 4.9.4-1
- 4.9.4

* Sat Nov 03 2012 Rex Dieter <rdieter@fedoraproject.org> - 4.9.3-1
- 4.9.3

* Sun Oct 28 2012 Kevin Kofler <Kevin@tigcc.ticalc.org> - 4.9.2-2
- rebuild for OCaml 4.00.1

* Sat Sep 29 2012 Rex Dieter <rdieter@fedoraproject.org> - 4.9.2-1
- 4.9.2

* Mon Sep 03 2012 Than Ngo <than@redhat.com> - 4.9.1-1
- 4.9.1 

* Thu Aug 09 2012 Rex Dieter <rdieter@fedoraproject.org> 4.9.0-2
- rebuild (avogadro/boost)

* Thu Jul 26 2012 Lukas Tinkl <ltinkl@redhat.com> - 4.9.0-1
- 4.9.0

* Thu Jul 19 2012 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 4.8.97-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_18_Mass_Rebuild

* Thu Jul 12 2012 Rex Dieter <rdieter@fedoraproject.org> - 4.8.97-1
- 4.8.97

* Wed Jun 27 2012 Jaroslav Reznik <jreznik@redhat.com> - 4.8.95-1
- 4.8.95

* Sat Jun 09 2012 Rex Dieter <rdieter@fedoraproject.org> - 4.8.90-1
- 4.8.90
- rebuild for OCaml 4.00.0 and update ocamlopt arch list

* Fri Jun 01 2012 Jaroslav Reznik <jreznik@redhat.com> - 4.8.80-1
- 4.8.80

* Mon Apr 30 2012 Jaroslav Reznik <jreznik@redhat.com> - 4.8.3-1
- 4.8.3

* Fri Mar 30 2012 Rex Dieter <rdieter@fedoraproject.org> - 4.8.2-1
- 4.8.2

* Mon Mar 05 2012 Jaroslav Reznik <jreznik@redhat.com> - 4.8.1-1
- 4.8.1

* Sun Jan 22 2012 Rex Dieter <rdieter@fedoraproject.org> - 4.8.0-1
- 4.8.0

* Sat Jan 07 2012 Kevin Kofler <Kevin@tigcc.ticalc.org> - 4.7.97-2
- Rebuild for new OCaml (3.12.1)
- Enable facile on ARM, ocamlopt available on ARM these days

* Wed Jan 04 2012 Rex Dieter <rdieter@fedoraproject.org> - 4.7.97-1
- 4.7.97

* Thu Dec 22 2011 Radek Novacek <rnovacek@redhat.com> - 4.7.95-1
- 4.7.95

* Sun Dec 04 2011 Rex Dieter <rdieter@fedoraproject.org> - 4.7.90-1
- 4.7.90

* Fri Nov 25 2011 Rex Dieter <rdieter@fedoraproject.org> 4.7.80-1
- 4.7.80

* Sat Oct 29 2011 Rex Dieter <rdieter@fedoraproject.org> 4.7.3-1
- 4.7.3

* Wed Oct 05 2011 Rex Dieter <rdieter@fedoraproject.org> 4.7.2-1
- 4.7.2

* Sat Sep 17 2011 Rex Dieter <rdieter@fedoraproject.org> 4.7.1-1
- 4.7.1

* Tue Aug 30 2011 Rex Dieter <rdieter@fedoraproject.org> 4.7.0-10
- first try

