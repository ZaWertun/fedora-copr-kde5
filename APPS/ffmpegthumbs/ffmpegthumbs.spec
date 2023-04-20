Name:    ffmpegthumbs
Version: 23.04.0
Release: 1%{?dist}
Summary: KDE ffmpegthumbnailer service

License: GPL-2.0-or-later
URL:     https://apps.kde.org/%{name}/
%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0: https://download.kde.org/%{stable}/release-service/%{version}/src/%{name}-%{version}.tar.xz
Source1: https://download.kde.org/%{stable}/release-service/%{version}/src/%{name}-%{version}.tar.xz.sig
Source2: gpgkey-D81C0CB38EB725EF6691C385BB463350D6EF31EF.gpg

BuildRequires: gnupg2
BuildRequires: gcc-c++
BuildRequires: cmake
BuildRequires: desktop-file-utils
BuildRequires: libappstream-glib

BuildRequires: extra-cmake-modules
BuildRequires: pkgconfig(libjpeg)
BuildRequires: cmake(KF5KIO)
BuildRequires: cmake(KF5I18n)
BuildRequires: ffmpeg-free-devel
BuildRequires: pkgconfig(taglib)

Provides: kffmpegthumbnailer = %{version}-%{release}
Provides: kdemultimedia-extras-freeworld = %{version}-%{release}

%description
%{summary}.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -p1


%build
%{cmake_kf5}

%cmake_build


%install
%cmake_install


%check
appstream-util validate-relax --nonet %{buildroot}%{_kf5_metainfodir}/org.kde.%{name}.metainfo.xml


%files
%license LICENSES/GPL-2.0-or-later.txt
%{_kf5_qtplugindir}/ffmpegthumbs.so
%dir %{_kf5_datadir}/kservices5
%{_kf5_datadir}/kservices5/ffmpegthumbs.desktop
%dir %{_kf5_datadir}/config.kcfg
%{_kf5_datadir}/config.kcfg/ffmpegthumbnailersettings5.kcfg
%dir %{_kf5_datadir}/qlogging-categories5
%{_kf5_datadir}/qlogging-categories5/ffmpegthumbs.categories
%{_kf5_metainfodir}/org.kde.%{name}.metainfo.xml


%changelog
* Thu Apr 20 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.04.0-1
- 23.04.0

* Thu Mar 02 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.12.3-1
- 22.12.3

* Thu Feb 02 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.12.2-1
- 22.12.2

* Wed Jan 04 2023 Justin Zobel <justin@1707.io> - 22.12.1-1
- Update to 22.12.1

* Thu Dec 29 2022 Neal Gompa <ngompa@fedoraproject.org> - 22.12.0-1
- Update to 22.12.0 and move to Fedora

* Sat Nov 19 2022 Sérgio Basto <sergio@serjux.com> - 22.08.3-1
- Update ffmpegthumbs to 22.08.3

* Sat Sep 24 2022 Sérgio Basto <sergio@serjux.com> - 22.08.1-1
- Update ffmpegthumbs to 22.08.1

* Sun Aug 07 2022 RPM Fusion Release Engineering <sergiomb@rpmfusion.org> - 22.04.3-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_37_Mass_Rebuild and ffmpeg
  5.1

* Thu Aug 04 2022 Leigh Scott <leigh123linux@gmail.com> - 22.04.3-1
- Update ffmpegthumbs to 22.04.3

* Tue Apr 26 2022 Leigh Scott <leigh123linux@gmail.com> - 22.04.0-1
- Update ffmpegthumbs to 22.04.0

* Sat Feb 26 2022 Sérgio Basto <sergio@serjux.com> - 21.12.2-1
- Update ffmpegthumbs to 21.12.2
- Add ffmpeg-5 PR
- lang has disappear from the package

* Wed Feb 09 2022 RPM Fusion Release Engineering <sergiomb@rpmfusion.org> - 21.04.2-4
- Rebuilt for https://fedoraproject.org/wiki/Fedora_36_Mass_Rebuild

* Thu Nov 11 2021 Leigh Scott <leigh123linux@gmail.com> - 21.04.2-3
- Rebuilt for new ffmpeg snapshot

* Mon Aug 02 2021 RPM Fusion Release Engineering <leigh123linux@gmail.com> - 21.04.2-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_35_Mass_Rebuild

* Sat Jun 12 2021 Sérgio Basto <sergio@serjux.com> - 21.04.2-1
- Update ffmpegthumbs to 21.04.2

* Sun Feb 21 2021 Sérgio Basto <sergio@serjux.com> - 20.12.2-1
- Update ffmpegthumbs to 20.12.2
- Add missing dependency on taglib

* Wed Feb 03 2021 RPM Fusion Release Engineering <leigh123linux@gmail.com> - 20.08.1-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_34_Mass_Rebuild

