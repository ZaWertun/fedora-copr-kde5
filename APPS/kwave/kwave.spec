%undefine __cmake_in_source_build
Name:           kwave
Version: 21.12.2
Release: 1%{?dist}
Summary:        Sound Editor for KDE
Summary(de):    Sound-Editor für KDE

# See the file LICENSES for the licensing scenario
License:        GPLv2+ and BSD and CC-BY-SA
URL:            http://kwave.sourceforge.net
%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0:        http://download.kde.org/%{stable}/release-service/%{version}/src/%{name}-%{version}.tar.xz

BuildRequires:  extra-cmake-modules
BuildRequires:  cmake(Qt5Multimedia)
BuildRequires:  cmake(KF5Archive)
BuildRequires:  cmake(KF5CoreAddons)
BuildRequires:  cmake(KF5WidgetsAddons)
BuildRequires:  cmake(KF5I18n)
BuildRequires:  cmake(KF5Completion)
BuildRequires:  cmake(KF5KIO)
BuildRequires:  cmake(KF5IconThemes)
BuildRequires:  cmake(KF5Crash)
BuildRequires:  cmake(KF5DBusAddons)
BuildRequires:  cmake(KF5TextWidgets)
BuildRequires:  cmake(KF5DocTools)
BuildRequires:  alsa-lib-devel
BuildRequires:  audiofile-devel >= 0.3.0
BuildRequires:  desktop-file-utils
BuildRequires:  fdupes
BuildRequires:  fftw-devel >= 3.0
BuildRequires:  flac-devel
BuildRequires:  gettext
BuildRequires:  id3lib-devel >= 3.8.1
BuildRequires:  ImageMagick
BuildRequires:  libappstream-glib
BuildRequires:  libmad-devel
BuildRequires:  libsamplerate-devel
BuildRequires:  libvorbis-devel
BuildRequires:  opus-devel
BuildRequires:  poxml
BuildRequires:  pulseaudio-libs-devel >= 0.9.16

Requires:       %{name}-doc = %{version}-%{release}

%description
With Kwave you can record, play back, import and edit many sorts of audio files
including multi-channel files. Kwave includes some plugins to transform audio
files in several ways and presents a graphical view with a complete zoom- and
scroll capability.

%description -l de
Mit Kwave können Sie ein- oder mehrkanalige Audio-Dateien aufnehmen, wieder-
geben, importieren und bearbeiten. Kwave verfügt über Plugins zum Umwandeln
von Audio-Dateien auf verschiedene Weise. Die grafische Oberfläche bietet
alle Möglichkeiten für Änderungen der Ansichtsgröße und zum Rollen.

%package doc
Summary:        User manuals for %{name}
Summary(de):    Benutzerhandbücher für %{name}
License:        GFDL
BuildArch:      noarch


%description doc
This package contains arch-independent files for %{name}, especially the
HTML documentation.

%description doc -l de
Dieses Paket enthält architekturunabhängige Dateien für %{name},
speziell die HTML-Dokumentation.

%prep
%autosetup -p1

%build
%{cmake_kf5} -DWITH_MP3=ON
%cmake_build

%install
%cmake_install
gzip -dS z %{buildroot}%{_datadir}/icons/hicolor/scalable/actions/*.svgz
gzip -dS z %{buildroot}%{_datadir}/icons/hicolor/scalable/apps/kwave.svgz

%find_lang %{name}

%check
appstream-util validate-relax --nonet %{buildroot}%{_kf5_metainfodir}/org.kde.%{name}.appdata.xml
desktop-file-validate %{buildroot}%{_kf5_datadir}/applications/org.kde.%{name}.desktop

%files -f %{name}.lang
%doc AUTHORS CHANGES README TODO
%license GNU-LICENSE LICENSES
%{_kf5_bindir}/%{name}
%{_kf5_datadir}/applications/org.kde.%{name}.desktop
%{_kf5_metainfodir}/org.kde.%{name}.appdata.xml
%{_kf5_datadir}/icons/hicolor/*/apps/%{name}.*
%{_kf5_datadir}/icons/hicolor/*/actions/%{name}*
%{_kf5_datadir}/%{name}/
%{_kf5_datadir}/kservicetypes5/%{name}-plugin.desktop
%{_kf5_qtplugindir}/%{name}/
%{_kf5_libdir}/lib%{name}.so.*
%{_kf5_libdir}/lib%{name}gui.so.*

%files doc
%{_kf5_docdir}/HTML/*/%{name}

%changelog
* Thu Feb 03 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 21.12.2-1
- 21.12.2

* Thu Jan 06 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 21.12.1-1
- 21.12.1

* Thu Dec 09 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 21.12.0-1
- 21.12.0

* Thu Nov 04 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 21.08.3-1
- 21.08.3

* Thu Oct 07 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 21.08.2-1
- 21.08.2

* Thu Sep 02 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 21.08.1-1
- 21.08.1

* Thu Aug 12 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 21.08.0-1
- 21.08.0

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

* Thu Jan  7 22:09:40 MSK 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.12.1-1
- 20.12.1

* Thu Dec 10 21:56:39 MSK 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.12.0-1
- 20.12.0

