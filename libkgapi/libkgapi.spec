
# trim changelog included in binary rpms
%global _changelog_trimtime %(date +%s -d "1 year ago")

Name:           libkgapi
Version: 20.04.3
Release: 1%{?dist}
Summary:        Library to access to Google services

License:        GPLv2+
URL:            https://cgit.kde.org/%{name}.git

%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0: https://download.kde.org/%{stable}/release-service/%{version}/src/%{name}-%{version}.tar.xz

# handled by qt5-srpm-macros, which defines %%qt5_qtwebengine_arches
%{?qt5_qtwebengine_arches:ExclusiveArch: %{qt5_qtwebengine_arches}}

BuildRequires:  kf5-rpm-macros
BuildRequires:  extra-cmake-modules
BuildRequires:  qt5-qtbase-devel
BuildRequires:  qt5-qtwebengine-devel
BuildRequires:  qt5-qtxmlpatterns-devel
BuildRequires:  qt5-qttools-static

BuildRequires:  kf5-kcoreaddons-devel
BuildRequires:  kf5-kio-devel
BuildRequires:  kf5-kwallet-devel
BuildRequires:  kf5-kwindowsystem-devel

BuildRequires:  kf5-kcalendarcore-devel
BuildRequires:  kf5-kcontacts-devel

BuildRequires:  cyrus-sasl-devel

BuildRequires:  cmake(KF5CalendarCore)
BuildRequires:  cmake(KF5I18n)

Obsoletes:      libkgoogle < 0.3.2
Provides:       libkgoogle = %{version}-%{release}

%description
Library to access to Google services, this package is needed by kdepim-runtime
to build akonadi-google resources.

%package        devel
Summary:        Development files for %{name}
Requires:       %{name}%{?_isa} = %{version}-%{release}
Requires:       kf5-kcoreaddons-devel
Requires:       kf5-kcalendarcore-devel
Requires:       kf5-kcontacts-devel
Obsoletes:      libkgoogle-devel < 0.3.2
Provides:       libkgoogle-devel = %{version}-%{release}
%description devel
Libraries and header files for developing applications that use akonadi-google
resources.


%prep
%autosetup -n %{name}-%{version}


%build
mkdir %{_target_platform}
pushd %{_target_platform}
%{cmake_kf5} ..
popd

%make_build -C %{_target_platform}


%install
make install/fast DESTDIR=%{buildroot} -C %{_target_platform}

%find_lang_kf5 libkgapi_qt


%ldconfig_scriptlets

