%global _lto_cflags %{nil}
%undefine __cmake_in_source_build

# trim changelog included in binary rpms
%global _changelog_trimtime %(date +%s -d "1 year ago")

Name:           libkgapi
Version: 23.08.3
Release: 1%{?dist}
Summary:        Library to access to Google services

License:        GPLv2+
URL:            https://cgit.kde.org/%{name}.git

%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0: https://download.kde.org/%{stable}/release-service/%{version}/src/%{name}-%{version}.tar.xz
Source1: https://download.kde.org/%{stable}/release-service/%{version}/src/%{name}-%{version}.tar.xz.sig
Source2: gpgkey-D81C0CB38EB725EF6691C385BB463350D6EF31EF.gpg

# handled by qt5-srpm-macros, which defines %%qt5_qtwebengine_arches
%{?qt5_qtwebengine_arches:ExclusiveArch: %{qt5_qtwebengine_arches}}

BuildRequires:  gnupg2
BuildRequires:  kf5-rpm-macros
BuildRequires:  extra-cmake-modules
BuildRequires:  qt5-qtbase-devel
BuildRequires:  qt5-qtwebengine-devel
BuildRequires:  qt5-qtxmlpatterns-devel
BuildRequires:  qt5-qttools-static

BuildRequires:  kf5-kcoreaddons-devel
BuildRequires:  kf5-kio-devel
BuildRequires:  kf5-kwallet-devel
BuildRequires:  kf5-kwindowsystem-devel

BuildRequires:  kf5-kcalendarcore-devel
BuildRequires:  kf5-kcontacts-devel

BuildRequires:  cyrus-sasl-devel

BuildRequires:  cmake(KF5CalendarCore)
BuildRequires:  cmake(KF5I18n)

Obsoletes:      libkgoogle < 0.3.2
Provides:       libkgoogle = %{version}-%{release}

%description
Library to access to Google services, this package is needed by kdepim-runtime
to build akonadi-google resources.

%package        devel
Summary:        Development files for %{name}
Requires:       %{name}%{?_isa} = %{version}-%{release}
Requires:       kf5-kcoreaddons-devel
Requires:       kf5-kcalendarcore-devel
Requires:       kf5-kcontacts-devel
Obsoletes:      libkgoogle-devel < 0.3.2
Provides:       libkgoogle-devel = %{version}-%{release}
%description devel
Libraries and header files for developing applications that use akonadi-google
resources.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -n %{name}-%{version}


%build
%cmake_kf5
%cmake_build


%install
%cmake_install

%find_lang_kf5 libkgapi_qt


%ldconfig_scriptlets

%files -f libkgapi_qt.lang
%doc README*
%license LICENSES/*.txt
%{_kf5_libdir}/libKPim5GAPIBlogger.so.5*
%{_kf5_libdir}/libKPim5GAPICalendar.so.*
%{_kf5_libdir}/libKPim5GAPICore.so.*
%{_kf5_libdir}/libKPim5GAPIDrive.so.*
%{_kf5_libdir}/libKPim5GAPILatitude.so.*
%{_kf5_libdir}/libKPim5GAPIMaps.so.*
%{_kf5_libdir}/libKPim5GAPITasks.so.*
%{_kf5_libdir}/libKPim5GAPIPeople.so.*
%{_libdir}/sasl2/libkdexoauth2.so*
%{_kf5_datadir}/qlogging-categories5/*.categories


%files devel
%{_kf5_libdir}/libKPim5GAPIBlogger.so
%{_kf5_archdatadir}/mkspecs/modules/qt_KGAPIBlogger.pri
%{_kf5_libdir}/libKPim5GAPICalendar.so
%{_kf5_archdatadir}/mkspecs/modules/qt_KGAPICalendar.pri
%{_kf5_libdir}/libKPim5GAPICore.so
%{_kf5_archdatadir}/mkspecs/modules/qt_KGAPICore.pri
%{_kf5_libdir}/libKPim5GAPIDrive.so
%{_kf5_archdatadir}/mkspecs/modules/qt_KGAPIDrive.pri
%{_kf5_libdir}/libKPim5GAPILatitude.so
%{_kf5_archdatadir}/mkspecs/modules/qt_KGAPILatitude.pri
%{_kf5_libdir}/libKPim5GAPIMaps.so
%{_kf5_archdatadir}/mkspecs/modules/qt_KGAPIMaps.pri
%{_kf5_libdir}/libKPim5GAPITasks.so
%{_kf5_archdatadir}/mkspecs/modules/qt_KGAPITasks.pri
%{_kf5_libdir}/libKPim5GAPIPeople.so
%{_kf5_archdatadir}/mkspecs/modules/qt_KGAPIPeople.pri
%{_kf5_libdir}/cmake/KPimGAPI/
%{_kf5_libdir}/cmake/KPim5GAPI/
%dir %{_includedir}/KPim5/
%{_includedir}/KPim5/KGAPI/


%changelog
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

