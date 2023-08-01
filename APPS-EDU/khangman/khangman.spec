Name:    khangman
Summary: Hangman game 
Version: 23.04.3
Release: 1%{?dist}

License: GPLv2+
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

BuildRequires: gnupg2
BuildRequires: desktop-file-utils
BuildRequires: extra-cmake-modules
BuildRequires: gettext
BuildRequires: kf5-kcompletion-devel
BuildRequires: kf5-kconfig-devel
BuildRequires: kf5-kconfigwidgets-devel
BuildRequires: kf5-kcoreaddons-devel
BuildRequires: kf5-kdbusaddons-devel
BuildRequires: kf5-kdeclarative-devel
BuildRequires: kf5-kdelibs4support-devel
BuildRequires: kf5-kdnssd-devel
BuildRequires: kf5-kguiaddons-devel
BuildRequires: kf5-ki18n-devel
BuildRequires: kf5-kiconthemes-devel
BuildRequires: kf5-kio-devel
BuildRequires: kf5-kitemmodels-devel
BuildRequires: kf5-kitemviews-devel
BuildRequires: kf5-kjobwidgets-devel
BuildRequires: kf5-knewstuff-devel
BuildRequires: kf5-knewstuff-devel
BuildRequires: kf5-knotifyconfig-devel
BuildRequires: kf5-kservice-devel
BuildRequires: kf5-kwidgetsaddons-devel
BuildRequires: kf5-kwindowsystem-devel
BuildRequires: kf5-kxmlgui-devel
BuildRequires: kf5-rpm-macros
BuildRequires: libappstream-glib
%global majmin_ver %(echo %{version} | cut -d. -f1,2)
BuildRequires: libkeduvocdocument-devel >= %{majmin_ver}
BuildRequires: pkgconfig(Qt5Core) pkgconfig(Qt5Qml) pkgconfig(Qt5Quick) pkgconfig(Qt5QuickWidgets) pkgconfig(Qt5Svg)

Requires: dustin-dustismo-roman-fonts
Requires: dustin-domestic-manners-fonts
Requires: kdeedu-data
# qml deps
Requires: qt5-qtgraphicaleffects
Requires: qt5-qtmultimedia
Requires: qt5-qtquickcontrols

# when split occurred
Conflicts: kdeedu < 4.7.0-10

Obsoletes: khangman-libs < 15.04
Obsoletes: khangman-devel < 15.04

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

%find_lang %{name} --all-name --with-html --with-man --with-qt

## unpackaged files
# omit bundled dustismo roman font
rm -fv %{buildroot}%{_kf5_datadir}/khangman/fonts/Dustismo_Roman.ttf
# omit bundled domestic manners font
rm -fv %{buildroot}%{_kf5_datadir}/khangman/fonts/Domestic_Manners.ttf
# bug, harmattan icon should not be installed (when harmattan build is off)
rm -vf %{buildroot}%{_kf5_datadir}/icons/hicolor/*/apps/khangman-harmattan.*


%check
appstream-util validate-relax --nonet %{buildroot}%{_kf5_metainfodir}/org.kde.%{name}.appdata.xml ||:
desktop-file-validate %{buildroot}%{_kf5_datadir}/applications/org.kde.%{name}.desktop ||:


%files -f %{name}.lang
%license COPYING*
%doc README
%{_kf5_bindir}/%{name}*
%{_sysconfdir}/xdg/%{name}.knsrc
%{_kf5_datadir}/applications/org.kde.%{name}.desktop
%{_kf5_metainfodir}/org.kde.%{name}.appdata.xml
%{_kf5_datadir}/icons/hicolor/*/apps/%{name}.*
%{_kf5_datadir}/%{name}/
#{_kf5_datadir}/kconf_update/%{name}*
#{_kf5_datadir}/knotifications5/%{name}.notifyrc
#{_kf5_datadir}/kxmlgui5/%{name}/
#{_kf5_datadir}/sounds/%{name}*
%{_kf5_datadir}/config.kcfg/%{name}.kcfg
%{_mandir}/man6/%{name}.6*


%changelog
* Thu Jul 06 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.04.3-1
- 23.04.3

* Thu Jun 08 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.04.2-1
- 23.04.2

* Thu May 11 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.04.1-1
- 23.04.1

* Fri Apr 21 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.04.0-1
- 23.04.0

* Thu Mar 02 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.12.3-1
- 22.12.3

* Thu Jan 05 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.12.1-1
- 22.12.1

