%global optflags %{optflags} -flto=auto
%global build_ldflags %{build_ldflags} -flto

Name:           kbackup
Version:        23.04.2
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
Source1: https://download.kde.org/%{stable}/release-service/%{version}/src/%{name}-%{version}.tar.xz.sig
Source2: gpgkey-D81C0CB38EB725EF6691C385BB463350D6EF31EF.gpg

BuildRequires:  gnupg2
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
BuildRequires:  cmake(KF5GuiAddons)
BuildRequires:  desktop-file-utils
BuildRequires:  libappstream-glib
BuildRequires:  gettext
BuildRequires:  kf5-rpm-macros

BuildRequires:  pkgconfig(libarchive)

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
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
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

