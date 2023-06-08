Name:    dragon
Summary: Media player
Version: 23.04.2
Release: 1%{?dist}

# code: KDE e.V. may determine that future GPL versions are accepted
# docs: GFDL
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

BuildRequires: gnupg2
BuildRequires: desktop-file-utils
BuildRequires: kf5-rpm-macros
BuildRequires: extra-cmake-modules
BuildRequires: qt5-qtbase-devel

BuildRequires: kf5-kconfig-devel
BuildRequires: kf5-kconfigwidgets-devel
BuildRequires: kf5-kcoreaddons-devel
BuildRequires: kf5-kcrash-devel
BuildRequires: kf5-kiconthemes-devel
BuildRequires: kf5-ki18n-devel
BuildRequires: kf5-kio-devel
BuildRequires: kf5-knotifications-devel
BuildRequires: kf5-kwidgetsaddons-devel
BuildRequires: kf5-kwindowsystem-devel
BuildRequires: kf5-kparts-devel
BuildRequires: kf5-solid-devel
BuildRequires: kf5-kdoctools-devel
BuildRequires: kf5-kxmlgui-devel
BuildRequires: kf5-kjobwidgets-devel
BuildRequires: kf5-kdbusaddons-devel

BuildRequires: phonon-qt5-devel

%if 0%{?fedora} > 22
%global appstream_validate 1
BuildRequires: libappstream-glib
%endif

# when split occurred
Obsoletes: kdemultimedia-dragonplayer < 6:4.8.80
Provides:  kdemultimedia-dragonplayer = 6:%{version}-%{release}
Provides:  dragonplayer = %{version}-%{release}

# translations moved here
Conflicts: kde-l10n < 17.03

%description
%{summary}.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -p1


%build
%cmake_kf5
%cmake_build


%install
%cmake_install

%find_lang %{name} --all-name --with-html --with-man


%check
%if 0%{?appstream_validate}
appstream-util validate-relax --nonet %{buildroot}%{_kf5_metainfodir}/org.kde.dragonplayer.appdata.xml
%endif
desktop-file-validate %{buildroot}%{_kf5_datadir}/applications/org.kde.dragonplayer.desktop

%files -f %{name}.lang
%license LICENSES/*.txt
#{_kf5_datadir}/kxmlgui5/dragonplayer
%{_kf5_datadir}/solid/actions/dragonplayer-opendvd.desktop
%{_kf5_datadir}/solid/actions/dragonplayer-openaudiocd.desktop
%{_kf5_bindir}/dragon
%{_sysconfdir}/xdg/dragonplayerrc
%{_kf5_datadir}/kservices5/ServiceMenus/dragonplayer_play_dvd.desktop
%{_kf5_metainfodir}/org.kde.dragonplayer.appdata.xml
%{_kf5_datadir}/applications/org.kde.dragonplayer.desktop
%{_kf5_datadir}/icons/hicolor/*/apps/dragonplayer.*
%{_kf5_datadir}/icons/oxygen/*/actions/player-volume-muted.*
%{_mandir}/man1/dragon.1*
#lang(en) %{_kf5_datadir}/doc/HTML/en/dragonplayer/
# -libs subpkg ? --rex
%{_kf5_plugindir}/parts/dragonpart.so


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

