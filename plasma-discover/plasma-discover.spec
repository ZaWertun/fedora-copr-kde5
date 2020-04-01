
%global base_name discover
%global kf5_version 5.45
%global flatpak_version 0.8.0
%global snapd_glib_version 1.39

Name:    plasma-discover
Summary: KDE and Plasma resources management GUI
Version: 5.18.4.1
Release: 1%{?dist}

# KDE e.V. may determine that future GPL versions are accepted
License: GPLv2 or GPLv3
URL:     https://cgit.kde.org/?p=%{base_name}.git

%global verdir %(echo %{version} | cut -d. -f1-3)
%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0: http://download.kde.org/%{stable}/plasma/%{verdir}/%{base_name}-%{version}.tar.xz

## upstream patches (in lookaside cache)

BuildRequires: appstream-qt-devel >= 0.11.1
BuildRequires: appstream-devel
BuildRequires: flatpak-devel >= %{flatpak_version}
BuildRequires: libstemmer-devel
BuildRequires: libyaml-devel
BuildRequires: desktop-file-utils
BuildRequires: libappstream-glib
BuildRequires: gettext
BuildRequires: libxml2-devel

BuildRequires: extra-cmake-modules >= %{kf5_version}
BuildRequires: kf5-attica-devel >= %{kf5_version}
BuildRequires: kf5-karchive-devel
BuildRequires: kf5-kconfig-devel
BuildRequires: kf5-kconfigwidgets-devel
BuildRequires: kf5-kcoreaddons-devel
BuildRequires: kf5-kdbusaddons-devel
BuildRequires: kf5-kdeclarative-devel
BuildRequires: kf5-ki18n-devel
BuildRequires: kf5-kiconthemes-devel
BuildRequires: kf5-kitemmodels-devel
BuildRequires: kf5-kio-devel
BuildRequires: kf5-kitemviews-devel
BuildRequires: kf5-knewstuff-devel >= %{kf5_version}
BuildRequires: kf5-knotifications-devel
BuildRequires: kf5-kcrash-devel
BuildRequires: kf5-ktextwidgets-devel
BuildRequires: kf5-kwallet-devel
BuildRequires: kf5-kwidgetsaddons-devel
BuildRequires: kf5-plasma-devel
BuildRequires: kf5-rpm-macros
BuildRequires: kf5-solid-devel

BuildRequires: cmake(Snapd) >= %{snapd_glib_version}
BuildRequires: pkgconfig(packagekitqt5)
BuildRequires: pkgconfig(phonon4qt5)
BuildRequires: pkgconfig(Qt5Concurrent)
BuildRequires: pkgconfig(Qt5DBus) >= 5.10.0
BuildRequires: pkgconfig(Qt5Network)
BuildRequires: pkgconfig(Qt5Qml)
BuildRequires: pkgconfig(Qt5QuickWidgets)
BuildRequires: pkgconfig(Qt5Svg)
BuildRequires: pkgconfig(Qt5Test)
BuildRequires: pkgconfig(Qt5Widgets)
BuildRequires: pkgconfig(Qt5Xml)

BuildRequires: kf5-kirigami2-devel >= 2.2
Requires: kf5-kirigami2%{?_isa} >= 2.2

Requires: %{name}-libs%{?_isa} = %{version}-%{release}

%description
KDE and Plasma resources management GUI.

%package libs
Summary: Runtime libraries for %{name}
Requires: PackageKit
Requires: qt5-qtquickcontrols2%{?_isa}
%description libs
%{summary}.

%package notifier
Summary: Plasma Discover Update Notifier
Obsoletes: plasma-discover-updater < 5.6.95
Provides:  plasma-discover-updater = %{version}-%{release}
Requires: %{name}-libs%{?_isa} = %{version}-%{release}
%description notifier
%{summary}.

%package flatpak
Summary: Plasma Discover flatpak support
Requires: %{name} = %{version}-%{release}
Requires: %{name}-libs = %{version}-%{release}
Requires: flatpak >= %{flatpak_version}
Requires: flatpak-libs%{?_isa} >= %{flatpak_version}
Supplements: (%{name} and flatpak)
%description flatpak
%{summary}.

%package snap
Summary: Plasma Discover snap support
Requires: %{name} = %{version}-%{release}
Requires: %{name}-libs = %{version}-%{release}
Requires: snapd-qt%{?_isa} >= %{snapd_glib_version}
Requires: snapd
Supplements: (%{name} and snapd)
%description snap
%{summary}.


