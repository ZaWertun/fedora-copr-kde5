Name:           skanlite
Version:        22.08.1
Release:        1%{?dist}
Summary:        Lightweight scanning program
# Actually: GPLv2 or GPLv3 or any later Version approved by KDE e.V.
License:        GPLv2 or GPLv3
URL:            https://www.kde.org/applications/graphics/%{name}/
Source0:        http://download.kde.org/stable/release-service/%{version}/src/%{name}-%{version}.tar.xz
Source1:        http://download.kde.org/stable/release-service/%{version}/src/%{name}-%{version}.tar.xz.sig
Source2:        gpgkey-D81C0CB38EB725EF6691C385BB463350D6EF31EF.gpg

BuildRequires:  gnupg2
BuildRequires:  cmake
BuildRequires:  cmake(KF5CoreAddons)
BuildRequires:  cmake(KF5DocTools)
BuildRequires:  cmake(KF5I18n)
BuildRequires:  cmake(KF5KIO)
BuildRequires:  cmake(KF5Sane)
BuildRequires:  cmake(KF5XmlGui)
BuildRequires:  desktop-file-utils
BuildRequires:  extra-cmake-modules
BuildRequires:  gcc-c++
BuildRequires:  libappstream-glib
BuildRequires:  libpng-devel
BuildRequires:  qt5-qtbase-devel
BuildRequires:  cmake(KSaneCore)

Requires:       kde-filesystem
Requires:       khotkeys


%description
Skanlite is a light-weight scanning application based on libksane.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -p1


%build
%cmake
%cmake_build


%install
%cmake_install
%find_lang %{name} --all-name --with-html
install -Dpm 0644 hotkeys_and_scripts/%{name}.khotkeys %{buildroot}%{_kf5_datadir}/khotkeys/%{name}.khotkeys


%check
desktop-file-validate %{buildroot}%{_datadir}/applications/org.kde.%{name}.desktop
appstream-util validate-relax --nonet %{buildroot}/%{_datadir}/metainfo/org.kde.%{name}.appdata.xml


%files -f %{name}.lang
%license LICENSES/*.txt
%{_kf5_bindir}/%{name}
%{_kf5_datadir}/khotkeys/%{name}.khotkeys
%{_kf5_datadir}/applications/org.kde.%{name}.desktop
%{_kf5_datadir}/icons/hicolor/*/apps/org.kde.%{name}.svg
%{_kf5_metainfodir}/org.kde.%{name}.appdata.xml


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

* Sun Mar 20 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 21.12.3-1
- 21.12.3

* Fri Jul 23 2021 Fedora Release Engineering <releng@fedoraproject.org> - 2.2.0-4
- Rebuilt for https://fedoraproject.org/wiki/Fedora_35_Mass_Rebuild

* Wed Jan 27 2021 Fedora Release Engineering <releng@fedoraproject.org> - 2.2.0-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_34_Mass_Rebuild

* Wed Jul 29 2020 Fedora Release Engineering <releng@fedoraproject.org> - 2.2.0-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_33_Mass_Rebuild

* Mon Jul 13 2020 Sandro Mani <manisandro@gmail.com> - 2.2.0-1
- Update to 2.2.0

* Thu Jan 30 2020 Fedora Release Engineering <releng@fedoraproject.org> - 2.1.0.1-4
- Rebuilt for https://fedoraproject.org/wiki/Fedora_32_Mass_Rebuild

* Fri Jul 26 2019 Fedora Release Engineering <releng@fedoraproject.org> - 2.1.0.1-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_31_Mass_Rebuild

* Sat Feb 02 2019 Fedora Release Engineering <releng@fedoraproject.org> - 2.1.0.1-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_30_Mass_Rebuild

* Sun Nov 04 2018 Sandro Mani <manisandro@gmail.com> - 2.1.0.1-1
- Modernize spec
- Update to 2.1.0.1

* Sat Jul 14 2018 Fedora Release Engineering <releng@fedoraproject.org> - 1.1-13
- Rebuilt for https://fedoraproject.org/wiki/Fedora_29_Mass_Rebuild

* Fri Feb 09 2018 Fedora Release Engineering <releng@fedoraproject.org> - 1.1-12
- Rebuilt for https://fedoraproject.org/wiki/Fedora_28_Mass_Rebuild

