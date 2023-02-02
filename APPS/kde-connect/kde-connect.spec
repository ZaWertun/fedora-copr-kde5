# enable experimental (default off) bluetooth support
#global bluetooth 1

%global module kdeconnect-kde

Name:           kde-connect
Version:        22.12.2
Release:        1%{?dist}
License:        GPLv2+
Summary:        KDE Connect client for communication with smartphones

%global version_major %(echo %{version} |cut -d. -f1)

Url:            https://community.kde.org/KDEConnect
#Url:            https://cgit.kde.org/kdeconnect-kde.git
%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0:        http://download.kde.org/%{stable}/release-service/%{version}/src/%{module}-%{version}.tar.xz
Source1:        http://download.kde.org/%{stable}/release-service/%{version}/src/%{module}-%{version}.tar.xz.sig
Source2:        gpgkey-D81C0CB38EB725EF6691C385BB463350D6EF31EF.gpg

# firewalld service definition, see https://bugzilla.redhat.com/show_bug.cgi?id=1257699#c2
Source3:        kde-connect.xml

## upstream patches

BuildRequires:  gnupg2
BuildRequires:  desktop-file-utils
BuildRequires:  firewalld-filesystem
BuildRequires:  libappstream-glib
BuildRequires:  gcc-c++

BuildRequires:  extra-cmake-modules >= 5.42
BuildRequires:  kf5-rpm-macros
BuildRequires:  cmake(KF5ConfigWidgets)
BuildRequires:  cmake(KF5DBusAddons)
BuildRequires:  cmake(KF5DocTools)
BuildRequires:  cmake(KF5I18n)
BuildRequires:  cmake(KF5IconThemes)
BuildRequires:  cmake(KF5KCMUtils)
BuildRequires:  cmake(KF5KIO)
BuildRequires:  cmake(KF5Kirigami2)
BuildRequires:  cmake(KF5Notifications)
BuildRequires:  cmake(KF5People)
BuildRequires:  cmake(KF5Service)
BuildRequires:  cmake(KF5Wayland)
BuildRequires:  cmake(KF5PeopleVCard)
BuildRequires:  cmake(KF5QQC2DesktopStyle)
BuildRequires:  cmake(KF5Package)
BuildRequires:  cmake(KF5GuiAddons)

%if 0%{?bluetooth}
BuildRequires:  cmake(Qt5Bluetooth)
%endif
BuildRequires:  cmake(Qt5DBus)
BuildRequires:  cmake(Qt5Multimedia)
BuildRequires:  cmake(Qt5Network)
BuildRequires:  cmake(Qt5Quick)
BuildRequires:  cmake(Qt5Test)
BuildRequires:  cmake(Qt5X11Extras)
BuildRequires:  cmake(Qt5QuickControls2)
BuildRequires:  cmake(Qt5WaylandClient)

BuildRequires:  qt5-qtbase-private-devel

BuildRequires:  cmake(Qca-qt5)

BuildRequires:  cmake(KF5PulseAudioQt)

BuildRequires:  libXtst-devel
BuildRequires:  pkgconfig(libfakekey)
BuildRequires:  pkgconfig(wayland-client)
BuildRequires:  cmake(PlasmaWaylandProtocols)

Obsoletes: kde-connect-kde4-ioslave < %{version}-%{release}
Obsoletes: kde-connect-kde4-libs < %{version}-%{release}

# upstream name
Provides:       kdeconnect-kde = %{version}-%{release}

Requires:       %{name}-libs%{?_isa} = %{version}-%{release}
Requires:       kdeconnectd = %{version}-%{release}

Requires:       fuse-sshfs
Requires:       qca-qt5-ossl%{?_isa}
# /usr/bin/plasmawindowed (make optional at least until this is split out for bug #1286431)
#Recommends:     plasma-workspace
# /usr/bin/kcmshell5
Requires:       kde-cli-tools

%description
KDE Connect adds communication between KDE and your smartphone.

