%undefine __cmake_in_source_build

# uncomment to enable bootstrap mode
#global bootstrap 1

%if !0%{?bootstrap}
%global tests 1
%endif

Name:           kamoso
Summary:        Application for taking pictures and videos from a webcam
Version:        21.08.3
Release:        1%{?dist}

License:        GPLv2+
URL:            https://userbase.kde.org/Kamoso

%global revision %(echo %{version} | cut -d. -f3)
%global majmin_ver %(echo %{version} | cut -d. -f1,2)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0: https://download.kde.org/%{stable}/release-service/%{version}/src/%{name}-%{version}.tar.xz

## upstream patches

## upstreamable patches

BuildRequires:  boost-devel
BuildRequires:  desktop-file-utils
BuildRequires:  extra-cmake-modules
BuildRequires:  gettext
BuildRequires:  kf5-rpm-macros
BuildRequires:  kf5-kconfig-devel
BuildRequires:  kf5-kcoreaddons-devel
BuildRequires:  kf5-kdeclarative-devel
BuildRequires:  kf5-kdoctools-devel
BuildRequires:  kf5-ki18n-devel
BuildRequires:  kf5-kio-devel
BuildRequires:  kf5-kwidgetsaddons-devel
BuildRequires:  kf5-solid-devel
BuildRequires:  cmake(KF5Notifications)

BuildRequires:  kf5-purpose-devel >= 1.1
BuildRequires:  libappstream-glib
BuildRequires:  pkgconfig(libaccounts-glib)
BuildRequires:  pkgconfig(libudev)
BuildRequires:  pkgconfig(Qt5Network)
BuildRequires:  pkgconfig(Qt5Qml)
BuildRequires:  pkgconfig(gstreamer-1.0)
BuildRequires:  pkgconfig(gstreamer-base-1.0)
BuildRequires:  pkgconfig(gstreamer-video-1.0)

%if 0%{?tests}
BuildRequires: time
BuildRequires: xorg-x11-server-Xvfb
%endif

# currently not linked, needs qml resources
Requires: kf5-purpose >= 1.1

%description
Kamoso is an application to take pictures and videos out of your webcam.


%prep
%autosetup -p1


%build
%{cmake_kf5} \
  -DBUILD_TESTING:BOOL=%{?tests:ON}%{!?tests:OFF}

%cmake_build


%install
%cmake_install

%find_lang kamoso --with-html


%check
appstream-util validate-relax --nonet %{buildroot}%{_kf5_metainfodir}/org.kde.kamoso.appdata.xml
desktop-file-validate %{buildroot}%{_kf5_datadir}/applications/org.kde.kamoso.desktop
%if 0%{?tests}
export CTEST_OUTPUT_ON_FAILURE=1
xvfb-run -a \
time \
make test ARGS="--output-on-failure --timeout 300" -C %{_target_platform} ||:
%endif


%files -f kamoso.lang
%doc AUTHORS
%license LICENSES/*.txt
%{_kf5_bindir}/kamoso
%{_libdir}/gstreamer-1.0/gstkamosoqt5videosink.so
%{_kf5_datadir}/icons/hicolor/*/actions/*
%{_kf5_datadir}/icons/hicolor/*/apps/kamoso.*
%{_kf5_datadir}/sounds/kamoso-shutter.wav
%{_kf5_datadir}/knotifications5/%{name}.notifyrc
%{_kf5_datadir}/applications/org.kde.kamoso.desktop
%{_kf5_metainfodir}/org.kde.kamoso.appdata.xml


%changelog
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

* Thu Jan  7 22:09:13 MSK 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.12.1-1
- 20.12.1

* Thu Dec 10 21:56:05 MSK 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.12.0-1
- 20.12.0

* Fri Nov  6 13:25:33 MSK 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.08.3-1
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

* Thu Jul 25 2019 Fedora Release Engineering <releng@fedoraproject.org> - 18.03.80-4
- Rebuilt for https://fedoraproject.org/wiki/Fedora_31_Mass_Rebuild

* Fri Feb 01 2019 Fedora Release Engineering <releng@fedoraproject.org> - 18.03.80-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_30_Mass_Rebuild

* Fri Jul 13 2018 Fedora Release Engineering <releng@fedoraproject.org> - 18.03.80-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_29_Mass_Rebuild

* Wed Apr 04 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.03.80-1
- 18.03.80 (part of kde-apps now)

* Fri Mar 23 2018 Rex Dieter <rdieter@fedoraproject.org> - 3.2.2-6
- use %%_kf5_metainfodir, %%make_build, %%find_lang --with-html

* Wed Feb 07 2018 Fedora Release Engineering <releng@fedoraproject.org> - 3.2.2-5
- Rebuilt for https://fedoraproject.org/wiki/Fedora_28_Mass_Rebuild

* Thu Jan 11 2018 Igor Gnatenko <ignatenkobrain@fedoraproject.org> - 3.2.2-4
- Remove obsolete scriptlets

* Thu Aug 03 2017 Fedora Release Engineering <releng@fedoraproject.org> - 3.2.2-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Binutils_Mass_Rebuild

* Wed Jul 26 2017 Fedora Release Engineering <releng@fedoraproject.org> - 3.2.2-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Mass_Rebuild

* Sat Feb 18 2017 Rex Dieter <rdieter@fedoraproject.org> - 3.2.2-1
- 3.2.2, update URL

* Fri Feb 10 2017 Fedora Release Engineering <releng@fedoraproject.org> - 3.2-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_26_Mass_Rebuild

