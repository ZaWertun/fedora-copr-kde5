Name:    juk 
Summary: Music player 
Version: 23.04.3
Release: 1%{?dist}

# code: KDE e.V. may determine that future GPL versions are accepted
# handbook doc: GFDL
License: (GPLv2 or GPLv3) and GFDL
URL:     https://cgit.kde.org/%{name}.git

%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0: https://download.kde.org/%{stable}/release-service/%{version}/src/%{name}-%{version}.tar.xz
Source1: https://download.kde.org/%{stable}/release-service/%{version}/src/%{name}-%{version}.tar.xz.sig
Source2: gpgkey-D81C0CB38EB725EF6691C385BB463350D6EF31EF.gpg

## upstream patches

## upstreamable patches

BuildRequires: gnupg2
BuildRequires: desktop-file-utils
BuildRequires: libappstream-glib

BuildRequires: extra-cmake-modules
BuildRequires: kf5-rpm-macros
BuildRequires: cmake(KF5CoreAddons)
BuildRequires: cmake(KF5Completion)
BuildRequires: cmake(KF5Config)
BuildRequires: cmake(KF5Crash)
BuildRequires: cmake(KF5DBusAddons)
BuildRequires: cmake(KF5GlobalAccel)
BuildRequires: cmake(KF5I18n)
BuildRequires: cmake(KF5IconThemes)
BuildRequires: cmake(KF5DocTools)
BuildRequires: cmake(KF5KIO)
BuildRequires: cmake(KF5JobWidgets)
BuildRequires: cmake(KF5Notifications)
BuildRequires: cmake(KF5TextWidgets)
BuildRequires: cmake(KF5XmlGui)
BuildRequires: cmake(KF5Wallet)
BuildRequires: cmake(KF5WidgetsAddons)
BuildRequires: cmake(KF5WindowSystem)
BuildRequires: cmake(KF5ItemModels)

BuildRequires: cmake(Qt5Gui)
BuildRequires: cmake(Qt5Svg)
BuildRequires: cmake(Qt5Network)
BuildRequires: cmake(Qt5Test)
BuildRequires: cmake(Qt5Widgets)

BuildRequires: cmake(Phonon4Qt5)
BuildRequires: pkgconfig(taglib)
BuildRequires: pkgconfig(zlib)

# when split occurred
Obsoletes: kdemultimedia-juk < 6:4.8.80
Provides:  kdemultimedia-juk = 6:%{version}-%{release}

# docs/translations moved here
Conflicts: kde-l10n < 17.08.3-2


%description
Juk is a jukebox, tagger and music collection manager.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -p1


%build
%cmake_kf5
%cmake_build


%install
%cmake_install

%find_lang %{name} --all-name --with-html


%check
appstream-util validate-relax --nonet %{buildroot}%{_kf5_metainfodir}/org.kde.juk.appdata.xml ||:
desktop-file-validate %{buildroot}%{_kf5_datadir}/applications/org.kde.juk.desktop

%files -f %{name}.lang
%license COPYING*
%{_kf5_bindir}/juk
%{_kf5_datadir}/juk/
%{_datadir}/dbus-1/interfaces/org.kde.juk.*.xml
%{_kf5_metainfodir}/org.kde.juk.appdata.xml
%{_kf5_datadir}/kio/servicemenus/%{name}servicemenu.desktop
%{_kf5_datadir}/applications/org.kde.juk.desktop
%{_kf5_datadir}/icons/hicolor/*/apps/juk.*
%{_kf5_datadir}/kxmlgui5/juk/
%{_kf5_datadir}/knotifications5/juk.*


%changelog
* Tue Jul 18 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.04.3-1
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