* Thu Dec 31 2020 Leigh Scott <leigh123linux@gmail.com> - 20.08.1-2
- Rebuilt for new ffmpeg snapshot

* Mon Sep 14 2020 Sérgio Basto <sergio@serjux.com> - 20.08.1-1
- Update ffmpegthumbs to 20.08.1

* Mon Aug 17 2020 RPM Fusion Release Engineering <leigh123linux@gmail.com> - 19.12.1-4
- Rebuilt for https://fedoraproject.org/wiki/Fedora_33_Mass_Rebuild

* Sat Feb 22 2020 RPM Fusion Release Engineering <leigh123linux@googlemail.com> - 19.12.1-3
- Rebuild for ffmpeg-4.3 git

* Tue Feb 04 2020 RPM Fusion Release Engineering <leigh123linux@gmail.com> - 19.12.1-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_32_Mass_Rebuild

* Wed Jan 22 2020 Sérgio Basto <sergio@serjux.com> - 19.12.1-1
- Update ffmpegthumbs to 19.12.1
- Fix build

* Wed Sep 25 2019 Leigh Scott <leigh123linux@googlemail.com> - 19.08.1-1
- 19.08.1

* Tue Aug 06 2019 Leigh Scott <leigh123linux@gmail.com> - 18.12.3-2
- Rebuild for new ffmpeg version

* Thu Mar 14 2019 Rex Dieter <rdieter@fedoraproject.org> - 18.12.3-1
- 18.12.3

* Mon Mar 04 2019 RPM Fusion Release Engineering <leigh123linux@gmail.com> - 18.12.2-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_30_Mass_Rebuild

* Thu Feb 28 2019 Rex Dieter <rdieter@fedoraproject.org> - 18.12.2-1
- 18.12.2

* Mon Sep 17 2018 Sérgio Basto <sergio@serjux.com> - 18.08.1-1
- Update ffmpegthumbs to 18.08.1

* Thu Jul 26 2018 RPM Fusion Release Engineering <leigh123linux@gmail.com> - 17.12.0-5
- Rebuilt for https://fedoraproject.org/wiki/Fedora_29_Mass_Rebuild

* Thu Mar 08 2018 RPM Fusion Release Engineering <leigh123linux@googlemail.com> - 17.12.0-4
- Rebuilt for new ffmpeg snapshot

* Thu Mar 01 2018 RPM Fusion Release Engineering <leigh123linux@googlemail.com> - 17.12.0-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_28_Mass_Rebuild

* Thu Jan 18 2018 Leigh Scott <leigh123linux@googlemail.com> - 17.12.0-2
- Rebuilt for ffmpeg-3.5 git

* Fri Dec 29 2017 Sérgio Basto <sergio@serjux.com> - 17.12.0-1
- Update ffmpegthumbs to 17.12.0

* Tue Oct 17 2017 Leigh Scott <leigh123linux@googlemail.com> - 17.08.1-2
- Rebuild for ffmpeg update

* Sun Oct 08 2017 Sérgio Basto <sergio@serjux.com> - 17.08.1-1
- Update to 17.08.1

* Thu Aug 31 2017 RPM Fusion Release Engineering <kwizart@rpmfusion.org> - 17.04.2-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Mass_Rebuild

* Sun Jun 11 2017 Sérgio Basto <sergio@serjux.com> - 17.04.2-1
- Update to 17.04.2

* Sat Apr 29 2017 Leigh Scott <leigh123linux@googlemail.com> - 16.12.3-2
- Rebuild for ffmpeg update

* Wed Mar 29 2017 Sérgio Basto <sergio@serjux.com> - 16.12.3-1
- Update to 16.12.3

* Sun Mar 19 2017 RPM Fusion Release Engineering <kwizart@rpmfusion.org> - 16.12.2-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_26_Mass_Rebuild

* Tue Feb 21 2017 Sérgio Basto <sergio@serjux.com> - 16.12.2-1
- Update ffmpegthumbs to 16.12.2 following Fedora KDE applications

* Thu Oct 13 2016 Sérgio Basto <sergio@serjux.com> - 16.08.2-1
- Update to 16.08.2

* Thu Sep 15 2016 Sérgio Basto <sergio@serjux.com> - 16.08.1-1
- Update to 16.08.1
- Drop Port-to-libavfilter-for-deinterlacing.patch is upstreamed.

* Sat Aug 20 2016 Sérgio Basto <sergio@serjux.com> - 16.04.3-1
- Update to 16.04.3, rfbz #4164, following kdemultimedia of Fedora proper,
  the ffmpegthumbs package is not ffmpegthumbnailer, neither kffmpegthumbnailer
  packages, these 3 packages have a very similar names but just ffmpegthumbs is
  part of kdemultimedia.

* Sat Jul 30 2016 Julian Sikorski <belegdol@fedoraproject.org> - 16.04.2-3
- Rebuilt for ffmpeg-3.1.1

