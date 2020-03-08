
Name:    baloo-widgets
Summary: Widgets for Baloo
Version: 19.12.3
Release: 1%{?dist}

# # KDE e.V. may determine that future LGPL versions are accepted
License: LGPLv2 or LGPLv3
URL:     https://cgit.kde.org/%{name}.git/

%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0: https://download.kde.org/%{stable}/release-service/%{version}/src/%{name}-%{version}.tar.xz

# translations moved here
Conflicts: kde-l10n < 17.04.0-2

Provides: kf5-baloo-widgets = %{version}-%{release}
Provides: kf5-baloo-widgets%{?_isa} = %{version}-%{release}

BuildRequires:  cmake(Qt5Widgets)
BuildRequires:  cmake(Qt5Test)

BuildRequires:  kf5-rpm-macros
BuildRequires:  extra-cmake-modules >= 5.19
BuildRequires:  cmake(KF5Config)
BuildRequires:  cmake(KF5KIO)
BuildRequires:  cmake(KF5I18n)
BuildRequires:  cmake(KF5FileMetaData)
BuildRequires:  cmake(KF5Baloo)

%description
%{summary}.

%package devel
Summary:  Developer files for %{name}
Provides: kf5-baloo-widgets-devel = %{version}-%{release}
Provides: kf5-baloo-widgets-devel%{?_isa} = %{version}-%{release}
Requires: %{name}%{?_isa} = %{version}-%{release}
Requires: qt5-qtbase-devel
Requires: kf5-kcoreaddons-devel
Requires: kf5-kio-devel
%description devel
%{summary}.


%prep
%autosetup


%build
mkdir %{_target_platform}
pushd %{_target_platform}
%{cmake_kf5} ..
popd

%make_build -C %{_target_platform}


%install
make install/fast DESTDIR=%{buildroot} -C %{_target_platform}

%find_lang %{name} --all-name


%ldconfig_scriptlets

%files -f %{name}.lang
%doc COPYING.LIB
%{_kf5_libdir}/libKF5BalooWidgets.so.*
%{_kf5_bindir}/baloo_filemetadata_temp_extractor
%{_kf5_qtplugindir}/baloofilepropertiesplugin.so
%{_kf5_qtplugindir}/kf5/kfileitemaction/tagsfileitemaction.so
%{_kf5_datadir}/kservices5/baloofilepropertiesplugin.desktop

%files devel
%{_kf5_libdir}/cmake/KF5BalooWidgets/
%{_kf5_includedir}/BalooWidgets/
%{_kf5_libdir}/libKF5BalooWidgets.so


%changelog
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

* Thu Jan 31 2019 Fedora Release Engineering <releng@fedoraproject.org> - 18.12.1-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_30_Mass_Rebuild

* Tue Jan 08 2019 Rex Dieter <rdieter@fedoraproject.org> - 18.12.1-1
- 18.12.1

* Sat Dec 08 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.12.0-1
- 18.12.0

* Tue Nov 06 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.08.3-1
- 18.08.3

* Wed Oct 10 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.08.2-1
- 18.08.2

* Fri Sep 07 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.08.1-1
- 18.08.1

* Wed Aug 15 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.08.0-1
- 18.08.0

* Thu Jul 12 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.04.3-1
- 18.04.3

* Tue Jun 05 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.04.2-1
- 18.04.2

* Tue May 08 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.04.1-1
- 18.04.1

* Sat Apr 14 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.04.0-1
- 18.04.0

* Tue Mar 06 2018 Rex Dieter <rdieter@fedoraproject.org> - 17.12.3-1
- 17.12.3

* Tue Feb 06 2018 Rex Dieter <rdieter@fedoraproject.org> - 17.12.2-1
- 17.12.2

* Thu Jan 11 2018 Rex Dieter <rdieter@fedoraproject.org> - 17.12.1-1
- 17.12.1

* Tue Dec 12 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.12.0-1
- 17.12.0

* Wed Nov 08 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.08.3-1
- 17.08.3

* Wed Oct 11 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.08.2-1
- 17.08.2

* Tue Sep 05 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.08.1-1
- 17.08.1

* Sat Aug 26 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.08.0-1
- 17.08.0

* Sun Aug 06 2017 Björn Esser <besser82@fedoraproject.org> - 17.04.3-2
- Rebuilt for AutoReq cmake-filesystem

* Fri Jul 28 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.04.3-1
- 17.04.3

