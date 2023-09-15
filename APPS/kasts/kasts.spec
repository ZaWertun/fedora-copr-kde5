%global kf5_min_version 5.88.0

Name:           kasts
Version:        23.08.1
Release:        1%{?dist}
License:        GPLv2 and GPLv2+ and GPLv3+ and BSD and LGPLv3+
Summary:        A mobile podcast application
Url:            https://apps.kde.org/%{name}
Source:         https://download.kde.org/stable/release-service/%{version}/src/%{name}-%{version}.tar.xz

BuildRequires:  desktop-file-utils
BuildRequires:  libappstream-glib

BuildRequires:  cmake
BuildRequires:  extra-cmake-modules
BuildRequires:  gcc-c++
BuildRequires:  taglib-devel

BuildRequires:  cmake(Qt5Core)
BuildRequires:  cmake(Qt5Keychain)
BuildRequires:  cmake(Qt5Multimedia)
BuildRequires:  cmake(Qt5Quick)
BuildRequires:  cmake(Qt5QuickControls2)
BuildRequires:  cmake(Qt5Svg)

BuildRequires:  cmake(KF5Config)         >= %{kf5_min_version}
BuildRequires:  cmake(KF5CoreAddons)     >= %{kf5_min_version}
BuildRequires:  cmake(KF5I18n)           >= %{kf5_min_version}
BuildRequires:  cmake(KF5Kirigami2)      >= %{kf5_min_version}
BuildRequires:  cmake(KF5KirigamiAddons)
BuildRequires:  cmake(KF5Syndication)    >= %{kf5_min_version}
BuildRequires:  cmake(KF5ThreadWeaver)   >= %{kf5_min_version}
BuildRequires:  kf5-rpm-macros           >= %{kf5_min_version}

%description
%{summary}.

%prep
%autosetup 

%build
%cmake_kf5
%cmake_build

%install
%cmake_install
%find_lang %{name}

%check
desktop-file-validate %{buildroot}%{_kf5_datadir}/applications/org.kde.%{name}.desktop
appstream-util validate-relax --nonet %{buildroot}%{_kf5_metainfodir}/org.kde.%{name}.appdata.xml

%files -f %{name}.lang
%{_kf5_bindir}/%{name}
%{_kf5_datadir}/applications/org.kde.%{name}.desktop
%{_kf5_datadir}/icons/hicolor/scalable/actions/media-playback-cloud.svg
%{_kf5_datadir}/icons/hicolor/scalable/apps/%{name}.svg
%{_kf5_datadir}/icons/hicolor/scalable/apps/%{name}-tray-dark.svg
%{_kf5_datadir}/icons/hicolor/scalable/apps/%{name}-tray-light.svg
%{_kf5_libdir}/libKastsSolidExtras.so
%{_kf5_libdir}/libKMediaSession.so
%{_kf5_libdir}/qt5/qml/org/kde/kmediasession/libkmediasession-qmlplugin.so
%{_kf5_libdir}/qt5/qml/org/kde/kmediasession/qmldir
%{_kf5_metainfodir}/org.kde.%{name}.appdata.xml
%{_kf5_qmldir}/org/kde/%{name}/solidextras/libkasts-solidextrasqmlplugin.so
%{_kf5_qmldir}/org/kde/%{name}/solidextras/qmldir
%license LICENSES/*


%changelog
* Thu Sep 14 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.08.1-1
- 23.08.1

* Sun Aug 27 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.08.0-1
- 23.08.0

* Thu Jul 06 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.04.3-1
- 23.04.3

* Sun Jun 11 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.04.2-1
- 23.04.2

* Sat May 13 2023 Marc Deop i Argemí <marcdeop@fedoraproject.org> - 23.04.1-1
- 23.04.1

* Thu Apr 20 2023 Marc Deop i Argemí <marcdeop@fedoraproject.org> - 23.04.0-1
- 23.04.0

* Fri Mar 31 2023 Marc Deop i Argemí <marcdeop@fedoraproject.org> - 23.03.90-1
- 23.03.90

* Mon Mar 20 2023 Marc Deop i Argemí <marcdeop@fedoraproject.org> - 23.03.80-1
- 23.03.80

* Mon Jan 30 2023 Justin Zobel <justin@1707.io> - 23.01.0-1
- Update to 23.01.0

* Thu Jan 19 2023 Fedora Release Engineering <releng@fedoraproject.org> - 22.11-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_38_Mass_Rebuild

* Thu Dec 01 2022 Justin Zobel <justin@1707.io> - 22.11-1
- Update to 22.11

* Wed Sep 28 2022 Justin Zobel <justin@1707.io> - 22.09.2-1
- Update to 22.09.2

* Thu Aug 25 2022 Justin Zobel <justin@1707.io> - 22.06-1
- Update to 22.06

* Thu Jul 21 2022 Fedora Release Engineering <releng@fedoraproject.org> - 22.02-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_37_Mass_Rebuild

* Tue Apr 19 2022 Onuralp SEZER <thunderbirdtr@fedoraproject.org> - 22.02-1
- Update to 22.02

* Sun Jan 16 2022 Onuralp SEZER <thunderbirdtr@fedoraproject.org> - 21.12-1
- Initial package