* Fri Jul 08 2016 Leigh Scott <leigh123linux@googlemail.com> - 16.04.2-2
- fix f23 build

* Fri Jul 08 2016 Leigh Scott <leigh123linux@googlemail.com> - 16.04.2-1
- Update to 16.04.2 release
- patch for ffmpeg-3.0

* Sun Oct 19 2014 Sérgio Basto <sergio@serjux.com> - 4.13.97-3
- Rebuilt for FFmpeg 2.4.3

* Fri Sep 26 2014 Nicolas Chauvet <kwizart@gmail.com> - 4.13.97-2
- Rebuilt for FFmpeg 2.4.x

* Wed Aug 06 2014 Rex Dieter <rdieter@fedoraproject.org> 4.13.97-1
- 4.13.97

* Wed Aug 06 2014 Rex Dieter <rdieter@fedoraproject.org> 4.13.3-1
- 4.13.3

* Sat Mar 29 2014 Sérgio Basto <sergio@serjux.com> - 4.11.3-2
- Rebuilt for ffmpeg-2.2

* Wed Nov 27 2013 Rex Dieter <rdieter@fedoraproject.org> 4.11.3-1
- 4.11.3

* Tue Oct 01 2013 Rex Dieter <rdieter@fedoraproject.org> 4.11.1-1
- 4.11.1

* Thu Aug 15 2013 Nicolas Chauvet <kwizart@gmail.com> - 4.10.1-3
- Rebuilt for FFmpeg 2.0.x

* Sun May 26 2013 Nicolas Chauvet <kwizart@gmail.com> - 4.10.1-2
- Rebuilt for x264/FFmpeg

* Fri Apr 05 2013 Rex Dieter <rdieter@fedoraproject.org> 4.10.1-1
- 4.10.1

* Wed Jan 16 2013 Rex Dieter <rdieter@fedoraproject.org> 4.9.97-1
- 4.9.97

* Sat Nov 24 2012 Nicolas Chauvet <kwizart@gmail.com> - 4.9.3-2
- Rebuilt for FFmpeg 1.0

* Thu Nov 08 2012 Rex Dieter <rdieter@fedoraproject.org> 4.9.3-1
- 4.9.3

* Wed Sep 12 2012 Rex Dieter <rdieter@fedoraproject.org> 4.9.1-1
- 4.9.1

* Thu Aug 30 2012 Rex Dieter <rdieter@fedoraproject.org> 4.9.0-1
- 4.9.0

* Mon Jun 18 2012 Rex Dieter <rdieter@fedoraproject.org> 4.8.90-4
- ffmpegthumbs

* Fri Mar 02 2012 Nicolas Chauvet <kwizart@gmail.com> - 4.7.2-4
- Rebuilt for c++ ABI breakage

* Tue Feb 28 2012 Nicolas Chauvet <kwizart@gmail.com> - 4.7.2-3
- Rebuilt for x264/FFmpeg

* Wed Feb 08 2012 Nicolas Chauvet <kwizart@gmail.com> - 4.7.2-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_17_Mass_Rebuild

* Tue Nov 01 2011 Rex Dieter <rdieter@fedoraproject.org> 4.7.2-1
- 4.7.2

* Mon Sep 26 2011 Nicolas Chauvet <kwizart@gmail.com> - 4.7.0-2
- Rebuilt for FFmpeg-0.8

* Fri Aug 12 2011 Magnus Tuominen magnus.tuominen@gmail.com> 4.7.0-1
- 4.7.0
- patch50 no longer needed

* Fri Apr 08 2011 Rex Dieter <rdieter@fedoraproject.org> 4.6.1-1
- 4.6.1

* Sun Jan 23 2011 Rex Dieter <rdieter@fedoraproject.org> - 4.6.0-1
- 4.6.0

* Thu Dec 09 2010 Rex Dieter <rdieter@fedoraproject.org> - 4.5.85-1
- 4.5.85 (4.6beta2)
- drop Obsoletes/Provides ffmpegthumnailer

* Mon Nov 22 2010 Rex Dieter <rdieter@fedoraproject.org> - 4.5.80-1
- 4.5.80 (4.6beta1)

* Mon Nov 22 2010 Rex Dieter <rdieter@fedoraproject.org> - 4.5.3-2
- Obsoletes: ffmpegthumbnailer-devel too

* Thu Nov 18 2010 Rex Dieter <rdieter@fedoraproject.org> - 4.5.3-1
- 4.5.3

* Fri Oct 15 2010 Rex Dieter <rdieter@fedoraproject.org> - 4.5.2-1
- 4.5.2

* Sun Sep 19 2010 Magnus Tuominen <magnus.tuominen@gmail.com> - 4.5.1-2
- drop patch
- obsolete < 15

* Mon Sep 13 2010 Magnus Tuominen <magnus.tuominen@gmail.com> - 4.5.1-1
- first attempt on kdemultimedia-extras-freeworld