* Thu Aug 03 2017 Fedora Release Engineering <releng@fedoraproject.org> - 1.1-11
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Binutils_Mass_Rebuild

* Thu Jul 27 2017 Fedora Release Engineering <releng@fedoraproject.org> - 1.1-10
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Mass_Rebuild

* Sat Feb 11 2017 Fedora Release Engineering <releng@fedoraproject.org> - 1.1-9
- Rebuilt for https://fedoraproject.org/wiki/Fedora_26_Mass_Rebuild

* Fri Nov 11 2016 Oroin Poplawsi <orion@cora.nwra.com> - 1.1-8
- Update URL and Source URL
- Validate appdata file
- Use %%license
- Cleanup spec

* Fri Feb 05 2016 Fedora Release Engineering <releng@fedoraproject.org> - 1.1-7
- Rebuilt for https://fedoraproject.org/wiki/Fedora_24_Mass_Rebuild

* Fri Jun 19 2015 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 1.1-6
- Rebuilt for https://fedoraproject.org/wiki/Fedora_23_Mass_Rebuild

* Sat May 02 2015 Kalev Lember <kalevlember@gmail.com> - 1.1-5
- Rebuilt for GCC 5 C++11 ABI change

* Thu Mar 26 2015 Richard Hughes <rhughes@redhat.com> - 1.1-4
- Add an AppData file for the software center

* Mon Aug 18 2014 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 1.1-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_21_22_Mass_Rebuild

* Sun Jun 08 2014 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 1.1-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_21_Mass_Rebuild

* Sun Jan 05 2014 Sven Lankes <sven@lank.es> - 1.1-1
- new upstream release

* Sun Aug 04 2013 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 0.8-6
- Rebuilt for https://fedoraproject.org/wiki/Fedora_20_Mass_Rebuild

* Fri Feb 15 2013 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 0.8-5
- Rebuilt for https://fedoraproject.org/wiki/Fedora_19_Mass_Rebuild

* Sat Jul 21 2012 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 0.8-4
- Rebuilt for https://fedoraproject.org/wiki/Fedora_18_Mass_Rebuild

* Sat Jan 14 2012 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 0.8-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_17_Mass_Rebuild

* Mon Dec 26 2011 Sven Lankes <sven@lank.es> - 0.8-2
- add libpng br

* Mon Dec 26 2011 Sven Lankes <sven@lank.es> - 0.8-1
- skanlite 0.8

* Tue Dec 06 2011 Adam Jackson <ajax@redhat.com> - 0.7-4
- Rebuild for new libpng

* Fri Jul 01 2011 Rex Dieter <rdieter@fedoraproject.org> 0.7-3
- s/kdegraphics-devel/pkgconfig(libksane)/
- use desktop-file-validate

* Wed Feb 09 2011 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 0.7-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_15_Mass_Rebuild

* Thu Dec 16 2010 Sven Lankes <sven@lank.es> - 0.7-1
- skanlite 0.7

* Fri Feb 12 2010 Rex Dieter <rdieter@fedoraproject.org> - 0.4-1
- skanlite-0.4-kde-4.4.0

* Wed Nov 25 2009 Rex Dieter <rdieter@fedoraproject.org> - 0.3-6 
- rebuild (kdegraphics)
- use %%find_lang --with-kde

* Tue Sep 01 2009 Sebastian Vahl <svahl@fedoraproject.org> - 0.3-5
- KDE 4.3.1

* Tue Aug 11 2009 Sebastian Vahl <fedora@deadbabylon.de> - 0.3-4
- KDE 4.3.0

* Sun Jul 26 2009 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 0.3-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_12_Mass_Rebuild

* Tue Jun 23 2009 Sven Lankes <sven@lank.es> - 0.3-2
- SPEC-Fixes from review

* Mon Jun 22 2009 Sven Lankes <sven@lank.es> - 0.3-1 
- Update to current upstream version
- Update license tag

* Thu Jan 08 2009 Teemu Rytilahti <tpr@d5k.net> - 0.2-2
- use source package from the kde's ftp-site instead of the svn

* Wed Jan 07 2009 Teemu Rytilahti <tpr@d5k.net> - 0.2-1
- initial package
