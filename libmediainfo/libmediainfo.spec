%global libzen_version  0.4.37

Name:           libmediainfo
Version:        18.12
Release:        1%{?dist}
Summary:        Library for supplies technical and tag information about a video or audio file
Summary(ru):    Библиотека для предоставления полной информации о видео или аудио файле

License:        BSD
URL:            http://mediaarea.net/MediaInfo
Source0:        http://mediaarea.net/download/source/%{name}/%{version}/%{name}_%{version}.tar.xz

BuildRequires:  cmake
BuildRequires:  gcc-c++
BuildRequires:  pkgconfig(libzen) >= %{libzen_version}
BuildRequires:  pkgconfig(zlib)
BuildRequires:  doxygen
BuildRequires:  pkgconfig(libcurl)
BuildRequires:  pkgconfig(tinyxml2)

Provides:       bundled(md5-plumb)

%description
This package contains the shared library for MediaInfo.
MediaInfo supplies technical and tag information about a video or
audio file.

What information can I get from MediaInfo?
* General: title, author, director, album, track number, date, duration...
* Video: codec, aspect, fps, bitrate...
* Audio: codec, sample rate, channels, language, bitrate...
* Text: language of subtitle
* Chapters: number of chapters, list of chapters

DivX, XviD, H263, H.263, H264, x264, ASP, AVC, iTunes, MPEG-1,
MPEG1, MPEG-2, MPEG2, MPEG-4, MPEG4, MP4, M4A, M4V, QuickTime,
RealVideo, RealAudio, RA, RM, MSMPEG4v1, MSMPEG4v2, MSMPEG4v3,
VOB, DVD, WMA, VMW, ASF, 3GP, 3GPP, 3GP2

What format (container) does MediaInfo support?
* Video: MKV, OGM, AVI, DivX, WMV, QuickTime, Real, MPEG-1,
  MPEG-2, MPEG-4, DVD (VOB) (Codecs: DivX, XviD, MSMPEG4, ASP,
  H.264, AVC...)
* Audio: OGG, MP3, WAV, RA, AC3, DTS, AAC, M4A, AU, AIFF
* Subtitles: SRT, SSA, ASS, SAMI


%description -l ru
Данный пакет содержит разделяемую библиотеку для MediaInfo.
MediaInfo предоставляет полную информацию о видео или аудио файле.

Какая информация может быть получена MediaInfo?
* Общее: title, author, director, album, track number, date, duration...
* Видео: codec, aspect, fps, bitrate...
* Аудио: codec, sample rate, channels, language, bitrate...
* Текст: язык субтитров
* Части: число частей, список частей

DivX, XviD, H263, H.263, H264, x264, ASP, AVC, iTunes, MPEG-1,
MPEG1, MPEG-2, MPEG2, MPEG-4, MPEG4, MP4, M4A, M4V, QuickTime,
RealVideo, RealAudio, RA, RM, MSMPEG4v1, MSMPEG4v2, MSMPEG4v3,
VOB, DVD, WMA, VMW, ASF, 3GP, 3GPP, 3GP2

Какой формат (контейнер) поддерживает MediaInfo?
* Видео: MKV, OGM, AVI, DivX, WMV, QuickTime, Real, MPEG-1,
  MPEG-2, MPEG-4, DVD (VOB) (Codecs: DivX, XviD, MSMPEG4, ASP,
  H.264, AVC...)
* Аудио: OGG, MP3, WAV, RA, AC3, DTS, AAC, M4A, AU, AIFF
* Субтитры: SRT, SSA, ASS, SAMI


%package        devel
Summary:        Include files and mandatory libraries for development
Summary(ru):    Пакет с файлами для разработки %{name}
Requires:       %{name}%{?_isa} = %{version}-%{release}
Requires:       libzen-devel%{?_isa} >= %{libzen_version}

%description    devel
Include files and mandatory libraries for development.

%description    devel -l ru
Файлы для разработки %{name}.

%prep
%autosetup -n MediaInfoLib

