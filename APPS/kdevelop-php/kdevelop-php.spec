Name:           kdevelop-php
Summary:        Php language and documentation plugins for KDevelop
Version:        22.08.1
Release:        1%{?dist}

# Most files LGPLv2+/GPLv2+
License:        GPLv2+
URL:            http://www.kde.org/
Source0:        https://download.kde.org/stable/release-service/%{version}/src/kdev-php-%{version}.tar.xz
Source1:        https://download.kde.org/stable/release-service/%{version}/src/kdev-php-%{version}.tar.xz.sig
Source2:        gpgkey-D81C0CB38EB725EF6691C385BB463350D6EF31EF.gpg

## kdevelop-pg-qt FTBFS s390x
ExcludeArch: s390x

BuildRequires:  gnupg2
BuildRequires:  gettext
BuildRequires:  kdevelop-pg-qt-devel >= 1.90.91
BuildRequires:  kdevplatform-devel >= %{version}

BuildRequires:  kf5-rpm-macros
BuildRequires:  extra-cmake-modules
BuildRequires:  grantlee-qt5-devel
BuildRequires:  kf5-knotifyconfig-devel
BuildRequires:  kf5-knewstuff-devel
BuildRequires:  kf5-kdelibs4support-devel
BuildRequires:  kf5-threadweaver-devel
BuildRequires:  kf5-ktexteditor-devel
BuildRequires:  cmake(KF5SyntaxHighlighting)
BuildRequires:  kf5-ki18n-devel
BuildRequires:  kf5-kcmutils-devel

BuildRequires:  qt5-qtbase-devel
BuildRequires:  qt5-qtwebkit-devel

%{?kdevelop_requires}

%description
%{summary}.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%setup -q -n kdev-php-%{version}


%build
%cmake_kf5
%cmake_build


%install
%cmake_install

# TODO Enable translations in stable build
%find_lang %{name} --all-name


