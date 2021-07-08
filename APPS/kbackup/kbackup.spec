%undefine __cmake_in_source_build
%global optflags %{optflags} -flto=auto
%global build_ldflags %{build_ldflags} -flto

Name:           kbackup
Version:        21.04.3
Release:        1%{?dist}
Summary:        Back up your data in a simple, user friendly way
Summary(fr):    Sauvegarder vos données de manière simple et conviviale
Summary(ru):    Простое, дружественное к пользователю резервное копирование

License:        GPLv2+
Url:            https://kde.org/applications/utilities/org.kde.kbackup

%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0: https://download.kde.org/%{stable}/release-service/%{version}/src/%{name}-%{version}.tar.xz

BuildRequires:  gcc-c++
BuildRequires:  cmake
BuildRequires:  extra-cmake-modules
BuildRequires:  cmake(Qt5Core)
BuildRequires:  cmake(Qt5Widgets)
BuildRequires:  cmake(KF5Notifications)
BuildRequires:  cmake(KF5I18n)
BuildRequires:  cmake(KF5KIO)
BuildRequires:  cmake(KF5DocTools)
BuildRequires:  cmake(KF5XmlGui)
BuildRequires:  cmake(KF5IconThemes)
BuildRequires:  cmake(KF5Archive)
BuildRequires:  cmake(KF5WidgetsAddons)
BuildRequires:  desktop-file-utils
BuildRequires:  libappstream-glib
BuildRequires:  gettext
BuildRequires:  kf5-rpm-macros

Requires:       hicolor-icon-theme

%description
KBackup is a program that lets you back up any directories or files,
whereby it uses an easy to use directory tree to select the things to back up.
The program was designed to be very simple in its use
so that it can be used by non-computer experts.
The storage format is the well known TAR format, whereby the data
is still stored in compressed format (bzip2 or gzip).

%description -l fr
KBackup est un programme qui vous permet de sauvegarder n'importe quels
fichiers ou répertoires que vous pouvez sélectionner dans une arborescence.
Il a été conçu pour être facile d'utilisation et est donc à la portée des
non-initiés à l'informatique.
Le format de stockage est le très connu format TAR, où les données sont
stockées compressées (bzip2 ou gzip).

%description -l ru
KBackup позволяет делать резервное копирование любых каталогов и файлов,
используя простое представление в виде дерева каталогов для выбора элементов
копирования.
Программа спроектирована очень простой в использовании даже не экспертами в
области компьютеров.
Формат хранения архивов - хорошо известный TAR, форматы сжатия bzip2 или gzip.

%prep
%setup -q

%build
%cmake_kf5 \
    -DCMAKE_AR=/usr/bin/gcc-ar \
    -DCMAKE_RANLIB=/usr/bin/gcc-ranlib \
    -DCMAKE_NM=/usr/bin/gcc-nm
%cmake_build

%install
%cmake_install

%find_lang %{name} --all-name --with-html --with-man

%check
appstream-util validate-relax --nonet %{buildroot}%{_datadir}/metainfo/org.kde.%{name}.appdata.xml
desktop-file-validate %{buildroot}%{_datadir}/applications/org.kde.%{name}.desktop

