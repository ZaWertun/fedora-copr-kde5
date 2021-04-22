%global unstable 0

Name:           kde-partitionmanager
Version:        21.04.0
Release:        1%{?dist}
Summary:        KDE Partition Manager

License:        GPLv3+
URL:            https://invent.kde.org/system/partitionmanager
%if 0%{?unstable}
Source0:        http://download.kde.org/unstable/release-service/%{version}/src/partitionmanager-%{version}.tar.xz
%else
Source0:        http://download.kde.org/stable/release-service/%{version}/src/partitionmanager-%{version}.tar.xz
%endif

BuildRequires:  cmake
BuildRequires:  desktop-file-utils
BuildRequires:  extra-cmake-modules
BuildRequires:  gettext
BuildRequires:  kf5-kconfig-devel
BuildRequires:  kf5-kconfigwidgets-devel
BuildRequires:  kf5-kcoreaddons-devel
BuildRequires:  kf5-kcrash-devel
BuildRequires:  kf5-ki18n-devel
BuildRequires:  kf5-kiconthemes-devel
BuildRequires:  kf5-kjobwidgets-devel
BuildRequires:  kf5-kio-devel
BuildRequires:  kf5-rpm-macros
BuildRequires:  kf5-kservice-devel
BuildRequires:  kf5-kxmlgui-devel
BuildRequires:  kf5-kwidgetsaddons-devel
BuildRequires:  kf5-kdbusaddons-devel
BuildRequires:  kpmcore-devel = %{version}
BuildRequires:  qt5-qtbase-devel

BuildRequires:  cmake(KF5DocTools)

BuildRequires:  pkgconfig(appstream-glib)
BuildRequires:  pkgconfig(blkid)
BuildRequires:  pkgconfig(libatasmart)

Requires:       kpmcore = %{version}
Requires:       kf5-filesystem

%description
KDE Partition Manager is a utility program to help you manage the disk devices,
partitions and file systems on your computer. It allows you to easily create, 
copy, move, delete, resize without losing data, backup and restore partitions.
 
KDE Partition Manager supports a large number of file systems, 
including ext2/3/4, reiserfs, NTFS, FAT16/32, jfs, xfs and more.
 
Starting from version 1.9.50 KDE Partition Manager has become the GUI part of 
KPMcore (KDE PartitionManager core) which contain the libraries used to 
manipulate filesystems.


%prep
%autosetup -p1 -n partitionmanager-%{version}


%build
%cmake_kf5
%cmake_build


%install
%cmake_install
%find_lang partitionmanager --with-kde --with-html


