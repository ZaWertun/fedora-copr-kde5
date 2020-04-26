Name:       elisa-player
Version:    20.04.0
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
# Build tarball with translations.
# git clone https://anongit.kde.org/releaseme.git
# cd releaseme
# RELEASEME_READONLY=1 ./tarme.rb --version 20.03.90 --origin trunk elisa
# Source0:    elisa-%{version}.tar.xz

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
BuildRequires:  qt5-qtbase-private-devel
Requires:       hicolor-icon-theme
Requires:       kde-filesystem
Requires:       qt5-qtquickcontrols

%description
Elisa is a simple music player aiming to provide a nice experience for its
users.

%prep
%autosetup -n elisa-%{version}

%build
mkdir build
pushd build
    %cmake_kf5 ..
    %make_build
popd

%install
%make_install -C build

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
%{_kf5_metainfodir}/org.kde.elisa.appdata.xml
%{_kf5_libdir}/elisa/
%{_kf5_libdir}/qt5/qml/org/kde/elisa/

%changelog
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