Currently, you can pair with your Android devices over Wifi using the
KDE Connect 1.0 app from Albert Vaka which you can obtain via Google Play, F-Droid
or the project website.

%package -n kdeconnectd
Summary: KDE Connect service
Requires: %{name}-libs%{?_isa} = %{version}-%{release}
Requires: firewalld-filesystem
%description -n kdeconnectd
%{summary}.

%package libs
Summary: Runtime libraries for %{name}
# I think we may want to drop this, forces kdeconnectd to pull in main pkg indirectly -- rex
Requires: %{name} = %{version}-%{release}
%description libs
%{summary}.

%package devel
Summary: Development files for %{name}
Requires: %{name}-libs%{?_isa} = %{version}-%{release}
%description devel
%{summary}.

%package nautilus
Summary: KDEConnect extention for nautilus
Requires: kdeconnectd = %{version}-%{release}
Requires: nautilus-python
Supplements: (kdeconnectd and nautilus)
%description nautilus
%{summary}.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -n %{module}-%{version} -p1


%build
%cmake_kf5 \
  %{?bluetooth:-DBLUETOOTH_ENABLED:BOOL=ON}
%cmake_build


%install
%cmake_install

# firewalld as shipped in f31+ provides it's own kdeconnect.xml
%if 0%{?fedora} && 0%{?fedora} < 31
install -m644 -p -D %{SOURCE3} %{buildroot}%{_prefix}/lib/firewalld/services/kde-connect.xml
%endif

%find_lang %{name} --all-name --with-html

# https://bugzilla.redhat.com/show_bug.cgi?id=1296523
desktop-file-edit --remove-key=OnlyShowIn %{buildroot}%{_sysconfdir}/xdg/autostart/org.kde.kdeconnect.daemon.desktop


%check
appstream-util validate-relax --nonet %{buildroot}%{_kf5_metainfodir}/org.kde.kdeconnect.appdata.xml ||:
appstream-util validate-relax --nonet %{buildroot}%{_kf5_metainfodir}/org.kde.kdeconnect.metainfo.xml
for i in %{buildroot}%{_datadir}/applications/org.kde.kdeconnect*.desktop ; do
desktop-file-validate $i ||:
done


