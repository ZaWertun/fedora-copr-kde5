
Name:    spectacle
Summary: Screenshot capture utility
Version: 20.04.3
Release: 1%{?dist}

License: GPLv2
URL:     https://www.kde.org/applications/graphics/spectacle/

%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0: https://download.kde.org/%{stable}/release-service/%{version}/src/%{name}-%{version}.tar.xz

## upstream patches

%global majmin %(echo %{version} | cut -d. -f1,2)

BuildRequires: desktop-file-utils
BuildRequires: libappstream-glib

BuildRequires: extra-cmake-modules
BuildRequires: kf5-rpm-macros
BuildRequires: kf5-kcoreaddons-devel
BuildRequires: kf5-kdeclarative-devel
BuildRequires: kf5-kwidgetsaddons-devel
BuildRequires: kf5-kdbusaddons-devel
BuildRequires: kf5-knotifications-devel
BuildRequires: kf5-kconfig-devel
BuildRequires: kf5-ki18n-devel
BuildRequires: kf5-kio-devel
BuildRequires: kf5-kxmlgui-devel
BuildRequires: kf5-kwindowsystem-devel
BuildRequires: kf5-kdoctools-devel
BuildRequires: kf5-kwayland-devel

BuildRequires: cmake(KF5NewStuff)
BuildRequires: cmake(KF5Screen)
BuildRequires: cmake(KF5GlobalAccel)
BuildRequires: cmake(KF5Kipi)
BuildRequires: kf5-libkipi-devel

BuildRequires: cmake(Qt5DBus)
BuildRequires: cmake(Qt5PrintSupport)
BuildRequires: cmake(Qt5Quick)
BuildRequires: cmake(Qt5X11Extras)

BuildRequires: pkgconfig(xcb-cursor)
BuildRequires: pkgconfig(xcb-image)
BuildRequires: pkgconfig(xcb-util)
BuildRequires: pkgconfig(xcb-xfixes)

BuildRequires: cmake(KDEExperimentalPurpose)

# f26+ upgrade path
%if 0%{?fedora} > 25
Obsoletes: ksnapshot <= 15.08.3
%endif

# translations moved here
Conflicts: kde-l10n < 17.03

%description
%{summary}.


%prep
%autosetup -p1


%build
mkdir %{_target_platform}
pushd %{_target_platform}
%{cmake_kf5} ..
popd

%make_build -C %{_target_platform}


%install
make install/fast DESTDIR=%{buildroot} -C %{_target_platform}

%find_lang %{name} --all-name --with-html


%check
appstream-util validate-relax --nonet %{buildroot}%{_kf5_metainfodir}/org.kde.spectacle.appdata.xml ||:
desktop-file-validate %{buildroot}%{_kf5_datadir}/applications/org.kde.spectacle.desktop

