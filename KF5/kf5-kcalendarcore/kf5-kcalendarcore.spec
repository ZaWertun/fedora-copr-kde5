%global framework kcalendarcore

# uncomment to enable bootstrap mode
#global bootstrap 1

%if !0%{?bootstrap}
%global tests 1
%endif

Name:    kf5-kcalendarcore
Epoch:   1
Version: 5.92.0
Release: 1%{?dist}
Summary: KDE Frameworks 5 Tier 1 KCalendarCore Library

License: LGPLv2+ and GPLv3+
URL:     https://cgit.kde.org/%{framework}.git

%global majmin %(echo %{version} | cut -d. -f1-2)
%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0: http://download.kde.org/%{stable}/frameworks/%{majmin}/%{framework}-%{version}.tar.xz

# libical (and thus kcalendarcore) not on all arches for RHEL8.
%if 0%{?rhel} == 8
ExclusiveArch: x86_64 ppc64le %{arm}
%endif

BuildRequires:  extra-cmake-modules >= %{majmin}
BuildRequires:  kf5-rpm-macros

BuildRequires:  bison
BuildRequires:  libical-devel

BuildRequires:  qt5-qtbase-devel

%if 0%{?tests}
BuildRequires: dbus-x11
BuildRequires: xorg-x11-server-Xvfb
%endif

%description
%{summary}.

%package        devel
Summary:        Development files for %{name}
Requires:       %{name}%{?_isa} = %{epoch}:%{version}-%{release}
Requires:       libical-devel
%description    devel
The %{name}-devel package contains libraries and header files for
developing applications that use %{name}.


%prep
%autosetup -n %{framework}-%{version}


%build
%{cmake_kf5} \
  -DBUILD_TESTING:BOOL=%{?tests:ON}%{!?tests:OFF}

%cmake_build


%install
%cmake_install

## TODO: poke upstream about failures seen on f30
#The following tests FAILED:
#         39 - testicaltimezones (Failed)
#        472 - Compat-libical3-AppleICal_1.5.ics (Failed)
#        473 - Compat-libical3-Evolution_2.8.2_timezone_test.ics (Failed)
#        475 - Compat-libical3-KOrganizer_3.1a.ics (Failed)
#        477 - Compat-libical3-MSExchange.ics (Failed)
#        478 - Compat-libical3-Mozilla_1.0.ics (Failed)
%check
%if 0%{?tests}
export CTEST_OUTPUT_ON_FAILURE=1
xvfb-run -a \
dbus-launch --exit-with-session \
make test ARGS="--output-on-failure --timeout 20" -C %{_target_platform} ||:
%endif


%ldconfig_scriptlets

