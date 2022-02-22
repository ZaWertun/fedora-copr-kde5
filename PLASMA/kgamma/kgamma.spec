%global base_name kgamma5

Name:    kgamma
Summary: A monitor calibration tool
Epoch:   1
Version: 5.24.2
Release: 1%{?dist}

License: GPLv2
URL:     https://cgit.kde.org/%{base_name}.git

%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0: http://download.kde.org/%{stable}/plasma/%{version}/%{base_name}-%{version}.tar.xz

BuildRequires: gcc gcc-c++
BuildRequires: desktop-file-utils
BuildRequires: gettext
BuildRequires: extra-cmake-modules
BuildRequires: kf5-rpm-macros
BuildRequires: kf5-kdelibs4support-devel
BuildRequires: kf5-kdoctools-devel
BuildRequires: kf5-ki18n-devel
BuildRequires: pkgconfig(xxf86vm)
BuildRequires: pkgconfig(Qt5Gui) pkgconfig(Qt5Widgets)
BuildRequires: pkgconfig(Qt5X11Extras)

Provides: %{base_name} = %{version}-%{release}

# when split occurred
Conflicts: kdegraphics < 7:4.6.95-10


%description
%{summary}.


%prep
%autosetup -n %{base_name}-%{version}


%build
%cmake_kf5
%cmake_build


%install
%cmake_install
%find_lang kcmkgamma --all-name --with-html


