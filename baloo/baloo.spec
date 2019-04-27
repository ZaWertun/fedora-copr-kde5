
%if ! 0%{?bootstrap}
%if 0%{?fedora} > 21
%define plasma5 1
%endif
%endif

%if 0%{?fedora} > 23
%define kf5_akonadi 1
%endif

Name:    baloo
Summary: A framework for searching and managing metadata
Version: 4.14.3
Release: 21%{?dist}

License: GPLv2 and LGPLv2
URL:     https://cgit.kde.org/?p=%{name}.git

%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0: http://download.kde.org/%{stable}/%{version}/src/%{name}-%{version}.tar.xz

%if 0%{?fedora} > 17 || 0%{?rhel} > 6
%define sysctl 1
%endif
Source1: 97-kde-baloo-filewatch-inotify.conf

## upstream patches

## downstream patches
Patch1: baloo-4.14.3-no_pim.patch
Patch2: baloo-4.14.3-no_tools.patch

# For AutoReq cmake-filesystem
BuildRequires: cmake
BuildRequires: doxygen
BuildRequires: kdelibs4-devel >= %{version}
BuildRequires: kdepimlibs-devel >= %{version}
BuildRequires: pkgconfig(QJson)
# for %%{_polkit_qt_policydir} macro
BuildRequires: polkit-qt-devel
BuildRequires: xapian-core-devel

# kio_tags/kio_timeline moved here from kde-runtime
Conflicts: kde-runtime < 4.12.90

Requires: %{name}-libs%{?_isa} = %{version}-%{release}

%description
%{summary}.

%if ! 0%{?kf5_akonadi}
%package file
Summary: File indexing and search for %{name}
BuildRequires: kfilemetadata-devel >= %{version}
# upgrade path, when split occurred
Obsoletes: baloo < 4.13.0-3
Obsoletes: nepomuk-core < 4.13.0-2
Requires: %{name}-libs%{?_isa} = %{version}-%{release}
# for kcm.  since this is split out, we can afford to add this dep
# and not worry about circular dependencies
Requires: kde-runtime
%description file
%{summary}.
%endif

%package devel
Summary:  Developer files for %{name}
Requires: %{name}-libs%{?_isa} = %{version}-%{release}
Requires: kdelibs4-devel
%description devel
%{summary}.

%package libs
Summary:  Runtime libraries for %{name}
%if 0%{?kf5_akonadi}
Obsoletes: baloo-akonadi < %{version}-%{release}
%endif
%{?kde4_version:Requires: kdelibs4%{?_isa} >= %{_kde4_version}}
%description libs
%{summary}.

%if ! 0%{?kf5_akonadi}
%package akonadi
Summary: Baloo indexer and search plugin for Akonadi
BuildRequires: pkgconfig(akonadi) >= 1.12.1
BuildRequires: akonadi-devel < 1.13.0-100
Requires: %{name}-libs%{?_isa} = %{version}-%{release}
# Upgrade path
Obsoletes: %{name} < 4.14.0-3
%description akonadi
%{summary}.
%endif


%prep
%setup -q

%if 0%{?kf5_akonadi}
%patch1 -p1 -b .no_pim
%endif

%if 0%{?plasma5}
%patch2 -p1 -b .no_tools
%endif


%build
mkdir %{_target_platform}
pushd %{_target_platform}
%{cmake_kde4} ..
popd

%make_build -C %{_target_platform}


%install
make install/fast DESTDIR=%{buildroot} -C %{_target_platform}

%if 0%{?plasma5}
## unpackaged files
rm -fv %{buildroot}%{_sysconfdir}/dbus-1/system.d/org.kde.baloo.filewatch.conf
rm -fv %{buildroot}%{_datadir}/dbus-1/system-services/org.kde.baloo.filewatch.service
rm -fv %{buildroot}%{_datadir}/dbus-1/interfaces/org.kde.baloo.file.indexer.xml
rm -fv %{buildroot}%{_kde4_bindir}/baloo*
rm -fv %{buildroot}%{_kde4_iconsdir}/hicolor/*/apps/baloo.*
rm -fv %{buildroot}%{_kde4_datadir}/autostart/baloo_file.desktop
rm -fv %{buildroot}%{_kde4_libdir}/kde4/{baloo_filesearchstore,kcm_baloofile}.so
rm -fv %{buildroot}%{_kde4_libexecdir}/kde_baloo_filewatch_raiselimit
rm -fv %{buildroot}%{_kde4_datadir}/kde4/services/{baloo_filesearchstore,kcm_baloofile}.desktop
rm -fv %{buildroot}%{_polkit_qt_policydir}/org.kde.baloo.filewatch.policy
%else
%if 0%{?sysctl}
install -p -m644 -D %{SOURCE1} %{buildroot}%{_prefix}/lib/sysctl.d/97-kde-baloo-filewatch-inotify.conf
install -p -m644 -D %{SOURCE1} %{buildroot}%{_sysconfdir}/sysctl.d/97-kde-baloo-filewatch-inotify.conf
%else
install -p -m644    %{SOURCE1} ./97-kde-baloo-filewatch-inotify.conf
%endif
%endif


%if ! 0%{?plasma5}
%files
%license COPYING COPYING.LIB
%{_kde4_bindir}/balooctl
%{_kde4_bindir}/baloosearch
%{_kde4_bindir}/balooshow
%{_kde4_iconsdir}/hicolor/*/apps/baloo.*

