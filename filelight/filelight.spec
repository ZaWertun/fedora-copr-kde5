Name:    filelight 
Summary: Graphical disk usage statistics 
Epoch:   1
Version: 19.08.1
Release: 1%{?dist}

# KDE e.V. may determine that future GPL versions are accepted
License: GPLv2 or GPLv3
URL:     http://utils.kde.org/projects/filelight
#URL:    https://cgit.kde.org/%{name}.git

%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0: http://download.kde.org/%{stable}/applications/%{version}/src/%{name}-%{version}.tar.xz

BuildRequires: desktop-file-utils
BuildRequires: libappstream-glib

BuildRequires: extra-cmake-modules
BuildRequires: kf5-rpm-macros
BuildRequires: cmake(KF5KIO)
BuildRequires: cmake(KF5DocTools)
BuildRequires: cmake(KF5I18n)
BuildRequires: cmake(KF5XmlGui)

BuildRequires: cmake(Qt5Gui)
BuildRequires: cmake(Qt5Widgets)

# when split occured
Conflicts: kdeutils-common < 6:4.7.80

# translations moved here
Conflicts: kde-l10n < 17.03

Obsoletes: kdeutils-filelight < 6:4.7.80
Provides:  kdeutils-filelight = 6:%{version}-%{release}


%description
Filelight allows you to quickly understand exactly where your diskspace
is being used by graphically representing your file system.


%prep
%autosetup


%build
mkdir %{_target_platform}
pushd %{_target_platform}
%{cmake_kf5} ..
popd

make %{?_smp_mflags} -C %{_target_platform}


%install
make install/fast DESTDIR=%{buildroot} -C %{_target_platform}

%find_lang %{name} --all-name --with-html


%check
appstream-util validate-relax --nonet %{buildroot}%{_kf5_metainfodir}/org.kde.%{name}.appdata.xml ||:
desktop-file-validate %{buildroot}%{_kf5_datadir}/applications/org.kde.%{name}.desktop

