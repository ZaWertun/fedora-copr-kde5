Name:    plasma-pa
Version: 5.26.0
Release: 1%{?dist}
Summary: Plasma applet for audio volume management using PulseAudio

License: LGPLv2+ and GPLv2+
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

## upstream

BuildRequires:  gnupg2
BuildRequires:  extra-cmake-modules
BuildRequires:  glib2-devel
BuildRequires:  kde-filesystem
BuildRequires:  kf5-kconfigwidgets-devel
BuildRequires:  kf5-kcoreaddons-devel
BuildRequires:  kf5-kdeclarative-devel
BuildRequires:  kf5-kdoctools-devel
BuildRequires:  kf5-kglobalaccel-devel
BuildRequires:  kf5-ki18n-devel
BuildRequires:  kf5-kpackage-devel
BuildRequires:  kf5-plasma-devel
BuildRequires:  kf5-rpm-macros >= 5.25.0-2
BuildRequires:  pkgconfig(gconf-2.0)
BuildRequires:  pkgconfig(libcanberra)
BuildRequires:  perl-generators
BuildRequires:  pulseaudio-libs-devel
BuildRequires:  qt5-qtbase-devel
BuildRequires:  qt5-qtdeclarative-devel
BuildRequires:  cmake(KF5Notifications)
BuildRequires:  cmake(KF5KCMUtils)
%if ! 0%{?bootstrap}
# required kpackage plugins (-packagestructure comes from plasma-workspace)
BuildRequires:  plasma-packagestructure
%endif

Requires: (pulseaudio or pipewire-pulseaudio)
# support systemsettings->multimedia->audio volume->advanced->automatically switch streams when a new output becomes available
Recommends: pulseaudio-module-gconf%{?_isa}

%description
%{summary}.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -p1


%build
%cmake_kf5
%cmake_build


%install
%cmake_install
%find_lang %{name} --all-name --with-html

## unpackaged files
rm -rfv %{buildroot}%{_kde4_appsdir}/kconf_update/


%files -f %{name}.lang
%license LICENSES/*.txt
%{_kf5_qmldir}/org/kde/plasma/private/volume/
%{_kf5_qtplugindir}/plasma/kcms/systemsettings/kcm_pulseaudio.so
%{_kf5_datadir}/kconf_update/*
%{_kf5_datadir}/kpackage/kcms/kcm_pulseaudio
%{_kf5_datadir}/applications/kcm_pulseaudio.desktop
%{_kf5_datadir}/kservices5/plasma-applet-org.kde.plasma.volume.desktop
%{_kf5_datadir}/plasma/plasmoids/org.kde.plasma.volume/
%{_kf5_metainfodir}/org.kde.plasma.volume.appdata.xml


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

* Wed May 05 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.21.5-2
- added patch for issue #433246

* Tue May 04 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.21.5-1
- 5.21.5

* Tue Apr 06 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.21.4-1
- 5.21.4

* Tue Mar 16 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.21.3-1
- 5.21.3

* Wed Mar 03 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.21.2-1
- 5.21.2

* Tue Feb 23 13:50:09 MSK 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.21.1-1
- 5.21.1

* Tue Feb 16 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.21.0-1
- 5.21.0

* Tue Jan  5 22:06:21 MSK 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.20.5-1
- 5.20.5

* Sun Jan  3 13:39:54 MSK 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.20.4-2
- require pulseaudio or pipewire-pulseaudio

* Tue Dec  1 22:30:48 MSK 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.20.4-1
- 5.20.4

* Wed Nov 11 11:10:23 MSK 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.20.3-1
- 5.20.3

* Tue Oct 27 16:56:31 MSK 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.20.2-1
- 5.20.2

* Tue Oct 20 17:02:48 MSK 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.20.1-1
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

* Tue Jun 11 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.16.0-1
- 5.16.0

* Tue May 07 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.15.5-1
- 5.15.5

* Sun Apr 28 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.15.4-1
- 5.15.4

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

* Sat Oct 06 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.14.0-1
- 5.14.0

* Fri Sep 14 2018 Martin Kyral <martin.kyral@gmail.com> - 5.13.90-1
- 5.13.90

* Tue Sep 04 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.13.5-1
- 5.13.5

* Thu Aug 02 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.13.4-1
- 5.13.4

* Fri Jul 13 2018 Fedora Release Engineering <releng@fedoraproject.org> - 5.13.3-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_29_Mass_Rebuild

* Wed Jul 11 2018 Martin Kyral <martin.kyral@gmail.com> - 5.13.3-1
- 5.13.3

* Mon Jul 09 2018 Martin Kyral <martin.kyral@gmail.com> - 5.13.2-1
- 5.13.2

* Tue Jun 19 2018 Martin Kyral <martin.kyral@gmail.com> - 5.13.1-1
- 5.13.1

* Sat Jun 09 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.13.0-1
- 5.13.0

* Mon May 21 2018 Martin Kyral <martin.kyral@gmail.com> - 5.12.90-1
- 5.12.90

* Sun May 20 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.12.5-3
- plasma-pa is using arch-dependent BuildRequires (#1545204)

* Mon May 07 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.12.5-2
- Requires: pulseaudio
- Recommends: pulseaudio-module-gconf (kde#393963)

* Tue May 01 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.12.5-1
- 5.12.5

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

* Wed Apr 26 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.9.5-2
- use %%find_lang --with-html

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

* Thu Feb 04 2016 Fedora Release Engineering <releng@fedoraproject.org> - 5.5.4-2
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

* Fri Nov 20 2015 Rex Dieter <rdieter@fedoraproject.org> 5.4.3-2
- .spec cosmetics, fix URL

* Thu Nov 05 2015 Daniel Vrátil <dvratil@fedoraproject.org> - 5.4.3-1
- Plasma 5.4.3

* Thu Oct 01 2015 Rex Dieter <rdieter@fedoraproject.org> - 5.4.2-1
- 5.4.2

* Wed Sep 09 2015 Rex Dieter <rdieter@fedoraproject.org> - 5.4.1-1
- 5.4.1

* Tue Aug 25 2015 Daniel Vrátil <dvratil@redhat.com> - 5.4.0-1
- Upate to 5.4.0
- add %%license

* Thu Aug 13 2015 Daniel Vrátil <dvratil@redhat.com> - 5.3.95-1
- Initial version