%prep
%autosetup -p1 -n discover-%{version}

# disable update notifier applet by default, since fedora uses plasma-pk-updates
#sed -i \
#  -e 's|X-KDE-PluginInfo-EnabledByDefault=.*|X-KDE-PluginInfo-EnabledByDefault=false|g' \
#  notifier/plasmoid/metadata.desktop


%build
mkdir %{_target_platform}
pushd %{_target_platform}
%{cmake_kf5} ..
popd

%make_build -C %{_target_platform}


%install
make install/fast DESTDIR=%{buildroot} -C %{_target_platform}

%find_lang libdiscover
%find_lang plasma-discover --with-html
%find_lang plasma-discover-notifier


%check
appstream-util validate-relax --nonet %{buildroot}%{_kf5_metainfodir}/org.kde.discover.appdata.xml ||:
appstream-util validate-relax --nonet %{buildroot}%{_kf5_metainfodir}/org.kde.discover.flatpak.appdata.xml ||:
appstream-util validate-relax --nonet %{buildroot}%{_kf5_metainfodir}/org.kde.discover.packagekit.appdata.xml ||:
## discovernotifier currently fails, fixme
## ? tag-invalid           : stock icon is not valid [update-none]
appstream-util validate-relax --nonet %{buildroot}%{_kf5_metainfodir}/org.kde.discovernotifier.appdata.xml ||:
appstream-util validate-relax --nonet %{buildroot}%{_kf5_metainfodir}/org.kde.discover.snap.appdata.xml ||:
desktop-file-validate %{buildroot}%{_datadir}/applications/org.kde.discover.desktop


%files -f plasma-discover.lang
%{_bindir}/plasma-discover
%{_kf5_metainfodir}/org.kde.discover.appdata.xml
%{_kf5_metainfodir}/org.kde.discover.packagekit.appdata.xml
%{_datadir}/applications/org.kde.discover.desktop
%{_datadir}/applications/org.kde.discover.urlhandler.desktop
%{_datadir}/icons/hicolor/*/apps/plasmadiscover.*
%{_datadir}/icons/hicolor/*/apps/flatpak-discover.*
%{_datadir}/discover/
%{_datadir}/kxmlgui5/plasmadiscover/
%{_kf5_libexecdir}/discover/
%dir %{_libexecdir}/discover

%files notifier -f plasma-discover-notifier.lang
%{_datadir}/knotifications5/discoverabstractnotifier.notifyrc
%{_sysconfdir}/xdg/autostart/org.kde.discover.notifier.desktop
%{_datadir}/applications/org.kde.discover.notifier.desktop
%{_libexecdir}/DiscoverNotifier

%ldconfig_scriptlets libs

