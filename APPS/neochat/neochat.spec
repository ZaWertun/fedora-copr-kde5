Name: neochat
Version: 23.08.0
Release: 5%{?dist}

License: GPL-2.0-only AND GPL-2.0-or-later AND GPL-3.0-only AND GPL-3.0-or-later AND BSD-3-Clause
URL: https://invent.kde.org/network/%{name}
Summary: Client for matrix, the decentralized communication protocol

%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0: https://download.kde.org/%{stable}/release-service/%{version}/src/%{name}-%{version}.tar.xz
Source1: https://download.kde.org/%{stable}/release-service/%{version}/src/%{name}-%{version}.tar.xz.sig
Source2: gpgkey-D81C0CB38EB725EF6691C385BB463350D6EF31EF.gpg

BuildRequires: cmake(Qt5Concurrent)
BuildRequires: cmake(Qt5Core)
BuildRequires: cmake(Qt5DBus)
BuildRequires: cmake(Qt5Keychain)
BuildRequires: cmake(Qt5LinguistTools)
BuildRequires: cmake(Qt5Multimedia)
BuildRequires: cmake(Qt5Network)
BuildRequires: cmake(Qt5QuickControls2)
BuildRequires: cmake(Qt5Svg)
BuildRequires: cmake(Qt5Widgets)

BuildRequires: cmake(KF5Config)
BuildRequires: cmake(KF5CoreAddons)
BuildRequires: cmake(KF5DBusAddons)
BuildRequires: cmake(KF5I18n)
BuildRequires: cmake(KF5ItemModels)
BuildRequires: cmake(KF5KIO)
BuildRequires: cmake(KF5Kirigami2)
BuildRequires: cmake(KF5KirigamiAddons)
BuildRequires: cmake(KF5Notifications)
BuildRequires: cmake(KF5QQC2DesktopStyle)
BuildRequires: cmake(KF5Sonnet)
BuildRequires: cmake(KF5SyntaxHighlighting)
BuildRequires: cmake(KF5WindowSystem)

BuildRequires: cmake(KQuickImageEditor)
BuildRequires: cmake(QCoro5)
BuildRequires: cmake(Quotient)
BuildRequires: pkgconfig(libcmark)

BuildRequires: cmake
BuildRequires: cmark
BuildRequires: gnupg2
BuildRequires: desktop-file-utils
BuildRequires: extra-cmake-modules
BuildRequires: gcc
BuildRequires: gcc-c++
BuildRequires: kf5-rpm-macros
BuildRequires: libappstream-glib

Requires: breeze-icon-theme
Requires: hicolor-icon-theme
Requires: kf5-kirigami2%{?_isa}
Requires: kf5-kirigami2-addons%{?_isa}
Requires: kf5-syntax-highlighting%{?_isa}
Requires: kquickimageeditor%{?_isa}
Requires: qqc2-breeze-style%{?_isa}
Requires: qqc2-desktop-style%{?_isa}
Requires: qt5-qtquickcontrols2%{?_isa}

Recommends: google-noto-emoji-color-fonts
Recommends: google-noto-emoji-fonts

Provides: spectral = %{?epoch:%{epoch}:}%{version}-%{release}
Obsoletes: spectral < 0-19.20201224gitfba0df0

%description
Neochat is a client for Matrix, the decentralized communication protocol for
instant messaging. It is a fork of Spectral, using KDE frameworks, most
notably Kirigami, KConfig and KI18n.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -p1


%build
%cmake_kf5
%cmake_build


%install
%cmake_install
%find_lang %{name} --with-qt