%files -f %{name}.lang
%license LICENSES/*.txt
%{_kf5_bindir}/kdeconnect-*
%dir %{_kf5_datadir}/kdeconnect/
%{_kf5_datadir}/kdeconnect/kdeconnect_clipboard_config.qml
%{_kf5_datadir}/plasma/plasmoids/org.kde.kdeconnect/
%{_kf5_datadir}/knotifications5/*
%{_kf5_datadir}/kservices5/*.desktop
%{_kf5_datadir}/qlogging-categories5/kdeconnect-kde.categories
%{_qt5_plugindir}/kcm_kdeconnect.so
%{_kf5_plugindir}/kfileitemaction/kdeconnectfileitemaction.so
%{_kf5_plugindir}/kio/kdeconnect.so
%{_datadir}/icons/hicolor/*/apps/kdeconnect*
%{_datadir}/icons/hicolor/*/status/{laptop,smartphone,tablet,tv}{connected,disconnected,trusted}.svg
%{_kf5_metainfodir}/org.kde.kdeconnect.appdata.xml
%{_kf5_metainfodir}/org.kde.kdeconnect.metainfo.xml
%{_datadir}/applications/org.kde.kdeconnect*.desktop
%{_qt5_archdatadir}/qml/org/kde/kdeconnect/
%{_datadir}/contractor/
%{_datadir}/Thunar/
%{_datadir}/zsh/
%{_datadir}/deepin/

%post -n kdeconnectd
%{?firewalld_reload}

%postun -n kdeconnectd
if [ $1 -eq 0 ] ; then
%{?firewalld_reload}
fi

%files -n kdeconnectd
%{_sysconfdir}/xdg/autostart/org.kde.kdeconnect.daemon.desktop
%{_datadir}/applications/org.kde.kdeconnect.daemon.desktop
%caps(cap_net_admin=pe) %{_libexecdir}/kdeconnectd
 %{_datadir}/dbus-1/services/org.kde.kdeconnect.service
# firewalld as shipped in f31+ provides it's own kdeconnect.xml
%if 0%{?fedora} && 0%{?fedora} < 31
%{_prefix}/lib/firewalld/services/kde-connect.xml
%endif

%ldconfig_scriptlets libs

%files libs
%{_kf5_libdir}/libkdeconnectpluginkcm.so.%{version_major}*
%{_kf5_libdir}/libkdeconnectinterfaces.so.%{version_major}*
%{_kf5_libdir}/libkdeconnectcore.so.%{version_major}*
%{_qt5_plugindir}/kdeconnect/
%{_kf5_datadir}/kdeconnect/kdeconnect_findthisdevice_config.qml
%{_kf5_datadir}/kdeconnect/kdeconnect_pausemusic_config.qml
%{_kf5_datadir}/kdeconnect/kdeconnect_runcommand_config.qml
%{_kf5_datadir}/kdeconnect/kdeconnect_sendnotifications_config.qml
%{_kf5_datadir}/kdeconnect/kdeconnect_share_config.qml

%files nautilus
%{_datadir}/nautilus-python/extensions/kdeconnect-share.py*


%changelog
* Thu Feb 02 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.12.2-1
- 22.12.2

* Fri Jan 06 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.12.1-2
- added
  kdeconnect-kde-22.12.1-fix-ssh-authentication-using-pubkey-on-recent-openssh-versions.patch

* Thu Jan 05 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.12.1-1
- 22.12.1

* Thu Dec 08 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.12.0-1
- 22.12.0

* Thu Nov 03 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.08.3-1
- 22.08.3

* Thu Oct 13 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.08.2-1
- 22.08.2

* Sat Sep 24 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.08.1-2
- rebuild

* Thu Sep 08 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.08.1-1
- 22.08.1

* Fri Aug 19 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.08.0-1
- 22.08.0

* Wed Jul 27 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.04.3-2
- rebuild

* Thu Jul 07 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.04.3-1
- 22.04.3

* Thu Jun 09 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.04.2-1
- 22.04.2

* Tue May 24 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.04.1-3
- release bump

* Thu May 12 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.04.1-1
- 22.04.1

* Sun Apr 24 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.04.0-3
- kdeconnect-kde-22.04.0-fix-crash-in-plugin-settings.patch updated

* Sat Apr 23 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.04.0-2
- added kdeconnect-kde-22.04.0-fix-crash-in-plugin-settings.patch

* Thu Apr 21 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.04.0-1
- 22.04.0

* Thu Mar 31 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 21.12.3-2
- rebuild

* Thu Mar 03 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 21.12.3-1
- 21.12.3

* Tue Feb 15 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 21.12.2-2
- rebuild

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

* Thu Jan  7 22:09:16 MSK 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.12.1-1
- 20.12.1

* Thu Dec 10 21:56:08 MSK 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.12.0-1
- 20.12.0

* Fri Nov  6 13:25:36 MSK 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.08.3-1
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

* Thu May 28 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.04.1-2
- bluetooth support disabled

* Tue May 19 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.04.1-1
- 20.04.1

* Fri May 08 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.04.0-2
- bluetooth support enabled

* Thu Apr 23 2020 Rex Dieter <rdieter@fedoraproject.org> - 20.04.0-1
- 20.04.0, part of release-service now
- add .desktop/appstream validation (permissive for now)

* Mon Mar 30 2020 Rex Dieter <rdieter@fedoraproject.org> - 1.4-2
- f31+ firewalld already supports kdeconnect

* Sun Mar 01 2020 Erich Eickmeyer <erich@ericheickmeyer.com> - 1.4-1
- 1.4

* Wed Jan 29 2020 Fedora Release Engineering <releng@fedoraproject.org> - 1.3.5-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_32_Mass_Rebuild

* Tue Jul 30 2019 Rex Dieter <rdieter@fedoraproject.org> - 1.3.5-1
- 1.3.5

* Thu Jul 25 2019 Fedora Release Engineering <releng@fedoraproject.org> - 1.3.4-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_31_Mass_Rebuild

* Wed Jun 19 2019 Rex Dieter <rdieter@fedoraproject.org> - 1.3.4-1
- 1.3.4

* Fri Feb 01 2019 Fedora Release Engineering <releng@fedoraproject.org> - 1.3.3-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_30_Mass_Rebuild

* Sat Nov 10 2018 Rex Dieter <rdieter@fedoraproject.org> - 1.3.3-1
- 1.3.3

* Fri Jul 13 2018 Fedora Release Engineering <releng@fedoraproject.org> - 1.3.1-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_29_Mass_Rebuild

* Thu May 31 2018 Rex Dieter <rdieter@fedoraproject.org> - 1.3.1-1
- 1.3.1

* Mon Apr 09 2018 Rex Dieter <rdieter@fedoraproject.org> - 1.3.0-1
- 1.3.0
- -nautilus subpkg (extention for nautilus)

* Sun Mar 04 2018 Rex Dieter <rdieter@fedoraproject.org> - 1.2.1-3
- use %%make_build %%ldconfig_scriptlets
- BR: gcc-c++

* Wed Feb 07 2018 Fedora Release Engineering <releng@fedoraproject.org> - 1.2.1-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_28_Mass_Rebuild

* Sat Jan 27 2018 Rex Dieter <rdieter@fedoraproject.org> - 1.2.1-1
- 1.2.1, update url

* Thu Jan 11 2018 Igor Gnatenko <ignatenkobrain@fedoraproject.org>
- Remove obsolete scriptlets

* Sat Oct 07 2017 Rex Dieter <rdieter@fedoraproject.org> - 1.2-2
- fix typo in Obsoletes

* Fri Oct 06 2017 Rex Dieter <rdieter@fedoraproject.org> - 1.2-1
- 1.2

* Thu Aug 03 2017 Fedora Release Engineering <releng@fedoraproject.org> - 1.0.3-4
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Binutils_Mass_Rebuild

* Wed Jul 26 2017 Fedora Release Engineering <releng@fedoraproject.org> - 1.0.3-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Mass_Rebuild

* Fri Feb 10 2017 Fedora Release Engineering <releng@fedoraproject.org> - 1.0.3-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_26_Mass_Rebuild

* Tue Dec 27 2016 Rex Dieter <rdieter@math.unl.edu> - 1.0.3-1
- kdeconnect-1.0.3 (#1408570), drop kde4 (compat) kioslave 

* Wed Oct 05 2016 Rex Dieter <rdieter@fedoraproject.org> - 1.0.1-2
- fix _with_kde4 conditional

* Wed Oct 05 2016 Rex Dieter <rdieter@fedoraproject.org> - 1.0.1-1.1
- -kde4-libs: inflate soname to avoid collisions (#1374869)
- fix Obsoletes

* Wed Sep 21 2016 Rex Dieter <rdieter@fedoraproject.org> - 1.0.1-1
- 1.0.1

* Thu Sep 01 2016 Rex Dieter <rdieter@fedoraproject.org> 1.0-2
- update URL (#1325177)

* Sun Aug 28 2016 Rex Dieter <rdieter@fedoraproject.org> - 1.0-1
- kde-connect-1.0

* Sun Jun 05 2016 Rex Dieter <rdieter@fedoraproject.org> - 0.9-7
- prep git snapshot (for 1.0 compatibility), but don't use yet
- kdeconnectd subpkg (#1324214)
- kdeconnectd does not autostart on MATE (#1296523)

* Fri Feb 19 2016 Rex Dieter <rdieter@fedoraproject.org> 0.9-6
- drop kde4 support (f24+)

* Thu Feb 04 2016 Fedora Release Engineering <releng@fedoraproject.org> - 0.9-5
- Rebuilt for https://fedoraproject.org/wiki/Fedora_24_Mass_Rebuild

* Sat Jan 09 2016 Rex Dieter <rdieter@fedoraproject.org> 0.9-4
- kde-connect-0.9g

* Tue Dec 01 2015 Rex Dieter <rdieter@fedoraproject.org> 0.9-3
- make plasma-workspace a soft dependency (#1286431)

* Thu Nov 19 2015 Rex Dieter <rdieter@fedoraproject.org> 0.9-2
- respin kde-connect-0.9f, includes translations

* Mon Nov 16 2015 Rex Dieter <rdieter@fedoraproject.org> 0.9-1
- kde-connect-0.9 (missing translations?)

* Tue Nov 10 2015 Rex Dieter <rdieter@fedoraproject.org> 0.8-10
- Requires: plasma-workspace kde-cli-tools (#1280078)

* Wed Sep 23 2015 Rex Dieter <rdieter@fedoraproject.org> 0.8-9
- include kde-connect firewalld service (#1115547)

* Thu Aug 27 2015 Helio Chissini de Castro <helio@kde.org> - 0.8-8
- Added buildreq for specific qca version that has proper headers

* Wed Aug 26 2015 Rex Dieter <rdieter@fedoraproject.org> - 0.8-7
- fresh snapshot, use releaseme to include translations
- tighten subpkg deps
- .spec cosmetics

* Fri Aug 07 2015 Helio Chissini de Castro <helio@kde.org> - 0.8-6
- Added missing requires, qca-qt5-ossl. Thanks to Stefano Cavallari <spiky.kiwi@gmail.com>

* Wed Aug 05 2015 Helio Chissini de Castro <helio@kde.org> - 0.8-5
- Update the KF5 snapshot.
- Added b revision for 0.8 KDE 4
- Added requires for fuse-ssh ( thanks to Sudhir Khanger )

* Wed Jun 17 2015 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 0.8-4
- Rebuilt for https://fedoraproject.org/wiki/Fedora_23_Mass_Rebuild

* Fri May 01 2015 Helio Chissini de Castro <helio@kde.org> - 0.8.3
- Added some missing buildrequires for rawhide

* Mon Apr 20 2015 Helio Chissini de Castro <helio@kde.org> - 0.8-2
- KDE Connect KF5 snapshot based on 0.8 and kioslave for KDE 4

* Sun Feb 22 2015 Rex Dieter <rdieter@fedoraproject.org> 0.8-1
- KDE Connect 0.8 available (#1195011)
- use %%{?_kde_runtime_requires} (instead of %%_kf5_version macro)

* Thu Oct 16 2014 Rex Dieter <rdieter@fedoraproject.org> - 0.7.3-1
- kde-connect-0.7.3
- BR: libfakekey-devel (and switch other BR's to pkgconfig style)

* Sat Aug 16 2014 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 0.7.2-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_21_22_Mass_Rebuild

* Sun Jul 06 2014 Rex Dieter <rdieter@fedoraproject.org> 0.7.2-1
- kde-connect-0.7.2 (#1116448)

* Sun Jun 29 2014 Rex Dieter <rdieter@fedoraproject.org> 0.7.1-1
- 0.7.1

* Sat Jun 28 2014 Rex Dieter <rdieter@fedoraproject.org> - 0.7-1
- kde-connect-0.7 (#1114196)
- Requires: fuse-sshfs (#1114197)
- Requires: qca-ossl
- -libs, -devel subpkgs

* Sun Jun 08 2014 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 0.6-0.3.20140305git52901898
- Rebuilt for https://fedoraproject.org/wiki/Fedora_21_Mass_Rebuild

* Wed Mar 05 2014 Martin Briza <mbriza@redhat.com> - 0.6-0.2.20140305git52901898
- Include the translations too

* Wed Mar 05 2014 Martin Briza <mbriza@redhat.com> - 0.6-0.1.20140305git52901898
- Updated to the latest upstream git to match the mobile app release

* Mon Feb 24 2014 Martin Briza <mbriza@redhat.com> - 0.5-1
- New release

* Thu Jan 02 2014 Martin Briza <mbriza@redhat.com> - 0.4.2-1
- Initial package