%files -f %{name}.lang
%doc AUTHORS
%license LICENSES/*.txt
%{_datadir}/kdevappwizard/
%{_datadir}/kdevphpsupport/
%{_datadir}/kservices5/*.desktop
%{_includedir}/kdev-php/*
%{_libdir}/libkdevphp*.so
%{_libdir}/cmake/KDevPHP/*.cmake
%{_kf5_qtplugindir}/kdevplatform/
%{_datadir}/qlogging-categories5/kdevphpsupport.categories
%{_datadir}/metainfo/org.kde.kdev-php.metainfo.xml


%changelog
* Thu Sep 08 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.08.1-1
- 22.08.1

* Fri Aug 19 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.08.0-1
- 22.08.0

* Thu Jul 07 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.04.3-1
- 22.04.3

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

* Thu Jul 22 2021 Fedora Release Engineering <releng@fedoraproject.org> - 5.6.2-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_35_Mass_Rebuild

* Tue Mar 30 2021 Jonathan Wakely <jwakely@redhat.com> - 5.6.2-2
- Rebuilt for removed libstdc++ symbol (#1937698)

* Tue Feb 02 2021 Jan Grulich <jgrulich@redhat.com> - 5.6.2-1
- 5.6.2

* Tue Jan 26 2021 Fedora Release Engineering <releng@fedoraproject.org> - 5.6.1-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_34_Mass_Rebuild

* Wed Dec  9 10:49:22 CET 2020 Jan Grulich <jgrulich@redhat.com> - 5.6.1-1
- 5.6.1

* Tue Sep 08 2020 Jan Grulich <jgrulich@redhat.com> - 5.6.0-1
- 5.6.0

* Tue Jul 28 2020 Fedora Release Engineering <releng@fedoraproject.org> - 5.5.2-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_33_Mass_Rebuild

* Tue Jun 02 2020 Jan Grulich <jgrulich@redhat.com> - 5.5.2-1
- 5.5.2

* Wed May 06 2020 Jan Grulich <jgrulich@redhat.com> - 5.5.1-1
- 5.5.1

* Mon Feb 03 2020 Jan Grulich <jgrulich@redhat.com> - 5.5.0-1
- 5.5.0

* Wed Jan 29 2020 Fedora Release Engineering <releng@fedoraproject.org> - 5.4.6-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_32_Mass_Rebuild

* Tue Jan 07 2020 Jan Grulich <jgrulich@redhat.com> - 5.4.6-1
- 5.4.6

* Tue Dec 03 2019 Jan Grulich <jgrulich@redhat.com> - 5.4.5-1
- 5.4.5

* Tue Nov 05 2019 Jan Grulich <jgrulich@redhat.com> - 5.4.4-1
- 5.4.4

* Tue Oct 22 2019 Jan Grulich <jgrulich@redhat.com> - 5.4.3-1
- 5.4.3

* Tue Sep 03 2019 Jan Grulich <jgrulich@redhat.com> - 5.4.2-1
- 5.4.2

* Tue Aug 13 2019 Jan Grulich <jgrulich@redhat.com> - 5.4.1-1
- 5.4.1

* Wed Aug 07 2019 Jan Grulich <jgrulich@redhat.com> - 5.4.0-1
- 5.4.0

* Thu Jul 25 2019 Fedora Release Engineering <releng@fedoraproject.org> - 5.3.3-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_31_Mass_Rebuild

* Thu Jul 18 2019 Jan Grulich <jgrulich@redhat.com> - 5.3.3-1
- 5.3.3

* Fri Mar 15 2019 Jan Grulich <jgrulich@redhat.com> - 5.3.2-1
- 5.3.2

* Fri Feb 01 2019 Fedora Release Engineering <releng@fedoraproject.org> - 5.3.1-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_30_Mass_Rebuild

* Tue Dec 11 2018 Jan Grulich <jgrulich@redhat.com> - 5.3.1-1
- 5.3.1

* Wed Nov 14 2018 Jan Grulich <jgrulich@redhat.com> - 5.3.0-1
- 5.3.0

* Tue Oct 02 2018 Jan Grulich <jgrulich@redhat.com> - 5.2.80-1
- 5.2.80 (beta)

* Mon Aug 27 2018 Jan Grulich <jgrulich@redhat.com> - 5.2.4-1
- 5.2.4

* Fri Jul 13 2018 Fedora Release Engineering <releng@fedoraproject.org> - 5.2.3-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_29_Mass_Rebuild

* Mon May 21 2018 Jan Grulich <jgrulich@redhat.com> - 5.2.3-1
- Update to 5.2.3

* Wed Feb 07 2018 Fedora Release Engineering <releng@fedoraproject.org> - 5.2.1-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_28_Mass_Rebuild

* Fri Nov 24 2017 Jan Grulich <jgrulich@redhat.com> - 5.2.1-1
- Update to 5.2.1

* Tue Nov 14 2017 Jan Grulich <jgrulich@redhat.com> - 5.2.0-1
- Update to 5.2.0

* Fri Oct 06 2017 Jan Grulich <jgrulich@redhat.com> - 5.1.80-1
- Update to 5.1.80 (beta)

* Tue Aug 29 2017 Jan Grulich <jgrulich@redhat.com> - 5.1.2-1
- Update to 5.1.2

* Thu Aug 03 2017 Fedora Release Engineering <releng@fedoraproject.org> - 5.1.1-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Binutils_Mass_Rebuild

* Wed Jul 26 2017 Fedora Release Engineering <releng@fedoraproject.org> - 5.1.1-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Mass_Rebuild

* Mon May 29 2017 Jan Grulich <jgrulich@redhat.com> - 5.1.1-1
- Update to 5.1.1

* Mon Mar 20 2017 Jan Grulich <jgrulich@redhat.com> - 5.1.0-1
- Update to 5.1.0

* Fri Feb 10 2017 Fedora Release Engineering <releng@fedoraproject.org> - 5.0.80-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_26_Mass_Rebuild

* Tue Jan 17 2017 Jan Grulich <jgrulich@redhat.com> - 5.0.80-1
- Update to 5.0.80 (beta)

* Fri Dec 02 2016 Jan Grulich <jgrulich@redhat.com> - 5.0.3-1
- Update to 5.0.3

* Mon Oct 17 2016 Jan Grulich <jgrulich@redhat.com> - 5.0.2-1
- Update to 5.0.2

* Mon Sep 19 2016 Jan Grulich <jgrulich@redhat.com> - 5.0.1-1
- Update to 5.0.1

* Wed Sep 07 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.0.0-2
- use %%{?kdevelop_requires}, cosmetics

* Wed Aug 31 2016 Helio Chissini de Castro <helio@kde.org> - 5.0.0-1
- New upstream version 5.0.0

* Wed Jun 08 2016 Jan Grulich <jgrulich@redhat.com> - 5.0.0-0.1.20160608git
- Package latest git snapshot to address GCC 6 related crashes
  Resolves: bz#1343439

* Thu Feb 04 2016 Fedora Release Engineering <releng@fedoraproject.org> - 4.90.91-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_24_Mass_Rebuild

* Tue Jan 26 2016 Jan Grulich <jgrulich@redhat.com> - 4.90.91-1
- Update to 4.90.91 (beta 2)

* Fri Oct 30 2015 Jan Grulich <jgrulich@redhat.com> - 4.90.90-1
- Update to 4.90.90 (beta 1)

* Tue Oct 13 2015 Jan Grulich <jgrulich@redhat.com> - 1.7.2-2
- Bump required kdevelop version

* Mon Oct 12 2015 Jan Grulich <jgrulich@redhat.com> - 1.7.2-1
- Update to 1.7.2

* Wed Jun 17 2015 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 1.7.1-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_23_Mass_Rebuild

* Sat May 02 2015 Kalev Lember <kalevlember@gmail.com> - 1.7.1-2
- Rebuilt for GCC 5 C++11 ABI change

* Wed Feb 04 2015 Jan Grulich <jgrulich@redhat.com> - 1.7.1-1
- Update to 1.7.1

* Fri Sep 26 2014 Kevin Kofler <Kevin@tigcc.ticalc.org> - 1.7.0-2
- Version the Requires: kdevelop correctly

* Wed Aug 27 2014 Jan Grulich <jgrulich@redhat.com> - 1.7.0-1
- Update to 1.7.0

* Fri Jul 11 2014 Jan Grulich <jgrulich@redhat.com> - 1.6.90-1
- Update to 1.6.90

* Sun Jun 08 2014 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 1.6.0-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_21_Mass_Rebuild

* Sun Dec 08 2013 Jan Grulich <jgrulich@redhat.com> - 1.6.0-1
- Update to 1.6.0

* Wed Nov 27 2013 Jan Grulich <jgrulich@redhat.com> - 1.5.90-1
- Update to 1.5.90

* Mon Nov 18 2013 Jan Grulich <jgrulich@redhat.com> - 1.5.80-1
- Update to 1.5.80

* Thu Oct 31 2013 Jan Grulich <jgrulich@redhat.com> - 1.5.2-1
- Update to 1.5.2

* Sat Aug 03 2013 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 1.5.1-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_20_Mass_Rebuild

* Thu May 30 2013 Jan Grulich <jgrulich@redhat.com> - 1.5.1-1
- Update to 1.5.1

* Fri Apr 26 2013 Jan Grulich <jgrulich@redhat.com> 1.5.0-1
- Update to 1.5.0

* Thu Apr 25 2013 Jan Grulich <jgrulich@redhat.com> 1.4.90-1
- Update to 1.4.90 (RC1)

* Thu Feb 14 2013 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 1.4.1-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_19_Mass_Rebuild

* Mon Nov 05 2012 Jan Grulich <jgrulich@redhat.com> 1.4.1-1
- Update to 1.4.1

* Wed Oct 24 2012 Radek Novacek <rnovacek@redhat.com> 1.4.0-1
- Update to 1.4.0

* Fri Sep 07 2012 Radek Novacek <rnovacek@redhat.com> 1.3.90-1
- Update to 1.3.90 (RC 1)

* Thu Aug 09 2012 Radek Novacek <rnovacek@redhat.com> 1.3.80-1
- Update to 1.3.80 (beta 1)

* Thu Jul 19 2012 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 1.3.1-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_18_Mass_Rebuild

* Mon Apr 16 2012 Radek Novacek <rnovacek@redhat.com> 1.3.1-1
- Update to 1.3.1

* Tue Mar 13 2012 Than Ngo <than@redhat.com> - 1.3.0-2
- add missing kdevelop-php-docs

* Mon Mar 12 2012 Jaroslav Reznik <jreznik@redhat.com> 1.3.0-1
- Update to 1.3.0

* Mon Feb 27 2012 Radek Novacek <rnovacek@redhat.com> 1.2.90-2
- Rebuild for kdevelop-pg-qt 1.0.0

* Sun Feb 26 2012 Radek Novacek <rnovacek@redhat.com> 1.2.90-1
- Update to 1.2.90 (RC 1)

* Tue Feb 14 2012 Jaroslav Reznik <jreznik@redhat.com> 1.2.82-1
- Update to 1.2.82 (beta 2)

* Mon Jan 23 2012 Radek Novacek <rnovacek@redhat.com> 1.2.81-1
- Update to 1.2.81 (1.3 beta)

* Fri Jan 13 2012 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 1.2.3-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_17_Mass_Rebuild

* Tue Jun 28 2011 Radek Novacek <rnovacek@redhat.com> 1.2.3-1
- Update to 1.2.3

* Mon Apr 11 2011 Radek Novacek <rnovacek@redhat.com> 1.2.2-2
- BuildRequires kdevelop-pg-qt >= 0.9.5

* Thu Apr 07 2011 Radek Novacek <rnovacek@redhat.com> 1.2.2-1
- Update to 1.2.2

* Mon Feb 07 2011 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 1.2.0-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_15_Mass_Rebuild

* Tue Jan 25 2011 Rex Dieter <rdieter@fedoraproject.org> - 1.2.0-1
- 1.2.0

* Fri Jan 21 2011 Kevin Kofler <Kevin@tigcc.ticalc.org> - 1.1.90-1
- Update to 1.1.90 (1.2 RC1)

* Wed Jan 05 2011 Rex Dieter <rdieter@fedoraproject.org> - 1.1.81-1
- 1.1.81
- License: GPLv2+

* Fri Dec 10 2010 Rex Dieter <rdieter@fedoraproject.org> - 1.1.1-3
- License: GPLv3+

* Fri Dec 10 2010 Rex Dieter <rdieter@fedoraproject.org> -  1.1.1-2
- License: GPLv2+

* Mon Dec 06 2010 Rex Dieter <rdieter@fedoraproject.org> - 1.1.1-1
- first try

