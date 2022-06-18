%global commit 836176d25bc2c3ae0c8739f99a0f360c9c78b70d
%global commitdate 20220613
%global shortcommit %(c=%{commit}; echo ${c:0:7})

Name:		krusader
Version:	2.7.2%{?commitdate:^git%{commitdate}.%{shortcommit}}
Release:	8%{?dist}
Summary:	An advanced twin-panel (commander-style) file-manager for KDE

License:	GPLv2+
URL:		https://www.krusader.org/
%if 0%{?commitdate}
Source0:	https://invent.kde.org/utilities/%{name}/-/archive/%{commit}/krusader-%{version}.tar.bz2
%else
Source0:	https://download.kde.org/stable/%{name}/%{version}/%{name}-%{version}.tar.xz
%endif

## upstream patches

## upstreamable patches

BuildRequires:	cmake
BuildRequires:	qt5-qtbase-devel
BuildRequires:	kf5-kio-devel >= 5.23.0
BuildRequires:	kf5-karchive-devel
BuildRequires:	kf5-kbookmarks-devel
BuildRequires:	kf5-kcodecs-devel
BuildRequires:	kf5-kcompletion-devel
BuildRequires:	kf5-kcoreaddons-devel
BuildRequires:	kf5-kconfig-devel
BuildRequires:	kf5-kdoctools-devel
BuildRequires:	kf5-ki18n-devel
BuildRequires:	kf5-kiconthemes-devel
BuildRequires:	kf5-kitemviews-devel
BuildRequires:	kf5-knotifications-devel
BuildRequires:	kf5-kparts-devel
BuildRequires:	kf5-solid-devel
BuildRequires:	kf5-ktextwidgets-devel
BuildRequires:	kf5-kwallet-devel
BuildRequires:	kf5-kwidgetsaddons-devel
BuildRequires:	kf5-kwindowsystem-devel
BuildRequires:	kf5-kxmlgui-devel
BuildRequires:	kf5-kguiaddons-devel
BuildRequires:	libattr-devel libacl-devel
BuildRequires:	extra-cmake-modules
BuildRequires:	zlib-devel bzip2-devel
BuildRequires:	desktop-file-utils

%description
Krusader is an advanced twin panel (commander style) file manager for KDE and
other desktops in the *nix world, similar to Midnight or Total Commander.
It provides all the file management features you could possibly want.
Plus: extensive archive handling, mounted filesystem support, FTP, advanced
search module, an internal viewer/editor, directory synchronisation,
file content comparisons, powerful batch renaming and much much more.
It supports a wide variety of archive formats and can handle other KIO slaves
such as smb or fish. It is (almost) completely customizable, very user
friendly, fast and looks great on your desktop! You should give it a try.


%prep
%if 0%{?commitdate}
%autosetup -p1 -n %{name}-%{commit}
%else
%autosetup -p1 -n %{name}-%{version}%{?beta:-%{beta}}
%endif


%build
%cmake_kf5
%cmake_build


%install
%cmake_install

# Merge applications into one software center item
mkdir -p %{buildroot}%{_datadir}/metainfo
cat > %{buildroot}%{_datadir}/metainfo/org.kde.krusader.root-mode.appdata.xml <<EOF
<?xml version="1.0" encoding="UTF-8"?>
<!-- Copyright 2014 Richard Hughes <richard@hughsie.com> -->
<component type="desktop">
  <metadata_license>CC0-1.0</metadata_license>
  <id>org.kde.krusader.root-mode.desktop</id>
  <metadata>
    <value key="X-Merge-With-Parent">krusader.desktop</value>
  </metadata>
</component>
EOF

# Make symlink relative and remove wrong EOL
pushd %{buildroot}%{_docdir}/HTML/
for i in *
do
	pushd %{buildroot}%{_docdir}/HTML/$i/krusader/
	for j in *.docbook
	do
		tr -d '\r' < $j > ${j}.tmp
		mv -f ${j}.tmp $j
	done
	ln -sf ../common
	popd
done
popd

%find_lang %{name} --with-kde || echo > %{name}.lang

%check
for i in %{buildroot}%{_datadir}/applications/org.kde.krusader*.desktop ; do
	desktop-file-validate $i
done