cp           Release/ReadMe_DLL_Linux.txt ReadMe.txt
mv           History_DLL.txt History.txt
sed -i 's/.$//' *.txt Source/Example/*

find . -type f -exec chmod 644 {} ';'

rm -rf Project/MSCS20*
rm -rf Source/ThirdParty/tinyxml2

%build
pushd Source/Doc/
    doxygen -u Doxyfile
    doxygen Doxyfile
popd
cp Source/Doc/*.html ./

mkdir Project/CMake/build
pushd Project/CMake/build
    %cmake ..
    %make_build
popd

%install
pushd Project/CMake/build
    %make_install
popd

install -m 644 -p Source/MediaInfoDLL/MediaInfoDLL.cs %{buildroot}%{_includedir}/MediaInfoDLL
install -m 644 -p Source/MediaInfoDLL/MediaInfoDLL.JNA.java %{buildroot}%{_includedir}/MediaInfoDLL
install -m 644 -p Source/MediaInfoDLL/MediaInfoDLL.JNative.java %{buildroot}%{_includedir}/MediaInfoDLL
install -m 644 -p Source/MediaInfoDLL/MediaInfoDLL.py %{buildroot}%{_includedir}/MediaInfoDLL
install -m 644 -p Source/MediaInfoDLL/MediaInfoDLL3.py %{buildroot}%{_includedir}/MediaInfoDLL

rm -f %{buildroot}%{_libdir}/%{name}.la


%files
%doc History.txt ReadMe.txt
%license License.html
%{_libdir}/%{name}.so.*

%files    devel
%doc Changes.txt Documentation.html Doc Source/Example
%{_includedir}/MediaInfo
%{_includedir}/MediaInfoDLL
%{_libdir}/%{name}.so
%{_libdir}/pkgconfig/*.pc
%{_libdir}/cmake/mediainfolib/

%changelog
* Thu Dec 13 2018 Vasiliy N. Glazov <vascom2@gmail.com> - 18.12-1
- Update to 18.12

* Tue Nov 27 2018 Igor Gnatenko <ignatenkobrain@fedoraproject.org> - 18.08.1-2
- Rebuild for tinyxml2 7.x

* Tue Sep 11 2018 Vasiliy N. Glazov <vascom2@gmail.com> - 18.08.1-1
- Update to 18.08.1

* Mon Sep 03 2018 Vasiliy N. Glazov <vascom2@gmail.com> - 18.08-1
- Update to 18.08

* Fri Jul 13 2018 Fedora Release Engineering <releng@fedoraproject.org> - 18.05-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_29_Mass_Rebuild

* Thu May 10 2018 Vasiliy N. Glazov <vascom2@gmail.com> - 18.05-1
- Update to 18.05

* Tue Mar 20 2018 Vasiliy N. Glazov <vascom2@gmail.com> - 18.03-1
- Update to 18.03

* Wed Feb 07 2018 Fedora Release Engineering <releng@fedoraproject.org> - 17.12-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_28_Mass_Rebuild

* Tue Jan 23 2018 Vasiliy N. Glazov <vascom2@gmail.com> - 17.12-2
- Rebuild because libtinyxml2 api change

* Fri Dec 22 2017 Vasiliy N. Glazov <vascom2@gmail.com> - 17.12-1
- Update to 17.12

* Fri Dec 01 2017 Vasiliy N. Glazov <vascom2@gmail.com> - 17.10-2
- Revert .so version

* Tue Nov 07 2017 Vasiliy N. Glazov <vascom2@gmail.com> - 17.10-1
- Update to 17.10
- New versioning scheme

* Wed Sep 13 2017 Vasiliy N. Glazov <vascom2@gmail.com> - 0.7.99-1
- Update to 0.7.99

* Tue Aug 15 2017 Vasiliy N. Glazov <vascom2@gmail.com> - 0.7.98-1
- Update to 0.7.98

* Thu Aug 03 2017 Fedora Release Engineering <releng@fedoraproject.org> - 0.7.97-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Binutils_Mass_Rebuild

* Wed Jul 26 2017 Fedora Release Engineering <releng@fedoraproject.org> - 0.7.97-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Mass_Rebuild

* Mon Jul 10 2017 Vasiliy N. Glazov <vascom2@gmail.com> - 0.7.97-1
- Update to 0.7.97

* Mon Jun 19 2017 Vasiliy N. Glazov <vascom2@gmail.com> - 0.7.96-1
- Update to 0.7.96

* Mon May 15 2017 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 0.7.95-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_26_27_Mass_Rebuild

* Wed May 10 2017 Vasiliy N. Glazov <vascom2@gmail.com> - 0.7.95-1
- Update to 0.7.95

* Thu Apr 06 2017 Vasiliy N. Glazov <vascom2@gmail.com> - 0.7.94-1
- Update to 0.7.94

* Mon Mar 06 2017 Vasiliy N. Glazov <vascom2@gmail.com> - 0.7.93-1
- Update to 0.7.93

* Mon Feb 06 2017 Vasiliy N. Glazov <vascom2@gmail.com> - 0.7.92.1-1
- Update to 0.7.92.1

* Mon Dec 05 2016 Vasiliy N. Glazov <vascom2@gmail.com> - 0.7.91-1
- Update to 0.7.91

* Fri Nov 11 2016 Vasiliy N. Glazov <vascom2@gmail.com> - 0.7.90-1
- Update to 0.7.90

* Tue Oct 04 2016 Vasiliy N. Glazov <vascom2@gmail.com> - 0.7.89-1
- Update to 0.7.89

* Thu Sep 15 2016 Vasiliy N. Glazov <vascom2@gmail.com> - 0.7.88-1
- Update to 0.7.88

* Wed Jul 06 2016 Vasiliy N. Glazov <vascom2@gmail.com> - 0.7.87-1
- Update to 0.7.87

* Wed Jun 01 2016 Vasiliy N. Glazov <vascom2@gmail.com> - 0.7.86-1
- Update to 0.7.86

* Sun May 22 2016 Rich Mattes <richmattes@gmail.com> - 0.7.85-2
- Rebuild for tinyxml2-3.0.0

* Thu May 05 2016 Vasiliy N. Glazov <vascom2@gmail.com> - 0.7.85-1
- Update to 0.7.85

* Fri Apr 01 2016 Vasiliy N. Glazov <vascom2@gmail.com> - 0.7.84-1
- Update to 0.7.84

* Wed Mar 02 2016 Vasiliy N. Glazov <vascom2@gmail.com> - 0.7.83-1
- Update to 0.7.83

* Thu Feb 04 2016 Vasiliy N. Glazov <vascom2@gmail.com> 0.7.82-1
- Update to 0.7.82

* Thu Feb 04 2016 Fedora Release Engineering <releng@fedoraproject.org> - 0.7.81-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_24_Mass_Rebuild

* Wed Jan 20 2016 Vasiliy N. Glazov <vascom2@gmail.com> 0.7.81-1
- Update to 0.7.81

* Thu Dec 03 2015 Vasiliy N. Glazov <vascom2@gmail.com> 0.7.80-1
- Update to 0.7.80

* Mon Sep 28 2015 Vasiliy N. Glazov <vascom2@gmail.com> 0.7.77-1
- Update to 0.7.77

* Fri Aug 14 2015 Vasiliy N. Glazov <vascom2@gmail.com> 0.7.76-1
- Update to 0.7.76

* Fri Jul 17 2015 Vasiliy N. Glazov <vascom2@gmail.com> 0.7.75-2
- Bump release

* Fri Jul 10 2015 Vasiliy N. Glazov <vascom2@gmail.com> 0.7.75-1
- Update to 0.7.75

* Wed Jun 17 2015 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 0.7.74-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_23_Mass_Rebuild

* Wed May 27 2015 Vasiliy N. Glazov <vascom2@gmail.com> 0.7.74-1
- Update to 0.7.74

* Wed Apr 22 2015 Vasiliy N. Glazov <vascom2@gmail.com> - 0.7.73-2
- Correct lib version

* Fri Apr 10 2015 Vasiliy N. Glazov <vascom2@gmail.com> 0.7.73-1
- Update to 0.7.73

* Tue Jan 13 2015 Vasiliy N. Glazov <vascom2@gmail.com> 0.7.72-1
- Update to 0.7.72
- Drop some patches

* Wed Dec 10 2014 Ivan Romanov <drizt@land.ru> - 0.7.71-3
- fixed epel7 building

* Sun Dec  7 2014 Ivan Romanov <drizt@land.ru> - 0.7.71-2
- use cmake

* Wed Nov 12 2014 Vasiliy N. Glazov <vascom2@gmail.com> 0.7.71-1
- Update to 0.7.71

* Thu Sep 25 2014 Vasiliy N. Glazov <vascom2@gmail.com> 0.7.70-1
- Update to 0.7.70

* Sun Aug 17 2014 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 0.7.69-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_21_22_Mass_Rebuild

* Sat Jun 07 2014 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 0.7.69-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_21_Mass_Rebuild

* Tue Jun 03 2014 Vasiliy N. Glazov <vascom2@gmail.com> 0.7.69-1
- Update to 0.7.69

* Fri May 23 2014 Vasiliy N. Glazov <vascom2@gmail.com> 0.7.68-2
- Update for tinyxml2 changes

* Tue Apr 08 2014 Vasiliy N. Glazov <vascom2@gmail.com> 0.7.68-1
- Update to 0.7.68

* Mon Feb 24 2014 Vasiliy N. Glazov <vascom2@gmail.com> 0.7.67-2
- Correct description and summary
- Added provides for md5
- Corrected obsolete m4 macros

* Fri Feb 21 2014 Vasiliy N. Glazov <vascom2@gmail.com> 0.7.67-1
- Update to 0.7.67

* Thu Dec 12 2013 Vasiliy N. Glazov <vascom2@gmail.com> 0.7.65-1
- Update to 0.7.65

* Sat Nov 02 2013 Vasiliy N. Glazov <vascom2@gmail.com> 0.7.64-4
- Some small corrections in spec

* Wed Jul 31 2013 Vasiliy N. Glazov <vascom2@gmail.com> 0.7.64-3
- Corrected make flags and use install macros

* Tue Jul 30 2013 Vasiliy N. Glazov <vascom2@gmail.com> 0.7.64-2
- just rebuild

* Fri Jul 12 2013 Vasiliy N. Glazov <vascom2@gmail.com> 0.7.64-1
- update to 0.7.64

* Fri May 31 2013 Vasiliy N. Glazov <vascom2@gmail.com> 0.7.63-1
- update to 0.7.63

* Tue Apr 23 2013 Vasiliy N. Glazov <vascom2@gmail.com> 0.7.62-2
- Removed dos2unix from BR
- Correcting encoding for all files
- Corrected config and build
- Enable curl support

* Wed Mar 20 2013 Vasiliy N. Glazov <vascom2@gmail.com> 0.7.62-1
- update to 0.7.62

* Tue Oct 23 2012 Vasiliy N. Glazov <vascom2@gmail.com> 0.7.61-1
- Update to 0.7.61

* Mon Sep 03 2012 Vasiliy N. Glazov <vascom2@gmail.com> 0.7.60-1
- Update to 0.7.60

* Tue Jun 05 2012 Vasiliy N. Glazov <vascom2@gmail.com> 0.7.58-1
- Update to 0.7.58

* Fri May 04 2012 Vasiliy N. Glazov <vascom2@gmail.com> 0.7.57-1
- Update to 0.7.57

* Wed Apr 11 2012 Vasiliy N. Glazov <vascom2@gmail.com> 0.7.56-1
- Update to 0.7.56

* Tue Mar 20 2012 Vasiliy N. Glazov <vascom2@gmail.com> 0.7.54-1
- Update to 0.7.54

* Thu Feb 09 2012 Vasiliy N. Glazov <vascom2@gmail.com> 0.7.53-1
- Update to 0.7.53

* Thu Dec 22 2011 Vasiliy N. Glazov <vascom2@gmail.com> 0.7.52-1
- Update to 0.7.52

* Tue Nov 22 2011 Vasiliy N. Glazov <vascom2@gmail.com> 0.7.51-2
- Added description in russian language

* Mon Nov 14 2011 Vasiliy N. Glazov <vascom2@gmail.com> 0.7.51-1
- Update to 0.7.51

* Tue Sep 27 2011 Vasiliy N. Glazov <vascom2@gmail.com> 0.7.50-1
- Update to 0.7.50

* Mon Sep 19 2011 Vasiliy N. Glazov <vascom2@gmail.com> 0.7.49-1
- Update to 0.7.49

* Fri Aug 19 2011 Vasiliy N. Glazov <vascom2@gmail.com> 0.7.48-1
- Update to 0.7.48

* Tue Aug 09 2011 Vasiliy N. Glazov <vascom2@gmail.com> 0.7.47-2
- Removed 0 from name

* Fri Aug 05 2011 Vasiliy N. Glazov <vascom2@gmail.com> 0.7.47-1
- Initial release
