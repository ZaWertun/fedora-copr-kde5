%global framework kitinerary
%global tests 1

Name:    kf5-%{framework}
Version: 23.08.5
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
%global majmin_ver %{version}
BuildRequires: kf5-kmime-devel >= %{majmin_ver}
BuildRequires: kf5-kcalendarcore-devel >= %{majmin_ver}
BuildRequires: kf5-kcontacts-devel >= %{majmin_ver}
BuildRequires: kf5-kpkpass-devel >= %{majmin_ver}

# kde-pim cmake
BuildRequires:  cmake(KPim5Mime)
BuildRequires:  cmake(KF5CalendarCore)
BuildRequires:  cmake(KPim5AkonadiContact)
BuildRequires:  cmake(KPim5PkPass)
BuildRequires:  cmake(KF5CalendarCore)

BuildRequires:  pkgconfig(Qt5Gui)
BuildRequires:  pkgconfig(Qt5Qml)

BuildRequires:  pkgconfig(libxml-2.0)
BuildRequires:  pkgconfig(poppler)
BuildRequires:  pkgconfig(zlib)
BuildRequires:  pkgconfig(openssl) >= 1.1
BuildRequires:  pkgconfig(protobuf)

BuildRequires:  libphonenumber-devel
BuildRequires:  pkgconfig(zxing)

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
%cmake_kf5 \
  -DBUILD_TESTING:BOOL=%{?tests:ON}%{!?tests:OFF}
%cmake_build


%install
%cmake_install
%find_lang %{name} --all-name


%check
%if 0%{?tests}
export CTEST_OUTPUT_ON_FAILURE=1
make test/fast ARGS="--output-on-failure --timeout 30" -C %{_vpath_builddir} ||:
%endif


%ldconfig_scriptlets

%files -f %{name}.lang
%doc README.md
%license LICENSES/*.txt
%{_kf5_libdir}/libKPim5Itinerary.so.5*
%{_kf5_datadir}/mime/packages/*.xml
%{_kf5_datadir}/qlogging-categories5/*.categories
%{_kf5_libexecdir}/kitinerary-extractor


%files devel
%{_includedir}/KPim5/KItinerary/
%{_includedir}/KPim5/kitinerary/
%{_includedir}/KPim5/kitinerary_version.h
%{_kf5_libdir}/libKPim5Itinerary.so
%{_kf5_libdir}/cmake/KPim5Itinerary/
%{_kf5_libdir}/cmake/KPimItinerary/


%changelog
* Sun Feb 18 2024 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.08.5-1
- 23.08.5

* Fri Dec 08 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.08.4-1
- 23.08.4

* Fri Nov 10 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.08.3-1
- 23.08.3

* Fri Oct 13 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.08.2-1
- 23.08.2

* Thu Sep 14 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.08.1-1
- 23.08.1

* Sun Aug 27 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.08.0-1
- 23.08.0

* Thu Jul 06 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.04.3-1
- 23.04.3

* Thu Jun 08 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.04.2-1
- 23.04.2

* Thu May 11 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.04.1-1
- 23.04.1

* Thu Apr 20 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.04.0-1
- 23.04.0

* Thu Mar 02 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.12.3-1
- 22.12.3

* Thu Feb 02 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.12.2-1
- 22.12.2

