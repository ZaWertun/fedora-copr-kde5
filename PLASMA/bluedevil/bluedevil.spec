Name:    bluedevil
Summary: Bluetooth stack for KDE
Version: 5.26.0
Release: 1%{?dist}

License: GPLv2+
URL:     https://cgit.kde.org/%{name}.git

%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0: http://download.kde.org/%{stable}/plasma/%(echo %{version} |cut -d. -f1-3)/%{name}-%{version}.tar.xz
Source1: http://download.kde.org/%{stable}/plasma/%(echo %{version} |cut -d. -f1-3)/%{name}-%{version}.tar.xz.sig
Source2: https://jriddell.org/esk-riddell.gpg

BuildRequires:  gnupg2
BuildRequires:  extra-cmake-modules
BuildRequires:  kf5-rpm-macros >= 5.25.0-2
BuildRequires:  qt5-qtbase-devel
BuildRequires:  qt5-qtdeclarative-devel

BuildRequires:  kf5-kcoreaddons-devel
BuildRequires:  kf5-kwidgetsaddons-devel
BuildRequires:  kf5-kdbusaddons-devel
BuildRequires:  kf5-knotifications-devel
BuildRequires:  kf5-kiconthemes-devel
BuildRequires:  kf5-plasma-devel
BuildRequires:  kf5-ki18n-devel
BuildRequires:  kf5-kio-devel
# 5.11 is when kf5-bluez-qt became Framework and changed API
BuildRequires:  kf5-bluez-qt-devel >= 5.11
BuildRequires:  kf5-kded-devel
BuildRequires:  kf5-kwindowsystem-devel
BuildRequires:  kf5-kdeclarative-devel
BuildRequires:  cmake(KF5DocTools)
BuildRequires:  cmake(KF5KCMUtils)

BuildRequires:  shared-mime-info

BuildRequires:  desktop-file-utils

Provides:       dbus-bluez-pin-helper

Obsoletes:      kbluetooth < 0.4.2-3
Obsoletes:      bluedevil-devel < 2.0.0-0.10

Requires:       bluez >= 5
Requires:       kf5-kded
Requires:       pulseaudio-module-bluetooth

# When -autostart was removed
Obsoletes:      bluedevil-autostart < 5.2.95

%description
BlueDevil is the bluetooth stack for KDE.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -n %{name}-%{version} -p1


%build
%cmake_kf5
%cmake_build


%install
%cmake_install
%find_lang %{name} --all-name --with-html


%check
desktop-file-validate %{buildroot}%{_kf5_datadir}/applications/org.kde.bluedevilsendfile.desktop
desktop-file-validate %{buildroot}%{_kf5_datadir}/applications/org.kde.bluedevilwizard.desktop


