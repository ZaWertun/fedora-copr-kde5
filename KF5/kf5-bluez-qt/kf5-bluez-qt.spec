%undefine __cmake_in_source_build
%global framework bluez-qt

Name:           kf5-%{framework}
Summary:        A Qt wrapper for Bluez
Version: 5.87.0
Release: 1%{?dist}

License:        LGPLv2+
URL:            https://cgit.kde.org/%{framework}.git

%global versiondir %(echo %{version} | cut -d. -f1-2)
%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0:        http://download.kde.org/%{stable}/frameworks/%{versiondir}/%{framework}-%{version}.tar.xz

BuildRequires:  extra-cmake-modules >= %{version}
BuildRequires:  kf5-rpm-macros >= %{version}
BuildRequires:  qt5-qtbase-devel
BuildRequires:  qt5-qtdeclarative-devel

# For %%{_udevrulesdir}
BuildRequires:  systemd

Requires:       kf5-filesystem >= %{version}
%if 0%{?fedora} > 22
Recommends:     bluez >= 5
%else
Requires:       bluez >= 5
%endif

%if 0%{?fedora} >= 22
## libbluedevil 5.2.2 was the last release
Obsoletes:      libbluedevil < 5.2.90
%endif

%description
BluezQt is Qt-based library written handle all Bluetooth functionality.

%package        devel
Summary:        Development files for %{name}
%if 0%{?fedora} >= 22
## libbluedevil 5.2.2 was the last release
Obsoletes:      libbluedevil-devel < 5.2.90
%endif
Requires:       %{name}%{?_isa} = %{version}-%{release}
Requires:       qt5-qtbase-devel
%description    devel
Development files for %{name}.


%prep
%autosetup -n %{framework}-%{version}


%build
%{cmake_kf5} \
  -DUDEV_RULES_INSTALL_DIR:PATH="%{_udevrulesdir}"

%cmake_build


%install
%cmake_install


%ldconfig_scriptlets

