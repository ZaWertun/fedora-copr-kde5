Name:       elisa-player
Version:    22.08.3
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
* Thu Nov 03 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.08.3-1
- 22.08.3

* Thu Oct 13 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.08.2-1
- 22.08.2

* Thu Sep 08 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.08.1-1
- 22.08.1

* Fri Aug 19 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.08.0-1
- 22.08.0

* Thu Jul 07 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.04.3-1
- 22.04.3

* Thu Jun 09 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.04.2-1
- 22.04.2

* Thu May 12 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.04.1-1
- 22.04.1

* Thu Apr 21 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.04.0-1
- 22.04.0

* Thu Mar 03 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 21.12.3-1
- 21.12.3

* Thu Feb 03 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 21.12.2-1
- 21.12.2

* Thu Jan 06 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 21.12.1-1
- 21.12.1

* Thu Dec 09 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 21.12.0-1
- 21.12.0

* Thu Nov 04 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 21.08.3-1
- 21.08.3

* Thu Oct 07 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 21.08.2-1
- 21.08.2

* Thu Sep 02 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 21.08.1-1
- 21.08.1

* Thu Aug 12 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 21.08.0-1
- 21.08.0

* Thu Jul 08 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 21.04.3-1
- 21.04.3

* Fri Jun 11 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 21.04.2-1
- 21.04.2

* Thu May 13 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 21.04.1-1
- 21.04.1

* Thu Apr 22 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 21.04.0-1
- 21.04.0

* Sat Mar 06 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.12.3-1
- 20.12.3

* Thu Feb 04 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.12.2-1
- 20.12.2

* Thu Jan  7 22:09:10 MSK 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.12.1-1
- 20.12.1

* Thu Dec 10 21:56:00 MSK 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.12.0-1
- 20.12.0

* Fri Nov  6 13:25:28 MSK 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.08.3-1
- 20.08.3

* Fri Oct 09 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.08.2-1
- 20.08.2

* Thu Sep 03 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.08.1-1
- 20.08.1

* Fri Aug 14 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.08.0-1
- 20.08.0

* Thu Jul 09 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.04.3-1
- 20.04.3

* Fri Jun 12 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.04.2-1
- 20.04.2

* Tue May 19 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.04.1-1
- 20.04.1

* Fri Apr 24 2020 Vasiliy N. Glazov <vascom2@gmail.com> - 20.04.0-1
- Update to 20.04.0

* Mon Apr 06 2020 Rex Dieter <rdieter@fedoraproject.org> - 20.03.90-2
- rebuild (qt5)

* Sat Apr 04 2020 Vasiliy N. Glazov <vascom2@gmail.com> - 20.03.90-1
- Update to 20.03.90

* Thu Apr 02 19:31:23 CET 2020 Robert-André Mauchin <zebob.m@gmail.com> - 20.03.80-1
- Update to 20.03.80 (#1800330)
- Update translations (#1820139)

* Tue Jan 28 2020 Fedora Release Engineering <releng@fedoraproject.org> - 19.12.1-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_32_Mass_Rebuild

* Sat Jan 11 15:32:31 CET 2020 Robert-André Mauchin <zebob.m@gmail.com> - 19.12.1-1
- Update to 19.12.1 (#1789485)
- Fix desktop file (#1790040)

* Sat Dec 14 05:41:09 CET 2019 Robert-André Mauchin <zebob.m@gmail.com> - 19.12.0-1
- Release 19.12.0 (#1773785)

* Mon Dec 09 2019 Jan Grulich <jgrulich@redhat.com> - 0.4.2-4
- rebuild (qt5)

* Wed Sep 25 2019 Jan Grulich <jgrulich@redhat.com> - 0.4.2-3
- rebuild (qt5)

* Wed Jul 24 2019 Fedora Release Engineering <releng@fedoraproject.org> - 0.4.2-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_31_Mass_Rebuild

* Wed Jul 17 00:24:49 CEST 2019 Robert-André Mauchin <zebob.m@gmail.com> - 0.4.2-1
- Release 0.4.2 (#1722265)

* Mon Jun 17 2019 Jan Grulich <jgrulich@redhat.com> - 0.4.0-2
- rebuild (qt5)

* Mon May 20 23:45:59 CEST 2019 Robert-André Mauchin <zebob.m@gmail.com> - 0.4.0-1
- Release 0.4.0

* Thu Jan 31 2019 Fedora Release Engineering <releng@fedoraproject.org> - 0.3.0-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_30_Mass_Rebuild

* Sun Oct 07 2018 Robert-André Mauchin <zebob.m@gmail.com> - 0.3.0-2
- Add qt5-qtquickcontrols

* Sun Sep 30 2018 Robert-André Mauchin <zebob.m@gmail.com> - 0.3.0-1
- Release 0.3.0

* Mon Jul 02 2018 Robert-André Mauchin <zebob.m@gmail.com> - 0.2.0-1
- Release 0.2.0

* Tue Apr 17 2018 Robert-André Mauchin <zebob.m@gmail.com> - 0.1.1-1
- Release 0.1.1

* Sat Apr 07 2018 Robert-André Mauchin <zebob.m@gmail.com> - 0.1-1
- Release 0.1

* Fri Feb 02 2018 Robert-André Mauchin <zebob.m@gmail.com> - 0.0.81-0.2.alpha2
- Rebuild with missing translations

* Thu Feb 01 2018 Robert-André Mauchin <zebob.m@gmail.com> - 0.0.81-0.1.alpha2
- Release 0.0.81

* Fri Dec 08 2017 Robert-André Mauchin <zebob.m@gmail.com> - 0.0.80-0.1.alpha1
- First RPM release