%files -f %{name}.lang
%doc README
%{_kf5_bindir}/bluedevil-sendfile
%{_kf5_bindir}/bluedevil-wizard
%{_kf5_plugindir}/kio/kio_obexftp.so
%{_kf5_plugindir}/kio/kio_bluetooth.so
%{_kf5_qtplugindir}/plasma/kcms/systemsettings/kcm_bluetooth.so
%{_kf5_plugindir}/kded/*.so
%{_kf5_datadir}/remoteview/bluetooth-network.desktop
%{_kf5_datadir}/kservices5/*.desktop
%{_kf5_datadir}/knotifications5/bluedevil.notifyrc
%{_kf5_datadir}/applications/kcm_bluetooth.desktop
%{_kf5_datadir}/applications/org.kde.bluedevilsendfile.desktop
%{_kf5_datadir}/applications/org.kde.bluedevilwizard.desktop
%{_kf5_datadir}/bluedevilwizard/
%{_kf5_datadir}/plasma/plasmoids/org.kde.plasma.bluetooth
%{_kf5_qmldir}/org/kde/plasma/private/bluetooth/
%{_kf5_metainfodir}/org.kde.plasma.bluetooth.appdata.xml
%{_kf5_datadir}/kpackage/kcms/kcm_bluetooth/contents/ui/*.qml
%{_datadir}/mime/packages/*.xml
%{_kf5_datadir}/qlogging-categories5/bluedevil.categories


%changelog
* Tue Oct 11 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.26.0-1
- 5.26.0

* Tue Sep 06 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.25.5-1
- 5.25.5

* Tue Aug 02 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.25.4-1
- 5.25.4

* Wed Jul 13 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.25.3-1
- 5.25.3

* Tue Jun 28 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.25.2-1
- 5.25.2

* Tue Jun 21 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.25.1-1
- 5.25.1

* Tue Jun 14 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.25.0-1
- 5.25.0

* Tue May 03 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.24.5-1
- 5.24.5

* Tue Mar 29 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.24.4-1
- 5.24.4

* Tue Mar 08 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.24.3-1
- 5.24.3

* Tue Feb 22 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.24.2-1
- 5.24.2

* Tue Feb 15 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.24.1-1
- 5.24.1

* Tue Feb 08 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.24.0-1
- 5.24.0

* Tue Jan 04 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.23.5-1
- 5.23.5

* Tue Nov 30 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.23.4-1
- 5.23.4

* Tue Nov 09 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.23.3-1
- 5.23.3

* Tue Oct 26 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.23.2-1
- 5.23.2

* Tue Oct 19 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.23.1-1
- 5.23.1

* Thu Oct 14 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.23.0-1
- 5.23.0

* Tue Aug 31 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.22.5-1
- 5.22.5

* Tue Jul 27 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.22.4-1
- 5.22.4

* Thu Jul 08 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.22.3-1
- 5.22.3

* Wed Jun 23 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.22.2.1-1
- 5.22.2.1

* Tue Jun 22 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.22.2-1
- 5.22.2

* Tue Jun 15 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.22.1-1
- 5.22.1

* Tue Jun 08 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.22.0-1
- 5.22.0

* Tue May 04 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.21.5-1
- 5.21.5

* Tue Apr 06 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.21.4-1
- 5.21.4

* Tue Mar 16 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.21.3-1
- 5.21.3

* Wed Mar 03 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.21.2-1
- 5.21.2

* Tue Feb 23 13:49:59 MSK 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.21.1-1
- 5.21.1

* Tue Feb 16 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.21.0-1
- 5.21.0

* Tue Jan  5 22:06:12 MSK 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.20.5-1
- 5.20.5

* Tue Dec  1 22:30:36 MSK 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.20.4-1
- 5.20.4

* Wed Nov 11 11:10:10 MSK 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.20.3-1
- 5.20.3

* Tue Oct 27 16:56:19 MSK 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.20.2-1
- 5.20.2

* Tue Oct 20 17:02:35 MSK 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.20.1-1
- 5.20.1

* Tue Oct 13 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.20.0-1
- 5.20.0

* Tue Sep 01 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.19.5-1
- 5.19.5

* Tue Jul 28 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.19.4-1
- 5.19.4

* Tue Jul 07 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.19.3-1
- 5.19.3

* Tue Jun 23 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.19.2-1
- 5.19.2

* Tue Jun 16 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.19.1-1
- 5.19.1

* Mon Jun 15 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.19.0-1
- 5.19.0

* Wed May 06 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.18.5-1
- 5.18.5

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

* Thu Jun 13 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.16.0-1
- 5.16.0

* Wed May 08 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.15.5-1
- 5.15.5

* Tue Feb 19 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.14.5-1
- 5.14.5

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

* Thu Jul 12 2018 Fedora Release Engineering <releng@fedoraproject.org> - 5.13.3-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_29_Mass_Rebuild

* Wed Jul 11 2018 Martin Kyral <martin.kyral@gmail.com> - 5.13.3-1
- 5.13.3

* Mon Jul 09 2018 Martin Kyral <martin.kyral@gmail.com> - 5.13.2-1
- 5.13.2

* Tue Jun 19 2018 Martin Kyral <martin.kyral@gmail.com> - 5.13.1-1
- 5.13.1

* Sat Jun 09 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.13.0-1
- 5.13.0

* Fri May 18 2018 Martin Kyral <martin.kyral@gmail.com> - 5.12.90-1
- 5.12.90

* Tue May 01 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.12.5-1
- 5.12.5

* Tue May 01 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.12.4-2
- .spec cleanup

* Tue Mar 27 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.12.4-1
- 5.12.4

* Tue Mar 06 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.12.3-1
- 5.12.3

* Wed Feb 21 2018 Jan Grulich <jgrulich@redhat.com> - 5.12.2-1
- 5.12.2

* Tue Feb 13 2018 Jan Grulich <jgrulich@redhat.com> - 5.12.1-1
- 5.12.1

* Wed Feb 07 2018 Fedora Release Engineering <releng@fedoraproject.org> - 5.12.0-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_28_Mass_Rebuild

* Fri Feb 02 2018 Jan Grulich <jgrulich@redhat.com> - 5.12.0-1
- 5.12.0

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

* Wed Aug 02 2017 Fedora Release Engineering <releng@fedoraproject.org> - 5.10.4-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Binutils_Mass_Rebuild

* Wed Jul 26 2017 Fedora Release Engineering <releng@fedoraproject.org> - 5.10.4-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Mass_Rebuild

* Fri Jul 21 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.10.4-1
- 5.10.4

* Tue Jun 27 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.10.3-1
- 5.10.3

* Thu Jun 15 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.10.2-1
- 5.10.2

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

* Fri Feb 10 2017 Fedora Release Engineering <releng@fedoraproject.org> - 5.8.5-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_26_Mass_Rebuild

* Wed Dec 28 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.8.5-1
- 5.8.5

* Tue Nov 22 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.8.4-1
- 5.8.4

* Tue Nov 01 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.8.3-1
- 5.8.3

* Tue Oct 18 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.8.2-1
- 5.8.2

* Tue Oct 11 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.8.1-1
- 5.8.1

* Thu Sep 29 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.8.0-1
- 5.8.0

* Thu Sep 22 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.7.95-1
- 5.7.95

* Tue Sep 13 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.7.5-1
- 5.7.5

* Tue Aug 23 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.7.4-1
- 5.7.4

* Tue Aug 02 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.7.3-1
- 5.7.3

* Tue Jul 19 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.7.2-1
- 5.7.2

* Sun Jul 17 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.7.1-2
- Requires: bluez (preparing for when/if kf5-bluez-qt drops hard bluez dep)

* Tue Jul 12 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.7.1-1
- 5.7.1

* Thu Jun 30 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.7.0-1
- 5.7.0

* Sat Jun 25 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.6.95-1
- 5.6.95

* Tue Jun 14 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.6.5-1
- 5.6.5

* Sat May 14 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.6.4-1
- 5.6.4

* Tue Apr 19 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.6.3-1
- 5.6.3

* Sat Apr 09 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.6.2-1
- 5.6.2

* Fri Apr 08 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.6.1-1
- 5.6.1

* Tue Mar 01 2016 Daniel Vrátil <dvratil@fedoraproject.org> - 5.5.5-1
- Plasma 5.5.5

* Wed Feb 03 2016 Fedora Release Engineering <releng@fedoraproject.org> - 5.5.4-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_24_Mass_Rebuild

* Wed Jan 27 2016 Daniel Vrátil <dvratil@fedoraproject.org> - 5.5.4-1
- Plasma 5.5.4

* Thu Jan 07 2016 Daniel Vrátil <dvratil@fedoraproject.org> - 5.5.3-1
- Plasma 5.5.3

* Thu Dec 31 2015 Rex Dieter <rdieter@fedoraproject.org> - 5.5.2-1
- 5.5.2

* Fri Dec 18 2015 Daniel Vrátil <dvratil@fedoraproject.org> - 5.5.1-1
- Plasma 5.5.1

* Thu Dec 03 2015 Daniel Vrátil <dvratil@fedoraproject.org> - 5.5.0-1
- Plasma 5.5.0

* Wed Nov 25 2015 Daniel Vrátil <dvratil@fedoraproject.org> - 5.4.95-1
- Plasma 5.4.95

* Thu Nov 05 2015 Daniel Vrátil <dvratil@fedoraproject.org> - 5.4.3-1
- Plasma 5.4.3

* Thu Oct 01 2015 Rex Dieter <rdieter@fedoraproject.org> - 5.4.2-1
- 5.4.2

* Wed Sep 09 2015 Rex Dieter <rdieter@fedoraproject.org> - 5.4.1-1
- 5.4.1

* Fri Aug 21 2015 Daniel Vrátil <dvratil@redhat.com> - 5.4.0-1
- Plasma 5.4.0

* Thu Aug 13 2015 Daniel Vrátil <dvratil@redhat.com> - 5.3.95-1
- Plasma 5.3.95

* Thu Jun 25 2015 Daniel Vrátil <dvratil@redhat.com> - 5.3.2-1
- Plasma 5.3.2

* Wed Jun 17 2015 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 5.3.1-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_23_Mass_Rebuild

* Thu Jun 11 2015 Daniel Vrátil <dvratil@redhat.com> - 5.3.1-2
- Rebuild against kf5-bluez-qt 5.11 (API/ABI break as BluezQt became a proper Framework)

* Tue May 26 2015 Daniel Vrátil <dvratil@redhat.com> - 5.3.1-1
- Plasma 5.3.1

* Sat Apr 25 2015 Rex Dieter <rdieter@fedoraproject.org> 5.3.0-1
- 5.3.0

* Sat Apr 25 2015 Rex Dieter <rdieter@fedoraproject.org> - 5.2.95-2
- drop Provides: -autostart (it's a lie, not included or supported anymore)
- .spec cosmetics

* Wed Apr 22 2015 Daniel Vrátil <dvratil@redhat.com> - 5.2.95-1
- Plasma 5.2.95

* Fri Mar 20 2015 Daniel Vrátil <dvratil@redhat.com> - 5.2.2-1
- Plasma 5.2.2

* Fri Feb 27 2015 Daniel Vrátil <dvratil@redhat.com> - 5.2.1-2
- Rebuild (GCC 5)

* Tue Feb 24 2015 Daniel Vrátil <dvratil@redhat.com> - 5.2.1-1
- Plasma 5.2.1

* Mon Jan 26 2015 Daniel Vrátil <dvratil@redhat.com> - 5.2.0-1
- Plasma 5.2.0

* Fri Jan 16 2015 Rex Dieter <rdieter@fedoraproject.org> 2.1-2
- -autostart: Autostart support for non-KDE environments (#1008602)

* Tue Jan 13 2015 Daniel Vrátil <dvratil@redhat.com> - 5.1.95-1.beta
- Plasma 5.1.95 Beta

* Tue Dec 23 2014 Rex Dieter <rdieter@fedoraproject.org> 2.1-1
- 2.1

* Sun Dec 14 2014 Rex Dieter <rdieter@fedoraproject.org> 2.0.0-2
- pull in upstream fix for systray icon visibility when offline (kde#341768)

* Sat Dec 13 2014 Rex Dieter <rdieter@fedoraproject.org> 2.0.0-1
- 2.0

* Fri Aug 15 2014 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 2.0.0-0.15.36f0438agit20140630
- Rebuilt for https://fedoraproject.org/wiki/Fedora_21_22_Mass_Rebuild

* Tue Aug 12 2014 Rex Dieter <rdieter@fedoraproject.org> 2.0.0-0.14.36f0438agit20140630
- update mime scriptlet

* Mon Jun 30 2014 Rex Dieter <rdieter@fedoraproject.org> 2.0.0-0.13.36f0438agit20140630
- 20140630 snapshot (#1114397)

* Sat Jun 07 2014 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 2.0.0-0.12.rc1
- Rebuilt for https://fedoraproject.org/wiki/Fedora_21_Mass_Rebuild

* Tue Dec 24 2013 Rex Dieter <rdieter@fedoraproject.org> 2.0.0-0.11.rc1
- bluedevil-2.0-rc1 respin

* Sat Dec 21 2013 Rex Dieter <rdieter@fedoraproject.org> - 2.0.0-0.10.rc1
- bluedevil-2.0-rc1
- fake 2.0.0 version (instead of 2.0) to avoid epoch
- Obsoletes: bluedevil-devel

* Thu Dec 19 2013 Rex Dieter <rdieter@fedoraproject.org> 2.0.0-0.8.20131219
- 20131219 snapshot

* Thu Dec 19 2013 Rex Dieter <rdieter@fedoraproject.org> 2.0.0-0.7.20131128
- try out crash fixer (kde review 114433)

* Thu Dec 12 2013 Rex Dieter <rdieter@fedoraproject.org>
- 2.0.0-0.6.20131128
- Obsolete dep on obexd (#998218)
- noarch -devel subpkg

* Mon Dec 09 2013 Rex Dieter <rdieter@fedoraproject.org> 2.0.0-0.5.20131128
- 20131128 snapshot

* Tue Oct 15 2013 Lukáš Tinkl <ltinkl@redhat.com> 2.0.0-0.4.20131015git
- updated git snapshot from the bluez5 branch
- translations included

* Fri Sep 20 2013 Rex Dieter <rdieter@fedoraproject.org> 2.0.0-0.3.20130907git
- fresh bluez5 branch snapshot

* Fri Sep 20 2013 Rex Dieter <rdieter@fedoraproject.org> 2.0.0-0.2.20130621
- drop Requires: obex-data-server (deprecated with bluez5)

* Wed Aug 14 2013 Rex Dieter <rdieter@fedoraproject.org> 2.0.0-0.1.20130621
- bluedevil-2.0.0-20130621 bluez5 branch snapshot

* Sat Aug 03 2013 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 1.3-4
- Rebuilt for https://fedoraproject.org/wiki/Fedora_20_Mass_Rebuild

* Wed Jun 19 2013 Rex Dieter <rdieter@fedoraproject.org> 1.3-3
- ExcludeArch: s390 s390x (#975736)

* Wed Feb 13 2013 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 1.3-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_19_Mass_Rebuild

* Mon Dec 31 2012 Rex Dieter <rdieter@fedoraproject.org> 1.3-1
- 1.3

* Wed Jul 18 2012 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 1.3-0.6.rc2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_18_Mass_Rebuild

* Mon Apr 30 2012 Rex Dieter <rdieter@fedoraproject.org> 1.3-0.5.rc2
- include translations (copied from -rc1)

* Sun Apr 29 2012 Rex Dieter <rdieter@fedoraproject.org> 1.3-0.4.rc2
- update to 1.3-rc2

* Wed Apr 25 2012 Rex Dieter <rdieter@fedoraproject.org> 1.3-0.3.rc1
- kde daemon crash (kde#284052)

* Thu Jan 12 2012 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 1.3-0.2.rc1
- Rebuilt for https://fedoraproject.org/wiki/Fedora_17_Mass_Rebuild

* Thu Nov 03 2011 Jaroslav Reznik <jreznik@redhat.com> - 1.3-0.1.rc1
- update to 1.3-rc1

* Mon Oct 10 2011 Jaroslav Reznik <jreznik@redhat.com> - 1.2.2-1
- update to 1.2.2

* Tue Sep 13 2011 Jaroslav Reznik <jreznik@redhat.com> - 1.2-1
- update to 1.2 final

* Mon Sep 05 2011 Jaroslav Reznik <jreznik@redhat.com> - 1.2-0.2.rc2
- update to 1.2-rc2

* Fri Aug 19 2011 Jaroslav Reznik <jreznik@redhat.com> - 1.2-0.1.rc1
- update to 1.2-rc1

* Tue Jul 26 2011 Jaroslav Reznik <jreznik@redhat.com> - 1.1.1-1
- update to 1.1.1

* Mon May 02 2011 Jaroslav Reznik <jreznik@redhat.com> - 1.1-2
- rebuilt for libbluedevil 1.9 snapshot

* Fri Apr 15 2011 Jaroslav Reznik <jreznik@redhat.com> - 1.1-1
- update to 1.1
- add pulseaudio-module-bluetooth req

* Mon Mar 28 2011 Jaroslav Reznik <jreznik@redhat.com> - 1.0.3-1
- update to 1.0.3

* Mon Feb 07 2011 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 1.0.2-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_15_Mass_Rebuild

* Wed Feb 02 2011 Kevin Kofler <Kevin@tigcc.ticalc.org> 1.0.2-2
- Requires: obex-data-server obexd (for file transfers)

* Wed Feb 02 2011 Lukas Tinkl <ltinkl@redhat.com> - 1.0.2-1
- 1.0.2 upstream version, fixes mainly for device pairing and obex crashes

* Tue Feb 01 2011 Rex Dieter <rdieter@fedoraproject.org> 1.0.1-3
- Requires: kdebase-runtime
- add scriptlets

* Sat Jan 29 2011 Kevin Kofler <Kevin@tigcc.ticalc.org> 1.0.1-2
- Provides: dbus-bluez-pin-helper (keeps blueman and hal off the KDE spin)
- fix kbluetooth Obsoletes to match 0.4.2-2.fc* properly

* Fri Jan 14 2011 Jaroslav Reznik <jreznik@redhat.com> 1.0.1-1
- update to 1.0.1

* Tue Nov 30 2010 Jaroslav Reznik <jreznik@redhat.com> 1.0-1
- update to 1.0 final

* Mon Sep 27 2010 Jaroslav Reznik <jreznik@redhat.com> 1.0-0.1.rc4.1
- update to rc4-1

* Thu Aug 19 2010 Jaroslav Reznik <jreznik@redhat.com> 1.0-0.1.rc3
- update to rc3

* Fri Aug 13 2010 Jaroslav Reznik <jreznik@redhat.com> 1.0-0.1.rc2
- initial package