%post file
if [ -f "%{_sysconfdir}/sysctl.d/97-kde-nepomuk-filewatch-inotify.conf" ]; then
  mv -f "%{_sysconfdir}/sysctl.d/97-kde-nepomuk-filewatch-inotify.conf" && \
        "%{_sysconfdir}/sysctl.d/97-kde-nepomuk-filewatch-inotify.conf.rpmsave"
fi

%files file
%if 0%{?sysctl}
%ghost %config(missingok,noreplace) %{_sysconfdir}/sysctl.d/97-kde-baloo-filewatch-inotify.conf
%{_prefix}/lib/sysctl.d/97-kde-baloo-filewatch-inotify.conf
%else
%doc 97-kde-baloo-filewatch-inotify.conf
%endif
%{_kde4_datadir}/autostart/baloo_file.desktop
%{_kde4_bindir}/baloo_file
%{_kde4_bindir}/baloo_file_cleaner
%{_kde4_bindir}/baloo_file_extractor
%{_datadir}/dbus-1/interfaces/org.kde.baloo.file.indexer.xml
%{_kde4_libexecdir}/kde_baloo_filewatch_raiselimit
%{_sysconfdir}/dbus-1/system.d/org.kde.baloo.filewatch.conf
%{_datadir}/dbus-1/system-services/org.kde.baloo.filewatch.service
%{_polkit_qt_policydir}/org.kde.baloo.filewatch.policy
%{_kde4_libdir}/kde4/baloo_filesearchstore.so
%{_kde4_datadir}/kde4/services/baloo_filesearchstore.desktop
%{_kde4_datadir}/kde4/services/kcm_baloofile.desktop
%{_kde4_libdir}/kde4/kcm_baloofile.so
%endif

%if ! 0%{?kf5_akonadi}
%files akonadi
%{_kde4_bindir}/akonadi_baloo_indexer
%{_kde4_datadir}/akonadi/agents/akonadibalooindexingagent.desktop
%{_kde4_libdir}/kde4/akonadi/akonadi_baloo_searchplugin.so
%{_kde4_libdir}/kde4/akonadi/akonadibaloosearchplugin.desktop
%{_kde4_libdir}/kde4/baloo_calendarsearchstore.so
%{_kde4_libdir}/kde4/baloo_contactsearchstore.so
%{_kde4_libdir}/kde4/baloo_emailsearchstore.so
%{_kde4_libdir}/kde4/baloo_notesearchstore.so
%{_kde4_datadir}/kde4/services/baloo_calendarsearchstore.desktop
%{_kde4_datadir}/kde4/services/baloo_contactsearchstore.desktop
%{_kde4_datadir}/kde4/services/baloo_emailsearchstore.desktop
%{_kde4_datadir}/kde4/services/baloo_notesearchstore.desktop
%endif

%files devel
%{_kde4_includedir}/baloo/
%{_kde4_libdir}/libbaloocore.so
%{_kde4_libdir}/libbaloopim.so
%{_kde4_libdir}/cmake/Baloo/
%if ! 0%{?plasma5}
%{_kde4_libdir}/libbaloofiles.so
%{_kde4_libdir}/libbalooqueryparser.so
%{_kde4_libdir}/libbalooxapian.so
%endif

%ldconfig_scriptlets libs

%files libs
%{_kde4_libdir}/libbaloocore.so.4*
%{_kde4_datadir}/kde4/servicetypes/baloosearchstore.desktop
%{_kde4_libdir}/libbaloopim.so.4*
%if ! 0%{?plasma5}
%{_kde4_libdir}/libbaloofiles.so.4*
%{_kde4_libdir}/libbalooqueryparser.so.4*
%{_kde4_libdir}/libbalooxapian.so.4*
%{_kde4_datadir}/kde4/services/baloosearch.protocol
%{_kde4_datadir}/kde4/services/plasma-runner-baloosearch.desktop
%{_kde4_datadir}/kde4/services/tags.protocol
%{_kde4_datadir}/kde4/services/timeline.protocol
%{_kde4_libdir}/kde4/kio_baloosearch.so
%{_kde4_libdir}/kde4/kio_tags.so
%{_kde4_libdir}/kde4/kio_timeline.so
%{_kde4_libdir}/kde4/krunner_baloosearchrunner.so
%endif


%changelog
* Thu Jul 12 2018 Fedora Release Engineering <releng@fedoraproject.org> - 4.14.3-21
- Rebuilt for https://fedoraproject.org/wiki/Fedora_29_Mass_Rebuild

* Thu Jun 28 2018 Rex Dieter <rdieter@fedoraproject.org> - 4.14.3-20
- use %%make_build %%license %%ldconfig_scriptlets

* Wed Feb 07 2018 Fedora Release Engineering <releng@fedoraproject.org> - 4.14.3-19
- Rebuilt for https://fedoraproject.org/wiki/Fedora_28_Mass_Rebuild

