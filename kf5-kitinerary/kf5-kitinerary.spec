%global framework kitinerary

# uncomment to enable bootstrap mode
#global bootstrap 1

%if !0%{?bootstrap}
%global tests 1
%endif

Name:    kf5-%{framework}
Version: 19.12.1
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

BuildRequires:  extra-cmake-modules
BuildRequires:  kf5-rpm-macros
BuildRequires:  shared-mime-info
BuildRequires:  cmake(KF5I18n)

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
%autosetup -n %{framework}-%{version} -p1


%build
%if 0%{fedora} == 29
# popplerutils.cpp:88:47: error: passing 'const GfxPath' as 'this' argument discards qualifiers [-fpermissive]
%global optflags %(echo %{optflags} -fpermissive)
%endif

mkdir %{_target_platform}
pushd %{_target_platform}
%{cmake_kf5} .. \
  -DBUILD_TESTING:BOOL=%{?tests:ON}%{!?tests:OFF}
popd

%make_build -C %{_target_platform}


%install
make install/fast DESTDIR=%{buildroot} -C %{_target_platform}

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
%license COPYING*
%{_kf5_libdir}/libKPimItinerary.so.5*
%{_kf5_datadir}/mime/packages/*.xml
%{_kf5_datadir}/qlogging-categories5/*.categories


%files devel
%{_includedir}/KPim/
%{_kf5_libdir}/libKPimItinerary.so
%{_kf5_libdir}/cmake/KPimItinerary/
%{_kf5_libexecdir}/%{framework}*


%changelog
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
