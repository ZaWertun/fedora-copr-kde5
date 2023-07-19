Name:           skanlite
Version:        23.04.3
Release:        1%{?dist}
Summary:        Lightweight scanning program
# Actually: GPLv2 or GPLv3 or any later Version approved by KDE e.V.
License:        GPLv2 or GPLv3
URL:            https://www.kde.org/applications/graphics/%{name}/
Source0:        http://download.kde.org/stable/release-service/%{version}/src/%{name}-%{version}.tar.xz
Source1:        http://download.kde.org/stable/release-service/%{version}/src/%{name}-%{version}.tar.xz.sig
Source2:        gpgkey-D81C0CB38EB725EF6691C385BB463350D6EF31EF.gpg

BuildRequires:  gnupg2
BuildRequires:  cmake
BuildRequires:  cmake(KF5CoreAddons)
BuildRequires:  cmake(KF5DocTools)
BuildRequires:  cmake(KF5I18n)
BuildRequires:  cmake(KF5KIO)
BuildRequires:  cmake(KF5Sane)
BuildRequires:  cmake(KF5XmlGui)
BuildRequires:  desktop-file-utils
BuildRequires:  extra-cmake-modules
BuildRequires:  gcc-c++
BuildRequires:  libappstream-glib
BuildRequires:  libpng-devel
BuildRequires:  qt5-qtbase-devel
BuildRequires:  cmake(KSaneCore)

Requires:       kde-filesystem
Requires:       khotkeys


%description
Skanlite is a light-weight scanning application based on libksane.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -p1


%build
%cmake
%cmake_build


%install
%cmake_install
%find_lang %{name} --all-name --with-html
install -Dpm 0644 hotkeys_and_scripts/%{name}.khotkeys %{buildroot}%{_kf5_datadir}/khotkeys/%{name}.khotkeys


%check
desktop-file-validate %{buildroot}%{_datadir}/applications/org.kde.%{name}.desktop
appstream-util validate-relax --nonet %{buildroot}/%{_datadir}/metainfo/org.kde.%{name}.appdata.xml


%files -f %{name}.lang
%license LICENSES/*.txt
%{_kf5_bindir}/%{name}
%{_kf5_datadir}/khotkeys/%{name}.khotkeys
%{_kf5_datadir}/applications/org.kde.%{name}.desktop
%{_kf5_datadir}/icons/hicolor/*/apps/org.kde.%{name}.svg
%{_kf5_metainfodir}/org.kde.%{name}.appdata.xml


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