* Wed Jul 26 2017 Fedora Release Engineering <releng@fedoraproject.org> - 17.04.2-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Mass_Rebuild

* Thu Jun 15 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.04.2-1
- 17.04.2

* Wed May 10 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.04.1-1
- 17.04.1

* Sun Apr 23 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.04.0-2
- Conflicts: kde-l10n < 17.04.0-2

* Fri Apr 14 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.04.0-1
- 17.04.0, cmake-style deps, includes translations

* Wed Mar 08 2017 Rex Dieter <rdieter@fedoraproject.org> - 16.12.3-1
- 16.12.3

* Wed Feb 08 2017 Rex Dieter <rdieter@fedoraproject.org> - 16.12.2-1
- 16.12.2

* Tue Jan 10 2017 Rex Dieter <rdieter@fedoraproject.org> - 16.12.1-1
- 16.12.1, update URL

* Mon Dec 26 2016 Rex Dieter <rdieter@math.unl.edu> - 16.12.0-1
- 16.12.0

* Wed Nov 30 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.08.3-1
- 16.08.3

* Thu Oct 13 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.08.2-1
- 16.08.2

* Tue Sep 06 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.08.1-1
- 16.08.1

* Fri Aug 12 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.08.0-1
- 16.08.0

* Sat Aug 06 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.07.90-1
- 16.07.90

* Fri Jul 29 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.07.80-1
- 16.07.80

* Fri Jul 08 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.04.3-1
- 16.04.3

* Sun Jun 12 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.04.2-1
- 16.04.2

* Sun May 08 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.04.1-1
- 16.04.1

* Mon Apr 18 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.04.0-1
- 16.04.0

* Sat Mar 12 2016 Rex Dieter <rdieter@fedoraproject.org> - 15.12.3-1
- 15.12.3

* Sun Feb 14 2016 Rex Dieter <rdieter@fedoraproject.org> - 15.12.2-1
- 15.12.2

* Wed Feb 03 2016 Fedora Release Engineering <releng@fedoraproject.org> - 15.12.1-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_24_Mass_Rebuild

* Mon Jan 18 2016 Rex Dieter <rdieter@fedoraproject.org> - 15.12.1-1
- 15.12.1

* Sun Dec 20 2015 Rex Dieter <rdieter@fedoraproject.org> 15.12.0-1
- 15.12.0

* Wed Dec 02 2015 Rex Dieter <rdieter@fedoraproject.org> - 15.08.3-1
- 15.08.3

* Tue Sep 15 2015 Rex Dieter <rdieter@fedoraproject.org> - 15.08.1-1
- 15.08.1

* Tue Sep 01 2015 Rex Dieter <rdieter@fedoraproject.org> 15.08.0-3
- Provides: kf5-baloo-widgets(-devel)

* Tue Sep 01 2015 Rex Dieter <rdieter@fedoraproject.org> 15.08.0-2
- fix -devel deps

* Thu Aug 20 2015 Than Ngo <than@redhat.com> - 15.08.0-1
- 15.08.0

* Wed Jun 17 2015 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 4.14.3-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_23_Mass_Rebuild

* Thu Apr 16 2015 Rex Dieter <rdieter@fedoraproject.org> 4.14.3-2
- rebuild (gcc5)

* Sat Nov 08 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.14.3-1
- 4.14.3

* Sat Oct 11 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.14.2-1
- 4.14.2

* Mon Sep 15 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.14.1-1
- 4.14.1

* Fri Aug 15 2014 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 4.14.0-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_21_22_Mass_Rebuild

* Thu Aug 14 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.14.0-1
- 4.14.0

* Tue Aug 05 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.13.97-1
- 4.13.97

* Mon Jul 14 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.13.3-1
- 4.13.3

* Mon Jun 09 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.13.2-1
- 4.13.2

* Sat Jun 07 2014 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 4.13.1-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_21_Mass_Rebuild

* Sat May 10 2014 Rex Dieter <rdieter@fedoraproject.org> 4.13.1-1
- 4.13.1

* Sat Apr 12 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.13.0-1
- 4.13.0

* Thu Apr 03 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.12.97-1
- 4.12.97

* Sat Mar 22 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.12.95-1
- 4.12.95

* Mon Mar 17 2014 Rex Dieter <rdieter@fedoraproject.org> 4.12.90-1
- baloo-widgets-4.12.90, first try

