# trim changelog included in binary rpms
%global _changelog_trimtime %(date +%s -d "1 year ago")

Name:           yakuake
Version:        23.08.0
Release:        1%{?dist}
Summary:        A drop-down terminal emulator

# KDE e.V. may determine that future GPL versions are accepted
License:	GPLv2 or GPLv3
#URL:		https://projects.kde.org/projects/extragear/utils/yakuake 
URL:		http://yakuake.kde.org/

%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0:    https://download.kde.org/%{stable}/release-service/%{version}/src/yakuake-%{version}.tar.xz
Source1:    https://download.kde.org/%{stable}/release-service/%{version}/src/yakuake-%{version}.tar.xz.sig
Source2:    gpgkey-D81C0CB38EB725EF6691C385BB463350D6EF31EF.gpg

## upstream fixes

# konsolepart
Requires:       konsole5-part

BuildRequires:  gnupg2
BuildRequires:  desktop-file-utils
BuildRequires:  extra-cmake-modules
BuildRequires:  gettext
BuildRequires:  kf5-rpm-macros
BuildRequires:  kf5-karchive-devel
BuildRequires:  kf5-kconfig-devel
BuildRequires:  kf5-kcoreaddons-devel
BuildRequires:  kf5-kcrash-devel
BuildRequires:  kf5-kdbusaddons-devel
BuildRequires:  kf5-kglobalaccel-devel
BuildRequires:  kf5-ki18n-devel
BuildRequires:  kf5-kiconthemes-devel
BuildRequires:  kf5-kio-devel
BuildRequires:  kf5-knewstuff-devel
BuildRequires:  kf5-knotifications-devel
BuildRequires:  kf5-knotifyconfig-devel
BuildRequires:  kf5-kparts-devel
BuildRequires:  kf5-kwidgetsaddons-devel
BuildRequires:  kf5-kwindowsystem-devel
%if 0%{?fedora}
%global appstream_validate 1
BuildRequires:  libappstream-glib
%endif
BuildRequires:  qt5-qtx11extras-devel
BuildRequires:  cmake(Qt5Svg)
BuildRequires:  cmake(KF5Wayland)

%description
Yakuake is a drop-down terminal emulator.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -p1


%build
%cmake_kf5
%cmake_build


%install
%cmake_install

%find_lang %{name}


%check
%if 0%{?appstream_validate}
appstream-util validate-relax --nonet %{buildroot}%{_kf5_metainfodir}/org.kde.yakuake.appdata.xml
%endif
desktop-file-validate  %{buildroot}%{_kf5_datadir}/applications/org.kde.yakuake.desktop

%files -f %{name}.lang
%doc AUTHORS ChangeLog TODO
%license LICENSES/*.txt
%{_kf5_bindir}/yakuake
%{_kf5_metainfodir}/org.kde.yakuake.appdata.xml
%{_kf5_datadir}/applications/org.kde.yakuake.desktop
%{_kf5_datadir}/knotifications5/yakuake.notifyrc
%{_kf5_datadir}/yakuake/
%{_kf5_datadir}/knsrcfiles/%{name}.knsrc
%{_kf5_datadir}/icons/hicolor/*/apps/yakuake.*
%{_kf5_datadir}/dbus-1/services/org.kde.yakuake.service


%changelog
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