%check
# Validate .desktop file
desktop-file-validate %{buildroot}%{_kf5_datadir}/applications/*partitionmanager.desktop

# Validate appdata file
appstream-util validate-relax --nonet %{buildroot}/%{_datadir}/metainfo/*.appdata.xml


%files -f partitionmanager.lang
%license LICENSES/*.txt
%{_kf5_bindir}/partitionmanager
%{_kf5_datadir}/applications/*partitionmanager.desktop
%{_kf5_datadir}/kxmlgui5/partitionmanager/
%{_kf5_datadir}/config.kcfg/partitionmanager.kcfg
%{_datadir}/icons/hicolor/*/*/*
%{_datadir}/metainfo/*partitionmanager.appdata.xml

%changelog
* Thu Apr 22 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 21.04.0-1
- 21.04.0

* Wed Mar 17 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.12.3-2
- added optional build dep: DocTools

* Wed Mar 17 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.12.3-1
- 20.12.3

* Fri Nov  6 14:47:06 MSK 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 4.2.0-1
- 4.2.0

* Fri Mar 06 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 4.1.0-1
- 4.1.0

* Thu May 02 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 4.0.0-1
- 4.0.0

* Fri Jul 13 2018 Fedora Release Engineering <releng@fedoraproject.org> - 3.3.1-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_29_Mass_Rebuild

* Wed Feb 07 2018 Fedora Release Engineering <releng@fedoraproject.org> - 3.3.1-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_28_Mass_Rebuild

* Tue Jan 30 2018 Mattia Verga <mattia.verga@email.it> - 3.3.1-1
- Update to 3.3.1

* Thu Jan 11 2018 Igor Gnatenko <ignatenkobrain@fedoraproject.org> - 3.3.0-2
- Remove obsolete scriptlets

* Tue Dec 26 2017 Mattia Verga <mattia.verga@email.it> - 3.3.0-1
- Update to 3.3.0

* Sun Dec 03 2017 Mattia Verga <mattia.verga@email.it> - 3.2.1-3
- Backport patch to fix mountpoint creation

* Fri Dec 01 2017 Mattia Verga <mattia.verga@email.it> - 3.2.1-2
- Backport patch from upstream to fix #1517718
- Use --with-html in find_lang
- Change appdata dir to metainfo

* Wed Nov 01 2017 Mattia Verga <mattia.verga@email.it> - 3.2.1-1
- Update to 3.2.1

* Sun Oct 01 2017 Mattia Verga <mattia.verga@tiscali.it> - 3.2.0-1
- Update to 3.2.0

* Wed Sep 06 2017 Mattia Verga <mattia.verga@tiscali.it> - 3.1.2-1
- Update to 3.1.2

* Thu Aug 03 2017 Fedora Release Engineering <releng@fedoraproject.org> - 3.1.0-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Binutils_Mass_Rebuild

* Wed Jul 26 2017 Fedora Release Engineering <releng@fedoraproject.org> - 3.1.0-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Mass_Rebuild

* Thu Jul 06 2017 Mattia Verga <mattia.verga@tiscali.it> - 3.1.0-1
- Update to 3.1.0
- Rename desktop and appdata files to lowercase

* Sun Jun 04 2017 Mattia Verga <mattia.verga@tiscali.it> - 3.0.1-3
- Rebuild for kpmcore 3.1.0

* Thu Feb 09 2017 Mattia Verga <mattia.verga@tiscali.it> - 3.0.1-2
- Rebuild for kpmcore 3.0.3

* Sun Jan 15 2017 Mattia Verga <mattia.verga@tiscali.it> - 3.0.1-1
- Upgrade to stable 3.0.1
- Link to kpmcore 3.0.2

* Sun Jan 01 2017 Mattia Verga <mattia.verga@tiscali.it> - 3.0.0-2
- Link to kpmcore 3.0.1

* Wed Dec 21 2016 Mattia Verga <mattia.verga@tiscali.it> - 3.0.0-1
- Upgrade to stable 3.0.0

* Wed Nov 09 2016 Mattia Verga <mattia.verga@tiscali.it> - 2.9.90-1
- Upgrade to unstable 2.9.90
- Extend LVM support

* Sun Jul 10 2016 Mattia Verga <mattia.verga@tiscali.it> - 2.2.1-2
- Remove nonexistent doc files to fix build

* Sun Jul 10 2016 Mattia Verga <mattia.verga@tiscali.it> - 2.2.1-1
- Update to stable 2.2.1

* Sat Jun 11 2016 Mattia Verga <mattia.verga@tiscali.it> - 2.2.0-1
- Update to stable 2.2.0

* Sun Mar 13 2016 Mattia Verga <mattia.verga@tiscali.it> - 2.1.0-1
- Update to stable 2.1.0

* Sun Feb 28 2016 Mattia Verga <mattia.verga@tiscali.it> - 2.0.3-1
- Update to stable 2.0.3
- Use pkgconfig for libs
- Alphabetically ordered BR

* Thu Feb 04 2016 Fedora Release Engineering <releng@fedoraproject.org>
- Rebuilt for https://fedoraproject.org/wiki/Fedora_24_Mass_Rebuild

* Mon Jan 18 2016 Mattia Verga <mattia.verga@tiscali.it> - 2.0.0-2
- KPMcore has been renamed to kpmcore, BR changed
- Better formatting of BR
- Move .desktop and appdata validation under %%check

* Thu Jan 14 2016 Mattia Verga <mattia.verga@tiscali.it> - 2.0.0-1
- Update to stable 2.0.0
- Bind to same KPMcore version
- Library removed from sources

* Thu Dec 03 2015 Mattia Verga <mattia.verga@tiscali.it> - 1.9.50-1
- Update to 1.9.50

* Sun Jun 21 2015 Mattia Verga <mattia.verga@tiscali.it> - 1.2.1-7
- Validate appdata file

* Wed Jun 17 2015 Mattia Verga <mattia.verga@tiscali.it> - 1.2.1-6
- Fix KF5 requires
- Remove hardened build option since it's now default

* Wed Jun 17 2015 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 1.2.1-5
- Rebuilt for https://fedoraproject.org/wiki/Fedora_23_Mass_Rebuild

* Sat May 02 2015 Kalev Lember <kalevlember@gmail.com> - 1.2.1-4
- Rebuilt for GCC 5 C++11 ABI change

* Sun Apr 05 2015 Mattia Verga <mattia.verga@tiscali.it> - 1.2.1-3
- Fix documentation files

* Sun Apr 05 2015 Mattia Verga <mattia.verga@tiscali.it> - 1.2.1-2
- Fix typos

* Sun Apr 05 2015 Mattia Verga <mattia.verga@tiscali.it> - 1.2.1-1
- 1.2.1 release, port to kf5
- License changed to GPLv3

* Sun Feb 22 2015 Mattia Verga <mattia.verga@tiscali.it> - 1.1.1-1
- 1.1.1 release

* Sun Nov 16 2014 Mattia Verga <mattia.verga@tiscali.it> - 1.1.0-3
- Fix detection of devices without partition table

* Sat Aug 16 2014 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 1.1.0-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_21_22_Mass_Rebuild

* Thu Jul 10 2014 Rex Dieter <rdieter@fedoraproject.org> 1.1.0-1
- 1.1.0 release, improve scriptlets/kde4 macro usage, include translations

* Sun Jun 08 2014 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 1.0.3-14.20130815svn
- Rebuilt for https://fedoraproject.org/wiki/Fedora_21_Mass_Rebuild

* Thu Aug 15 2013 Mattia Verga <mattia.verga@tiscali.it> - 1.0.3-13.20130815svn
- Upgrade to svn 2013-05-15 - adds support for decrypting LUKS volumes
- Compress source with xz

* Sat Aug 03 2013 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 1.0.3-12.20130624svn
- Rebuilt for https://fedoraproject.org/wiki/Fedora_20_Mass_Rebuild

* Sun Jun 23 2013 Mattia Verga <mattia.verga@tiscali.it> - 1.0.3-11.20130624svn
- Fix to enable udisks2 backend compatibility

* Thu Feb 14 2013 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 1.0.3-10.20121209svn
- Rebuilt for https://fedoraproject.org/wiki/Fedora_19_Mass_Rebuild

* Sun Dec 09 2012 Mattia Verga <mattia.verga@tiscali.it> - 1.0.3-9.20121209svn
- Upgrade to svn 2012-12-09
- Add BTRFS support
- Add exfat support

* Thu Jul 19 2012 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 1.0.3-8.20120205svn
- Rebuilt for https://fedoraproject.org/wiki/Fedora_18_Mass_Rebuild

* Fri Apr 13 2012 Mattia Verga <mattia.verga@tiscali.it> - 1.0.3-7.20120205svn
- Enable PIE following change in Pakaging Guidelines

* Wed Mar 14 2012 Mattia Verga <mattia.verga@tiscali.it> - 1.0.3-6.20120205svn
- Rebuilt for parted-3.1

* Mon Jan 30 2012 Mattia Verga <mattia.verga@tiscali.it> - 1.0.3-5.20120205svn
- Upgrade to svn 2012-02-05
- Fix bug #787194

* Mon Jan 30 2012 Mattia Verga <mattia.verga@tiscali.it> - 1.0.3-5.20120130svn
- bugfix release

* Tue Dec 27 2011 Mattia Verga <mattia.verga@tiscali.it> - 1.0.3-5.20111223svn
- Added kde-filesystem to Requires
- Added icon cache refresh at installation/uninstallation

* Fri Dec 23 2011 Mattia Verga <mattia.verga@tiscali.it> - 1.0.3-4.20111223svn
- Upgrade to svn 2011-12-22
- Fix bug #757661
- Removed language detection, no translations in svn

* Mon Feb 07 2011 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 1.0.3-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_15_Mass_Rebuild

* Wed Sep 01 2010 Thomas Janssen <thomasj@fedoraproject.org> 1.0.3-1
- 1.0.3 bugfix release

* Tue May 18 2010 Thomas Janssen <thomasj@fedoraproject.org> 1.0.2-1
- bugfix release

* Wed Mar 31 2010 Thomas Janssen <thomasj@fedoraproject.org> 1.0.1-1%{?dist}.2
- rebuild for new parted

* Sun Feb 14 2010 Thomas Janssen <thomasj@fedoraproject.org> 1.0.1-1%{?dist}.1
- DSO fix for rawhide

* Mon Jan 25 2010 Thomas Janssen <thomasj@fedoraproject.org> 1.0.1-1
- New upstream source

* Wed Nov 18 2009 Thomas Janssen <thomasj@fedoraproject.org> 1.0.0-2
- Moved desktop-file-validate to %%install
- Removed superfluous BR qt4-devel

* Wed Nov 18 2009 Thomas Janssen <thomasj@fedoraproject.org> 1.0.0-1
- Initial Fedora release