* Thu Jun 02 2016 Rex Dieter <rdieter@fedoraproject.org> - 3.2-1
- 3.2, update URL

* Thu Feb 04 2016 Fedora Release Engineering <releng@fedoraproject.org> - 3.1.0-4
- Rebuilt for https://fedoraproject.org/wiki/Fedora_24_Mass_Rebuild

* Mon Jan 11 2016 Rex Dieter <rdieter@fedoraproject.org> 3.1.0-3
- install/validate appdata, Requires: kf5-purpose

* Wed Dec 30 2015 Rex Dieter <rdieter@fedoraproject.org> 3.1.0-2
- BR: boost, udev

* Wed Dec 30 2015 Rex Dieter <rdieter@fedoraproject.org> 3.1.0-1
- kamoso-3.1

* Mon Sep 28 2015 Rex Dieter <rdieter@fedoraproject.org> 3.0-1
- kamoso-3.0

* Wed Jun 17 2015 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 2.0.2-23.20140902git
- Rebuilt for https://fedoraproject.org/wiki/Fedora_23_Mass_Rebuild

* Sat May 02 2015 Kalev Lember <kalevlember@gmail.com> - 2.0.2-22.20140902git
- Rebuilt for GCC 5 C++11 ABI change

* Wed Nov 19 2014 Rex Dieter <rdieter@fedoraproject.org> 2.0.2-21.20140902git
- git snapshot, Webcam not working in kamoso (#1163698)

* Sat Aug 16 2014 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 2.0.2-20
- Rebuilt for https://fedoraproject.org/wiki/Fedora_21_22_Mass_Rebuild

* Wed Jul 23 2014 Kevin Kofler <Kevin@tigcc.ticalc.org> - 2.0.2-19
- use the latest GStreamer 1 patch from git.reviewboard.kde.org

* Wed Jul 23 2014 Kevin Kofler <Kevin@tigcc.ticalc.org> - 2.0.2-18
- build against GStreamer 1 and QtGStreamer 1 on F21+ (#1092655)

* Sun Jun 08 2014 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 2.0.2-17
- Rebuilt for https://fedoraproject.org/wiki/Fedora_21_Mass_Rebuild

* Thu Mar 20 2014 Rex Dieter <rdieter@fedoraproject.org> 2.0.2-16
- rebuild (kde-4.13)

* Fri Nov 08 2013 Rex Dieter <rdieter@fedoraproject.org> - 2.0.2-15
- simplify -runtime dep, .spec cleanup

* Wed Nov  6 2013 Alexey Kurov <nucleo@fedoraproject.org> - 2.0.2-14
- Requires: kde-runtime (#986964)

* Tue Nov  5 2013 Alexey Kurov <nucleo@fedoraproject.org> - 2.0.2-13
- Requires: oxygen-icon-theme (#986964)

* Sat Aug 03 2013 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 2.0.2-12
- Rebuilt for https://fedoraproject.org/wiki/Fedora_20_Mass_Rebuild

* Thu Jun 27 2013 Rex Dieter <rdieter@fedoraproject.org> 2.0.2-11
- rebuild (libkipi)

* Thu Feb 14 2013 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 2.0.2-10
- Rebuilt for https://fedoraproject.org/wiki/Fedora_19_Mass_Rebuild

* Fri Dec 21 2012 Rex Dieter <rdieter@fedoraproject.org> 2.0.2-9
- fix build for older libkipi

* Fri Dec 21 2012 Rex Dieter <rdieter@fedoraproject.org> 2.0.2-8
- Kamoso has a missing icon for the pictures button (848079)
- pull in upstream fix for broken about dialog

* Wed Nov 21 2012 Rex Dieter <rdieter@fedoraproject.org> 2.0.2-7
- fix build against libkipi-4.9.50+ (kde#307147)

* Thu Jul 19 2012 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 2.0.2-6
- Rebuilt for https://fedoraproject.org/wiki/Fedora_18_Mass_Rebuild

* Mon Jun 11 2012 Rex Dieter <rdieter@fedoraproject.org> 2.0.2-5
- rename icons to avoid conflict with kdeplasma-addons' krunner plugin

* Tue May 29 2012 Alexey Kurov <nucleo@fedoraproject.org> - 2.0.2-4
- fix build against libkipi-4.8.80

* Fri Jan 13 2012 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 2.0.2-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_17_Mass_Rebuild

* Fri Jul 01 2011 Rex Dieter <rdieter@fedoraproject.org> 2.0.2-2
- s/libkipi-devel/pkgconfig(libkipi)/

* Mon May 30 2011 Alexey Kurov <nucleo@fedoraproject.org> - 2.0.2-1
- kamoso-2.0.2

* Sun May 29 2011 Alexey Kurov <nucleo@fedoraproject.org> - 2.0-1
- kamoso-2.0-final
- License: GPLv2+

* Wed Feb 23 2011 Alexey Kurov <nucleo@fedoraproject.org> - 2.0-0.4.beta1
- kamoso-2.0-beta1

* Tue Feb 22 2011 Alexey Kurov <nucleo@fedoraproject.org> - 2.0-0.3.alpha2
- BR: libkipi-devel

* Fri Feb  4 2011 Alexey Kurov <nucleo@fedoraproject.org> - 2.0-0.2.alpha2
- License: GPLv2+ and GPLv3+

* Thu Feb  3 2011 Alexey Kurov <nucleo@fedoraproject.org> - 2.0-0.1.alpha2
- Initial RPM release
