Name:           kwave
Version: 23.08.3
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
Source1:        http://download.kde.org/%{stable}/release-service/%{version}/src/%{name}-%{version}.tar.xz.sig
Source2:        gpgkey-D81C0CB38EB725EF6691C385BB463350D6EF31EF.gpg

BuildRequires:  gnupg2
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
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -p1

%build
%cmake_kf5 -DWITH_MP3=ON
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
%{_kf5_qtplugindir}/%{name}/
%{_kf5_libdir}/lib%{name}.so.*
%{_kf5_libdir}/lib%{name}gui.so.*

%files doc
%{_kf5_docdir}/HTML/*/%{name}

%changelog
* Fri Nov 10 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.08.3-1
- 23.08.3

* Fri Oct 13 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.08.2-1
- 23.08.2

* Thu Sep 14 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.08.1-1
- 23.08.1

* Sun Aug 27 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.08.0-1
- 23.08.0

* Thu Jul 06 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.04.3-1
- 23.04.3

* Thu Jun 08 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.04.2-1
- 23.04.2

* Thu May 11 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.04.1-1
- 23.04.1

* Thu Apr 20 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.04.0-1
- 23.04.0

* Thu Mar 02 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.12.3-1
- 22.12.3

* Thu Feb 02 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.12.2-1
- 22.12.2