%files -f %{name}.lang
%license COPYING
%{_kf5_bindir}/spectacle
%{_kf5_metainfodir}/org.kde.spectacle.appdata.xml
%{_kf5_libdir}/kconf_update_bin/spectacle-migrate-shortcuts
%{_kf5_datadir}/applications/org.kde.spectacle.desktop
%{_kf5_datadir}/dbus-1/interfaces/org.kde.Spectacle.xml
%{_kf5_datadir}/dbus-1/services/org.kde.Spectacle.service
%{_kf5_datadir}/icons/hicolor/*/apps/spectacle.*
%{_kf5_datadir}/knotifications5/spectacle.notifyrc
%{_kf5_datadir}/kconf_update/spectacle_shortcuts.upd
%{_kf5_datadir}/kconf_update/spectacle_newConfig.upd
%{_kf5_datadir}/kglobalaccel/*.desktop
%{_kf5_datadir}/qlogging-categories5/*.categories


%changelog
* Thu Jul 09 2020 Yaroslav Sidlovsky <zawertun@otl.ru> - 20.04.3-1
- 20.04.3

* Fri Jun 12 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.04.2-1
- 20.04.2

* Tue May 19 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.04.1-1
- 20.04.1

* Fri Apr 24 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.04.0-1
- 20.04.0

* Fri Mar 06 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 19.12.3-1
- 19.12.3

* Fri Feb 07 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 19.12.2-1
- 19.12.2

* Fri Jan 10 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 19.12.1-1
- 19.12.1

* Thu Dec 12 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 19.12.0-1
- 19.12.0

* Fri Nov 08 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 19.08.3-1
- 19.08.3

* Thu Oct 10 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 19.08.2-1
- 19.08.2

* Thu Sep 05 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 19.08.1-1
- 19.08.1

* Thu Aug 15 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 19.08.0-1
- 19.08.0

* Thu Jul 11 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 19.04.3-1
- 19.04.3

* Thu Jun 06 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 19.04.2-1
- 19.04.2

* Thu May 09 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 19.04.1-1
- 19.04.1

* Sat Apr 27 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 19.04.0-1
- 19.04.0

* Tue Feb 05 2019 Rex Dieter <rdieter@fedoraproject.org> - 18.12.2-1
- 18.12.2

* Sun Feb 03 2019 Fedora Release Engineering <releng@fedoraproject.org> - 18.12.1-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_30_Mass_Rebuild

* Tue Jan 08 2019 Rex Dieter <rdieter@fedoraproject.org> - 18.12.1-1
- 18.12.1

* Sun Dec 16 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.12.0-1
- 18.12.0

* Tue Nov 06 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.08.3-1
- 18.08.3

* Wed Oct 10 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.08.2-1
- 18.08.2

* Fri Sep 07 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.08.1-1
- 18.08.1

* Fri Aug 10 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.04.3-1
- 18.04.3

* Sat Jul 14 2018 Fedora Release Engineering <releng@fedoraproject.org> - 18.04.2-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_29_Mass_Rebuild

* Wed Jun 06 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.04.2-1
- 18.04.2

* Wed May 09 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.04.1-1
- 18.04.1

* Thu Apr 19 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.04.0-1
- 18.04.0

* Sun Apr 08 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.03.90-1
- 18.03.90

* Tue Mar 06 2018 Rex Dieter <rdieter@fedoraproject.org> - 17.12.3-1
- 17.12.3

* Thu Feb 08 2018 Rex Dieter <rdieter@fedoraproject.org> - 17.12.2-1
- 17.12.2

* Thu Jan 18 2018 Igor Gnatenko <ignatenkobrain@fedoraproject.org> - 17.12.1-2
- Remove obsolete scriptlets

* Thu Jan 11 2018 Rex Dieter <rdieter@fedoraproject.org> - 17.12.1-1
- 17.12.1

* Thu Dec 28 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.12.0-1
- 17.12.0

* Wed Nov 08 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.08.3-1
- 17.08.3

* Wed Oct 11 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.08.2-1
- 17.08.2

* Thu Sep 28 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.08.1-1
- 17.08.1

* Thu Aug 03 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.04.3-1
- 17.04.3

* Thu Aug 03 2017 Fedora Release Engineering <releng@fedoraproject.org> - 17.04.2-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Binutils_Mass_Rebuild

* Thu Jul 27 2017 Fedora Release Engineering <releng@fedoraproject.org> - 17.04.2-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Mass_Rebuild

* Thu Jun 15 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.04.2-1
- 17.04.2

* Thu May 11 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.04.1-1
- 17.04.1

* Fri May 05 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.04.0-1
- 17.04.0

* Thu Mar 09 2017 Rex Dieter <rdieter@fedoraproject.org> - 16.12.3-1
- 16.12.3

* Mon Feb 20 2017 Rex Dieter <rdieter@fedoraproject.org> - 16.12.2-2
- Obsoletes: ksnapshot (f26+)

* Thu Feb 09 2017 Rex Dieter <rdieter@fedoraproject.org> - 16.12.2-1
- 16.12.2

* Thu Jan 12 2017 Rex Dieter <rdieter@fedoraproject.org> - 16.12.1-1
- 16.12.1

* Thu Jan 12 2017 Rex Dieter <rdieter@fedoraproject.org> - 16.08.3-2
- update URL

* Mon Dec 05 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.08.3-1
- 16.08.3

* Thu Oct 13 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.08.2-1
- 16.08.2

* Wed Sep 07 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.08.1-1
- 16.08.1

* Sat Aug 13 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.08.0-1
- 16.08.0

* Sat Aug 06 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.07.90-1
- 16.07.90

* Sat Jul 30 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.07.80-1
- 16.07.80

* Sun Jul 10 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.04.3-1
- 16.04.3

* Sun Jun 12 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.04.2-1
- 16.04.2

* Sun May 08 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.04.1-1
- 16.04.1

* Tue Apr 26 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.04.0-2
- backport upstream fixes

* Fri Apr 15 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.04.0-1
- 16.04.0

* Thu Apr 14 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.03.80-1
- 16.03.80

* Sat Apr 09 2016 Rex Dieter <rdieter@fedoraproject.org> - 15.12.3-2
- update URL, rebuild (libkscreen-qt5)

* Tue Mar 15 2016 Rex Dieter <rdieter@fedoraproject.org> - 15.12.3-1
- 15.12.3

* Mon Feb 15 2016 Rex Dieter <rdieter@fedoraproject.org> - 15.12.2-1
- 15.12.2

* Fri Feb 05 2016 Fedora Release Engineering <releng@fedoraproject.org> - 15.12.0-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_24_Mass_Rebuild

* Sun Jan 10 2016 Rex Dieter <rdieter@fedoraproject.org> 15.12.0-2
- add icon scriptlets
- License: GPLv2
- version: 15.12.0, so Source URL works (15.12.1 not publically available yet)

* Sun Jan 10 2016 Rex Dieter <rdieter@fedoraproject.org> 15.12.1-1
- spectacle-15.12.1