* Sun Jan 07 2018 Igor Gnatenko <ignatenkobrain@fedoraproject.org> - 4.14.3-18
- Remove obsolete scriptlets

* Sun Aug 06 2017 Björn Esser <besser82@fedoraproject.org> - 4.14.3-17
- Rebuilt for AutoReq cmake-filesystem

* Wed Aug 02 2017 Fedora Release Engineering <releng@fedoraproject.org> - 4.14.3-16
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Binutils_Mass_Rebuild

* Wed Jul 26 2017 Fedora Release Engineering <releng@fedoraproject.org> - 4.14.3-15
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Mass_Rebuild

* Thu Mar 30 2017 Rex Dieter <rdieter@fedoraproject.org> - 4.14.3-14
- move kfilemetadata dep to -file

* Fri Mar 24 2017 Rex Dieter <rdieter@fedoraproject.org> - 4.14.3-13
- update URL, build even more minimally, libbaloo{core|pim}

* Fri Feb 10 2017 Fedora Release Engineering <releng@fedoraproject.org> - 4.14.3-12
- Rebuilt for https://fedoraproject.org/wiki/Fedora_26_Mass_Rebuild

* Fri Dec 23 2016 Rex Dieter <rdieter@math.unl.edu> - 4.14.3-11
- rebuild (xapian)

* Wed May 11 2016 Rex Dieter <rdieter@fedoraproject.org> - 4.14.3-10
- -libs: Obsoletes: baloo-akonadi on f24+ (#1334533)

* Sat Apr 30 2016 Rex Dieter <rdieter@fedoraproject.org> - 4.14.3-9
- kdepim4 needs libbaloopim (f24+)

* Fri Apr 29 2016 Rex Dieter <rdieter@fedoraproject.org> - 4.14.3-8
- -libs: drop Requires: baloo (#1329899)
- update URL

* Fri Feb 19 2016 Rex Dieter <rdieter@fedoraproject.org> - 4.14.3-7
- omit akonadi/pim related bits (f24+)
- baloo FTBFS on rawhide (#1307340)

* Wed Feb 03 2016 Fedora Release Engineering <releng@fedoraproject.org> - 4.14.3-6
- Rebuilt for https://fedoraproject.org/wiki/Fedora_24_Mass_Rebuild

* Wed Jun 17 2015 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 4.14.3-5
- Rebuilt for https://fedoraproject.org/wiki/Fedora_23_Mass_Rebuild

* Sat May 23 2015 Rex Dieter <rdieter@fedoraproject.org> - 4.14.3-4
- update URL
- omit main (baloo) pkg on f22+ (where it is replaced by kf5-baloo)

* Thu Apr 16 2015 Rex Dieter <rdieter@fedoraproject.org> 4.14.3-3
- rebuild (gcc5)

* Sun Mar 01 2015 Rex Dieter <rdieter@fedoraproject.org> 4.14.3-2
- rebuild (gcc5)

* Sat Nov 08 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.14.3-1
- 4.14.3

* Sat Oct 11 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.14.2-1
- 4.14.2

* Mon Sep 15 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.14.1-1
- 4.14.1

* Tue Aug 19 2014 Daniel Vrátil <dvratil@redhat.com> - 4.14.0-4
- Fix upgrade path for -akonadi (Obosoletes baloo < 4.14.0-3)

* Mon Aug 18 2014 Daniel Vrátil <dvratil@redhat.com> - 4.14.0-3
- Improve coinstallability with kf5-baloo
- Move Akonadi indexer to -akonadi subpackage
- Move stores and plugins to -libs subpackage

* Fri Aug 15 2014 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 4.14.0-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_21_22_Mass_Rebuild

* Thu Aug 14 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.14.0-1
- 4.14.0

* Tue Aug 05 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.13.97-1
- 4.13.97

* Mon Jul 14 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.13.3-1
- 4.13.3

* Mon Jun 09 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.13.2-1
- 4.13.2

* Sat Jun 07 2014 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 4.13.1-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_21_Mass_Rebuild

* Sat May 10 2014 Rex Dieter <rdieter@fedoraproject.org> 4.13.1-1
- 4.13.1

* Thu May 01 2014 Rex Dieter <rdieter@fedoraproject.org> 4.13.0-5
- sync upstream patches/commits (using %%autosetup)

* Wed Apr 23 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.13.0-4
- -file: Obsoletes: nepomuk-core (upgrade path)
- -file: %%post scriptlet to remove nepomuk inotify sysctl

* Wed Apr 23 2014 Rex Dieter <rdieter@fedoraproject.org> 4.13.0-3
- -file subpkg, port sysctl handling from nepomuk-core

* Tue Apr 15 2014 Rex Dieter <rdieter@fedoraproject.org> 4.13.0-2
- respin

* Sat Apr 12 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.13.0-1
- 4.13.0

* Thu Apr 03 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.12.97-1
- 4.12.97

* Sat Mar 22 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.12.95-1
- 4.12.95

* Mon Mar 17 2014 Rex Dieter <rdieter@fedoraproject.org> 4.12.90-1
- baloo-4.12.90, first try

