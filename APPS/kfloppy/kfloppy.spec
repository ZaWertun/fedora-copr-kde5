Name:    kfloppy
Summary: Floppy formatting tool 
Version: 23.04.3
Release: 1%{?dist}

License: GPLv2+
URL:     http://utils.kde.org/projects/%{name}
#URL:    https://cgit.kde.org/%{name}.git

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
BuildRequires: kf5-rpm-macros
BuildRequires: cmake(KF5I18n)
BuildRequires: cmake(KF5CoreAddons)
BuildRequires: cmake(KF5KIO)
BuildRequires: cmake(KF5DocTools)
BuildRequires: cmake(KF5KDELibs4Support)
BuildRequires: cmake(KF5XmlGui)

# translations moved here
Conflicts: kde-l10n < 17.03

# when split occured
Conflicts: kdeutils-common < 6:4.7.80

Obsoletes: kdeutils-kfloppy < 6:4.7.80
Provides:  kdeutils-kfloppy = 6:%{version}-%{release}


%description
KFloppy is a utility that provides a straightforward graphical means
to format 3.5" and 5.25" floppy disks.


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
desktop-file-validate %{buildroot}%{_kde4_datadir}/applications/org.kde.kfloppy.desktop


%files -f %{name}.lang
%license COPYING
%doc README
%{_kf5_bindir}/kfloppy
%{_kf5_datadir}/qlogging-categories5/%{name}*
%{_kf5_metainfodir}/org.kde.kfloppy.appdata.xml
%{_kf5_datadir}/applications/org.kde.kfloppy.desktop
%{_kf5_datadir}/icons/hicolor/*/apps/kfloppy.*


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