%files -f %{name}.lang
%license LICENSES/*.txt
%doc doc-extras/actions_tutorial.txt
%doc AUTHORS ChangeLog README NEWS TODO
%{_sysconfdir}/xdg/kio_isorc
%{_bindir}/*
%{_kf5_plugindir}/kio/kio_{iso,krarc}.so
%{_datadir}/metainfo/org.kde.%{name}*.appdata.xml
%{_datadir}/applications/org.kde.krusader*.desktop
%{_datadir}/doc/HTML/*/krusader/*
%{_datadir}/icons/hicolor/*/apps/*.png
%{_datadir}/krusader/*
%{_datadir}/kxmlgui5/krusader/*.rc
%{_mandir}/man1/krusader.1.gz


%changelog
* Sat Jun 18 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 2.7.2-8
- Build from commit 836176d2, 2022-06-13

* Sat Jun 18 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 2.7.2-7
- Added krusader-2.7.2-fix-opening-archives-with-kf5-kio-5.95.0.patch

* Thu Jan 20 2022 Fedora Release Engineering <releng@fedoraproject.org> - 2.7.2-6
- Rebuilt for https://fedoraproject.org/wiki/Fedora_36_Mass_Rebuild

* Thu Jul 22 2021 Fedora Release Engineering <releng@fedoraproject.org> - 2.7.2-5
- Rebuilt for https://fedoraproject.org/wiki/Fedora_35_Mass_Rebuild

* Tue Jan 26 2021 Fedora Release Engineering <releng@fedoraproject.org> - 2.7.2-4
- Rebuilt for https://fedoraproject.org/wiki/Fedora_34_Mass_Rebuild

* Tue Jul 28 2020 Fedora Release Engineering <releng@fedoraproject.org> - 2.7.2-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_33_Mass_Rebuild

* Wed Jan 29 2020 Fedora Release Engineering <releng@fedoraproject.org> - 2.7.2-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_32_Mass_Rebuild

* Thu Sep 19 2019 Vitaly Zaitsev <vitaly@easycoding.org> - 2.7.2-1
- Updated to version 2.7.2.

* Thu Jul 25 2019 Fedora Release Engineering <releng@fedoraproject.org> - 2.7.1-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_31_Mass_Rebuild

* Fri Feb 01 2019 Fedora Release Engineering <releng@fedoraproject.org> - 2.7.1-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_30_Mass_Rebuild

* Mon Aug 13 2018 rad.n@centrum.cz - 2.7.1-1
- Update to 2.7.1

* Fri Jul 13 2018 Fedora Release Engineering <releng@fedoraproject.org> - 2.7.0-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_29_Mass_Rebuild

* Fri May 18 2018 Vitaly Zaitsev <vitaly@easycoding.org> - 2.7.0-1
- Updated to version 2.7.0.

* Wed Feb 07 2018 Fedora Release Engineering <releng@fedoraproject.org> - 2.6.0-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_28_Mass_Rebuild

* Fri Sep 15 2017 Radek Novacek <rad.n@centrum.cz> - 2.6.0-1
- Update to 2.6.0

* Thu Aug 03 2017 Fedora Release Engineering <releng@fedoraproject.org> - 2.5.0-4
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Binutils_Mass_Rebuild

* Wed Jul 26 2017 Fedora Release Engineering <releng@fedoraproject.org> - 2.5.0-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Mass_Rebuild

* Fri Feb 10 2017 Fedora Release Engineering <releng@fedoraproject.org> - 2.5.0-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_26_Mass_Rebuild

* Tue Oct 25 2016 Radek Novacek <rnovacek@redhat.com> - 2.5.0-1
- Update to 2.5.0

* Thu Feb 04 2016 Fedora Release Engineering <releng@fedoraproject.org> - 2.4.0-0.16.beta3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_24_Mass_Rebuild

* Wed Jun 17 2015 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 2.4.0-0.15.beta3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_23_Mass_Rebuild

* Sat May 02 2015 Kalev Lember <kalevlember@gmail.com> - 2.4.0-0.14.beta3
- Rebuilt for GCC 5 C++11 ABI change

* Thu Mar 26 2015 Richard Hughes <rhughes@redhat.com> - 2.4.0-0.13.beta3
- Add an AppData file for the software center

* Sun Aug 17 2014 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 2.4.0-0.12.beta3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_21_22_Mass_Rebuild

* Sun Jun 08 2014 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 2.4.0-0.11.beta3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_21_Mass_Rebuild

* Sat Aug 03 2013 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 2.4.0-0.10.beta3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_20_Mass_Rebuild

* Thu Feb 14 2013 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 2.4.0-0.9.beta3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_19_Mass_Rebuild

* Tue Oct 30 2012 Kevin Kofler <Kevin@tigcc.ticalc.org> - 2.4.0-0.8.beta3
- Fix crash in video file preview on F3 (kde#309159, upstream patch)

* Sun Oct 28 2012 Kevin Kofler <Kevin@tigcc.ticalc.org> - 2.4.0-0.7.beta3
- Update to 2.4.0-beta3

* Mon Oct 15 2012 Radek Novacek <rnovacek@redhat.com> - 2.4.0-0.6.beta2
- Update to 2.4.0-beta2
- Drop g++ 4.7 patch (upstreamed)
- Add patch for generating manual page

* Thu Jul 19 2012 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 2.4.0-0.5.beta1
- Rebuilt for https://fedoraproject.org/wiki/Fedora_18_Mass_Rebuild

* Mon Jan 16 2012 Radek Novacek <rnovacek@redhat.com> 2.4.0-0.4.beta1
- Fix build failure with g++ 4.7

* Fri Jan 13 2012 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 2.4.0-0.3.beta1
- Rebuilt for https://fedoraproject.org/wiki/Fedora_17_Mass_Rebuild

* Thu Jul 28 2011 Kevin Kofler <Kevin@tigcc.ticalc.org> 2.4.0-0.2.beta1
- Run desktop-file-validate

* Mon Jun 27 2011 Radek Novacek <rnovacek@redhat.com> 2.4.0-0.1.beta1
- Update to 2.4.0-beta1
- Drop kdesu invocation patch, fixed upstream
- Drop xz patch, fixed upstream
- Install manpage

* Tue May 24 2011 Kevin Kofler <Kevin@tigcc.ticalc.org> 2.3.0-0.3.beta1
- Fix kdesu invocation for kdebase-runtime 4.6 (#705870, kde#271264)

* Mon Feb 07 2011 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 2.3.0-0.2.beta1
- Rebuilt for https://fedoraproject.org/wiki/Fedora_15_Mass_Rebuild

* Sun Jan 30 2011 Kevin Kofler <Kevin@tigcc.ticalc.org> 2.3.0-0.1.beta1
- Update to 2.3.0-beta1, drop upstreamed patches
- Update upstream URL to http://www.krusader.org/
- Use upstream xz patch (from git master, will be in the next release)
- Split out default-mimetypes patch for legacy lzma/tar.lzma, fix .bz2 (non-tar)

* Fri Aug 06 2010 Kevin Kofler <Kevin@tigcc.ticalc.org> 2.2.0-0.5.beta1
- fix Krusader not terminating properly (#620328, kde#240319, SF#3015094)
  (patch backported from upstream SVN trunk)

* Thu Jun 03 2010 Kevin Kofler <Kevin@tigcc.ticalc.org> 2.2.0-0.4.beta1
- fix "rename selects extension" patch to actually select all

* Thu Jun 03 2010 Kevin Kofler <Kevin@tigcc.ticalc.org> 2.2.0-0.3.beta1
- fix regression making single-click mode unusable
- fix regression: "rename selects extension" not working (SF#3003338)
- force reloading when opening a file in the viewer (SF#2969403)
- fix bogus toplevel entries being shown in the viewer menu (SF#2981303)
- fix the text viewer not being read-only (SF#2898151)

* Thu Jun 03 2010 Kevin Kofler <Kevin@tigcc.ticalc.org> 2.2.0-0.2.beta1
- BR kdebase4-devel for libkonq
- Drop explicit Requires: kdebase-runtime because it is dragged in by kdebase

* Thu Jun 03 2010 Kevin Kofler <Kevin@tigcc.ticalc.org> 2.2.0-0.1.beta1
- Update to 2.2.0-beta1 (#593523), fixes FTBFS (#565179), refresh bug (#504117)
- Drop gcc-4.4 patch, fixed upstream
- Apply XZ patches (#593525)
- Requires: kdebase-runtime (#575101)

* Fri Jul 24 2009 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> 2.0.0-2.1
- Rebuilt for https://fedoraproject.org/wiki/Fedora_12_Mass_Rebuild

* Mon Apr 20 2009 Marcin Garski <mgarski[AT]post.pl> 2.0.0-1.1
- Update to final 2.0.0

* Sun Mar 15 2009 Marcin Garski <mgarski[AT]post.pl> 2.0.0-0.5.beta2
- Fix compile error with Qt 4.5

* Wed Feb 25 2009 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> 2.0.0-0.4.beta2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_11_Mass_Rebuild

* Sun Dec 28 2008 Marcin Garski <mgarski[AT]post.pl> 2.0.0-0.3.beta2
- Update to 2.0.0-beta2

* Sat Oct 18 2008 Marcin Garski <mgarski[AT]post.pl> 2.0.0-0.2.beta1
- Incorporate minor bug fixes from Krusader's SVN

* Thu Oct 16 2008 Marcin Garski <mgarski[AT]post.pl> 2.0.0-0.1.beta1
- Update to 2.0.0-beta1

* Wed Oct 15 2008 Rex Dieter <rdieter@fedoraproject.org> 1.90.0-3
- s/crystalsvg/hicolor/ icon theme, so they show for everyone (#467076)

* Sun Apr 13 2008 Marcin Garski <mgarski[AT]post.pl> 1.90.0-2
- Update to 1.90.0
- Remove krusader-1.80.0-gcc43-compile-fix.patch, merged upstream

* Thu Feb 14 2008 Marcin Garski <mgarski[AT]post.pl> 1.80.0-5
- GCC 4.3 compile fix

* Sat Dec 15 2007 Marcin Garski <mgarski[AT]post.pl> 1.80.0-4
- Remove kdebindings-devel dependency (#425081)

* Sun Dec 09 2007 Marcin Garski <mgarski[AT]post.pl> 1.80.0-3
- BR: kdelibs3-devel kdebase3-devel

* Fri Aug 31 2007 Marcin Garski <mgarski[AT]post.pl> 1.80.0-2
- Fix license tag

* Thu Aug 02 2007 Marcin Garski <mgarski[AT]post.pl> 1.80.0-1
- Update to 1.80.0 (#249903)
- Preserve upstream .desktop vendor

* Fri Apr 20 2007 Marcin Garski <mgarski[AT]post.pl> 1.80.0-0.1.beta2
- Updated to version 1.80.0-beta2
- Drop X-Fedora category

* Fri Sep 01 2006 Marcin Garski <mgarski[AT]post.pl> 1.70.1-2
- Rebuild for Fedora Core 6
- Spec tweak

* Sat Jul 29 2006 Marcin Garski <mgarski[AT]post.pl> 1.70.1-1
- Updated to version 1.70.1 which fix CVE-2006-3816 (#200323)

* Mon Feb 13 2006 Marcin Garski <mgarski[AT]post.pl> 1.70.0-1
- Remove all patches (merged upstream)
- Updated to version 1.70.0

* Mon Jan 16 2006 Marcin Garski <mgarski[AT]post.pl> 1.60.1-6
- Remove --enable-final

* Mon Jan 16 2006 Marcin Garski <mgarski[AT]post.pl> 1.60.1-5
- Remove --disable-dependency-tracking

* Sun Jan 15 2006 Marcin Garski <mgarski[AT]post.pl> 1.60.1-4
- Change "/etc/profile.d/qt.sh" to "%%{_sysconfdir}/profile.d/qt.sh"
- Add --disable-debug --disable-dependency-tracking & --enable-final

* Wed Dec 14 2005 Marcin Garski <mgarski[AT]post.pl> 1.60.1-3
- Add to BR libacl-devel

* Tue Dec 13 2005 Marcin Garski <mgarski[AT]post.pl> 1.60.1-2
- Fix for modular X.Org

* Mon Dec 12 2005 Marcin Garski <mgarski[AT]post.pl> 1.60.1-1
- Updated to version 1.60.1 which fix CVE-2005-3856

* Sun Oct 23 2005 Marcin Garski <mgarski[AT]post.pl> 1.60.0-4
- Added update-mime-database and gtk-update-icon-cache (bug #171547)

* Thu Aug 25 2005 Marcin Garski <mgarski[AT]post.pl> 1.60.0-3
- Include .la files
- Include actions_tutorial.txt
- Fix krusader_root-mode.desktop file to show only in KDE and under System
  category
- Fix compile warnings

* Fri Aug 12 2005 Marcin Garski <mgarski[AT]post.pl> 1.60.0-2
- Spec improvements for Fedora Extras

* Wed Aug 10 2005 Marcin Garski <mgarski[AT]post.pl> 1.60.0-1
- Updated to version 1.60.0 & clean up for Fedora Extras

* Fri Dec 17 2004 Marcin Garski <mgarski[AT]post.pl> 1.51.fc2kde331
- Updated to version 1.51

* Thu Nov 11 2004 Marcin Garski <mgarski[AT]post.pl> 1.50.fc2kde331
- Added Requires:

* Tue Nov 02 2004 Marcin Garski <mgarski[AT]post.pl> 1.50.fc2
- Updated to version 1.50 & spec cleanup

* Fri Aug 06 2004 Marcin Garski <mgarski[AT]post.pl> 1.40-1.fc2
- Updated to version 1.40

* Wed Jun 23 2004 Marcin Garski <mgarski[AT]post.pl> 1.40-beta2.fc2
- Updated to version 1.40-beta2

* Wed Jun 02 2004 Marcin Garski <mgarski[AT]post.pl> 1.40-beta1.fc2
- Rebuild for Fedora Core 2 & huge spec cleanup

* Mon Nov 17 2003 11:05:00 Marian POPESCU <softexpert[AT]libertysurf.fr> [1.30]
- Updated to 1.30 release + changed description to match the official one

* Thu Jul 03 2003 17:00:00 Marcin Garski <mgarski[AT]post.pl> [1.20]
- Initial specfile
