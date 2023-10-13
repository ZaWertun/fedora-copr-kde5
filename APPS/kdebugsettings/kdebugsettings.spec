Name:    kdebugsettings
Summary: Configure debug output from Qt5 applications
Version: 23.08.2
Release: 1%{?dist}

License: LGPLv2+
URL:     https://www.kde.org/applications/utilities/

%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0:        https://download.kde.org/%{stable}/release-service/%{version}/src/%{name}-%{version}.tar.xz
Source1:        https://download.kde.org/%{stable}/release-service/%{version}/src/%{name}-%{version}.tar.xz.sig
Source2:        gpgkey-D81C0CB38EB725EF6691C385BB463350D6EF31EF.gpg

BuildRequires:  gnupg2
BuildRequires:  desktop-file-utils

BuildRequires:  qt5-qtbase-devel

BuildRequires:  extra-cmake-modules
BuildRequires:  kf5-rpm-macros
BuildRequires:  kf5-kcoreaddons-devel
BuildRequires:  kf5-kconfig-devel
BuildRequires:  kf5-kconfigwidgets-devel
BuildRequires:  kf5-kdbusaddons-devel
BuildRequires:  kf5-ki18n-devel
BuildRequires:  kf5-kguiaddons-devel
BuildRequires:  kf5-kiconthemes-devel
BuildRequires:  kf5-kwidgetsaddons-devel
BuildRequires:  kf5-kitemviews-devel
BuildRequires:  kf5-kcompletion-devel
BuildRequires:  kf5-kio-devel

# translations moved here
Conflicts: kde-l10n < 17.03

%description
An application to enable/disable qCDebug


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup


%build
%cmake_kf5
%cmake_build


%install
%cmake_install

%find_lang %{name} --all-name --with-html --with-man


%check
desktop-file-validate %{buildroot}%{_kf5_datadir}/applications/org.kde.kdebugsettings.desktop


%files -f %{name}.lang
%license LICENSES/*.txt
%{_kf5_bindir}/kdebugsettings
%{_kf5_libdir}/libkdebugsettings.so.*
%{_kf5_libdir}/libkdebugsettingscore.so.*
%{_kf5_datadir}/qlogging-categories5/*categories
%{_kf5_datadir}/applications/org.kde.kdebugsettings.desktop
"%{_kf5_datadir}/kdebugsettings/groups/Full Debug Ruqola"
%{_kf5_metainfodir}/org.kde.%{name}.appdata.xml


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