%files -f libkgapi_qt.lang
%doc README*
%license LICENSE
%{_kf5_libdir}/libKPimGAPIBlogger.so.5*
%{_kf5_libdir}/libKPimGAPICalendar.so.*
%{_kf5_libdir}/libKPimGAPIContacts.so.*
%{_kf5_libdir}/libKPimGAPICore.so.*
%{_kf5_libdir}/libKPimGAPIDrive.so.*
%{_kf5_libdir}/libKPimGAPILatitude.so.*
%{_kf5_libdir}/libKPimGAPIMaps.so.*
%{_kf5_libdir}/libKPimGAPITasks.so.*
%{_libdir}/sasl2/libkdexoauth2.so*
%{_kf5_datadir}/qlogging-categories5/*.categories


%files devel
%{_kf5_libdir}/libKPimGAPIBlogger.so
%{_kf5_archdatadir}/mkspecs/modules/qt_KGAPIBlogger.pri
%{_kf5_libdir}/libKPimGAPICalendar.so
%{_kf5_archdatadir}/mkspecs/modules/qt_KGAPICalendar.pri
%{_kf5_libdir}/libKPimGAPIContacts.so
%{_kf5_archdatadir}/mkspecs/modules/qt_KGAPIContacts.pri
%{_kf5_libdir}/libKPimGAPICore.so
%{_kf5_archdatadir}/mkspecs/modules/qt_KGAPICore.pri
%{_kf5_libdir}/libKPimGAPIDrive.so
%{_kf5_archdatadir}/mkspecs/modules/qt_KGAPIDrive.pri
%{_kf5_libdir}/libKPimGAPILatitude.so
%{_kf5_archdatadir}/mkspecs/modules/qt_KGAPILatitude.pri
%{_kf5_libdir}/libKPimGAPIMaps.so
%{_kf5_archdatadir}/mkspecs/modules/qt_KGAPIMaps.pri
%{_kf5_libdir}/libKPimGAPITasks.so
%{_kf5_archdatadir}/mkspecs/modules/qt_KGAPITasks.pri
%{_kf5_libdir}/cmake/KPimGAPI/
%dir %{_includedir}/KPim/
%{_includedir}/KPim/KGAPI/
%{_includedir}/KPim/kgapi_version.h


%changelog
* Thu Jul 09 2020 Yaroslav Sidlovsky <zawertun@otl.ru> - 20.04.3-1
- 20.04.3

* Fri Jun 12 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.04.2-1
- 20.04.2

* Tue May 19 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.04.1-1
- 20.04.1

* Fri Apr 24 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.04.0-1
- 20.04.0

* Fri Mar 06 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 19.12.3-1
- 19.12.3

* Fri Feb 07 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 19.12.2-1
- 19.12.2

* Fri Jan 10 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 19.12.1-1
- 19.12.1

* Thu Dec 12 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 19.12.0-1
- 19.12.0

* Fri Nov 08 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 19.08.3-1
- 19.08.3

* Thu Oct 10 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 19.08.2-1
- 19.08.2

* Thu Sep 05 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 19.08.1-1
- 19.08.1

* Thu Aug 15 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 19.08.0-1
- 19.08.0

* Thu Jul 11 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 19.04.3-1
- 19.04.3

* Thu Jun 06 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 19.04.2-1
- 19.04.2

* Thu May 09 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 19.04.1-1
- 19.04.1

* Sun Apr 28 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 19.04.0-1
- 19.04.0

* Tue Feb 05 2019 Rex Dieter <rdieter@fedoraproject.org> - 18.12.2-1
- 18.12.2

* Fri Feb 01 2019 Fedora Release Engineering <releng@fedoraproject.org> - 18.12.1-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_30_Mass_Rebuild

* Tue Jan 08 2019 Rex Dieter <rdieter@fedoraproject.org> - 18.12.1-1
- 18.12.1

* Fri Dec 14 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.12.0-1
- 18.12.0

* Tue Nov 06 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.08.3-1
- 18.08.3

* Wed Oct 10 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.08.2-1
- 18.08.2

* Mon Oct 01 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.08.1-1
- 18.08.1

* Fri Jul 13 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.04.3-1
- 18.04.3

* Fri Jul 13 2018 Fedora Release Engineering <releng@fedoraproject.org> - 18.04.2-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_29_Mass_Rebuild

* Wed Jun 06 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.04.2-1
- 18.04.2

* Wed May 09 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.04.1-1
- 18.04.1

* Fri Apr 20 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.04.0-1
- 18.04.0

* Tue Mar 06 2018 Rex Dieter <rdieter@fedoraproject.org> - 17.12.3-1
- 17.12.3

* Tue Feb 06 2018 Rex Dieter <rdieter@fedoraproject.org> - 17.12.2-1
- 17.12.2

* Thu Jan 11 2018 Rex Dieter <rdieter@fedoraproject.org> - 17.12.1-1
- 17.12.1

* Tue Dec 12 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.12.0-1
- 17.12.0

* Wed Dec 06 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.11.90-1
- 17.11.90

* Wed Nov 22 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.11.80-1
- 17.11.80

* Wed Nov 08 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.08.3-1
- 17.08.3

* Mon Sep 25 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.08.1-1
- 17.08.1

* Fri Jul 28 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.04.3-1
- 17.04.3

* Wed Jul 26 2017 Fedora Release Engineering <releng@fedoraproject.org> - 17.04.2-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Mass_Rebuild

* Thu Jun 15 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.04.2-1
- 17.04.2

* Sun May 14 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.04.1 -1
- 17.04.1

* Fri Feb 10 2017 Fedora Release Engineering <releng@fedoraproject.org> - 5.3.1-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_26_Mass_Rebuild

* Mon Jan 16 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.3.1-2
- rebuild (kde-apps-16.12)

* Thu Oct 20 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.3.1-1
- 5.3.1 (#1387323)

* Mon Aug 08 2016 Daniel Vrátil <dvratil@fedoraproject.org> - 5.3.0-1
- LibKGAPI 5.3.0

* Thu Feb 04 2016 Fedora Release Engineering <releng@fedoraproject.org> - 5.1.0-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_24_Mass_Rebuild

* Sat Jan 23 2016 Robert Scheck <robert@fedoraproject.org> - 5.1.0-2
- Rebuild for libical 2.0.0

* Mon Dec 07 2015 Jan Grulich <jgrulich@redhat.com> - 5.1.0-1
- Update to 5.1.0 (KF5-based)

* Fri Nov 13 2015 Rex Dieter <rdieter@fedoraproject.org> - 2.2.0-6
- fix(drop) hard-coded kdepimlibs dep (it won't be changing any time soon)
- use %%license
- .spec cosmetics
- trim changelog

* Wed Jul 05 2015 Daniel Vrátil <dvratil@redhat.com> - 2.2.0-5
- Pull upstream patch to fix timezone conversion

* Wed Jun 17 2015 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 2.2.0-4
- Rebuilt for https://fedoraproject.org/wiki/Fedora_23_Mass_Rebuild

* Sat May 02 2015 Kalev Lember <kalevlember@gmail.com> - 2.2.0-3
- Rebuilt for GCC 5 C++11 ABI change

* Sun Aug 17 2014 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 2.2.0-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_21_22_Mass_Rebuild

* Tue Aug 05 2014 Rex Dieter <rdieter@fedoraproject.org> 2.2.0-1
- 2.2.0

* Thu Jun 19 2014 Rex Dieter <rdieter@fedoraproject.org> 2.1.0-3
- BR: kdelibs4-webkit-devel

* Sat Jun 07 2014 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 2.1.0-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_21_Mass_Rebuild

* Wed Mar 19 2014 Dan Vrátil <dvratil@redhat.com> - 2.1.0-1
- 2.1.0

* Thu Dec 05 2013 Dan Vrátil <dvratil@redhat.com> - 2.0.2-1
- 2.0.2

* Sat Aug 03 2013 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 2.0.1-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_20_Mass_Rebuild

* Sat Jun 01 2013 Dan Vrátil <dvratil@redhat.com> 2.0.1-1
- 2.0.1

* Wed May 22 2013 Dan Vrátil <dvratil@redhat.com> 2.0.0-1
- 2.0.0

* Thu Feb 14 2013 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 0.4.4-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_19_Mass_Rebuild

* Sun Dec 16 2012 Dan Vrátil <dvratil@redhat.com> 0.4.4-1
- 0.4.4

* Tue Nov 27 2012 Dan Vrátil <dvratil@redhat.com> 0.4.3-3
- Rebuild against qjson 0.8.1

* Fri Nov 23 2012 Dan Vrátil <dvratil@redhat.com> 0.4.3-2
- Rebuild against qjson 0.8.0

* Sun Nov 11 2012 Mario Santagiuliana <fedora@marionline.it> 0.4.3-1
- Update to new version 0.4.3

* Sun Aug 26 2012 Rex Dieter <rdieter@fedoraproject.org> 0.4.2-1
- 0.4.2

* Sat Aug 11 2012 Mario Santagiuliana <fedora@marionline.it> 0.4.1-1
- Update to new version 0.4.1

* Thu Jul 19 2012 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 0.4.0-6
- Rebuilt for https://fedoraproject.org/wiki/Fedora_18_Mass_Rebuild

* Wed Jun 20 2012 Rex Dieter <rdieter@fedoraproject.org> 
- 0.4.0-5
- -devel: tighten subpkg dep via %%_isa, Req: kdepimlibs-devel
- Parsing token page failed (kde#301240)

* Sun Jun 10 2012 Rex Dieter <rdieter@fedoraproject.org> 0.4.0-4
- -devel: track files closer
- pkgconfig-style deps

* Thu Jun 07 2012 Mario Santagiuliana <fedora@marionline.it> 0.4.0-3
- Update spec file following Gregor Tätzner request:
https://bugzilla.redhat.com/show_bug.cgi?id=817622#c8

* Thu May 31 2012 Mario Santagiuliana <fedora@marionline.it> 0.4.0-2
- Update spec file following Rex Dieter and Kevin Kofler suggestion
- Add obsolete and provide for devel subpkg

* Thu May 31 2012 Mario Santagiuliana <fedora@marionline.it> 0.4.0-1
- Update to new version 0.4.0
- Update to new licence GPLv2+
- Update to new name libkgapi
- Add obsolete and provide libkgoogle

* Wed May 30 2012 Mario Santagiuliana <fedora@marionline.it> 0.3.2-1.20120530gitf18d699
- Update spec comment to new git repository
- Update to new version 0.3.2
- Snapshot f18d699d9ef7ceceff06c2bb72fc00f34811c503

* Mon Apr 30 2012 Mario Santagiuliana <fedora@marionline.it> 0.3.1-1.20120430gitefb3215
- Rename package from akonadi-google to libkgoogle
- Update spec file
- Snapshot efb32159c283168cc2ab1a39e6fa3c8a30fbc941

* Mon Apr 30 2012 Mario Santagiuliana <fedora@marionline.it> 0.3.1-1
- New version 0.3.1

* Thu Apr 01 2012 Mario Santagiuliana <fedora@marionline.it> 0.3-1.20120402git3e0a93e
- New version 0.3
- Update to git snapshot 3e0a93e1b24cd7b6e394cf76d153c428246f9fa9
- Obsolete akonadi-google-tasks
- Fix error in changelog

* Thu Mar 01 2012 Mario Santagiuliana <fedora@marionline.it> 0.2-12.20120301git41cd7c5
- Update to git snapshot 41cd7c5d6e9cfb62875fd21f8a920a235b7a7d9c

* Wed Jan 20 2012 Mario Santagiuliana <fedora@marionline.it> 0.2-11.20120121gitbe021c6
- Update to git snapshot be021c6f12e6804976dcac203a1864686a219c26

* Wed Jan 20 2012 Mario Santagiuliana <fedora@marionline.it> 0.2-10.20120120git11bf6ad
- Update spec file follow comment 1:
https://bugzilla.redhat.com/show_bug.cgi?id=783317#c1
- Update to git snapshot 11bf6ad40dd93eda1f880a99d592009ea3ff47ac
- Include LICENSE

* Thu Jan 19 2012 Mario Santagiuliana <fedora@marionline.it> 0.2-9.20120119git754771b
- Create spec file for Fedora Review
- Source package create from git snapshot 754771b6081b194aedf750fac76a9af2709a5de3

* Wed Nov 16 2011 Dan Vratil <dan@progdan.cz> 0.2-8.1
- Initial SPEC
