Name:           kio-gdrive
Version:        23.04.3
Release:        1%{?dist}
Summary:        An Google Drive KIO slave for KDE

License:        GPLv2+
URL:            https://community.kde.org/KIO_GDrive
# use releaseme
Source0:        http://download.kde.org/stable/release-service/%{version}/src/%{name}-%{version}.tar.xz
Source1:        http://download.kde.org/stable/release-service/%{version}/src/%{name}-%{version}.tar.xz.sig
Source2:        gpgkey-D81C0CB38EB725EF6691C385BB463350D6EF31EF.gpg

# handled by qt5-srpm-macros, which defines %%qt5_qtwebengine_arches
# arch's where libkgapi is available (due to inderect dependencies on qtwebengine)
%{?qt5_qtwebengine_arches:ExclusiveArch: %{qt5_qtwebengine_arches}}

BuildRequires:  gnupg2
BuildRequires:  cmake(KF5I18n)
BuildRequires:  cmake(KF5KIO)
BuildRequires:  cmake(KF5Notifications)
BuildRequires:  extra-cmake-modules
BuildRequires:  desktop-file-utils
BuildRequires:  kaccounts-integration-devel
BuildRequires:  libkgapi-devel
BuildRequires:  libaccounts-glib-devel
BuildRequires:  kf5-kdoctools-devel
BuildRequires:  libappstream-glib
BuildRequires:  intltool

%description
Provides KIO Access to Google Drive using the gdrive:/// protocol.

%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup

%build
%cmake_kf5
%cmake_build

%install
%cmake_install
%find_lang kio5_gdrive --all-name --with-html

%check
desktop-file-validate %{buildroot}%{_datadir}/remoteview/*.desktop
# appstream-util validate-relax --nonet %{buildroot}%{_datadir}/metainfo/*.metainfo.xml

%files -f kio5_gdrive.lang
%license COPYING
%doc HACKING README.md
%{_qt5_plugindir}/kf5/kio/gdrive.so
%{_kf5_plugindir}/kfileitemaction/gdrivecontextmenuaction.so
%{_kf5_plugindir}/propertiesdialog/gdrivepropertiesplugin.so
%{_qt5_plugindir}/kaccounts/daemonplugins/gdrive.so
%{_kf5_datadir}/accounts/services/kde/google-drive.service
%{_kf5_datadir}/knotifications5/gdrive.notifyrc
%{_kf5_datadir}/remoteview/gdrive-network.desktop
%{_kf5_datadir}/metainfo/org.kde.kio_gdrive.metainfo.xml

%changelog
* Tue Jul 18 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.04.3-1
- 23.04.3

* Thu Jun 08 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.04.2-1
- 23.04.2

* Thu May 11 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.04.1-1
- 23.04.1

* Thu May 11 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.04.0-2
- rebuild

* Thu Apr 20 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.04.0-1
- 23.04.0

* Thu Mar 02 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.12.3-1
- 22.12.3

