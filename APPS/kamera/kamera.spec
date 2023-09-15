Name:    kamera
Summary: Digital camera support for KDE 
Version: 23.08.1
Release: 1%{?dist}

License: GPLv2
URL:     https://www.kde.org/applications/graphics/
%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0: https://download.kde.org/%{stable}/release-service/%{version}/src/kamera-%{version}.tar.xz
Source1: https://download.kde.org/%{stable}/release-service/%{version}/src/kamera-%{version}.tar.xz.sig
Source2: gpgkey-D81C0CB38EB725EF6691C385BB463350D6EF31EF.gpg

BuildRequires: gnupg2
BuildRequires: desktop-file-utils
BuildRequires: extra-cmake-modules
BuildRequires: kf5-kconfig-devel
BuildRequires: kf5-kconfigwidgets-devel
BuildRequires: kf5-kdoctools-devel
BuildRequires: kf5-ki18n-devel
BuildRequires: kf5-kio-devel
BuildRequires: kf5-kxmlgui-devel
BuildRequires: kf5-rpm-macros
BuildRequires: cmake(KF5KCMUtils)

BuildRequires: pkgconfig(libgphoto2)
BuildRequires: cmake(Qt5Core)

# when split occurred
Conflicts: kdegraphics < 7:4.6.95-10

# translations moved here
Conflicts: kde-l10n < 17.03

Requires: kde-cli-tools

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
%find_lang %{name} --all-name --with-html


%check
desktop-file-validate %{buildroot}%{_kf5_datadir}/applications/kcm_%{name}.desktop


%files -f %{name}.lang
%doc AUTHORS README
%license LICENSES/*.txt
%{_kf5_qtplugindir}/kf5/kio/kio_%{name}.so
%{_kf5_datadir}/solid/actions/solid_camera.desktop
%{_kf5_datadir}/applications/kcm_%{name}.desktop
%{_kf5_datadir}/qlogging-categories5/%{name}.categories
%{_kf5_qtplugindir}/plasma/kcms/systemsettings_qwidgets/kcm_%{name}.so
%{_kf5_metainfodir}/*.metainfo.xml


%changelog
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

