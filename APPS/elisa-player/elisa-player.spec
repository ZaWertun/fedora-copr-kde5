Name:       elisa-player
Version:    23.08.3
Release:    1%{?dist}
Summary:    Elisa music player

# Main program LGPLv3+
# Background image CC-BY-SA
License:    LGPLv3+ and CC-BY-SA
URL:        https://community.kde.org/Elisa

%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0:    https://download.kde.org/%{stable}/release-service/%{version}/src/elisa-%{version}.tar.xz
Source1:    https://download.kde.org/%{stable}/release-service/%{version}/src/elisa-%{version}.tar.xz.sig
Source2:    gpgkey-D81C0CB38EB725EF6691C385BB463350D6EF31EF.gpg
# Build tarball with translations.
# git clone https://anongit.kde.org/releaseme.git
# cd releaseme
# RELEASEME_READONLY=1 ./tarme.rb --version 20.03.90 --origin trunk elisa
# Source0:    elisa-%{version}.tar.xz

BuildRequires:  gnupg2
BuildRequires:  gcc-c++
BuildRequires:  cmake
BuildRequires:  extra-cmake-modules
BuildRequires:  desktop-file-utils
BuildRequires:  libappstream-glib
BuildRequires:  cmake(Qt5Core)
BuildRequires:  cmake(Qt5Network)
BuildRequires:  cmake(Qt5Qml)
BuildRequires:  cmake(Qt5Sql)
BuildRequires:  cmake(Qt5Multimedia)
BuildRequires:  cmake(Qt5Svg)
BuildRequires:  cmake(Qt5Gui)
BuildRequires:  cmake(Qt5Widgets)
BuildRequires:  cmake(Qt5Quick)
BuildRequires:  cmake(Qt5QuickTest)
BuildRequires:  cmake(Qt5QuickControls2)
BuildRequires:  cmake(Qt5DBus)
BuildRequires:  cmake(KF5I18n)
BuildRequires:  cmake(KF5Config)
BuildRequires:  cmake(KF5KCMUtils)
BuildRequires:  cmake(KF5KIO)
BuildRequires:  cmake(KF5Baloo)
BuildRequires:  cmake(KF5Declarative)
BuildRequires:  cmake(KF5CoreAddons)
BuildRequires:  cmake(KF5FileMetaData)
BuildRequires:  cmake(KF5ConfigWidgets)
BuildRequires:  cmake(KF5Package)
BuildRequires:  cmake(KF5DocTools)
BuildRequires:  cmake(KF5XmlGui)
BuildRequires:  cmake(KF5Crash)
BuildRequires:  cmake(KF5DBusAddons)
BuildRequires:  cmake(KF5Kirigami2)
BuildRequires:  cmake(KF5IconThemes)
BuildRequires:  qt5-qtbase-private-devel
Requires:       hicolor-icon-theme
Requires:       kde-filesystem
Requires:       qt5-qtquickcontrols

%description
Elisa is a simple music player aiming to provide a nice experience for its
users.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -n elisa-%{version} -p1


%build
%cmake_kf5
%cmake_build


%install
%cmake_install
%find_lang elisa --all-name --with-kde --with-html


%check
desktop-file-validate %{buildroot}%{_kf5_datadir}/applications/org.kde.elisa.desktop
appstream-util validate-relax --nonet %{buildroot}%{_kf5_metainfodir}/org.kde.elisa.appdata.xml


%files -f elisa.lang
%license COPYING
%{_kf5_bindir}/elisa
%{_kf5_datadir}/applications/org.kde.elisa.desktop
%{_kf5_datadir}/icons/hicolor/*/apps/elisa*
%{_kf5_datadir}/qlogging-categories5/elisa.categories
%{_kf5_datadir}/dbus-1/services/org.kde.elisa.service
%{_kf5_metainfodir}/org.kde.elisa.appdata.xml
%{_kf5_libdir}/elisa/
%{_kf5_libdir}/qt5/qml/org/kde/elisa/


%changelog
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

