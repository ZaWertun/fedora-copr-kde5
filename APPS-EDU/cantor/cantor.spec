# uncomment to enable bootstrap mode
#global bootstrap 1

%if !0%{?bootstrap}
%global qalculate 1
%if 0%{?fedora} < 26
%global libr 1
%endif
%global libspectre 1
%ifarch %{arm} %{ix86} x86_64 aarch64
%global luajit 1
%endif
%global python3 1
%endif

Name:    cantor
Summary: KDE Frontend to Mathematical Software
Version: 22.04.3
Release: 1%{?dist}

License: GPLv2+
URL:     https://edu.kde.org/cantor/

%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0: https://download.kde.org/%{stable}/release-service/%{version}/src/%{name}-%{version}.tar.xz

## upstream fixes

## upstreamable patches

%global majmin_ver %(echo %{version} | cut -d. -f1,2)
BuildRequires: analitza-devel >= %{majmin_ver}

BuildRequires: desktop-file-utils
BuildRequires: libappstream-glib

BuildRequires: extra-cmake-modules
BuildRequires: kf5-rpm-macros
BuildRequires: cmake(KF5Archive)
BuildRequires: cmake(KF5Completion)
BuildRequires: cmake(KF5Config)
BuildRequires: cmake(KF5CoreAddons)
BuildRequires: cmake(KF5Crash)
BuildRequires: cmake(KF5DocTools)
BuildRequires: cmake(KF5I18n)
BuildRequires: cmake(KF5IconThemes)
BuildRequires: cmake(KF5KIO)
BuildRequires: cmake(KF5NewStuff)
BuildRequires: cmake(KF5Parts)
BuildRequires: cmake(KF5Pty)
BuildRequires: cmake(KF5SyntaxHighlighting)
BuildRequires: cmake(KF5TextEditor)
BuildRequires: cmake(KF5TextWidgets)
BuildRequires: cmake(KF5XmlGui)

BuildRequires: pkgconfig(Qt5PrintSupport)
BuildRequires: pkgconfig(Qt5Svg)
BuildRequires: pkgconfig(Qt5Widgets)
BuildRequires: pkgconfig(Qt5Xml)
BuildRequires: pkgconfig(Qt5XmlPatterns)
BuildRequires: pkgconfig(Qt5Test)
BuildRequires: pkgconfig(Qt5Help)
BuildRequires: pkgconfig(Qt5WebEngine)
BuildRequires: poppler-qt5-devel

# optional deps/plugins
%if 0%{?qalculate}
BuildRequires: pkgconfig(libqalculate)
%endif
%if 0%{?libspectre}
BuildRequires: pkgconfig(libspectre)
%endif
%if 0%{?luajit}
BuildRequires: pkgconfig(luajit)
%endif
%if 0%{?python3}
BuildRequires: python3-devel
%else
Obsoletes: cantor-python3 < %{version}-%{release}
%endif

Requires: %{name}-libs%{?_isa} = %{version}-%{release}

%description
%{summary}.

%package -n python3-%{name}
Summary: %{name} python3 backend
Requires: %{name}-libs%{?_isa} = %{version}-%{release}
Obsoletes: cantor <  16.08.0-3
%{?python_provide:%python_provide python3-%{name}}
%description -n python3-%{name}
%{name} python3 backend.

%package  libs
Summary:  Runtime files for %{name}
# when split occurred
Conflicts: kdeedu-math-libs < 4.7.0-10
Provides: %{name}-part = %{version}-%{release}
Requires: %{name} = %{version}-%{release}
%description libs
%{summary}.

%if 0%{?libr}
%package R
Summary: R backend for %{name}
BuildRequires: pkgconfig(libR)
Obsoletes: kdeedu-math-cantor-R < 4.7.0-10
Provides:  kdeedu-math-cantor-R = %{version}-%{release}
Requires: %{name}-libs%{?_isa} = %{version}-%{release}
%description R 
%{summary}.
%endif

