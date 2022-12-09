%global framework kitinerary

# uncomment to enable bootstrap mode
#global bootstrap 1

%if !0%{?bootstrap}
%global tests 1
%endif

Name:    kf5-%{framework}
Version: 22.12.0
Release: 1%{?dist}
Summary: A library containing itinerary data model and itinerary extraction code

License: LGPLv2+
URL:     https://cgit.kde.org/%{framework}.git

%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0:        https://download.kde.org/%{stable}/release-service/%{version}/src/%{framework}-%{version}.tar.xz
Source1:        https://download.kde.org/%{stable}/release-service/%{version}/src/%{framework}-%{version}.tar.xz.sig
Source2:        gpgkey-D81C0CB38EB725EF6691C385BB463350D6EF31EF.gpg

BuildRequires:  gnupg2
BuildRequires:  extra-cmake-modules
BuildRequires:  kf5-rpm-macros
BuildRequires:  shared-mime-info
BuildRequires:  cmake(KF5I18n)

BuildRequires:  qt5-qtbase-private-devel

# kde-pim pkgs
#global majmin_ver %(echo %{version} | cut -d. -f1,2)
%global majmin_ver %{version}
BuildRequires: kf5-kmime-devel >= %{majmin_ver}
BuildRequires: kf5-kcalendarcore-devel >= %{majmin_ver}
BuildRequires: kf5-kcontacts-devel >= %{majmin_ver}
BuildRequires: kf5-kpkpass-devel >= %{majmin_ver}

# kde-pim cmake
BuildRequires:  cmake(KF5Mime)
BuildRequires:  cmake(KF5CalendarCore)
BuildRequires:  cmake(KF5Contacts)
BuildRequires:  cmake(KPimPkPass)
BuildRequires:  cmake(KF5CalendarCore)

BuildRequires:  pkgconfig(Qt5Gui)
BuildRequires:  pkgconfig(Qt5Qml)

BuildRequires:  pkgconfig(libxml-2.0)
BuildRequires:  pkgconfig(poppler)
BuildRequires:  pkgconfig(zlib)
BuildRequires:  pkgconfig(openssl) >= 1.1
BuildRequires:  pkgconfig(protobuf)

BuildRequires:  libphonenumber-devel
# TODO: https://github.com/glassechidna/zxing-cpp
#BuildRequires:  zxing-cpp-devel

%if 0%{?tests}
BuildRequires: dbus-x11
BuildRequires: xorg-x11-server-Xvfb
%endif

%description
%{summary}

%package        devel
Summary:        Development files for %{name}
Requires:       %{name}%{?_isa} = %{version}-%{release}
%description    devel
The %{name}-devel package contains libraries and header files for
developing applications that use %{name}.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -n %{framework}-%{version} -p1


%build
%if 0%{fedora} == 29
# popplerutils.cpp:88:47: error: passing 'const GfxPath' as 'this' argument discards qualifiers [-fpermissive]
%global optflags %(echo %{optflags} -fpermissive)
%endif

%cmake_kf5 \
  -DBUILD_TESTING:BOOL=%{?tests:ON}%{!?tests:OFF}
%cmake_build


%install
%cmake_install

%find_lang %{name} --all-name


%check
%if 0%{?tests}
export CTEST_OUTPUT_ON_FAILURE=1
#xvfb-run -a \
#dbus-launch --exit-with-session \
make test/fast ARGS="--output-on-failure --timeout 10" -C %{_target_platform} ||:
%endif


%ldconfig_scriptlets

%files -f %{name}.lang
%doc README.md
%license LICENSES/*.txt
%{_kf5_libdir}/libKPimItinerary.so.5*
%{_kf5_datadir}/mime/packages/*.xml
%{_kf5_datadir}/qlogging-categories5/*.categories
%{_kf5_libexecdir}/kitinerary-extractor


%files devel
%{_includedir}/KPim/
%{_kf5_libdir}/libKPimItinerary.so
%{_kf5_libdir}/cmake/KPimItinerary/


%changelog
* Thu Dec 08 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.12.0-1
- 22.12.0

* Thu Nov 17 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.08.3-5
- rebuild

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

* Wed Jul 27 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.04.3-10
- rebuild

* Thu Jul 07 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.04.3-1
- 22.04.3

* Thu Jun 09 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.04.2-1
- 22.04.2

* Thu May 12 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.04.1-1
- 22.04.1

* Thu Apr 21 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.04.0-1
- 22.04.0

* Thu Mar 31 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 21.12.3-2
- rebuild

* Thu Mar 03 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 21.12.3-1
- 21.12.3

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

* Tue Apr 20 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.12.3-2
- kitinerary-extractor moved back from devel package

* Sat Mar 06 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.12.3-1
- 20.12.3

* Thu Feb 04 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.12.2-1
- 20.12.2

* Thu Jan  7 22:09:24 MSK 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.12.1-1
- 20.12.1

* Thu Dec 10 21:56:19 MSK 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.12.0-1
- 20.12.0

* Fri Nov  6 13:25:49 MSK 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.08.3-1
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

* Mon May 06 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 19.04.0-1
- 19.04.0

* Tue Feb 05 2019 Rex Dieter <rdieter@fedoraproject.org> - 18.12.2-1
- 18.12.2

* Fri Feb 01 2019 Fedora Release Engineering <releng@fedoraproject.org> - 18.12.1-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_30_Mass_Rebuild

* Fri Jan 25 2019 Marek Kasik <mkasik@redhat.com> - 18.12.1-2
- Rebuild for poppler-0.73.0

* Tue Jan 08 2019 Rex Dieter <rdieter@fedoraproject.org> - 18.12.1-1
- 18.12.1

* Fri Dec 14 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.12.0-1
- 18.12.0

* Tue Nov 06 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.08.3-1
- 18.08.3

* Thu Oct 18 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.08.2-2
- add explicit pkgnames and versioning to build deps

* Wed Oct 10 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.08.2-1
- 18.08.2

* Mon Oct 01 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.08.1-1
- 18.08.1