%files
%license LICENSES/*.txt
%{_kf5_datadir}/qlogging-categories5/*kcalendarcore.*
%{_kf5_libdir}/libKF5CalendarCore.so.*

%files devel
%{_kf5_includedir}/kcalcore_version.h
%{_kf5_includedir}/KCalendarCore/
%{_kf5_libdir}/libKF5CalendarCore.so
%{_kf5_libdir}/cmake/KF5CalendarCore/
%{_kf5_archdatadir}/mkspecs/modules/qt_KCalendarCore.pri
%{_kf5_libdir}/pkgconfig/KF5CalendarCore.pc


%changelog
* Sun Mar 13 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 1:5.92.0-1
- 5.92.0

* Mon Feb 14 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 1:5.91.0-1
- 5.91.0

* Sat Jan 08 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 1:5.90.0-1
- 5.90.0

* Mon Dec 13 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 1:5.89.0-1
- 5.89.0

* Sat Nov 13 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 1:5.88.0-1
- 5.88.0

* Sat Oct 09 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 1:5.87.0-1
- 5.87.0

* Sat Sep 11 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 1:5.86.0-1
- 5.86.0

* Sat Aug 14 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 1:5.85.0-1
- 5.85.0

* Sat Jul 10 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 1:5.84.0-1
- 5.84.0

* Sun Jun 13 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 1:5.83.0-1
- 5.83.0

* Sat May 08 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 1:5.82.0-1
- 5.82.0

* Sun Apr 11 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 1:5.81.0-1
- 5.81.0

* Sat Mar 13 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 1:5.80.0-1
- 5.80.0

* Sat Feb 13 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 1:5.79.0-1
- 5.79.0

* Sat Jan  9 16:30:15 MSK 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 1:5.78.0-1
- 5.78.0

* Mon Dec 14 16:50:08 MSK 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 1:5.77.0-1
- 5.77.0

* Sun Nov 15 22:13:37 MSK 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 1:5.76.0-1
- 5.76.0

* Sat Oct 10 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 1:5.75.0-1
- 5.75.0

* Thu Sep 17 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 1:5.74.0-1
- 5.74.0

* Mon Aug 10 2020 Yaroslav Sidlovsky <zawertun@otl.ru> - 1:5.73.0-1
- 5.73.0

* Mon Jul 13 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 1:5.72.0-1
- 5.72.0

* Sun Jun 14 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 1:5.71.0-1
- 5.71.0

* Sun May 10 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 1:5.70.0-1
- 5.70.0

* Sat Apr 11 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 1:5.69.0-1
- 5.69.0

* Fri Mar 20 2020 Rex Dieter <rdieter@fedoraproject.org> - 1:5.68.0-1
- 5.68.0

* Sun Feb 02 2020 Rex Dieter <rdieter@fedoraproject.org> - 1:5.67.0-1
- 5.67.0

* Wed Jan 29 2020 Fedora Release Engineering <releng@fedoraproject.org> - 1:5.66.0-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_32_Mass_Rebuild

* Sat Jan 18 2020 Rex Dieter <rdieter@fedoraproject.org> - 1:5.66.0-1
- 5.66.0

* Tue Nov 12 2019 Rex Dieter <rdieter@fedoraproject.org> - 1:5.64.0-2
- -devel: fix dep on main pkg (add epoch)

* Mon Nov 11 2019 Rex Dieter <rdieter@fedoraproject.org> - 1:5.64.0-1
- move to kde frameworks
- drop Provides: kf5-kcalcore (not used anywhere)

* Mon Nov 11 2019 Rex Dieter <rdieter@fedoraproject.org> - 19.08.3-2
- CMakeLists.txt: fix PIM_VERSION (wasn't bumped verison 19.08.2)

* Mon Nov 11 2019 Rex Dieter <rdieter@fedoraproject.org> - 19.08.3-1
- 19.08.3

* Fri Oct 18 2019 Rex Dieter <rdieter@fedoraproject.org> - 19.08.2-1
- 19.08.2

* Thu Jul 25 2019 Fedora Release Engineering <releng@fedoraproject.org> - 19.04.3-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_31_Mass_Rebuild

* Fri Jul 12 2019 Rex Dieter <rdieter@fedoraproject.org> - 19.04.3-1
- 19.04.3

* Wed Jun 05 2019 Rex Dieter <rdieter@fedoraproject.org> - 19.04.2-1
- 19.04.2

* Fri Mar 08 2019 Rex Dieter <rdieter@fedoraproject.org> - 18.12.3-1
- 18.12.3

* Tue Feb 05 2019 Rex Dieter <rdieter@fedoraproject.org> - 18.12.2-1
- 18.12.2

* Fri Feb 01 2019 Fedora Release Engineering <releng@fedoraproject.org> - 18.12.1-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_30_Mass_Rebuild

* Tue Jan 08 2019 Rex Dieter <rdieter@fedoraproject.org> - 18.12.1-1
- 18.12.1

* Fri Dec 14 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.12.0-1
- 18.12.0

* Tue Nov 06 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.08.3-1
- 18.08.3

* Wed Oct 10 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.08.2-1
- 18.08.2

* Mon Oct 01 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.08.1-1
- 18.08.1

* Fri Jul 13 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.04.3-1
- 18.04.3

* Fri Jul 13 2018 Fedora Release Engineering <releng@fedoraproject.org> - 18.04.2-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_29_Mass_Rebuild

* Wed Jun 06 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.04.2-1
- 18.04.2

* Wed May 09 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.04.1-1
- 18.04.1

* Fri Apr 20 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.04.0-1
- 18.04.0

* Tue Mar 06 2018 Rex Dieter <rdieter@fedoraproject.org> - 17.12.3-1
- 17.12.3

* Tue Feb 06 2018 Rex Dieter <rdieter@fedoraproject.org> - 17.12.2-1
- 17.12.2

* Thu Jan 11 2018 Rex Dieter <rdieter@fedoraproject.org> - 17.12.1-1
- 17.12.1

* Tue Dec 12 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.12.0-1
- 17.12.0

* Wed Dec 06 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.11.90-1
- 17.11.90

* Wed Nov 22 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.11.80-1
- 17.11.80

* Wed Nov 08 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.08.3-1
- 17.08.3

* Mon Sep 25 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.08.1-1
- 17.08.1

* Fri Jul 28 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.04.3-1
- 17.04.3

* Wed Jul 26 2017 Fedora Release Engineering <releng@fedoraproject.org> - 17.04.2-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Mass_Rebuild

* Thu Jun 15 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.04.2-1
- 17.04.2

* Thu May 11 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.04.1-1
- 17.04.1

* Thu Mar 09 2017 Rex Dieter <rdieter@fedoraproject.org> - 16.12.3-1
- 16.12.3

* Thu Feb 09 2017 Rex Dieter <rdieter@fedoraproject.org> - 16.12.2-1
- 16.12.2

* Mon Jan 16 2017 Rex Dieter <rdieter@fedoraproject.org> - 16.12.1-1
- 16.12.1

* Mon Dec 05 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.08.3-1
- 16.08.3

* Thu Oct 13 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.08.2-1
- 16.08.2

* Thu Sep 08 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.08.1-1
- 16.08.1

* Sat Sep 03 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.08.0-1
- 16.08.0

* Sun Jul 10 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.04.3-1
- 16.04.3

* Sun Jun 12 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.04.2-1
- 16.04.2

* Sun May 08 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.04.1-1
- 16.04.1

* Sun May 01 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.04.0-1
- 16.04.0
- Provides: kf5-kcalcore(-devel)
- update URL
- support bootstrap
- add %%check

* Tue Mar 15 2016 Rex Dieter <rdieter@fedoraproject.org> - 15.12.3-1
- 15.12.3

* Sun Feb 14 2016 Rex Dieter <rdieter@fedoraproject.org> - 15.12.2-1
- 15.12.2

* Sat Feb 06 2016 Rex Dieter <rdieter@fedoraproject.org> 15.12.1-1
- 15.12.1

* Thu Feb 04 2016 Fedora Release Engineering <releng@fedoraproject.org> - 15.12.0-4
- Rebuilt for https://fedoraproject.org/wiki/Fedora_24_Mass_Rebuild

* Sat Jan 23 2016 Robert Scheck <robert@fedoraproject.org> - 15.12.0-3
- Rebuild for libical 2.0.0

* Tue Dec 15 2015 Jan Grulich <jgrulich@redhat.com> - 15.12.0-2
- Remove obsoletes/conflicts with kdepimlibs

* Tue Dec 15 2015 Jan Grulich <jgrulich@redhat.com> - 15.12-0-1
- Update to 15.12.0

* Mon Dec 07 2015 Jan Grulich <jgrulich@redhat.com> - 15.11.90-1
- Update to 15.11.90

* Thu Dec 03 2015 Jan Grulich <jgrulich@redhat.com> - 15.11.80-1
- Update to 15.11.80

* Mon Aug 24 2015 Daniel Vrátil <dvratil@redhat.com> - 15.08.0-1
- Initial version