%package devel
Summary:  Development files for %{name}
# when split occurred
Conflicts: kdeedu-devel < 4.7.0-10
Requires: %{name}-libs%{?_isa} = %{version}-%{release}
%description devel
%{summary}.


%prep
%autosetup -p1


%build
%cmake_kf5
%cmake_build


%install
%cmake_install
%find_lang %{name} --all-name --with-html


%check
appstream-util validate-relax --nonet %{buildroot}%{_kf5_metainfodir}/org.kde.%{name}.appdata.xml ||:
desktop-file-validate %{buildroot}%{_kf5_datadir}/applications/org.kde.%{name}.desktop


%files -f %{name}.lang
%doc README*
%license LICENSES/*.txt
%{_kf5_bindir}/cantor*
%{_kf5_metainfodir}/org.kde.%{name}.appdata.xml
%{_kf5_datadir}/applications/org.kde.%{name}.desktop
%{_kf5_datadir}/knsrcfiles/%{name}.knsrc
%{_kf5_datadir}/knsrcfiles/%{name}_sage.knsrc
%{_kf5_datadir}/knsrcfiles/%{name}_scilab.knsrc
%{_kf5_datadir}/knsrcfiles/%{name}_maxima.knsrc
%{_kf5_datadir}/knsrcfiles/%{name}_octave.knsrc
%{_kf5_datadir}/knsrcfiles/%{name}_kalgebra.knsrc
%if 0%{?luajit}
%{_kf5_datadir}/knsrcfiles/%{name}_lua.knsrc
%endif
%if 0%{?qalculate}
%{_kf5_datadir}/knsrcfiles/%{name}_qalculate.knsrc
%endif
%dir %{_kf5_datadir}/kxmlgui5/cantor/
%{_datadir}/icons/hicolor/*/*/*
%{_kf5_datadir}/cantor/
%{_kf5_datadir}/config.kcfg/*.kcfg
%exclude %{_kf5_datadir}/config.kcfg/pythonbackend.kcfg
%{_kf5_datadir}/kxmlgui5/cantor/cantor_scripteditor.rc
%{_kf5_datadir}/kxmlgui5/cantor/cantor_shell.rc
%{_kf5_datadir}/kxmlgui5/cantor/cantor_*assistant.rc
%{_kf5_datadir}/mime/packages/%{name}.xml

%if 0%{?python3}
%files -n python3-%{name}
%{_kf5_datadir}/knsrcfiles/%{name}_python.knsrc
%{_kf5_bindir}/cantor_pythonserver
%{_kf5_libdir}/cantor_pythonbackend.so
%{_kf5_qtplugindir}/cantor/backends/cantor_pythonbackend.so
%{_kf5_datadir}/config.kcfg/pythonbackend.kcfg
%endif

%if 0%{?libr}
%files R
%{_kf5_bindir}/cantor_rserver
%{_kf5_qtplugindir}/cantor/backends/cantor_rbackend.so
%{_kf5_datadir}/config.kcfg/rserver.kcfg
%{_kf5_datadir}/knsrcfiles/%{name}_r.knsrc
%endif

%ldconfig_scriptlets libs

%files libs
%{_libdir}/libcantorlibs.so.*
%{_libdir}/libcantor_config.so
%{_kf5_datadir}/kxmlgui5/cantor/cantor_part.rc
%{_kf5_datadir}/knsrcfiles/cantor-documentation.knsrc
## plugins
%dir %{_kf5_qtplugindir}/cantor/
%{_kf5_qtplugindir}/cantor/assistants/
%{_kf5_qtplugindir}/cantor/panels/
%{_kf5_qtplugindir}/kf5/parts/cantorpart.so
%dir %{_kf5_qtplugindir}/cantor/backends/
%{_kf5_qtplugindir}/cantor/backends/cantor_kalgebrabackend.so
%if 0%{?luajit}
%{_kf5_qtplugindir}/cantor/backends/cantor_luabackend.so
%endif
%{_kf5_qtplugindir}/cantor/backends/cantor_maximabackend.so
%{_kf5_qtplugindir}/cantor/backends/cantor_octavebackend.so
%if 0%{?qalculate}
%{_kf5_qtplugindir}/cantor/backends/cantor_qalculatebackend.so
%endif
%{_kf5_qtplugindir}/cantor/backends/cantor_sagebackend.so
%{_kf5_qtplugindir}/cantor/backends/cantor_scilabbackend.so

%files devel
%{_includedir}/cantor/
%{_libdir}/libcantorlibs.so
%{_libdir}/cmake/Cantor/
%{_kf5_datadir}/config.kcfg/octavebackend.kcfg.in


%changelog
* Thu Jul 07 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.04.3-1
- 22.04.3

* Thu Jun 09 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.04.2-1
- 22.04.2

* Thu May 12 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.04.1-1
- 22.04.1

* Fri Apr 22 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.04.0-1
- 22.04.0

* Thu Mar 03 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 21.12.3-1
- 21.12.3

* Thu Feb 03 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 21.12.2-1
- 21.12.2

* Thu Jan 06 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 21.12.1-1
- 21.12.1

* Thu Dec 09 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 21.12.0-1
- 21.12.0

* Thu Nov 04 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 21.08.3-1
- 21.08.3

* Thu Oct 07 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 21.08.2-1
- 21.08.2

* Thu Sep 02 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 21.08.1-1
- 21.08.1

* Thu Aug 12 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 21.08.0-1
- 21.08.0

* Thu Jul 08 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 21.04.3-1
- 21.04.3

* Fri Jun 11 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 21.04.2-1
- 21.04.2

* Fri May 14 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 21.04.1-1
- 21.04.1

* Fri Apr 23 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 21.04.0-1
- 21.04.0

* Tue Mar 09 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.12.3-1
- 20.12.3

* Fri Feb  5 00:48:04 MSK 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.12.2-1
- 20.12.2

* Fri Jan  8 18:02:57 MSK 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.12.1-1
- 20.12.1

* Mon Dec 14 13:04:32 MSK 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.12.0-1
- 20.12.0

* Sat Nov  7 02:22:25 MSK 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.08.3-1
- 20.08.3

* Fri Oct 09 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.08.2-1
- 20.08.2

* Fri Sep 04 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.08.1-1
- 20.08.1

* Sun Aug 16 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.08.0-1
- 20.08.0

* Fri Jul 10 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.04.3-1
- 20.04.3

* Sat Jun 13 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.04.2-1
- 20.04.2

* Thu May 21 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.04.1-1
- 20.04.1

* Mon Apr 27 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.04.0-1
- 20.04.0

* Fri Feb 21 2020 Than Ngo <than@redhat.com> - 19.12.2-2
- Fixed bz#1799106, FTBFS

* Tue Feb 04 2020 Rex Dieter <rdieter@fedoraproject.org> - 19.12.2-1
- 19.12.2

* Tue Jan 28 2020 Fedora Release Engineering <releng@fedoraproject.org> - 19.12.1-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_32_Mass_Rebuild

* Thu Jan 16 2020 Rex Dieter <rdieter@fedoraproject.org> - 19.12.1-1
- 19.12.1

* Thu Jan 16 2020 Rex Dieter <rdieter@fedoraproject.org> - 19.08.3-3
- cantor fails to build with Python 3.9: Could NOT find PythonLibs3 (#1791770)

* Fri Nov 29 2019 Mukundan Ragavan <nonamedotc@gmail.com> - 19.08.3-2
- rebuild for libqalculate

* Tue Nov 12 2019 Rex Dieter <rdieter@fedoraproject.org> - 19.08.3-1
- 19.08.3

* Thu Oct 17 2019 Rex Dieter <rdieter@fedoraproject.org> - 19.08.2-1
- 19.08.2

* Sun Sep 29 2019 Rex Dieter <rdieter@fedoraproject.org> - 19.08.1-1
- 19.08.1

* Tue Aug 27 2019 Mukundan Ragavan <nonamedotc@gmail.com> - 19.08.0-2
- rebuild for libqalculate

* Tue Aug 20 2019 Rex Dieter <rdieter@fedoraproject.org> - 19.08.0-1
- 19.08.0

* Mon Aug 19 2019 Miro Hrončok <mhroncok@redhat.com> - 19.04.3-3
- Rebuilt for Python 3.8

* Wed Jul 24 2019 Fedora Release Engineering <releng@fedoraproject.org> - 19.04.3-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_31_Mass_Rebuild

* Thu Jul 11 2019 Rex Dieter <rdieter@fedoraproject.org> - 19.04.3-1
- 19.04.3

* Wed Jul 10 2019 Rex Dieter <rdieter@fedoraproject.org> - 19.04.2-2
- add python-3.8 support (#1705420)

* Tue Jun 04 2019 Rex Dieter <rdieter@fedoraproject.org> - 19.04.2-1
- 19.04.2

* Fri May 10 2019 Rex Dieter <rdieter@fedoraproject.org> - 19.04.1-1
- 19.04.1

* Sun Apr 21 2019 Mukundan Ragavan <nonamedotc@gmail.com> - 18.12.3-3
- rebuild for libqalculate

* Sat Mar 23 2019 Mukundan Ragavan <nonamedotc@gmail.com> - 18.12.3-2
- rebuild for libqalculate

* Fri Mar 08 2019 Rex Dieter <rdieter@fedoraproject.org> - 18.12.3-1
- 18.12.3

* Tue Feb 05 2019 Rex Dieter <rdieter@fedoraproject.org> - 18.12.2-1
- 18.12.2

* Thu Jan 31 2019 Fedora Release Engineering <releng@fedoraproject.org> - 18.12.1-4
- Rebuilt for https://fedoraproject.org/wiki/Fedora_30_Mass_Rebuild

* Mon Jan 21 2019 Mukundan Ragavan <nonamedotc@gmail.com> - 18.12.1-3
- rebuild for libqlaculate.so.20

* Sat Jan 12 2019 Miro Hrončok <mhroncok@redhat.com> - 18.12.1-2
- Remove python2 (#1659935) (#1342488)

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

* Tue Aug 21 2018 Mukundan Ragavan <nonamedotc@gmail.com> - 18.04.3-2
- rebuild for libqalculate.so.19()

* Fri Aug 10 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.04.3-1
- 18.04.3

* Thu Jul 12 2018 Fedora Release Engineering <releng@fedoraproject.org> - 18.04.2-5
- Rebuilt for https://fedoraproject.org/wiki/Fedora_29_Mass_Rebuild

* Fri Jun 29 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.04.2-4
- fix python-3.7
- macro'ize python3 suport (+bootstrap'able)

* Fri Jun 22 2018 Mukundan Ragavan <nonamedotc@gmail.com> - 18.04.2-3
- rebuild for libqalculate.so.18()

* Tue Jun 19 2018 Miro Hrončok <mhroncok@redhat.com> - 18.04.2-2
- Rebuilt for Python 3.7

* Wed Jun 06 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.04.2-1
- 18.04.2
- use %%make_build %%ldconfig_scriptlets

* Fri May 18 2018 Mukundan Ragavan <nonamedotc@gmail.com> - 18.04.1-2
- rebuild for libqalculate.so.17()

* Tue May 08 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.04.1-1
- 18.04.1

* Fri Apr 13 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.04.0-1
- 18.04.0

* Wed Apr 11 2018 Mukundan Ragavan <nonamedotc@gmail.com> - 18.03.90-2
- rebuild for libqalculate.so.16()

* Mon Apr 09 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.03.90-1
- 18.03.90

* Sat Mar 10 2018 Mukundan Ragavan <nonamedotc@gmail.com> - 17.12.3-2
- rebuild for libqalculate.so.14()

* Tue Mar 06 2018 Rex Dieter <rdieter@fedoraproject.org> - 17.12.3-1
- 17.12.3

* Wed Feb 07 2018 Fedora Release Engineering <releng@fedoraproject.org> - 17.12.2-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_28_Mass_Rebuild

* Tue Feb 06 2018 Rex Dieter <rdieter@fedoraproject.org> - 17.12.2-1
- 17.12.2

* Thu Jan 11 2018 Rex Dieter <rdieter@fedoraproject.org> - 17.12.1-1
- 17.12.1

* Sun Jan 07 2018 Igor Gnatenko <ignatenkobrain@fedoraproject.org> - 17.12.0-2
- Remove obsolete scriptlets

* Thu Dec 28 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.12.0-1
- 17.12.0

* Wed Nov 08 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.08.3-1
- 17.08.3

* Wed Oct 11 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.08.2-1
- 17.08.2

* Wed Sep 06 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.08.1-1
- 17.08.1

* Wed Aug 02 2017 Fedora Release Engineering <releng@fedoraproject.org> - 16.12.3-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Binutils_Mass_Rebuild

* Wed Jul 26 2017 Fedora Release Engineering <releng@fedoraproject.org> - 16.12.3-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Mass_Rebuild

* Fri Mar 10 2017 Rex Dieter <rdieter@fedoraproject.org> - 16.12.3-1
- 16.12.3

* Mon Feb 20 2017 Rex Dieter <rdieter@fedoraproject.org> - 16.12.2-3
- cantor: FTBFS in rawhide (#1423091)

* Mon Feb 20 2017 Rex Dieter <rdieter@fedoraproject.org> - 16.12.2-2
- rebuild

* Thu Feb 09 2017 Rex Dieter <rdieter@fedoraproject.org> - 16.12.2-1
- 16.12.2

* Mon Jan 30 2017 Rex Dieter <rdieter@fedoraproject.org> - 16.12.1-1
- 16.12.1, update URL, support %%bootstrap

* Sat Jan 28 2017 Mukundan Ragavan <nonamedotc@gmail.com> - 16.08.3-3
- rebuild for libqalculate.so.6

* Wed Dec 28 2016 Adam Williamson <awilliam@redhat.com>
- Fix build with Python 3

* Mon Dec 19 2016 Miro Hrončok <mhroncok@redhat.com>
- Rebuild for Python 3.6

* Mon Dec 05 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.08.3-1
- 16.08.3

* Thu Oct 13 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.08.2-1
- 16.08.2

* Mon Sep 19 2016 Peter Robinson <pbrobinson@fedoraproject.org> 16.08.1-2
- aarch64 now has LuaJIT

* Wed Sep 07 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.08.1-1
- 16.08.1

* Tue Sep 06 2016 Than Ngo <than@redhat.com> - 16.08.0-6
- fix build failure with luajit 2.1

* Tue Sep 06 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.08.0-5
- fix luajit-2.1 detection (#1371250)

* Tue Sep 06 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.08.0-4
- python subpkgs: add Obsoletes for upgrade path
- multilib fixes: move plugins to -libs, make plugins depend on -libs
- omit/workaround luajit FTBFS on f25+ (for now)

* Tue Sep 06 2016 Than Ngo <than@redhat.com> - 16.08.0-3
- fixed bz#1342488 - cantor requires both Python 2 and Python 3 

* Mon Aug 29 2016 Igor Gnatenko <ignatenko@redhat.com> - 16.08.0-2
- Rebuild for LuaJIT 2.1.0

* Fri Aug 12 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.08.0-1
- 16.08.0

* Sat Aug 06 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.07.90-1
- 16.07.90

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

* Sun Feb 14 2016 Rex Dieter <rdieter@fedoraproject.org> - 15.12.2-1
- 15.12.2

* Wed Feb 03 2016 Fedora Release Engineering <releng@fedoraproject.org> - 15.12.1-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_24_Mass_Rebuild

* Mon Jan 25 2016 Rex Dieter <rdieter@fedoraproject.org> - 15.12.1-1
- 15.12.1

* Wed Dec 23 2015 Rex Dieter <rdieter@fedoraproject.org> 15.12.0-1
- 15.12.0

* Wed Dec 23 2015 Rex Dieter <rdieter@fedoraproject.org> 15.08.3-2
- cosmetics, bump analitza dep

* Fri Nov 13 2015 Rex Dieter <rdieter@fedoraproject.org> 15.08.3-1
- 15.08.3, python-3.5 fix, .spec cosmetics

* Tue Nov 10 2015 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 15.08.1-2
- Rebuilt for https://fedoraproject.org/wiki/Changes/python3.5

* Mon Sep 14 2015 Rex Dieter <rdieter@fedoraproject.org> - 15.08.1-1
- 15.08.1

* Thu Aug 20 2015 Than Ngo <than@redhat.com> - 15.08.0-1
- 15.08.0

* Wed Jun 17 2015 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 15.04.2-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_23_Mass_Rebuild

* Wed Jun 10 2015 Rex Dieter <rdieter@fedoraproject.org> - 15.04.2-1
- 15.04.2

* Tue May 26 2015 Rex Dieter <rdieter@fedoraproject.org> - 15.04.1-1
- 15.04.1

* Sun May  3 2015 Peter Robinson <pbrobinson@fedoraproject.org> 15.04.0-2
- LuaJIT not available on all architectures

* Thu Apr 09 2015 Rex Dieter <rdieter@fedoraproject.org> 15.04.0-1
- 15.04.0

* Thu Apr 09 2015 Rex Dieter <rdieter@fedoraproject.org> 15.03.97-1
- 15.03.97

* Sun Mar 01 2015 Rex Dieter <rdieter@fedoraproject.org> - 14.12.3-1
- 14.12.3

* Tue Feb 24 2015 Than Ngo <than@redhat.com> - 14.12.2-1
- 14.12.2

* Sat Jan 31 2015 Rex Dieter <rdieter@fedoraproject.org> 14.12.1-2
- Requires: kate4-part

* Sat Jan 17 2015 Rex Dieter <rdieter@fedoraproject.org> - 14.12.1-1
- 14.12.1

* Sat Jan 17 2015 Rex Dieter <rdieter@fedoraproject.org> 4.14.3-2
- fixups for kde-apps-14.12

* Sat Nov 08 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.14.3-1
- 4.14.3

* Sun Oct 12 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.14.2-1
- 4.14.2

* Tue Sep 16 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.14.1-1
- 4.14.1

* Fri Aug 15 2014 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 4.14.0-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_21_22_Mass_Rebuild

* Fri Aug 15 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.14.0-1
- 4.14.0

* Tue Aug 05 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.13.97-1
- 4.13.97

* Tue Jul 15 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.13.3-1
- 4.13.3

* Mon Jun 09 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.13.2-1
- 4.13.2

* Sat Jun 07 2014 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 4.13.1-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_21_Mass_Rebuild

* Sun May 11 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.13.1-1
- 4.13.1

* Fri May 09 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.13.0-3
- Missing cantor python interface (#1095918)
- Provides: cantor-part

* Thu May  8 2014 Tom Callaway <spot@fedoraproject.org> - 4.13.0-2
- rebuild against R without libRblas/libRlapack

* Sat Apr 12 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.13.0-1
- 4.13.0

* Fri Apr 04 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.12.97-1
- 4.12.97

* Sat Mar 22 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.12.95-1
- 4.12.95

* Wed Mar 19 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.12.90-1
- 4.12.90

* Sun Mar 02 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.12.3-1
- 4.12.3

* Fri Jan 31 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.12.2-1
- 4.12.2

* Fri Jan 10 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.12.1-1
- 4.12.1

* Sat Dec 21 2013 Rex Dieter <rdieter@fedoraproject.org> 4.12.0-2
- rebuild (R)

* Thu Dec 19 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.12.0-1
- 4.12.0

* Sun Dec 01 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.11.97-1
- 4.11.97

* Thu Nov 21 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.11.95-1
- 4.11.95

* Sat Nov 16 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.11.90-1
- 4.11.90

* Sat Nov 02 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.11.3-1
- 4.11.3

* Sat Sep 28 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.11.2-1
- 4.11.2

* Wed Sep 04 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.11.1-1
- 4.11.1

* Thu Aug 08 2013 Than Ngo <than@redhat.com> - 4.11.0-1
- 4.11.0

* Thu Jul 25 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.10.97-1
- 4.10.97

* Tue Jul 23 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.10.95-1
- 4.10.95

* Fri Jun 28 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.10.90-1
- 4.10.90

* Sat Jun 01 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.10.4-1
- 4.10.4

* Sun May 26 2013 Kevin Kofler <Kevin@tigcc.ticalc.org> - 4.10.3-2
- fix SAGE backend for SAGE 5.8 (kde#316299), patch from upstream bugs.kde.org

* Mon May 06 2013 Than Ngo <than@redhat.com> - 4.10.3-1
- 4.10.3

* Sun Mar 31 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.10.2-1
- 4.10.2

* Sat Mar 02 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.10.1-1
- 4.10.1

* Thu Feb 07 2013 Rex Dieter <rdieter@fedoraproject.org> 4.10.0-2
- recent libgfortran-related commits breaks cantor-R support (kde#314253)

* Fri Feb 01 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.10.0-1
- 4.10.0

* Tue Jan 22 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.9.98-1
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

* Sat Sep 29 2012 Rex Dieter <rdieter@fedoraproject.org> - 4.9.2-1
- 4.9.2

* Mon Sep 03 2012 Than Ngo <than@redhat.com> - 4.9.1-1
- 4.9.1

* Thu Jul 26 2012 Lukas Tinkl <ltinkl@redhat.com> - 4.9.0-1
- 4.9.0

* Wed Jul 18 2012 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 4.8.97-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_18_Mass_Rebuild

* Thu Jul 12 2012 Rex Dieter <rdieter@fedoraproject.org> - 4.8.97-1
- 4.8.97

* Wed Jun 27 2012 Jaroslav Reznik <jreznik@redhat.com> - 4.8.95-1
- 4.8.95

* Sat Jun 09 2012 Rex Dieter <rdieter@fedoraproject.org> - 4.8.90-1
- 4.8.90

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

* Wed Jan 04 2012 Radek Novacek <rnovacek@redhat.com> - 4.7.97-1
- 4.7.97

* Thu Dec 22 2011 Radek Novacek <rnovacek@redhat.com> - 4.7.95-1
- 4.7.95

* Sun Dec 04 2011 Rex Dieter <rdieter@fedoraproject.org> - 4.7.90-1
- 4.7.90

* Sat Dec 03 2011 Rex Dieter <rdieter@fedoraproject.org> 4.7.80-2
- BR: analitza-devel pkgconfig(libqalculate) 

* Fri Nov 25 2011 Rex Dieter <rdieter@fedoraproject.org> 4.7.80-1
- 4.7.80

* Wed Nov  9 2011 Tom Callaway <spot@fedoraproject.org> 4.7.3-2
- rebuild for R 2.14.0

* Sat Oct 29 2011 Rex Dieter <rdieter@fedoraproject.org> 4.7.3-1
- 4.7.3

* Sat Oct 08 2011 Rex Dieter <rdieter@fedoraproject.org> 4.7.2-3
- Requires: kate-part

* Sat Oct 08 2011 Kevin Kofler <Kevin@tigcc.ticalc.org> 4.7.2-2
- restore R support (unconditionally, was temporarily disabled on F17+)

* Wed Oct 05 2011 Rex Dieter <rdieter@fedoraproject.org> 4.7.2-1
- 4.7.2

* Wed Sep 21 2011 Rex Dieter <rdieter@fedoraproject.org> 4.7.1-2
- License: GPLv2+
- %%doc COPYING

* Sat Sep 17 2011 Rex Dieter <rdieter@fedoraproject.org> 4.7.1-1
- 4.7.1

* Tue Aug 30 2011 Rex Dieter <rdieter@fedoraproject.org> 4.7.0-10
- first try

