Name:           tokodon 
Version:        23.04.3
Release:        1%{?dist}
License:        GPLv3 and CC0 and BSD and LGPLv2+ and GPLv3+ and GPLv2
# For a breakdown of the licensing, see PACKAGE-LICENSING
Summary:        Kirigami-based mastodon client
Url:            https://invent.kde.org/network/tokodon
Source0:        https://download.kde.org/stable/release-service/%{version}/src/%{name}-%{version}.tar.xz

# Doesn't build due to compiler segfault on PPC64le: https://bugzilla.redhat.com/show_bug.cgi?id=2171964
ExcludeArch:    ppc64le

BuildRequires:  gcc
BuildRequires:  gcc-c++
BuildRequires:  cmake
BuildRequires:  extra-cmake-modules
BuildRequires:  kf5-rpm-macros
BuildRequires:  desktop-file-utils
BuildRequires:  libappstream-glib
BuildRequires:  appstream

BuildRequires:  cmake(KF5Config)
BuildRequires:  cmake(KF5CoreAddons)
BuildRequires:  cmake(KF5DBusAddons)
BuildRequires:  cmake(KF5I18n)
BuildRequires:  cmake(KF5KIO)
BuildRequires:  cmake(KF5Kirigami2)
BuildRequires:  cmake(KF5KirigamiAddons)
BuildRequires:  cmake(KF5Notifications)
BuildRequires:  cmake(KF5QQC2DesktopStyle)
BuildRequires:  cmake(KF5WindowSystem)
BuildRequires:  cmake(Qt5Core)
BuildRequires:  cmake(Qt5Gui)
BuildRequires:  cmake(Qt5Keychain)
BuildRequires:  cmake(Qt5Multimedia)
BuildRequires:  cmake(Qt5Quick)
BuildRequires:  cmake(Qt5QuickControls2)
BuildRequires:  cmake(Qt5Svg)
BuildRequires:  cmake(Qt5WebSockets)
BuildRequires:  cmake(Qt5Widgets)

Requires:       kf5-kirigami2
Requires:       kf5-kirigami2-addons
Requires:       hicolor-icon-theme

%description
Tokodon is a Mastodon client for Plasma and Plasma Mobile.

%prep
%autosetup -n %{name}-%{version}

%build
%cmake_kf5
%cmake_build

%install
%cmake_install
%find_lang %{name}

%check
appstream-util validate-relax --nonet %{buildroot}%{_kf5_datadir}/metainfo/org.kde.%{name}.appdata.xml
desktop-file-validate %{buildroot}%{_kf5_datadir}/applications/org.kde.%{name}.desktop

%files -f %{name}.lang
%doc README.md

%license LICENSES/

%{_kf5_bindir}/%{name}

%{_kf5_datadir}/applications/org.kde.%{name}.desktop
%{_kf5_datadir}/icons/hicolor/scalable/apps/org.kde.%{name}.svg
%{_kf5_datadir}/knotifications5/tokodon.notifyrc
%{_kf5_datadir}/qlogging-categories5/tokodon.categories
%{_kf5_metainfodir}/org.kde.%{name}.appdata.xml

%changelog
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

* Sun Feb 19 2023 Justin Zobel <justin@1707.io> - 23.02.0-1
- Update to 23.02.0

* Sat Jan 21 2023 Fedora Release Engineering <releng@fedoraproject.org> - 23.01.0-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_38_Mass_Rebuild

* Mon Jan 02 2023 Justin Zobel <justin@1707.io> - 23.01.0-1
- Update to 23.01.0

* Thu Dec 22 2022 Marcus Müller <marcus@hostalia.de> - 22.11.2-1
- Update to 22.11.2
- Fixes RHBZ #2154524

* Thu Dec 01 2022 Justin Zobel <justin@1707.io> - 22.11-1
- Update to 22.11

* Wed Sep 28 2022 Justin Zobel <justin@1707.io> - 22.09-1
- Update to 22.09

* Thu Aug 25 2022 Justin Zobel <justin@1707.io> - 22.06-1
- Update to 22.06

* Sat Jul 23 2022 Fedora Release Engineering <releng@fedoraproject.org> - 22.02-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_37_Mass_Rebuild

* Sat Apr 16 2022 Onuralp SEZER <thunderbirdtr@fedoraproject.org> - 22.02-1
- 22.02

* Sat Jan 15 2022 Justin Zobel <justin@1707.io> - 21.12-1
- Update to 21.12

* Thu Nov 04 2021 Onuralp SEZER <thunderbirdtr@fedoraproject.org> - 21.08-1
- initial version tokodon
