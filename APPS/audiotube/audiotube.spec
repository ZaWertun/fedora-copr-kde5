%global kf5_min_version 5.86.0

Name:           audiotube
Version:        23.08.4
Release:        1%{?dist}
License:        GPLv2+
Summary:        AudioTube can search YouTube Music, list albums and artists, play automatically generated playlists, albums and allows to put your own playlist together.
Url:            https://apps.kde.org/audiotube/
Source:         https://download.kde.org/stable/release-service/%{version}/src/%{name}-%{version}.tar.xz

BuildRequires:  gnupg2
BuildRequires:  desktop-file-utils
BuildRequires:  libappstream-glib

BuildRequires:  gcc-c++
BuildRequires:  extra-cmake-modules >= %{kf5_min_version}
BuildRequires:  kf5-rpm-macros      >= %{kf5_min_version}

BuildRequires: pybind11-devel
BuildRequires: python3-devel
BuildRequires: python3-ytmusicapi >= 1.0.2
BuildRequires: yt-dlp

BuildRequires: cmake(Qt5Core)
BuildRequires: cmake(Qt5Network)
BuildRequires: cmake(Qt5Qml)
BuildRequires: cmake(Qt5QmlModels)
BuildRequires: cmake(Qt5Quick)
BuildRequires: cmake(Qt5QuickControls2)
BuildRequires: cmake(Qt5QuickControls2)
BuildRequires: cmake(Qt5Svg)
BuildRequires: cmake(Qt5Widgets)

BuildRequires: cmake(QCoro5)

BuildRequires: cmake(KF5CoreAddons)   >= %{kf5_min_version}
BuildRequires: cmake(KF5Crash)        >= %{kf5_min_version}
BuildRequires: cmake(KF5I18n)         >= %{kf5_min_version}
BuildRequires: cmake(KF5Kirigami2)    >= %{kf5_min_version}
BuildRequires: cmake(KF5KirigamiAddons)
BuildRequires: cmake(KF5WindowSystem) >= %{kf5_min_version}

BuildRequires: cmake(FutureSQL5)

Requires:   hicolor-icon-theme
Requires:   kf5-kirigami2
Requires:   python3-ytmusicapi
Requires:   yt-dlp

%description
%{summary}.

%prep
%autosetup -p1

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
%license LICENSES/*
%{_kf5_bindir}/%{name}
%{_kf5_datadir}/applications/org.kde.%{name}.desktop
%{_kf5_metainfodir}/org.kde.%{name}.appdata.xml
%{_kf5_datadir}/icons/hicolor/scalable/apps/org.kde.%{name}.svg


%changelog
* Fri Dec 08 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.08.4-1
- 23.08.4

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

* Wed Jan 18 2023 Fedora Release Engineering <releng@fedoraproject.org> - 22.11-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_38_Mass_Rebuild

* Thu Dec 01 2022 Justin Zobel <justin@1707.io> - 22.11-1
- Update to 22.11

* Wed Sep 28 2022 Justin Zobel <justin@1707.io> - 22.09-1
- Update to 22.09

* Thu Aug 25 2022 Justin Zobel <justin@1707.io> - 22.06-1
- Update to 22.06

* Sun Jul 31 2022 Onuralp SEZER <thunderbirdtr@fedoraproject.org> - 22.04-4
- kf5-kirigami requirement added. Fix BZ#2112614

* Wed Jul 20 2022 Fedora Release Engineering <releng@fedoraproject.org> - 22.04-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_37_Mass_Rebuild

* Thu Jun 16 2022 Python Maint <python-maint@redhat.com> - 22.04-2
- Rebuilt for Python 3.11

* Wed May 04 2022 Justin Zobel <justin@1707.io> - 22.04-1
- Update to 22.04

* Thu Feb 10 2022 Onuralp SEZER <thunderbirdtr@fedoraproject.org> - 22.02-1
- Plasma mobile version 22.02

* Sun Jan 16 2022 Onuralp SEZER <thunderbirdtr@fedoraproject.org> - 21.12-1
- Initial version of package
