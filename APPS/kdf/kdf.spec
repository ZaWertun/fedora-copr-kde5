Name:    kdf
Summary: View disk usage
Version: 23.08.2
Release: 1%{?dist}

License: GPLv2+
URL:     http://utils.kde.org/projects/%{name}

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
BuildRequires: cmake(KF5ConfigWidgets)
BuildRequires: cmake(KF5CoreAddons)
BuildRequires: cmake(KF5DocTools)
BuildRequires: cmake(KF5I18n)
BuildRequires: cmake(KF5IconThemes)
BuildRequires: cmake(KF5KIO)
BuildRequires: cmake(KF5KCMUtils)
BuildRequires: cmake(KF5Notifications)
BuildRequires: cmake(KF5WidgetsAddons)
BuildRequires: cmake(KF5XmlGui)

# when split occured
Conflicts: kdeutils-common < 6:4.7.80

# translations moved here
Conflicts: kde-l10n < 17.03

Obsoletes: kdeutils-kdf < 6:4.7.80
Provides:  kdeutils-kdf = 6:%{version}-%{release}

%description
KDiskFree displays the available file devices (hard drive partitions,
floppy and CD/DVD drives, etc.) along with information on their capacity,
free space, type and mount point.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup


%build
%cmake_kf5
%cmake_build


%install
%cmake_install
%find_lang %{name} --all-name --with-html


%check
desktop-file-validate %{buildroot}%{_kf5_datadir}/applications/*kdf.desktop ||:


%ldconfig_scriptlets

%files -f %{name}.lang
%license LICENSES/*.txt
%{_kf5_bindir}/kdf
%{_kf5_datadir}/qlogging-categories5/%{name}*
%{_kf5_datadir}/icons/hicolor/*/apps/*
%{_kf5_bindir}/kwikdisk
%{_kf5_libdir}/libkdfprivate.so.*
%{_kf5_qtplugindir}/plasma/kcms/systemsettings_qwidgets/kcm_kdf.so
%{_kf5_datadir}/applications/kcm_kdf.desktop
%{_kf5_datadir}/applications/org.kde.kdf.desktop
%{_kf5_datadir}/applications/org.kde.kwikdisk.desktop
%{_kf5_metainfodir}/org.kde.*.appdata.xml
%{_kf5_datadir}/kxmlgui5/kdf/


%changelog
* Fri Oct 13 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.08.2-1
- 23.08.2

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