%files
%doc README.md
%license LICENSES/*.txt
%{_kf5_datadir}/qlogging-categories5/*categories
%{_libdir}/libKF5BluezQt.so.*
%{_kf5_qmldir}/org/kde/bluezqt/
%{_udevrulesdir}/61-kde-bluetooth-rfkill.rules

%files devel
%{_kf5_includedir}/BluezQt/
%{_kf5_includedir}/bluezqt_version.h
%{_kf5_libdir}/libKF5BluezQt.so
%{_kf5_libdir}/cmake/KF5BluezQt/
%{_qt5_archdatadir}/mkspecs/modules/qt_BluezQt.pri


%changelog
* Sat Oct 09 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.87.0-1
- 5.87.0

* Sat Sep 11 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.86.0-1
- 5.86.0

* Sat Aug 14 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.85.0-1
- 5.85.0

* Sat Jul 10 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.84.0-1
- 5.84.0

* Sun Jun 13 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.83.0-1
- 5.83.0

* Sat May 08 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.82.0-1
- 5.82.0

* Sun Apr 11 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.81.0-1
- 5.81.0

* Sat Mar 13 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.80.0-1
- 5.80.0

* Sat Feb 13 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.79.0-1
- 5.79.0

* Sat Jan  9 16:30:13 MSK 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.78.0-1
- 5.78.0

* Mon Dec 14 16:50:06 MSK 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.77.0-1
- 5.77.0

* Sun Nov 15 22:13:35 MSK 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.76.0-1
- 5.76.0

* Sat Oct 10 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.75.0-1
- 5.75.0

* Thu Sep 17 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.74.0-1
- 5.74.0

* Mon Aug 10 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.73.0-1
- 5.73.0

* Mon Jul 13 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.72.0-1
- 5.72.0

* Sun Jun 14 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.71.0-1
- 5.71.0

* Sun May 10 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.70.0-1
- 5.70.0

* Sat Apr 11 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.69.0-1
- 5.69.0

* Mon Mar 16 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.68.0-1
- 5.68.0

* Thu Feb 27 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.67.0-2
- rebuild

* Sun Feb 02 2020 Rex Dieter <rdieter@fedoraproject.org> - 5.67.0-1
- 5.67.0

* Wed Jan 29 2020 Fedora Release Engineering <releng@fedoraproject.org> - 5.66.0-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_32_Mass_Rebuild

* Tue Jan 07 2020 Rex Dieter <rdieter@fedoraproject.org> - 5.66.0-1
- 5.66.0

* Tue Dec 17 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.65.0-1
- 5.65.0

* Fri Nov 08 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.64.0-1
- 5.64.0

* Tue Oct 22 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.63.0-1
- 5.63.0

* Mon Sep 16 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.62.0-1
- 5.62.0

* Wed Aug 07 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.61.0-1
- 5.61.0

* Thu Jul 25 2019 Fedora Release Engineering <releng@fedoraproject.org> - 5.60.0-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_31_Mass_Rebuild

* Sat Jul 13 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.60.0-1
- 5.60.0

* Thu Jun 06 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.59.0-1
- 5.59.0

* Tue May 07 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.58.0-1
- 5.58.0

* Tue Apr 09 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.57.0-1
- 5.57.0

* Tue Mar 05 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.56.0-1
- 5.56.0

* Mon Feb 04 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.55.0-1
- 5.55.0

* Fri Feb 01 2019 Fedora Release Engineering <releng@fedoraproject.org> - 5.54.0-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_30_Mass_Rebuild

* Tue Jan 08 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.54.0-1
- 5.54.0

* Sun Dec 09 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.53.0-1
- 5.53.0

* Sun Nov 04 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.52.0-1
- 5.52.0

* Wed Oct 10 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.51.0-1
- 5.51.0

* Tue Sep 04 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.50.0-1
- 5.50.0

* Tue Aug 07 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.49.0-1
- 5.49.0

* Fri Jul 13 2018 Fedora Release Engineering <releng@fedoraproject.org> - 5.48.0-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_29_Mass_Rebuild

* Mon Jul 09 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.48.0-1
- 5.48.0

* Sat Jun 02 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.47.0-1
- 5.47.0

* Sat May 05 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.46.0-1
- 5.46.0

* Sun Apr 08 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.45.0-1
- 5.45.0

* Sat Mar 03 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.44.0-1
- 5.44.0

* Wed Feb 07 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.43.0-1
- 5.43.0

* Mon Jan 08 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.42.0-1
- 5.42.0

* Mon Dec 04 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.41.0-1
- 5.41.0

* Fri Nov 10 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.40.0-1
- 5.40.0

* Sun Oct 08 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.39.0-1
- 5.39.0

* Mon Sep 11 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.38.0-1
- 5.38.0

* Fri Aug 25 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.37.0-1
- 5.37.0

* Thu Aug 03 2017 Fedora Release Engineering <releng@fedoraproject.org> - 5.36.0-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Binutils_Mass_Rebuild

* Wed Jul 26 2017 Fedora Release Engineering <releng@fedoraproject.org> - 5.36.0-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Mass_Rebuild

* Mon Jul 03 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.36.0-1
- 5.36.0

* Sun Jun 04 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.35.0-1
- 5.35.0

* Mon May 15 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.34.0-1
- 5.34.0

* Mon Apr 03 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.33.0-1
- 5.33.0

* Sat Mar 04 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.32.0-1
- 5.32.0

* Mon Feb 06 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.31.0-1
- 5.31.0

* Fri Dec 16 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.29.0-1
- 5.29.0

* Fri Dec 16 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.29.0-1
- 5.29.0

* Mon Oct 03 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.27.0-1
- 5.27.0

* Wed Sep 07 2016 Daniel Vrátil <dvratil@fedoraproject.org> - 5.26.0-1
- KDE Frameworks 5.26.0

* Mon Aug 08 2016 Daniel Vrátil <dvratil@fedoraproject.org> - 5.25.0-1
- KDE Frameworks 5.25.0

* Sun Jul 17 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.24.0-2
- Recommends: bluez (f23+)

* Wed Jul 06 2016 Daniel Vrátil <dvratil@fedoraproject.org> - 5.24.0-1
- KDE Frameworks 5.24.0

* Tue Jun 07 2016 Daniel Vrátil <dvratil@fedoraproject.org> - 5.23.0-1
- KDE Frameworks 5.23.0

* Mon May 16 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.22.0-1
- KDE Frameworks 5.22.0

* Mon May 16 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.21.0-2
- update URL, use autosetup

* Mon Apr 04 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.21.0-1
- KDE Frameworks 5.21.0

* Mon Mar 14 2016 Daniel Vrátil <dvratil@fedoraproject.org> - 5.20.0-1
- KDE Frameworks 5.20.0

* Thu Feb 11 2016 Daniel Vrátil <dvratil@fedoraproject.org> - 5.19.0-1
- KDE Frameworks 5.19.0

* Thu Feb 04 2016 Fedora Release Engineering <releng@fedoraproject.org> - 5.18.0-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_24_Mass_Rebuild

* Thu Jan 14 2016 Rex Dieter <rdieter@fedoraproject.org> 5.18.0-2
- update URL, use %%license

* Sun Jan 03 2016 Daniel Vrátil <dvratil@fedoraproject.org> - 5.18.0-1
- KDE Frameworks 5.18.0

* Tue Dec 08 2015 Daniel Vrátil <dvratil@fedoraproject.org> - 5.17.0-1
- KDE Frameworks 5.17.0

* Sun Nov 08 2015 Daniel Vrátil <dvratil@fedoraproject.org> - 5.16.0-1
- KDE Frameworks 5.16.0

* Thu Oct 08 2015 Daniel Vrátil <dvratil@redhat.com> - 5.15.0-1
- KDE Frameworks 5.15.0

* Wed Sep 16 2015 Daniel Vrátil <dvratil@redhat.com> - 5.14.0-1
- KDE Frameworks 5.14.0

* Wed Aug 19 2015 Daniel Vrátil <dvratil@redhat.com> - 5.13.0-1
- KDE Frameworks 5.13.0

* Wed Aug 19 2015 Daniel Vrátil <dvratil@redhat.com> - 5.13.0-1
- KDE Frameworks 5.13.0

* Tue Aug 11 2015 Daniel Vrátil <dvratil@redhat.com> - 5.13.0-0.1
- KDE Frameworks 5.13

* Thu Jul 09 2015 Rex Dieter <rdieter@fedoraproject.org> - 5.12.0-1
- 5.12.0, -devel Obsoletes too (f22+), update URLs

* Tue Jun 16 2015 Daniel Vrátil <dvratil@redhat.com> - 5.11.0-2
- Only obsolete libbluedevil on F>=22

* Wed Jun 10 2015 Daniel Vrátil <dvratil@redhat.com> - 5.11.0-1
- KDE Frameworks 5.11.0

* Tue May 26 2015 Daniel Vrátil <dvratil@redhat.com> - 5.3.1-1
- Plasma 5.3.1

* Sat Apr 25 2015 Rex Dieter <rdieter@fedoraproject.org> 5.3.0-1
- 5.3.0, %%build: explicitly set -DUDEV_RULES_INSTALL_DIR=%%_udevrulesdir

* Sat Apr 25 2015 Rex Dieter <rdieter@fedoraproject.org> 5.2.95-3
- Fix libbluedevil Obsoletes (including -devel), .spec cosmetics

* Wed Apr 22 2015 Daniel Vrátil <dvratil@redhat.com> - 5.2.95-2
- install %%doc
- Obsoletes: libbluedevil

* Wed Apr 22 2015 Daniel Vrátil <dvratil@redhat.com> - 5.2.95-1
- Plasma 5.2.95