%files libs -f libdiscover.lang
%license COPYING COPYING.LIB
%{_datadir}/knsrcfiles/discover_ktexteditor_codesnippets_core.knsrc
%dir %{_libdir}/plasma-discover/
%{_libdir}/plasma-discover/libDiscoverNotifiers.so
%{_libdir}/plasma-discover/libDiscoverCommon.so
%dir %{_kf5_qtplugindir}/discover-notifier/
%{_kf5_qtplugindir}/discover-notifier/DiscoverPackageKitNotifier.so
%dir %{_kf5_qtplugindir}/discover
%{_kf5_qtplugindir}/discover/kns-backend.so
%{_kf5_qtplugindir}/discover/packagekit-backend.so
%dir %{_datadir}/libdiscover
%dir %{_datadir}/libdiscover/categories
%{_datadir}/libdiscover/categories/packagekit-backend-categories.xml
%{_kf5_datadir}/qlogging-categories5/*categories

%files flatpak
%{_datadir}/applications/org.kde.discover-flatpak.desktop
%{_kf5_metainfodir}/org.kde.discover.flatpak.appdata.xml
%{_kf5_qtplugindir}/discover-notifier/FlatpakNotifier.so
%{_kf5_qtplugindir}/discover/flatpak-backend.so
%{_datadir}/libdiscover/categories/flatpak-backend-categories.xml

%files snap
%{_datadir}/dbus-1/system.d/org.kde.discover.libsnapclient.conf
%{_libexecdir}/discover/SnapMacaroonDialog
%{_kf5_libexecdir}/kauth/libsnap_helper
%{_kf5_metainfodir}/org.kde.discover.snap.appdata.xml
%{_kf5_qtplugindir}/discover/snap-backend.so
%{_datadir}/dbus-1/system-services/org.kde.discover.libsnapclient.service
%{_datadir}/polkit-1/actions/org.kde.discover.libsnapclient.policy
%{_datadir}/applications/org.kde.discover.snap.urlhandler.desktop
%{_datadir}/libdiscover/categories/snap-backend-categories.xml


%changelog
* Wed Apr 01 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.18.4.1-1
- 5.18.4.1

* Wed Mar 11 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.18.3-1
- 5.18.3

* Wed Feb 26 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.18.2-1
- 5.18.2

* Wed Feb 19 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.18.1-1
- 5.18.1

* Tue Feb 11 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.18.0-1
- 5.18.0

* Thu Jan 09 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.17.5-1
- 5.17.5

* Tue Dec 03 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.17.4-1
- 5.17.4

* Tue Nov 12 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.17.3-1
- 5.17.3

* Wed Oct 30 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.17.2-1
- 5.17.2

* Wed Oct 23 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.17.1-1
- 5.17.1

* Tue Oct 15 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.17.0-1
- 5.17.0

* Tue Sep 03 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.16.5-1
- 5.16.5

* Tue Jul 30 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.16.4-1
- 5.16.4

* Tue Jul 09 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.16.3-1
- 5.16.3

* Tue Jun 25 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.16.2-1
- 5.16.2

* Tue Jun 18 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.16.1-1
- 5.16.1

* Tue Jun 11 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.16.0-1
- 5.16.0

* Tue May 07 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.15.5-1
- 5.15.5

* Tue Feb 19 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.14.5.1-1
- 5.14.5.1

* Fri Feb 01 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.14.5-1
- 5.14.5 (branch)
- revert upstream commit leading to kde bug 399981

* Tue Nov 27 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.14.4-1
- 5.14.4

* Thu Nov 08 2018 Martin Kyral <martin.kyral@gmail.com> - 5.14.3-1
- 5.14.3

* Wed Oct 24 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.14.2-1
- 5.14.2

* Tue Oct 16 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.14.1-1
- 5.14.1

* Fri Oct 05 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.14.0-1
- 5.14.0

* Fri Sep 14 2018 Martin Kyral <martin.kyral@gmail.com> - 5.13.90-1
- 5.13.90

* Tue Sep 04 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.13.5-1
- 5.13.5

* Thu Aug 02 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.13.4-1
- 5.13.4

* Fri Jul 20 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.13.3-4
- use %%_qt5_qmldir

* Wed Jul 11 2018 Martin Kyral <martin.kyral@gmail.com> - 5.13.3-1
- 5.13.3

* Mon Jul 09 2018 Martin Kyral <martin.kyral@gmail.com> - 5.13.2-1
- 5.13.2

* Tue Jun 19 2018 Martin Kyral <martin.kyral@gmail.com> - 5.13.1-1
- 5.13.1

* Mon Jun 18 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.13.0-3
- bump deps
- use Supplements
- %%check: validate apddata consistently

* Tue Jun 12 2018 Neal Gompa <ngompa13@gmail.com> - 5.13.0-2
- Enable snap backend and build as subpackage
- Use rich supplements for flatpak backend subpackage
- Fix file lists to completely separate flatpak and snap backend plugins

* Sat Jun 09 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.13.0-1
- 5.13.0

* Mon May 21 2018 Martin Kyral <martin.kyral@gmail.com> - 5.12.90-1
- 5.12.90

* Sun May 20 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.12.5.1-2
- pull in upstream fix

* Thu May 17 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.12.5.1-1
- 5.12.5.1
- +appdata validation

* Wed May 16 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.12.5-2
- pull in upstream fixes

* Tue May 01 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.12.5-1
- 5.12.5

* Tue May 01 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.12.4-2
- cleanup

* Tue Mar 27 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.12.4-1
- 5.12.4

* Tue Mar 06 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.12.3-1
- 5.12.3

* Wed Feb 21 2018 Jan Grulich <jgrulich@redhat.com> - 5.12.2-1
- 5.12.2

* Tue Feb 13 2018 Jan Grulich <jgrulich@redhat.com> - 5.12.1-1
- 5.12.1

* Fri Feb 09 2018 Fedora Release Engineering <releng@fedoraproject.org> - 5.12.0-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_28_Mass_Rebuild

* Fri Feb 02 2018 Jan Grulich <jgrulich@redhat.com> - 5.12.0-1
- 5.12.0

* Thu Jan 18 2018 Igor Gnatenko <ignatenkobrain@fedoraproject.org> - 5.11.95-3
- Remove obsolete scriptlets

* Tue Jan 16 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.11.95-2
- -flatpak subpkg
- drop -muon references (Obsoletes mostly)

* Mon Jan 15 2018 Jan Grulich <jgrulich@redhat.com> - 5.11.95-1
- 5.11.95

* Tue Jan 02 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.11.5-1
- 5.11.5

* Thu Nov 30 2017 Martin Kyral <martin.kyral@gmail.com> - 5.11.4-1
- 5.11.4

* Wed Nov 08 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.11.3-1
- 5.11.3

* Wed Oct 25 2017 Martin Kyral <martin.kyral@gmail.com> - 5.11.2-1
- 5.11.2

* Tue Oct 17 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.11.1-1
- 5.11.1

* Wed Oct 11 2017 Martin Kyral <martin.kyral@gmail.com> - 5.11.0-1
- 5.11.0

* Thu Aug 24 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.10.5-1
- 5.10.5

* Thu Aug 03 2017 Fedora Release Engineering <releng@fedoraproject.org> - 5.10.4-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Binutils_Mass_Rebuild

* Thu Jul 27 2017 Fedora Release Engineering <releng@fedoraproject.org> - 5.10.4-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Mass_Rebuild

* Sat Jul 22 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.10.4-1
- 5.10.4

* Fri Jul 07 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.10.3-2
- make kf5-kirigami2 dep versioned
- pull in upstream fixes

* Tue Jun 27 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.10.3-1
- 5.10.3

* Thu Jun 15 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.10.2-1
- 5.10.2

* Thu Jun 15 2017 Jan Grulich <jgrulich@redhat.com> - 5.10.1-2
- Require flatpak to be present for flatpak backend

* Tue Jun 06 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.10.1-1
- 5.10.1

* Wed May 31 2017 Jan Grulich <jgrulich@redhat.com> - 5.10.0-1
- 5.10.0

* Wed Apr 26 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.9.5-1
- 5.9.5

* Thu Mar 23 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.9.4-1
- 5.9.4

* Sat Mar 04 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.9.3-2
- rebuild

* Wed Mar 01 2017 Jan Grulich <jgrulich@redhat.com> - 5.9.3-1
- 5.9.3

* Tue Feb 21 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.8.6-1
- 5.8.6

* Sat Feb 11 2017 Fedora Release Engineering <releng@fedoraproject.org> - 5.8.5-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_26_Mass_Rebuild

* Wed Dec 28 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.8.5-1
- 5.8.5

* Wed Nov 23 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.8.4-2
- pull in upstream fixes

* Tue Nov 22 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.8.4-1
- 5.8.4

* Mon Nov 14 2016 Rex Dieter <rdieter@fedoraproejct.org> - 5.8.3-2
- pull in upstream fixes (appstream FTBFS #1392571)

* Tue Nov 01 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.8.3-1
- 5.8.3

* Tue Oct 18 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.8.2-1
- 5.8.2

* Tue Oct 11 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.8.1-1
- 5.8.1

* Sat Oct 01 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.8.0-2
- bump appstream dep

* Thu Sep 29 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.8.0-1
- 5.8.0

* Fri Sep 23 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.7.95-2
- (Build)Requires: kf5-kirigami

* Thu Sep 22 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.7.95-1
- 5.7.95

* Sat Sep 17 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.7.5-2
- rebuild (appstream)

* Tue Sep 13 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.7.5-1
- 5.7.5

* Tue Aug 23 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.7.4-1
- 5.7.4

* Tue Aug 02 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.7.3-1
- 5.7.3

* Mon Jul 25 2016 Helio Chissini de Castro <helio@kde.org> - 5.7.2-2
- Add missing Requires for qtquick controls

* Tue Jul 19 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.7.2-1
- 5.7.2

* Tue Jul 12 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.7.1-1
- 5.7.1

* Thu Jun 30 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.7.0-1
- 5.7.0

* Sat Jun 25 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.6.95-1
- 5.6.95, -updater => -notifier

* Tue Jun 14 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.6.5-1
- 5.6.5

* Sat May 14 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.6.4-1
- 5.6.4

* Mon Apr 18 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.6.2-2
- bindir/muon-discover symlink

* Wed Apr 13 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.6.2-1
- 5.6.2

* Sat Apr 09 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.6.1-2
- License: GPLv2 or GPLv3 (KDE e.V)
- remove some commented/unused items from .spec
- expand comment why updater applet is disabled by default

* Fri Apr 08 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.6.1-1
- 5.6.1

* Sat Mar 05 2016 Daniel Vrátil <dvratil@fedoraproject.org> - 5.5.95-1
- Plasma 5.5.95

* Tue Mar 01 2016 Daniel Vrátil <dvratil@fedoraproject.org> - 5.5.5-1
- Plasma 5.5.5

* Thu Feb 04 2016 Fedora Release Engineering <releng@fedoraproject.org> - 5.5.4-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_24_Mass_Rebuild

* Wed Jan 27 2016 Daniel Vrátil <dvratil@fedoraproject.org> - 5.5.4-1
- Plasma 5.5.4

* Thu Jan 07 2016 Daniel Vrátil <dvratil@fedoraproject.org> - 5.5.3-1
- Plasma 5.5.3

* Thu Dec 31 2015 Rex Dieter <rdieter@fedoraproject.org> - 5.5.2-1
- 5.5.2

* Tue Dec 29 2015 Rex Dieter <rdieter@fedoraproject.org> - 5.5.1-2
- update description, summary, url
- -updater: disable updater plasmoid by default

* Fri Dec 18 2015 Daniel Vrátil <dvratil@fedoraproject.org> - 5.5.1-1
- Plasma 5.5.1

* Sun Dec 13 2015 Rex Dieter <rdieter@fedoraproject.org> 5.5.0-2
- rebuild (appstream)

* Thu Dec 03 2015 Daniel Vrátil <dvratil@fedoraproject.org> - 5.5.0-1
- Plasma 5.5.0

* Wed Nov 25 2015 Daniel Vrátil <dvratil@fedoraproject.org> - 5.4.95-1
- Plasma 5.4.95

* Thu Nov 05 2015 Daniel Vrátil <dvratil@fedoraproject.org> - 5.4.3-1
- Plasma 5.4.3

* Tue Nov 03 2015 Rex Dieter <rdieter@fedoraproject.org> 5.4.2-5
- more upstream fixes

* Thu Oct 29 2015 Rex Dieter <rdieter@fedoraproject.org> 5.4.2-4
- rebuild (PackageKit-Qt)

* Thu Oct 29 2015 Rex Dieter <rdieter@fedoraproject.org> 5.4.2-3
- -libs: (explicitly) Requires: PackageKit

* Wed Oct 28 2015 Rex Dieter <rdieter@fedoraproject.org> 5.4.2-2
- backport fix package removal (kde#354415)

* Fri Oct 02 2015 Rex Dieter <rdieter@fedoraproject.org> - 5.4.2-1
- 5.4.2

* Tue Sep 29 2015 Rex Dieter <rdieter@fedoraproject.org> 5.4.1-2
- pull in upstream fixes (notably discover .desktop rename)

* Wed Sep 09 2015 Rex Dieter <rdieter@fedoraproject.org> - 5.4.1-1
- 5.4.1

* Sat Jun 27 2015 Rex Dieter <rdieter@fedoraproject.org> 5.3.2-1
- 5.3.2

* Sat Jun 27 2015 Rex Dieter <rdieter@fedoraproject.org> 5.3.1-5
- rebuild (appstream)

* Sat Jun 27 2015 Rex Dieter <rdieter@fedoraproject.org> 5.3.1-4
- rebuild (appstream)

* Wed Jun 17 2015 Rex Dieter <rdieter@fedoraproject.org> 5.3.1-3
- BR: kf5-kiconthemes-devel kf5-kio-devel kf5-kitemviews-devel

* Wed Jun 17 2015 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 5.3.1-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_23_Mass_Rebuild

* Sun May 31 2015 Daniel Vrátil <dvratil@redhat.com> 5.3.1-1
- Plasma 5.3.1

* Sun May 03 2015 Rex Dieter <rdieter@fedoraproject.org> 5.3.0-1
- 5.3.0

* Mon Apr 20 2015 Rex Dieter <rdieter@fedoraproject.org> 5.2.2-2
- -discover, -updater, -libs subpkgs (w/ main metapackage)

* Mon Apr 20 2015 Rex Dieter <rdieter@fedoraproject.org> 5.2.2-1
- 5.2.2, %%license COPYING

* Tue Mar 17 2015 Rex Dieter <rdieter@fedoraproject.org> 5.2.1-3
- fix .desktop validation errors

* Mon Mar 16 2015 Rex Dieter <rdieter@fedoraproject.org> 5.2.1-2
- cleanup for review

* Mon Mar 16 2015 Elia Devito <eliadevito@yahoo.it> 5.2.1-1
- Initial SPEC file