%files -f %{name}.lang
%license COPYING
%doc AUTHORS ChangeLog README
%{_bindir}/%{name}
%{_datadir}/metainfo/org.kde.%{name}.appdata.xml
%{_datadir}/applications/org.kde.%{name}.desktop
%{_datadir}/icons/hicolor/*/apps/*.png
%{_datadir}/icons/hicolor/*/actions/*.png
%{_datadir}/icons/hicolor/*/mimetypes/*.png
%{_datadir}/kxmlgui5/%{name}
%{_datadir}/mime/packages/%{name}.xml
%{_mandir}/man1/%{name}.1*


%changelog
* Thu Jul 08 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 21.04.3-1
- 21.04.3

* Fri Jun 11 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 21.04.2-1
- 21.04.2

* Thu May 13 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 21.04.1-1
- 21.04.1

* Thu Apr 22 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 21.04.0-1
- 21.04.0

* Sat Mar 06 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.12.3-1
- 20.12.3

* Thu Feb 04 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.12.2-1
- 20.12.2

* Thu Jan  7 22:09:14 MSK 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.12.1-1
- 20.12.1

* Thu Dec 10 21:56:05 MSK 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.12.0-1
- 20.12.0

* Fri Nov  6 13:25:33 MSK 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.08.3-1
- 20.08.3

* Fri Oct 09 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.08.2-1
- 20.08.2

* Thu Sep 03 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.08.1-1
- 20.08.1

* Fri Aug 14 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.08.0-1
- 20.08.0

* Thu Jul 09 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.04.3-1
- 20.04.3

* Fri Jun 12 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.04.2-1
- 20.04.2

* Tue May 19 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.04.1-1
- 20.04.1

* Sun Apr 26 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.04.0-1
- 20.04.0

* Wed Jan 29 2020 Fedora Release Engineering <releng@fedoraproject.org> - 19.08.2-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_32_Mass_Rebuild

* Thu Nov 07 2019 Vasiliy N. Glazov <vascom2@gmail.com> 19.08.2-1
- Update to 19.08.2
- Enable LTO

* Wed Sep 25 2019 Vasiliy N. Glazov <vascom2@gmail.com> 19.08.1-1
- Update to 19.08.1

* Thu Jul 25 2019 Fedora Release Engineering <releng@fedoraproject.org> - 19.04.0-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_31_Mass_Rebuild

* Mon Apr 22 2019 Vasiliy N. Glazov <vascom2@gmail.com> 19.04.0-1
- Update to 19.04.0

* Mon Feb 25 2019 Vasiliy N. Glazov <vascom2@gmail.com> 18.12.2-1
- Update to 18.12.2

* Fri Feb 01 2019 Fedora Release Engineering <releng@fedoraproject.org> - 18.12.1-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_30_Mass_Rebuild

* Fri Jan 11 2019 Vasiliy N. Glazov <vascom2@gmail.com> 18.12.1-1
- Update to 18.12.1

* Fri Dec 14 2018 Vasiliy N. Glazov <vascom2@gmail.com> 18.12.0-1
- Update to 18.12.0

* Mon Nov 12 2018 Vasiliy N. Glazov <vascom2@gmail.com> 18.08.3-1
- Update to 18.08.3

* Thu Oct 18 2018 Vasiliy N. Glazov <vascom2@gmail.com> 18.08.2-1
- Switch to Qt5/KF5

* Fri Jul 13 2018 Fedora Release Engineering <releng@fedoraproject.org> - 0.8-12
- Rebuilt for https://fedoraproject.org/wiki/Fedora_29_Mass_Rebuild

* Wed Feb 07 2018 Fedora Release Engineering <releng@fedoraproject.org> - 0.8-11
- Rebuilt for https://fedoraproject.org/wiki/Fedora_28_Mass_Rebuild

* Thu Aug 03 2017 Fedora Release Engineering <releng@fedoraproject.org> - 0.8-10
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Binutils_Mass_Rebuild

* Wed Jul 26 2017 Fedora Release Engineering <releng@fedoraproject.org> - 0.8-9
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Mass_Rebuild

* Fri Feb 10 2017 Fedora Release Engineering <releng@fedoraproject.org> - 0.8-8
- Rebuilt for https://fedoraproject.org/wiki/Fedora_26_Mass_Rebuild

* Thu Feb 04 2016 Fedora Release Engineering <releng@fedoraproject.org> - 0.8-7
- Rebuilt for https://fedoraproject.org/wiki/Fedora_24_Mass_Rebuild

* Wed Jun 17 2015 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 0.8-6
- Rebuilt for https://fedoraproject.org/wiki/Fedora_23_Mass_Rebuild

* Sat May 02 2015 Kalev Lember <kalevlember@gmail.com> - 0.8-5
- Rebuilt for GCC 5 C++11 ABI change

* Sat Aug 16 2014 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 0.8-4
- Rebuilt for https://fedoraproject.org/wiki/Fedora_21_22_Mass_Rebuild

* Sat Aug 16 2014 Rex Dieter <rdieter@fedoraproject.org> 0.8-3
- use kde4 macros, update scriptlets

* Sun Jun 08 2014 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 0.8-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_21_Mass_Rebuild

* Wed Mar 26 2014 Vasiliy N. Glazov <vascom2@gmail.com> - 0.8-1
- Update to 0.8
- Clean spec
- Add russian description

* Sat Aug 03 2013 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 0.7.1-6
- Rebuilt for https://fedoraproject.org/wiki/Fedora_20_Mass_Rebuild

* Thu Feb 14 2013 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 0.7.1-5
- Rebuilt for https://fedoraproject.org/wiki/Fedora_19_Mass_Rebuild

* Thu Jul 19 2012 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 0.7.1-4
- Rebuilt for https://fedoraproject.org/wiki/Fedora_18_Mass_Rebuild

* Tue Feb 28 2012 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 0.7.1-3
- Rebuilt for c++ ABI breakage

* Fri Jan 13 2012 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 0.7.1-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_17_Mass_Rebuild

* Mon Mar 14 2011 Alain Portal <alain.portal[AT]univ-montp2[DOT]fr> 0.7.1-1
  - New upstream version
  - Update patch0

* Mon Feb  7 2011 Alain Portal <alain.portal[AT]univ-montp2[DOT]fr> 0.7-1
  - New upstream version
  - Update patch0
  - New patch1 : update french handbook

* Mon Feb 07 2011 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 0.6.4-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_15_Mass_Rebuild

* Mon May 31 2010 Alain Portal <alain.portal[AT]univ-montp2[DOT]fr> 0.6.4-1
  - New upstream version
  - Update patch0
  - Patches 1 & 2 no more needed

* Sun Jan 10 2010 Alain Portal <alain.portal[AT]univ-montp2[DOT]fr> 0.6.3-3
  - Updated %%defattr macro

* Sat Jan  9 2010 Alain Portal <alain.portal[AT]univ-montp2[DOT]fr> 0.6.3-2
  - Don't add Application category in desktop file
  - Add update-mime-database in scriptlets
  - Update gtk-update-icon-cache use in scriptlets
  - Add %%posttrans scriptlet

* Sat Jan  9 2010 Alain Portal <alain.portal[AT]univ-montp2[DOT]fr> 0.6.3-1
  - New upstream version
  - Clean spec file for kde4
  - New patch0 : l10n-fr
  - New patch1 : update french handbook
  - New patch2 : Improve desktop entries

* Wed Feb 25 2009 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 0.5.4-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_11_Mass_Rebuild

* Sun Sep 14 2008 Paul F. Johnson <paul@all-the-johnsons.co.uk> 0.5.4-1
- bump to new version
- desktop patch fix

* Mon Mar 31 2008 Rex Dieter <rdieter@fedoraproject.org> 0.5.3-4
- gcc43 patch (#434062)

* Wed Mar 12 2008 Kevin Kofler <Kevin tigcc ticalc org> 0.5.3-3
- BR kdelibs3-devel instead of kdelibs-devel (#434062)

* Mon Feb 18 2008 Fedora Release Engineering <rel-eng@fedoraproject.org> - 0.5.3-2
- Autorebuild for GCC 4.3

* Thu Oct 18 2007 Alain Portal <aportal[AT]univ-montp2[DOT]fr> 0.5.3-1
  - New upstream version
  - Update patch 0
  - Remove patch 1 that is no more needed

* Thu Sep  6 2007 Alain Portal <aportal[AT]univ-montp2[DOT]fr> 0.5.2-1
  - New upstream version
  - Update patch 1
  - Remove patch 2 that is no more needed

* Tue Aug 21 2007 Alain Portal <aportal[AT]univ-montp2[DOT]fr> 0.5.1-8
  - Licence tag clarification

* Thu Jul 12 2007  Alain Portal <aportal[AT]univ-montp2[DOT]fr> 0.5.1-7
  - Disable parallel building until this is fixed by upstream

* Thu Jul 12 2007  Alain Portal <aportal[AT]univ-montp2[DOT]fr> 0.5.1-6
  - Try a build without parallel make support

* Thu Jul 12 2007  Alain Portal <aportal[AT]univ-montp2[DOT]fr> 0.5.1-5
  - Fix date in changelog

* Thu Jul 12 2007  Alain Portal <aportal[AT]univ-montp2[DOT]fr> 0.5.1-4
  - Test if gtk-update-icon-cache exists before running it

* Wed Jul 11 2007  Alain Portal <aportal[AT]univ-montp2[DOT]fr> 0.5.1-3
  - Add BR gettext

* Mon Oct  2 2006 Alain Portal <aportal[AT]univ-montp2[DOT]fr> 0.5.1-2
  - Really add patch1

* Mon Oct  2 2006 Alain Portal <aportal[AT]univ-montp2[DOT]fr> 0.5.1-1
  - New upstream tarball
  - Update patch1
  - Add new patch for french doc
  - Add Application category in desktop file
  - Fix symlink: english is the only language where common directory is in
  LANG directory while for other, common is in LANG/docs directory

* Wed Sep 27 2006 Alain Portal <aportal[AT]univ-montp2[DOT]fr> 0.5-6
  - Link the good directories

* Tue Sep 26 2006 Alain Portal <aportal[AT]univ-montp2[DOT]fr> 0.5-5
  - Fix absolute symlinks

* Mon Sep 25 2006 Alain Portal <aportal[AT]univ-montp2[DOT]fr> 0.5-4
  - Install only one desktop file
  - Don't remove absolute symlinks
  - Update patch0

* Mon Sep 25 2006 Alain Portal <aportal[AT]univ-montp2[DOT]fr> 0.5-3
  - desktop-file-install don't work as I expected, so update patch0

* Mon Sep 25 2006 Alain Portal <aportal[AT]univ-montp2[DOT]fr> 0.5-2
  - Use macro for make
  - Don't own some directories
  - Update patch0 and patch1
  - Improve desktop-file installation

* Mon Sep 25 2006 Alain Portal <aportal[AT]univ-montp2[DOT]fr> 0.5-1
  - New upstream version
  - Update patch0 and patch1
  - Remove patch2 that is no more needed

* Mon Sep 25 2006 Alain Portal <aportal[AT]univ-montp2[DOT]fr> 0.4.2-4
  - Requires(post,postun) desktop-file-utils no more needed since FC-5
  - Add %%post an %%postun for icons
  - Remove absolute symlinks

* Thu Sep 21 2006 Alain Portal <aportal[AT]univ-montp2[DOT]fr> 0.4.2-3
  - Use macro for configure instead of hardcoding path
  - Use macro style instead of variable style

* Thu Sep 21 2006 Alain Portal <aportal[AT]univ-montp2[DOT]fr> 0.4.2-2
  - Add patch to fix some typo in fr.po
  - Add patch to frenchify x-kbp.desktop

* Wed Sep 20 2006 Alain Portal <aportal[AT]univ-montp2[DOT]fr> 0.4.2-1
  - Initial Fedora RPM
  - Add patch to frenchify kbackup.desktop