* Wed Nov 18 22:21:58 MSK 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.08.3-1
- 20.08.3

* Tue Sep 15 2020 Rex Dieter <rdieter@fedoraproject.org> - 20.08.1-1
- 20.08.1

* Tue Aug 18 2020 Rex Dieter <rdieter@fedoraproject.org> - 20.08.0-1
- 20.08.0

* Tue Jul 28 2020 Fedora Release Engineering <releng@fedoraproject.org> - 20.04.3-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_33_Mass_Rebuild

* Sun Jul 12 2020 Marie Loise Nolden <loise@kde.org> - 20.04.3-1
- Update kwave to 20.04.3

* Fri Jun 12 2020 Marie Loise Nolden <loise@kde.org> - 20.04.2-1
- Update kwave to 20.04.2

* Tue May 26 2020 Sérgio Basto <sergio@serjux.com> - 20.04.1-1
- Update kwave to 20.04.1

* Thu Mar 26 2020 Sérgio Basto <sergio@serjux.com> - 19.12.3-1
- Update kwave to 19.12.3

* Wed Jan 29 2020 Fedora Release Engineering <releng@fedoraproject.org> - 19.12.1-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_32_Mass_Rebuild

* Tue Jan 21 2020 Sérgio Basto <sergio@serjux.com> - 19.12.1-1
- Update kwave to 19.12.1

* Wed Sep 25 2019 Sérgio Basto <sergio@serjux.com> - 19.08.1-1
- Update kwave to 19.08.1

* Thu Jul 25 2019 Fedora Release Engineering <releng@fedoraproject.org> - 18.12.3-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_31_Mass_Rebuild

* Tue May 07 2019 Sérgio Basto <sergio@serjux.com> - 18.12.3-1
- Update kwave to 18.12.3

* Fri Feb 01 2019 Fedora Release Engineering <releng@fedoraproject.org> - 18.08.1-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_30_Mass_Rebuild

* Sun Sep 16 2018 Sérgio Basto <sergio@serjux.com> - 18.08.1-1
- Update kwave to 18.08.1
- Reenable doc package.

* Thu Aug 02 2018 Sérgio Basto <sergio@serjux.com> - 18.04.3-2
- Decompressicons, icons should not be gzipped

* Fri Jul 27 2018 Sérgio Basto <sergio@serjux.com> - 18.04.3-1
- Update kwave to 18.04.3
- Disable doc as workaround.

* Thu Jul 26 2018 RPM Fusion Release Engineering <leigh123linux@gmail.com> - 17.12.3-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_29_Mass_Rebuild

* Sun Mar 11 2018 Sérgio Basto <sergio@serjux.com> - 17.12.3-1
- Update kwave to 17.12.3

* Thu Feb 22 2018 Sérgio Basto <sergio@serjux.com> - 17.12.2-1
- Update kwave to 17.12.2

* Tue Jan 30 2018 Sérgio Basto <sergio@serjux.com> - 17.12.1-1
- Update kwave to 17.12.1

* Tue Jan 02 2018 Sérgio Basto <sergio@serjux.com> - 17.12.0-2
- Use _kf5_metainfodir to fix appdata directory issue

* Fri Dec 29 2017 Sérgio Basto <sergio@serjux.com> - 17.12.0-1
- Update kwave to 17.12.0

* Sun Oct 08 2017 Sérgio Basto <sergio@serjux.com> - 17.08.1-1
- Update kwave to 17.08.1

* Thu Aug 31 2017 RPM Fusion Release Engineering <kwizart@rpmfusion.org> - 17.04.2-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Mass_Rebuild

* Wed Jul 12 2017 Leigh Scott <leigh123linux@googlemail.com> - 17.04.2-1
- Update to 17.04.2

* Sun Mar 19 2017 RPM Fusion Release Engineering <kwizart@rpmfusion.org> - 16.12.2-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_26_Mass_Rebuild

* Sat Feb 18 2017 Leigh Scott <leigh123linux@googlemail.com> - 16.12.2-1
- Initial port to kf5

* Sun Feb 15 2015 Mario Blättermann <mario.blaettermann@gmail.com> - 0.8.99-7
- Add BSD license

* Sat Feb 07 2015 Mario Blättermann <mario.blaettermann@gmail.com> - 0.8.99-6
- Add mp3 support via libmad

* Tue Feb 03 2015 Mario Blättermann <mario.blaettermann@gmail.com> - 0.8.99-5
- Remove gcc-c++ from BR
- Fix %%post and %%postun
- Move lsm file to the -doc subpackage

* Mon Feb 02 2015 Mario Blättermann <mario.blaettermann@gmail.com> - 0.8.99-4
- Move the documentation to a noarch subpackage

* Sat Jan 31 2015 Mario Blättermann <mario.blaettermann@gmail.com> - 0.8.99-3
- Add update-desktop-database scriptlet

* Wed Jan 28 2015 Mario Blättermann <mario.blaettermann@gmail.com> - 0.8.99-2
- Generate png icons

* Fri Jan 16 2015 Mario Blättermann <mario.blaettermann@gmail.com> - 0.8.99-1
- Initial package
