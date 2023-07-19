Name:    keditbookmarks
Summary: Bookmark organizer and editor
Version: 23.04.3
Release: 1%{?dist}

# Documentation is GFDL, rest GPLv2 and GPLv3 (note: NOT any later version)
License: GPLv2 and GPLv3 and GFDL
URL:     https://www.kde.org/

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
BuildRequires: cmake(KF5Bookmarks)
BuildRequires: cmake(KF5CoreAddons)
BuildRequires: cmake(KF5DocTools)
BuildRequires: cmake(KF5I18n)
BuildRequires: cmake(KF5IconThemes)
BuildRequires: cmake(KF5KIO)
BuildRequires: cmake(KF5Parts)
BuildRequires: cmake(KF5WindowSystem)

BuildRequires: cmake(Qt5Core)

# translations moved here
Conflicts: kde-l10n < 17.03

Requires:      %{name}-libs%{?_isa} = %{version}-%{release}

%description
keditbookmarks is a bookmark organizer and editor.


%package libs
Summary:       Runtime libraries for %{name}
Requires:      %{name} = %{version}-%{release}
%description libs
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

## unpackaged files
rm -fv %{buildroot}%{_kf5_libdir}/libkbookmarkmodel_private.so


%check
desktop-file-validate %{buildroot}%{_kf5_datadir}/applications/org.kde.%{name}.desktop


%files -f %{name}.lang
%license COPYING*
%{_kf5_bindir}/keditbookmarks
%{_kf5_bindir}/kbookmarkmerger
%{_kf5_datadir}/applications/org.kde.%{name}.desktop
%{_kf5_datadir}/config.kcfg/keditbookmarks.kcfg
%{_kf5_datadir}/qlogging-categories5/%{name}.categories
%{_mandir}/man1/kbookmarkmerger.1*

%ldconfig_scriptlets libs

%files libs
%{_kf5_libdir}/libkbookmarkmodel_private.so.*


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