%files -f %{name}.lang
%license COPYING*
%doc AUTHORS README*
%{_kf5_bindir}/filelight
%{_kf5_datadir}/applications/org.kde.filelight.desktop
%{_kf5_metainfodir}/org.kde.filelight.appdata.xml
%{_kf5_datadir}/icons/hicolor/*/*/*filelight.*
%{_sysconfdir}/xdg/filelightrc
%{_kf5_datadir}/kxmlgui5/filelight/
%{_kf5_datadir}/qlogging-categories5/*.categories


%changelog
* Thu Sep 05 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 1:19.08.1-1
- 19.08.1

* Thu Aug 15 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 1:19.08.0-1
- 19.08.0

* Thu Jul 11 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 1:19.04.3-1
- 19.04.3

* Thu Jun 06 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 1:19.04.2-1
- 19.04.2

* Thu May 09 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 1:19.04.1-1
- 19.04.1

* Sat Apr 27 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 1:19.04.0-1
- 19.04.0

* Tue Feb 05 2019 Rex Dieter <rdieter@fedoraproject.org> - 1:18.12.2-1
- 18.12.2

* Thu Jan 31 2019 Fedora Release Engineering <releng@fedoraproject.org> - 1:18.12.1-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_30_Mass_Rebuild

* Tue Jan 08 2019 Rex Dieter <rdieter@fedoraproject.org> - 1:18.12.1-1
- 18.12.1

* Mon Dec 17 2018 Rex Dieter <rdieter@fedoraproject.org> - 1:18.12.0-1
- 18.12.0

* Tue Nov 06 2018 Rex Dieter <rdieter@fedoraproject.org> - 1:18.08.3-1
- 18.08.3

* Wed Oct 10 2018 Rex Dieter <rdieter@fedoraproject.org> - 1:18.08.2-1
- 18.08.2

* Fri Jul 13 2018 Rex Dieter <rdieter@fedoraproject.org> - 1:18.04.3-1
- 18.04.3

* Fri Jul 13 2018 Fedora Release Engineering <releng@fedoraproject.org> - 1:18.04.2-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_29_Mass_Rebuild

* Wed Jun 06 2018 Rex Dieter <rdieter@fedoraproject.org> - 1:18.04.2-1
- 18.04.2

* Wed May 09 2018 Rex Dieter <rdieter@fedoraproject.org> - 1:18.04.1-1
- 18.04.1

* Fri Apr 20 2018 Rex Dieter <rdieter@fedoraproject.org> - 1:18.04.0-1
- 18.04.0

* Tue Mar 06 2018 Rex Dieter <rdieter@fedoraproject.org> - 1:17.12.3-1
- 17.12.3

* Thu Feb 08 2018 Rex Dieter <rdieter@fedoraproject.org> - 1:17.12.2-1
- 17.12.2

* Wed Feb 07 2018 Fedora Release Engineering <releng@fedoraproject.org> - 1:17.12.1-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_28_Mass_Rebuild

* Thu Jan 11 2018 Rex Dieter <rdieter@fedoraproject.org> - 1:17.12.1-1
- 17.12.1

* Fri Dec 29 2017 Rex Dieter <rdieter@fedoraproject.org> - 1:17.12.0-1
- 17.12.0

* Fri Sep 29 2017 Rex Dieter <rdieter@fedoraproject.org> - 1:17.08.1-1
- 17.08.1

* Thu Aug 03 2017 Rex Dieter <rdieter@fedoraproject.org> - 1:17.04.3-1
- 17.04.3

* Wed Aug 02 2017 Fedora Release Engineering <releng@fedoraproject.org> - 1:17.04.2-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Binutils_Mass_Rebuild

* Wed Jul 26 2017 Fedora Release Engineering <releng@fedoraproject.org> - 1:17.04.2-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Mass_Rebuild

* Thu Jun 15 2017 Rex Dieter <rdieter@fedoraproject.org> - 1:17.04.2-1
- 17.04.2

* Sat Jun 03 2017 Rex Dieter <rdieter@fedoraproject.org> - 1:17.04.1-1
- 17.04.1

* Thu Mar 09 2017 Rex Dieter <rdieter@fedoraproject.org> - 1:16.12.3-1
- 16.12.3

* Thu Feb 09 2017 Rex Dieter <rdieter@fedoraproject.org> - 1:16.12.2-1
- 16.12.2

* Fri Jan 13 2017 Rex Dieter <rdieter@fedoraproject.org> - 1:16.12.1-1
- 16.12.1

* Mon Dec 05 2016 Rex Dieter <rdieter@fedoraproject.org> - 1:16.08.3-1
- 16.08.3

* Thu Oct 13 2016 Rex Dieter <rdieter@fedoraproject.org> - 1:16.08.2-1
- 16.08.2

* Thu Sep 08 2016 Rex Dieter <rdieter@fedoraproject.org> - 1:16.08.1-1
- 16.08.1

* Sat Aug 13 2016 Rex Dieter <rdieter@fedoraproject.org> - 1:16.08.0-1
- 16.08.0

* Sat Aug 06 2016 Rex Dieter <rdieter@fedoraproject.org> - 1:16.07.90-1
- 16.07.90

* Sun Jul 10 2016 Rex Dieter <rdieter@fedoraproject.org> - 1:16.04.3-1
- 16.04.3

* Sun Jun 12 2016 Rex Dieter <rdieter@fedoraproject.org> - 1:16.04.2-1
- 16.04.2

* Sun May 08 2016 Rex Dieter <rdieter@fedoraproject.org> - 1:16.04.1-1
- 16.04.1

* Tue Apr 26 2016 Rex Dieter <rdieter@fedoraproject.org> - 1:16.04.0-1
- 16.04.0

* Tue Mar 15 2016 Rex Dieter <rdieter@fedoraproject.org> - 1:15.12.3-1
- 15.12.3

* Mon Feb 15 2016 Rex Dieter <rdieter@fedoraproject.org> - 1:15.12.2-1
- 15.12.2

* Wed Feb 03 2016 Fedora Release Engineering <releng@fedoraproject.org> - 1:15.12.1-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_24_Mass_Rebuild

* Fri Jan 22 2016 Rex Dieter <rdieter@fedoraproject.org> 1:15.12.1-1
- 15.12.1

* Sun Dec 20 2015 Rex Dieter <rdieter@fedoraproject.org> - 1:15.12.0-1
- 15.12.0

* Thu Aug 20 2015 Than Ngo <than@redhat.com> - 15.08.0-1
- 15.08.0

* Wed Jun 17 2015 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 1:15.04.2-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_23_Mass_Rebuild

* Wed Jun 10 2015 Rex Dieter <rdieter@fedoraproject.org> - 1:15.04.2-1
- 15.04.2

* Wed May 27 2015 Rex Dieter <rdieter@fedoraproject.org> - 1:15.04.1-1
- 15.04.1

* Tue Apr 14 2015 Rex Dieter <rdieter@fedoraproject.org> - 1:15.04.0-1
- 15.04.0

* Sun Mar 01 2015 Rex Dieter <rdieter@fedoraproject.org> - 1:14.12.3-1
- 14.12.3

* Tue Feb 24 2015 Than Ngo <than@redhat.com> - 14.12.2-1
- 14.12.2

* Sat Jan 17 2015 Rex Dieter <rdieter@fedoraproject.org> - 1:14.12.1-1
- 14.12.1

* Sat Nov 08 2014 Rex Dieter <rdieter@fedoraproject.org> - 1:4.14.3-1
- 4.14.3

* Sun Oct 12 2014 Rex Dieter <rdieter@fedoraproject.org> - 1:4.14.2-1
- 4.14.2

* Tue Sep 16 2014 Rex Dieter <rdieter@fedoraproject.org> - 1:4.14.1-1
- 4.14.1

* Sat Aug 16 2014 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 1:4.14.0-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_21_22_Mass_Rebuild

* Fri Aug 15 2014 Rex Dieter <rdieter@fedoraproject.org> - 1:4.14.0-1
- 4.14.0

* Tue Aug 05 2014 Rex Dieter <rdieter@fedoraproject.org> - 1:4.13.97-1
- 4.13.97

* Tue Jul 15 2014 Rex Dieter <rdieter@fedoraproject.org> - 1:4.13.3-1
- 4.13.3

* Mon Jun 09 2014 Rex Dieter <rdieter@fedoraproject.org> - 1:4.13.2-1
- 4.13.2

* Sat Jun 07 2014 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 1:4.13.1-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_21_Mass_Rebuild

* Sun May 11 2014 Rex Dieter <rdieter@fedoraproject.org> - 1:4.13.1-1
- 4.13.1

* Sat Apr 12 2014 Rex Dieter <rdieter@fedoraproject.org> - 1:4.13.0-1
- 4.13.0

* Fri Apr 04 2014 Rex Dieter <rdieter@fedoraproject.org> - 1:4.12.97-1
- 4.12.97

* Sat Mar 22 2014 Rex Dieter <rdieter@fedoraproject.org> - 1:4.12.95-1
- 4.12.95

* Wed Mar 19 2014 Rex Dieter <rdieter@fedoraproject.org> - 1:4.12.90-1
- 4.12.90

* Sun Mar 02 2014 Rex Dieter <rdieter@fedoraproject.org> - 1:4.12.3-1
- 4.12.3

* Fri Jan 31 2014 Rex Dieter <rdieter@fedoraproject.org> - 1:4.12.2-1
- 4.12.2

* Fri Jan 10 2014 Rex Dieter <rdieter@fedoraproject.org> - 1:4.12.1-1
- 4.12.1

* Thu Dec 19 2013 Rex Dieter <rdieter@fedoraproject.org> - 1:4.12.0-1
- 4.12.0

* Sun Dec 01 2013 Rex Dieter <rdieter@fedoraproject.org> - 1:4.11.97-1
- 4.11.97

* Thu Nov 21 2013 Rex Dieter <rdieter@fedoraproject.org> - 1:4.11.95-1
- 4.11.95

* Sat Nov 16 2013 Rex Dieter <rdieter@fedoraproject.org> - 1:4.11.90-1
- 4.11.90

* Sat Nov 02 2013 Rex Dieter <rdieter@fedoraproject.org> - 1:4.11.3-1
- 4.11.3

* Sat Sep 28 2013 Rex Dieter <rdieter@fedoraproject.org> - 1:4.11.2-1
- 4.11.2

* Wed Sep 04 2013 Rex Dieter <rdieter@fedoraproject.org> - 1:4.11.1-1
- 4.11.1

* Thu Aug 08 2013 Than Ngo <than@redhat.com> - 4.11.0-1
- 4.11.0

* Thu Jul 25 2013 Rex Dieter <rdieter@fedoraproject.org> - 1:4.10.97-1
- 4.10.97

* Tue Jul 23 2013 Rex Dieter <rdieter@fedoraproject.org> - 1:4.10.95-1
- 4.10.95

* Fri Jun 28 2013 Rex Dieter <rdieter@fedoraproject.org> - 1:4.10.90-1
- 4.10.90

* Sat Jun 01 2013 Rex Dieter <rdieter@fedoraproject.org> - 1:4.10.4-1
- 4.10.4

* Mon May 06 2013 Than Ngo <than@redhat.com> - 4.10.3-1
- 4.10.3

* Sun Mar 31 2013 Rex Dieter <rdieter@fedoraproject.org> - 1:4.10.2-1
- 4.10.2

* Sat Mar 02 2013 Rex Dieter <rdieter@fedoraproject.org> - 1:4.10.1-1
- 4.10.1

* Fri Feb 01 2013 Rex Dieter <rdieter@fedoraproject.org> - 1:4.10.0-1
- 4.10.0

* Tue Jan 22 2013 Rex Dieter <rdieter@fedoraproject.org> - 1:4.9.98-1
- 4.9.98

* Fri Jan 04 2013 Rex Dieter <rdieter@fedoraproject.org> - 1:4.9.97-1
- 4.9.97

* Thu Dec 20 2012 Rex Dieter <rdieter@fedoraproject.org> - 1:4.9.95-1
- 4.9.95

* Tue Dec 04 2012 Rex Dieter <rdieter@fedoraproject.org> - 1:4.9.90-1
- 4.9.90

* Mon Dec 03 2012 Than Ngo <than@redhat.com> - 1:4.9.4-1
- 4.9.4

* Sat Nov 03 2012 Rex Dieter <rdieter@fedoraproject.org> - 1:4.9.3-1
- 4.9.3

* Sat Sep 29 2012 Rex Dieter <rdieter@fedoraproject.org> - 1:4.9.2-1
- 4.9.2

* Mon Sep 03 2012 Than Ngo <than@redhat.com> - 1:4.9.1-1
- 4.9.1

* Thu Jul 26 2012 Lukas Tinkl <ltinkl@redhat.com> - 1:4.9.0-1
- 4.9.0

* Thu Jul 19 2012 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 1:4.8.97-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_18_Mass_Rebuild

* Thu Jul 12 2012 Rex Dieter <rdieter@fedoraproject.org> - 1:4.8.97-1
- 4.8.97

* Wed Jun 27 2012 Jaroslav Reznik <jreznik@redhat.com> - 1:4.8.95-1
- 4.8.95

* Sat Jun 09 2012 Rex Dieter <rdieter@fedoraproject.org> - 1:4.8.90-1
- 4.8.90

* Tue Jun 05 2012 Rex Dieter <rdieter@fedoraproject.org> 1:4.8.80-2
- filelight missing versioned kdelibs4 dependency (#827497)

* Fri Jun 01 2012 Jaroslav Reznik <jreznik@redhat.com> - 1:4.8.80-1
- 4.8.80

* Mon Apr 30 2012 Jaroslav Reznik <jreznik@redhat.com> - 1:4.8.3-1
- 4.8.3

* Fri Mar 30 2012 Rex Dieter <rdieter@fedoraproject.org> - 1:4.8.2-1
- 4.8.2

* Mon Mar 05 2012 Jaroslav Reznik <jreznik@redhat.com> - 1:4.8.1-1
- 4.8.1

* Sun Jan 22 2012 Rex Dieter <rdieter@fedoraproject.org> - 1:4.8.0-1
- 4.8.0

* Wed Jan 04 2012 Rex Dieter <rdieter@fedoraproject.org> - 1:4.7.97-1
- 4.7.97

* Tue Dec 27 2011 Rex Dieter <rdieter@fedoraproject.org> 1:4.7.95-2
- epoch: 1

* Thu Dec 22 2011 Radek Novacek <rnovacek@redhat.com> - 4.7.95-1
- 4.7.95

* Mon Dec 19 2011 Rex Dieter <rdieter@fedoraproject.org> 4.7.90-2
- 4.7.90 (#761559)

* Tue Feb 08 2011 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 1.9-6.rc3.1
- Rebuilt for https://fedoraproject.org/wiki/Fedora_15_Mass_Rebuild

* Tue Sep  7 2010 Neal Becker <ndbecker2@gmail.com> - 1.9-6.rc3
- Patch to mainWindow.cpp for gcc4.5

* Sat Jan  2 2010 Neal Becker <ndbecker2@gmail.com> - 1.9-5.rc3
- Drop filelight.ChangeLog.diff

* Sat Jan  2 2010 Neal Becker <ndbecker2@gmail.com> - 1.9-4.rc3
- fix directory

* Sun Aug  2 2009 Neal Becker <ndbecker2@gmail.com> - 1.9-3.rc2
- fix some rpmlint complaints:
- Follow cmake_kde4 recipe
  untabify
  convert ChangeLog to utf8
  don't mark filelightrc as config

* Fri Jul 31 2009 Neal Becker <ndbecker2@gmail.com> - 1.9-2.rc2
- Updates for kde4
- Update to 1.9rc2

* Fri Jul 24 2009 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 1.0-16
- Rebuilt for https://fedoraproject.org/wiki/Fedora_12_Mass_Rebuild

* Tue Feb 24 2009 Neal Becker <ndbecker2@gmail.com> - 1.0-15
- use noreplace on config

* Tue Feb 24 2009 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 1.0-14
- Rebuilt for https://fedoraproject.org/wiki/Fedora_11_Mass_Rebuild

* Fri Jul 18 2008 Tom "spot" Callaway <tcallawa@redhat.com> - 1.0-13
- fix license tag

* Tue Feb 19 2008 Fedora Release Engineering <rel-eng@fedoraproject.org> - 1.0-12
- Autorebuild for GCC 4.3

* Thu Jan  3 2008 Neal Becker <ndbecker2@gmail.com> - 1.0-11
- Make that br kdelibs3-devel

* Thu Jan  3 2008 Neal Becker <ndbecker2@gmail.com> - 1.0-9
- BR qt-devel for /etc/profile.d/qt.sh

* Thu Oct 05 2006 Christian Iseli <Christian.Iseli@licr.org> 1.0-9
 - rebuilt for unwind info generation, broken in gcc-4.1.1-21

* Mon Sep 11 2006 Neal Becker <ndbecker2@gmail.com> - 1.0-8
- Add --delete-original to desktop-file-install
- Add gtk-update-icon-cache
- fix defattr

* Sat Sep  9 2006 Neal Becker <ndbecker2@gmail.com> - 1.0-7
- Cleanup
- Add req hicolor-icon-theme
- find_lang removed
- qt-devel BR removed

* Wed Sep  6 2006 Neal Becker <ndbecker2@gmail.com> - 1.0-6
- oops, config is for filelightrc
- filelight.desktop patch fed upstream

* Wed Sep  6 2006 Neal Becker <ndbecker2@gmail.com> - 1.0-5
- desktop-install-file per rdieter@math.unl.edu

* Tue Sep  5 2006 Neal Becker <ndbecker2@gmail.com> - 1.0-4
- Put filelightrc in datadir/config

* Tue Sep  5 2006 Neal Becker <ndbecker2@gmail.com> - 1.0-3
- Add dist to release tag
- add delete-original to desktop-file-install
- Fix changelog entries (no macros)

* Tue Sep  5 2006 Neal Becker <ndbecker2@gmail.com> - 1.0-2
- remove files -f
- patch filelight.desktop (remove space)
- mv filelightrc to sysconfdir
- fix buildroot
- setup -q
- Remove Packager

* Sat Sep  2 2006 Neal Becker <ndbecker2@gmail.com> - 1.0-1
- Remove find_lang
- fix icons/hicolor
- --disable-rpath

* Mon Aug 22 2005 Dag Wieers <dag@wieers.com> - 0.6.4-1 - +/
- Initial package. (using DAR)
