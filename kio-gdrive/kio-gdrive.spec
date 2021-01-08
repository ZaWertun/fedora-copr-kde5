%undefine __cmake_in_source_build

Name:           kio-gdrive
Version:        20.12.1
Release:        1%{?dist}
Summary:        An Google Drive KIO slave for KDE

License:        GPLv2+
URL:            https://community.kde.org/KIO_GDrive
# use releaseme
Source0:        http://download.kde.org/stable/release-service/%{version}/src/%{name}-%{version}.tar.xz

# handled by qt5-srpm-macros, which defines %%qt5_qtwebengine_arches
# arch's where libkgapi is available (due to inderect dependencies on qtwebengine)
%{?qt5_qtwebengine_arches:ExclusiveArch: %{qt5_qtwebengine_arches}}

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
%autosetup

%build
%{cmake_kf5}
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
* Thu Jan  7 22:09:33 MSK 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.12.1-1
- 20.12.1

* Thu Dec 10 21:56:30 MSK 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.12.0-1
- 20.12.0

* Fri Nov  6 13:26:01 MSK 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.08.3-1
- 20.08.3

* Fri Oct 09 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.08.2-1
- 20.08.2

* Thu Sep 03 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.08.1-1
- 20.08.1

* Thu Aug 20 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.08.0-1
- 20.08.0

* Tue Jul 28 2020 Fedora Release Engineering <releng@fedoraproject.org> - 1.3.0-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_33_Mass_Rebuild

* Sun Jul 12 2020 Rex Dieter <rdieter@fedoraproject.org> - 1.3.0-2
- rebuild (kaccounts)

* Mon May 25 2020 Vasiliy N. Glazov <vascom2@gmail.com> - 1.3.0-1
- Update to 1.3.0

* Wed Jan 29 2020 Fedora Release Engineering <releng@fedoraproject.org> - 1.2.7-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_32_Mass_Rebuild

* Fri Nov 08 2019 Vasiliy N. Glazov <vascom2@gmail.com> - 1.2.7-2
- Enable LTO

* Fri Sep 06 2019 Vasiliy N. Glazov <vascom2@gmail.com> - 1.2.7-1
- Update to 1.2.7

* Thu Jul 25 2019 Fedora Release Engineering <releng@fedoraproject.org> - 1.2.6-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_31_Mass_Rebuild

* Mon Jun 24 2019 Vasiliy N. Glazov <vascom2@gmail.com> 1.2.6-2
- Rebuild

* Mon May 20 2019 Vasiliy N. Glazov <vascom2@gmail.com> 1.2.6-1
- Update to 1.2.6

* Fri Feb 01 2019 Fedora Release Engineering <releng@fedoraproject.org> - 1.2.5-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_30_Mass_Rebuild

* Wed Oct 24 2018 Vasiliy N. Glazov <vascom2@gmail.com> 1.2.5-1
- Update to 1.2.5

* Mon Jul 16 2018 Vasiliy N. Glazov <vascom2@gmail.com> 1.2.4-1
- Update to 1.2.4
- Clean spec and BRs

* Fri Jul 13 2018 Fedora Release Engineering <releng@fedoraproject.org> - 1.2.1-4
- Rebuilt for https://fedoraproject.org/wiki/Fedora_29_Mass_Rebuild

* Wed Feb 07 2018 Fedora Release Engineering <releng@fedoraproject.org> - 1.2.1-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_28_Mass_Rebuild

* Sun Oct 1 2017 Wolnei Tomazelli Junior <wolnei@fedoraproject.org> -  1.2.1-2
- Fix bogus date

* Sun Oct 1 2017 Wolnei Tomazelli Junior <wolnei@fedoraproject.org> -  1.2.1-1
- Build fixes
- Updated translations

* Thu Aug 03 2017 Fedora Release Engineering <releng@fedoraproject.org> - 1.2.0-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Binutils_Mass_Rebuild

* Wed Jul 26 2017 Fedora Release Engineering <releng@fedoraproject.org> - 1.2.0-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Mass_Rebuild

* Sat Jul 22 2017 Wolnei Tomazelli Junior <wolnei@fedoraproject.org> -  1.2.0-1
- Integration with KAccounts
- Google Drive free space is now reported

* Wed May 17 2017 Rex Dieter <rdieter@fedoraproject.org> - 1.1.2-2
- rebuild (libkgapi), use %%find_lang for HTML docs too
- ExclusiveArch: %%{?qt5_qtwebengine_arches}

* Mon May 15 2017 Wolnei Tomazelli Junior <wolnei@fedoraproject.org> -  1.1.2-1
- Updated translations - v1.1.2
* Fri Feb 17 2017 Wolnei Tomazelli Junior <wolnei@fedoraproject.org> -  1.1.1-1
- Fixed wrong write permissions in the top-level accounts folder - v1.1.1
* Sun Jan 29 2017 Wolnei Tomazelli Junior <wolnei@fedoraproject.org> -  1.1.0-1
- update version 1.1
* Sat Jan 28 2017 Wolnei Tomazelli Junior <wolnei@fedoraproject.org> -  1.0.5-2
- Initial version of the package