%files -f kcmkgamma.lang
%doc ChangeLog
%license LICENSES/*
%{_kf5_datadir}/kgamma/
%{_kf5_qtplugindir}/plasma/kcminit/kcm_kgamma_init.so
%{_kf5_qtplugindir}/plasma/kcms/systemsettings/kcm_kgamma.so


%changelog
* Tue Feb 22 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 1:5.24.2-1
- 5.24.2

* Tue Feb 15 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 1:5.24.1-1
- 5.24.1

* Tue Feb 15 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 1:5.24.0-1
- new version

* Tue Jan 04 2022 Marc Deop <marcdeop@fedoraproject.org> - 1:5.23.5-1
- 5.23.5

* Tue Dec 14 2021 Marc Deop <marcdeop@fedoraproject.org> - 1:5.23.4-1
- 5.23.4

* Wed Nov 10 2021 Rex Dieter <rdieter@fedoraproject.org> - 1:5.23.3-1
- 5.23.3

* Tue Oct 26 2021 Rex Dieter <rdieter@fedoraproject.org> - 1:5.23.2-1
- 5.23.2

* Sat Oct 23 2021 Marc Deop <marcdeop@fedoraproject.org> - 1:5.23.1-1
- 5.23.1

* Fri Oct 08 2021 Marc Deop <marcdeop@fedoraproject.org> - 1:5.23.0-1
- 5.23.0

* Fri Sep 17 2021 Marc Deop <marcdeop@fedoraproject.org> - 1:5.22.90-1
- 5.22.90

* Tue Aug 31 2021 Jan Grulich <jgrulich@redhat.com> - 1:5.22.5-1
- 5.22.5

* Tue Jul 27 2021 Jan Grulich <jgrulich@redhat.com> - 1:5.22.4-1
- 5.22.4

* Thu Jul 22 2021 Fedora Release Engineering <releng@fedoraproject.org> - 1:5.22.3-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_35_Mass_Rebuild

* Mon Jul 12 2021 Jan Grulich <jgrulich@redhat.com> - 1:5.22.3-1
- 5.22.3

* Tue Jun 22 2021 Jan Grulich <jgrulich@redhat.com> - 1:5.22.2.1-1
- 5.22.2.1

* Tue Jun 22 2021 Jan Grulich <jgrulich@redhat.com> - 1:5.22.2-1
- 5.22.2

* Tue Jun 15 2021 Jan Grulich <jgrulich@redhat.com> - 1:5.22.1-1
- 5.22.1

* Sun Jun 06 2021 Jan Grulich <jgrulich@redhat.com> - 1:5.22.0-1
- 5.22.0

* Thu May 13 2021 Rex Dieter <rdieter@fedoraproject.org> - 1:5.21.90-1
- 5.21.90

* Tue May 04 2021 Jan Grulich <jgrulich@redhat.com> - 1:5.21.5-1
- 5.21.5

* Tue Apr 06 2021 Jan Grulich <jgrulich@redhat.com> - 1:5.21.4-1
- 5.21.4

* Tue Mar 16 2021 Jan Grulich <jgrulich@redhat.com> - 1:5.21.3-1
- 5.21.3

* Tue Mar 02 2021 Jan Grulich <jgrulich@redhat.com> - 1:5.21.2-1
- 5.21.2

* Tue Feb 23 2021 Jan Grulich <jgrulich@redhat.com> - 1:5.21.1-1
- 5.21.1

* Thu Feb 11 2021 Jan Grulich <jgrulich@redhat.com> - 1:5.21.0-1
- 5.21.0

* Tue Jan 26 2021 Fedora Release Engineering <releng@fedoraproject.org> - 1:5.20.90-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_34_Mass_Rebuild

* Thu Jan 21 2021 Jan Grulich <jgrulich@redhat.com> - 1:5.20.90-1
- 5.20.90 (beta)

* Tue Jan  5 16:03:30 CET 2021 Jan Grulich <jgrulich@redhat.com> - 1:5.20.5-1
- 5.20.5

* Tue Dec  1 09:42:57 CET 2020 Jan Grulich <jgrulich@redhat.com> - 1:5.20.4-1
- 5.20.4

* Wed Nov 11 08:22:39 CET 2020 Jan Grulich <jgrulich@redhat.com> - 1:5.20.3-1
- 5.20.3

* Tue Oct 27 14:22:21 CET 2020 Jan Grulich <jgrulich@redhat.com> - 1:5.20.2-1
- 5.20.2

* Tue Oct 20 15:28:17 CEST 2020 Jan Grulich <jgrulich@redhat.com> - 1:5.20.1-1
- 5.20.1

* Sun Oct 11 19:50:02 CEST 2020 Jan Grulich <jgrulich@redhat.com> - 1:5.20.0-1
- 5.20.0

* Fri Sep 18 2020 Jan Grulich <jgrulich@redhat.com> - 1:5.19.90-1
- 5.19.90

* Tue Sep 01 2020 Jan Grulich <jgrulich@redhat.com> - 1:5.19.5-1
- 5.19.5

* Tue Jul 28 2020 Jan Grulich <jgrulich@redhat.com> - 1:5.19.4-1
- 5.19.4

* Tue Jul 28 2020 Fedora Release Engineering <releng@fedoraproject.org> - 1:5.19.3-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_33_Mass_Rebuild

* Tue Jul 07 2020 Jan Grulich <jgrulich@redhat.com> - 1:5.19.3-1
- 5.19.3

* Tue Jun 23 2020 Jan Grulich <jgrulich@redhat.com> - 1:5.19.2-1
- 5.19.2

* Wed Jun 17 2020 Martin Kyral <martin.kyral@gmail.com> - 5.19.1-1
- 5.19.1

* Tue Jun 9 2020 Martin Kyral <martin.kyral@gmail.com> - 5.19.0-1
- 5.19.0

* Fri May 15 2020 Martin Kyral <martin.kyral@gmail.com> - 5.18.90-1
- 5.18.90

* Tue May 05 2020 Jan Grulich <jgrulich@redhat.com> - 1:5.18.5-1
- 5.18.5

* Sat Apr 04 2020 Rex Dieter <rdieter@fedoraproject.org> - 1:5.18.4.1-1
- 5.18.4.1

* Tue Mar 31 2020 Jan Grulich <jgrulich@redhat.com> - 1:5.18.4-1
- 5.18.4

* Tue Mar 10 2020 Jan Grulich <jgrulich@redhat.com> - 1:5.18.3-1
- 5.18.3

* Tue Feb 25 2020 Jan Grulich <jgrulich@redhat.com> - 1:5.18.2-1
- 5.18.2

* Tue Feb 18 2020 Jan Grulich <jgrulich@redhat.com> - 1:5.18.1-1
- 5.18.1

* Tue Feb 11 2020 Jan Grulich <jgrulich@redhat.com> - 1:5.18.0-1
- 5.18.0

* Wed Jan 29 2020 Fedora Release Engineering <releng@fedoraproject.org> - 1:5.17.90-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_32_Mass_Rebuild

* Thu Jan 16 2020 Jan Grulich <jgrulich@redhat.com> - 1:5.17.90-1
- 5.17.90

* Wed Jan 08 2020 Jan Grulich <jgrulich@redhat.com> - 1:5.17.5-1
- 5.17.5

* Thu Dec 05 2019 Jan Grulich <jgrulich@redhat.com> - 1:5.17.4-1
- 5.17.4

* Wed Nov 13 2019 Martin Kyral <martin.kyral@gmail.com> - 5.17.3-1
- 5.17.3

* Wed Oct 30 2019 Jan Grulich <jgrulich@redhat.com> - 1:5.17.2-1
- 5.17.2

* Wed Oct 23 2019 Jan Grulich <jgrulich@redhat.com> - 1:5.17.1-1
- 5.17.1

* Thu Oct 10 2019 Jan Grulich <jgrulich@redhat.com> - 1:5.17.0-1
- 5.17.0

* Fri Sep 20 2019 Martin Kyral <martin.kyral@gmail.com> - 5.16.90-1
- 5.16.90

* Fri Sep 06 2019 Martin Kyral <martin.kyral@gmail.com> - 5.16.5-1
- 5.16.5

* Tue Jul 30 2019 Martin Kyral <martin.kyral@gmail.com> - 5.16.4-1
- 5.16.4

* Thu Jul 25 2019 Fedora Release Engineering <releng@fedoraproject.org> - 1:5.16.3-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_31_Mass_Rebuild

* Wed Jul 10 2019 Martin Kyral <martin.kyral@gmail.com> - 5.16.3-1
- 5.16.3

* Wed Jun 26 2019 Martin Kyral <martin.kyral@gmail.com> - 5.16.2-1
- 5.16.2

* Tue Jun 18 2019 Rex Dieter <rdieter@fedoraproject.org> - 1:5.16.1-1
- 5.16.1

* Tue Jun 11 2019 Martin Kyral <martin.kyral@gmail.com> - 5.16.0-1
- 5.16.0

* Thu May 16 2019 Martin Kyral <martin.kyral@gmail.com> - 5.15.90-1
- 5.15.90

* Thu May 09 2019 Martin Kyral <martin.kyral@gmail.com> - 5.15.5-1
- 5.15.5

* Wed Apr 03 2019 Rex Dieter <rdieter@fedoraproject.org> - 1:5.15.4-1
- 5.15.4

* Tue Mar 12 2019 Martin Kyral <martin.kyral@gmail.com> - 5.15.3-1
- 5.15.3

* Tue Feb 26 2019 Rex Dieter <rdieter@fedoraproject.org> - 1:5.15.2-1
- 5.15.2

* Tue Feb 19 2019 Rex Dieter <rdieter@fedoraproject.org> - 1:5.15.1-1
- 5.15.1

* Wed Feb 13 2019 Martin Kyral <martin.kyral@gmail.com> - 5.15.0-1
- 5.15.0

* Fri Feb 01 2019 Fedora Release Engineering <releng@fedoraproject.org> - 1:5.14.90-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_30_Mass_Rebuild

* Sun Jan 20 2019 Martin Kyral <martin.kyral@gmail.com> - 5.14.90-1
- 5.14.90

* Tue Nov 27 2018 Rex Dieter <rdieter@fedoraproject.org> - 1:5.14.4-1
- 5.14.4

* Thu Nov 08 2018 Martin Kyral <martin.kyral@gmail.com> - 5.14.3-1
- 5.14.3

* Wed Oct 24 2018 Rex Dieter <rdieter@fedoraproject.org> - 1:5.14.2-1
- 5.14.2

* Tue Oct 16 2018 Rex Dieter <rdieter@fedoraproject.org> - 1:5.14.1-1
- 5.14.1

* Fri Oct 05 2018 Rex Dieter <rdieter@fedoraproject.org> - 1:5.14.0-1
- 5.14.0

* Fri Sep 14 2018 Martin Kyral <martin.kyral@gmail.com> - 5.13.90-1
- 5.13.90

* Tue Sep 04 2018 Rex Dieter <rdieter@fedoraproject.org> - 1:5.13.5-1
- 5.13.5

* Thu Aug 02 2018 Rex Dieter <rdieter@fedoraproject.org> - 1:5.13.4-1
- 5.13.4

* Mon Jul 23 2018 Than Ngo <than@redhat.com> - 5.13.3-3
- fixed FTBFS

* Fri Jul 13 2018 Fedora Release Engineering <releng@fedoraproject.org> - 1:5.13.3-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_29_Mass_Rebuild

* Wed Jul 11 2018 Martin Kyral <martin.kyral@gmail.com> - 5.13.3-1
- 5.13.3

* Mon Jul 09 2018 Martin Kyral <martin.kyral@gmail.com> - 5.13.2-1
- 5.13.2

* Tue Jun 19 2018 Martin Kyral <martin.kyral@gmail.com> - 5.13.1-1
- 5.13.1

* Sat Jun 09 2018 Rex Dieter <rdieter@fedoraproject.org> - 1:5.13.0-1
- 5.13.0

* Fri May 18 2018 Martin Kyral <martin.kyral@gmail.com> - 5.12.90-1
- 5.12.90

* Tue May 01 2018 Rex Dieter <rdieter@fedoraproject.org> - 1:5.12.5-1
- 5.12.5

* Tue Mar 27 2018 Rex Dieter <rdieter@fedoraproject.org> - 1:5.12.4-1
- 5.12.4

* Tue Mar 06 2018 Rex Dieter <rdieter@fedoraproject.org> - 1:5.12.3-1
- 5.12.3

* Wed Feb 21 2018 Jan Grulich <jgrulich@redhat.com> - 5.12.2-1
- 5.12.2

* Tue Feb 13 2018 Jan Grulich <jgrulich@redhat.com> - 5.12.1-1
- 5.12.1

* Wed Feb 07 2018 Fedora Release Engineering <releng@fedoraproject.org> - 1:5.12.0-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_28_Mass_Rebuild

* Fri Feb 02 2018 Jan Grulich <jgrulich@redhat.com> - 5.12.0-1
- 5.12.0

* Mon Jan 15 2018 Jan Grulich <jgrulich@redhat.com> - 5.11.95-1
- 5.11.95

* Tue Jan 02 2018 Rex Dieter <rdieter@fedoraproject.org> - 1:5.11.5-1
- 5.11.5

* Thu Nov 30 2017 Martin Kyral <martin.kyral@gmail.com> - 5.11.4-1
- 5.11.4

* Wed Nov 08 2017 Rex Dieter <rdieter@fedoraproject.org> - 1:5.11.3-1
- 5.11.3

* Wed Oct 25 2017 Martin Kyral <martin.kyral@gmail.com> - 5.11.2-1
- 5.11.2

* Tue Oct 17 2017 Rex Dieter <rdieter@fedoraproject.org> - 1:5.11.1-1
- 5.11.1

* Wed Oct 11 2017 Martin Kyral <martin.kyral@gmail.com> - 5.11.0-1
- 5.11.0

* Thu Aug 24 2017 Rex Dieter <rdieter@fedoraproject.org> - 1:5.10.5-1
- 5.10.5

* Thu Aug 03 2017 Fedora Release Engineering <releng@fedoraproject.org> - 1:5.10.4-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Binutils_Mass_Rebuild

* Wed Jul 26 2017 Fedora Release Engineering <releng@fedoraproject.org> - 1:5.10.4-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Mass_Rebuild

* Fri Jul 21 2017 Rex Dieter <rdieter@fedoraproject.org> - 1:5.10.4-1
- 5.10.4

* Tue Jun 27 2017 Rex Dieter <rdieter@fedoraproject.org> - 1:5.10.3-1
- 5.10.3

* Thu Jun 15 2017 Rex Dieter <rdieter@fedoraproject.org> - 1:5.10.2-1
- 5.10.2

* Tue Jun 06 2017 Rex Dieter <rdieter@fedoraproject.org> - 1:5.10.1-1
- 5.10.1

* Wed May 31 2017 Jan Grulich <jgrulich@redhat.com> - 5.10.0-1
- 5.10.0

* Wed Apr 26 2017 Rex Dieter <rdieter@fedoraproject.org> - 1:5.9.5-2
- use %%find_lang --with-html

* Wed Apr 26 2017 Rex Dieter <rdieter@fedoraproject.org> - 1:5.9.5-1
- 5.9.5

* Thu Mar 23 2017 Rex Dieter <rdieter@fedoraproject.org> - 1:5.9.4-1
- 5.9.4

* Sat Mar 04 2017 Rex Dieter <rdieter@fedoraproject.org> - 1:5.9.3-2
- rebuild

* Wed Mar 01 2017 Jan Grulich <jgrulich@redhat.com> - 5.9.3-1
- 5.9.3

* Tue Feb 21 2017 Rex Dieter <rdieter@fedoraproject.org> - 1:5.8.6-1
- 5.8.6

* Fri Feb 10 2017 Fedora Release Engineering <releng@fedoraproject.org> - 1:5.8.5-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_26_Mass_Rebuild

* Wed Dec 28 2016 Rex Dieter <rdieter@fedoraproject.org> - 1:5.8.5-1
- 5.8.5

* Tue Nov 22 2016 Rex Dieter <rdieter@fedoraproject.org> - 1:5.8.4-1
- 5.8.4

* Tue Nov 01 2016 Rex Dieter <rdieter@fedoraproject.org> - 1:5.8.3-1
- 5.8.3

* Tue Oct 18 2016 Rex Dieter <rdieter@fedoraproject.org> - 1:5.8.2-1
- 5.8.2

* Tue Oct 11 2016 Rex Dieter <rdieter@fedoraproject.org> - 1:5.8.1-1
- 5.8.1

* Thu Sep 29 2016 Rex Dieter <rdieter@fedoraproject.org> - 1:5.8.0-1
- 5.8.0

* Thu Sep 22 2016 Rex Dieter <rdieter@fedoraproject.org> - 1:5.7.95-1
- 5.7.95

* Tue Sep 13 2016 Rex Dieter <rdieter@fedoraproject.org> - 1:5.7.5-1
- 5.7.5

* Tue Aug 23 2016 Rex Dieter <rdieter@fedoraproject.org> - 1:5.7.4-1
- 5.7.4

* Tue Aug 02 2016 Rex Dieter <rdieter@fedoraproject.org> - 1:5.7.3-1
- 5.7.3

* Tue Jul 19 2016 Rex Dieter <rdieter@fedoraproject.org> - 1:5.7.2-1
- 5.7.2

* Tue Jul 12 2016 Rex Dieter <rdieter@fedoraproject.org> - 1:5.7.1-1
- 5.7.1

* Thu Jun 30 2016 Rex Dieter <rdieter@fedoraproject.org> - 1:5.7.0-1
- 5.7.0

* Sat Jun 25 2016 Rex Dieter <rdieter@fedoraproject.org> - 1:5.6.95-1
- 5.6.95

* Tue Jun 14 2016 Rex Dieter <rdieter@fedoraproject.org> - 1:5.6.5-1
- 5.6.5

* Sat May 14 2016 Rex Dieter <rdieter@fedoraproject.org> - 1:5.6.4-1
- 5.6.4

* Tue Apr 19 2016 Rex Dieter <rdieter@fedoraproject.org> - 1:5.6.3-1
- 5.6.3

* Sat Apr 09 2016 Rex Dieter <rdieter@fedoraproject.org> - 1:5.6.2-1
- 5.6.2

* Fri Apr 08 2016 Rex Dieter <rdieter@fedoraproject.org> - 1:5.6.1-1
- 5.6.1

* Tue Mar 01 2016 Daniel Vrátil <dvratil@fedoraproject.org> - 5.5.5-1
- Plasma 5.5.5

* Thu Feb 04 2016 Fedora Release Engineering <releng@fedoraproject.org> - 1:5.5.4-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_24_Mass_Rebuild

* Wed Jan 27 2016 Daniel Vrátil <dvratil@fedoraproject.org> - 5.5.4-1
- Plasma 5.5.4

* Thu Jan 07 2016 Daniel Vrátil <dvratil@fedoraproject.org> - 5.5.3-1
- Plasma 5.5.3

* Thu Jan 07 2016 Daniel Vrátil <dvratil@fedoraproject.org> - 5.5.3-1
- Plasma 5.5.3

* Thu Dec 31 2015 Rex Dieter <rdieter@fedoraproject.org> - 1:5.5.2-1
- 5.5.2

* Fri Dec 18 2015 Daniel Vrátil <dvratil@fedoraproject.org> - 5.5.1-1
- Plasma 5.5.1

* Thu Dec 03 2015 Daniel Vrátil <dvratil@fedoraproject.org> - 5.5.0-1
- Plasma 5.5.0

* Wed Nov 25 2015 Daniel Vrátil <dvratil@fedoraproject.org> - 5.4.95-1
- Plasma 5.4.95

* Thu Nov 05 2015 Daniel Vrátil <dvratil@fedoraproject.org> - 5.4.3-1
- Plasma 5.4.3

* Thu Oct 01 2015 Rex Dieter <rdieter@fedoraproject.org> - 1:5.4.2-1
- 5.4.2

* Thu Sep 10 2015 Rex Dieter <rdieter@fedoraproject.org> 1:5.4.1-1
- kgamma5-5.4.1

* Wed Jun 17 2015 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 15.04.2-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_23_Mass_Rebuild

* Wed Jun 10 2015 Rex Dieter <rdieter@fedoraproject.org> - 15.04.2-1
- 15.04.2

* Thu May 28 2015 Rex Dieter <rdieter@fedoraproject.org> - 15.04.1-1
- 15.04.1

* Fri Apr 17 2015 Rex Dieter <rdieter@fedoraproject.org> - 15.04.0-1
- 15.04.0

* Sun Mar 01 2015 Rex Dieter <rdieter@fedoraproject.org> - 14.12.3-1
- 14.12.3

* Tue Feb 24 2015 Than Ngo <than@redhat.com> - 14.12.2-1
- 14.12.2

* Sat Jan 17 2015 Rex Dieter <rdieter@fedoraproject.org> - 14.12.1-1
- 14.12.1

* Sun Nov 09 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.14.3-1
- 4.14.3

* Sun Oct 12 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.14.2-1
- 4.14.2

* Tue Sep 16 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.14.1-1
- 4.14.1

* Sun Aug 17 2014 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 4.14.0-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_21_22_Mass_Rebuild

* Fri Aug 15 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.14.0-1
- 4.14.0

* Tue Aug 05 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.13.97-1
- 4.13.97

* Tue Jul 15 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.13.3-1
- 4.13.3

* Mon Jun 09 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.13.2-1
- 4.13.2

* Sun Jun 08 2014 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 4.13.1-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_21_Mass_Rebuild

* Sun May 11 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.13.1-1
- 4.13.1

* Sat Apr 12 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.13.0-1
- 4.13.0

* Fri Apr 04 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.12.97-1
- 4.12.97

* Sun Mar 23 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.12.95-1
- 4.12.95

* Wed Mar 19 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.12.90-1
- 4.12.90

* Sun Mar 02 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.12.3-1
- 4.12.3

* Fri Jan 31 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.12.2-1
- 4.12.2

* Fri Jan 10 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.12.1-1
- 4.12.1

* Thu Dec 19 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.12.0-1
- 4.12.0

* Sun Dec 01 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.11.97-1
- 4.11.97

* Thu Nov 21 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.11.95-1
- 4.11.95

* Sat Nov 16 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.11.90-1
- 4.11.90

* Sat Nov 02 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.11.3-1
- 4.11.3

* Sat Sep 28 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.11.2-1
- 4.11.2

* Wed Sep 04 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.11.1-1
- 4.11.1

* Thu Aug 08 2013 Than Ngo <than@redhat.com> - 4.11.0-1
- 4.11.0

* Thu Jul 25 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.10.97-1
- 4.10.97

* Tue Jul 23 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.10.95-1
- 4.10.95

* Fri Jun 28 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.10.90-1
- 4.10.90

* Sat Jun 01 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.10.4-1
- 4.10.4

* Mon May 06 2013 Than Ngo <than@redhat.com> - 4.10.3-1
- 4.10.3

* Thu Apr 25 2013 Rex Dieter <rdieter@fedoraproject.org> 4.10.2-2
- Requires: kde-runtime

* Mon Apr 01 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.10.2-1
- 4.10.2

* Sat Mar 02 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.10.1-1
- 4.10.1

* Fri Feb 01 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.10.0-1
- 4.10.0

* Tue Jan 22 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.9.98-1
- 4.9.98

* Fri Jan 04 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.9.97-1
- 4.9.97

* Thu Dec 20 2012 Rex Dieter <rdieter@fedoraproject.org> - 4.9.95-1
- 4.9.95

* Tue Dec 04 2012 Rex Dieter <rdieter@fedoraproject.org> - 4.9.90-1
- 4.9.90

* Mon Dec 03 2012 Than Ngo <than@redhat.com> - 4.9.4-1
- 4.9.4

* Sat Nov 03 2012 Rex Dieter <rdieter@fedoraproject.org> - 4.9.3-1
- 4.9.3

* Sat Sep 29 2012 Rex Dieter <rdieter@fedoraproject.org> - 4.9.2-1
- 4.9.2

* Mon Sep 03 2012 Than Ngo <than@redhat.com> - 4.9.1-1
- 4.9.1

* Thu Jul 26 2012 Lukas Tinkl <ltinkl@redhat.com> - 4.9.0-1
- 4.9.0

* Thu Jul 19 2012 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 4.8.97-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_18_Mass_Rebuild

* Thu Jul 12 2012 Rex Dieter <rdieter@fedoraproject.org> - 4.8.97-1
- 4.8.97

* Thu Jun 28 2012 Rex Dieter <rdieter@fedoraproject.org> - 4.8.95-1
- 4.8.95

* Sun Jun 10 2012 Rex Dieter <rdieter@fedoraproject.org> - 4.8.90-1
- 4.8.90

* Sun Jun 03 2012 Jaroslav Reznik <jreznik@redhat.com> - 4.8.80-1
- 4.8.80

* Mon Apr 30 2012 Jaroslav Reznik <jreznik@redhat.com> - 4.8.3-1
- 4.8.3

* Fri Mar 30 2012 Rex Dieter <rdieter@fedoraproject.org> - 4.8.2-1
- 4.8.2

* Mon Mar 05 2012 Jaroslav Reznik <jreznik@redhat.com> - 4.8.1-1
- 4.8.1

* Sun Jan 22 2012 Rex Dieter <rdieter@fedoraproject.org> - 4.8.0-1
- 4.8.0

* Wed Jan 04 2012 Radek Novacek <rnovacek@redhat.com> - 4.7.97-1
- 4.7.97

* Wed Dec 21 2011 Radek Novacek <rnovacek@redhat.com> - 4.7.95-1
- 4.7.95

* Sun Dec 04 2011 Rex Dieter <rdieter@fedoraproject.org> - 4.7.90-1
- 4.7.90

* Fri Nov 25 2011 Rex Dieter <rdieter@fedoraproject.org> 4.7.80-1
- 4.7.80

* Sat Oct 29 2011 Rex Dieter <rdieter@fedoraproject.org> 4.7.3-1
- 4.7.3

* Wed Oct 05 2011 Rex Dieter <rdieter@fedoraproject.org> 4.7.2-1
- 4.7.2

* Wed Sep 07 2011 Than Ngo <than@redhat.com> - 4.7.1-1
- 4.7.1

* Tue Jul 26 2011 Jaroslav Reznik <jreznik@redhat.com> 4.7.0-1
- 4.7.0

* Mon Jul 11 2011 Rex Dieter <rdieter@fedoraproject.org> 4.6.95-1
- 4.6.95
- fix URL

* Wed Jul 06 2011 Rex Dieter <rdieter@fedoraproject.org> 4.6.90-2
- fix Source0 URL
- Conflicts: kdegraphics < 7:4.6.90-10

* Tue Jul 05 2011 Rex Dieter <rdieter@fedoraproject.org>  4.6.90-1
- first try