%check
appstream-util validate-relax --nonet %{buildroot}%{_metainfodir}/*.appdata.xml
desktop-file-validate %{buildroot}%{_datadir}/applications/*.desktop


%files -f %{name}.lang
%license LICENSES/*
%doc README.md
%{_bindir}/%{name}
%{_datadir}/applications/*.desktop
%{_datadir}/icons/hicolor/*/apps/*
%{_metainfodir}/*.appdata.xml
%{_kf5_datadir}/knotifications5/%{name}.notifyrc
%{_kf5_datadir}/krunner/dbusplugins/*.desktop
%{_kf5_datadir}/qlogging-categories5/%{name}.categories


%changelog
* Tue Sep 05 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.08.0-5
- rebuilt

* Sun Aug 27 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.08.0-1
- 23.08.0

* Tue Jul 18 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.04.3-1
- 23.04.3

* Tue Jun 06 2023 Marc Deop i Argemí <marcdeop@fedoraproject.org> - 23.04.2-1
- 23.04.2

* Sat May 13 2023 Marc Deop i Argemí <marcdeop@fedoraproject.org> - 23.04.1-1
- 23.04.1

* Thu Apr 20 2023 Marc Deop i Argemí <marcdeop@fedoraproject.org> - 23.04.0-1
- 23.04.0

* Fri Mar 31 2023 Marc Deop i Argemí <marcdeop@fedoraproject.org> - 23.03.90-1
- 23.03.90

* Mon Mar 20 2023 Marc Deop i Argemí <marcdeop@fedoraproject.org> - 23.03.80-1
- 23.03.80

* Tue Feb 07 2023 Marc Deop <marcdeop@fedoraproject.org> - 23.01.0-2
- Require kf5-kirigami2-addons

* Mon Jan 30 2023 Justin Zobel <justin@1707.io> - 23.01.0-1
- Update to 23.01.0

* Fri Jan 27 2023 Jens Petersen <petersen@redhat.com> - 22.11-4
- rebuild f38 against newer cmark

* Thu Jan 19 2023 Fedora Release Engineering <releng@fedoraproject.org> - 22.11-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_38_Mass_Rebuild

* Wed Dec 21 2022 Vitaly Zaitsev <vitaly@easycoding.org> - 22.11-2
- Rebuilt against libquotient 0.7.0 with E2EE enabled.
- Switched to SPDX license tag.

* Thu Dec 01 2022 Justin Zobel <justin@1707.io> - 22.11-1
- Update to 22.11

* Wed Sep 28 2022 Justin Zobel <justin@1707.io> - 22.09-1
- Update to 22.09

* Fri Jul 22 2022 Fedora Release Engineering <releng@fedoraproject.org> - 22.06-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_37_Mass_Rebuild

* Fri Jun 24 2022 Vitaly Zaitsev <vitaly@easycoding.org> - 22.06-1
- Updated to version 22.06.

* Sun Apr 24 2022 Vitaly Zaitsev <vitaly@easycoding.org> - 22.04-1
- Updated to version 22.04.

* Wed Feb 09 2022 Vitaly Zaitsev <vitaly@easycoding.org> - 22.02-1
- Updated to version 22.02.

* Thu Jan 20 2022 Fedora Release Engineering <releng@fedoraproject.org> - 21.12-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_36_Mass_Rebuild

* Sun Jan 09 2022 Vitaly Zaitsev <vitaly@easycoding.org> - 21.12-2
- Backported upstream patch with qcoro 0.4.0 build fixes.

* Tue Dec 07 2021 Vitaly Zaitsev <vitaly@easycoding.org> - 21.12-1
- Updated to version 21.12.

* Thu Jul 22 2021 Fedora Release Engineering <releng@fedoraproject.org> - 1.2.0-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_35_Mass_Rebuild

* Tue Jun 01 2021 Vitaly Zaitsev <vitaly@easycoding.org> - 1.2.0-1
- Updated to version 1.2.0.

* Tue Feb 23 2021 Vitaly Zaitsev <vitaly@easycoding.org> - 1.1.1-1
- Updated to version 1.1.1.

* Tue Feb 23 2021 Vitaly Zaitsev <vitaly@easycoding.org> - 1.1.0-1
- Updated to version 1.1.0.

* Tue Jan 26 2021 Fedora Release Engineering <releng@fedoraproject.org> - 1.0.1-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_34_Mass_Rebuild

* Wed Jan 13 2021 Vitaly Zaitsev <vitaly@easycoding.org> - 1.0.1-1
- Updated to version 1.0.1.

* Wed Dec 23 2020 Vitaly Zaitsev <vitaly@easycoding.org> - 1.0-1
- Updated to version 1.0.

* Tue Dec 15 2020 Vitaly Zaitsev <vitaly@easycoding.org> - 0.1.0-0.2.20201214git54b0773
- Updated to the latest Git snapshot.

* Mon Nov 23 2020 Vitaly Zaitsev <vitaly@easycoding.org> - 0.1.0-0.1.20201123git5d4e787
- Initial SPEC release.
