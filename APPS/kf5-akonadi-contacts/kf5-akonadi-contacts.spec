%global framework akonadi-contacts
%global tests 1

Name:    kf5-%{framework}
Version: 23.08.3
Release: 1%{?dist}
Summary: The Akonadi Contacts Library

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

# handled by qt5-srpm-macros, which defines %%qt5_qtwebengine_arches
%{?qt5_qtwebengine_arches:ExclusiveArch: %{qt5_qtwebengine_arches}}

BuildRequires:  gnupg2
BuildRequires:  cyrus-sasl-devel
BuildRequires:  extra-cmake-modules
BuildRequires:  kf5-rpm-macros
%global kf5_ver 5.39
BuildRequires:  kf5-kdbusaddons-devel >= %{kf5_ver}
BuildRequires:  kf5-kio-devel >= %{kf5_ver}
BuildRequires:  kf5-kconfig-devel >= %{kf5_ver}
BuildRequires:  kf5-ki18n-devel >= %{kf5_ver}
BuildRequires:  kf5-ki18n-devel >= %{kf5_ver}
BuildRequires:  kf5-kitemmodels-devel >= %{kf5_ver}
BuildRequires:  kf5-prison-devel >= %{kf5_ver}
BuildRequires:  cmake(KF5KCMUtils)

BuildRequires:  cmake(Grantlee5) >= 5.1
BuildRequires:  qt5-qtwebengine-devel
BuildRequires:  cmake(Qt5Widgets) >= 5.8

%global majmin_ver %{version}
BuildRequires:  kf5-akonadi-mime-devel >= %{majmin_ver}
BuildRequires:  kf5-akonadi-server-devel >= %{majmin_ver}
BuildRequires:  kf5-kcalendarcore-devel >= %{majmin_ver}
BuildRequires:  kf5-kcontacts-devel >= %{majmin_ver}
BuildRequires:  kf5-kmime-devel >= %{majmin_ver}
BuildRequires:  kf5-grantleetheme-devel >= %{majmin_ver}
BuildRequires:  kf5-libkleo-devel >= %{majmin_ver}

%if 0%{?tests}
BuildRequires: kf5-akonadi-server >= %{majmin_ver}
BuildRequires: kf5-akonadi-server-mysql
BuildRequires: xorg-x11-server-Xvfb
%endif

# split from kf5-akonadi/kdepimlibs in 16.07
Obsoletes: kf5-akonadi < 16.07
Obsoletes: kf5-akonadi-contact < 16.07
Provides:  kf5-akonadi-contact = %{version}-%{release}

%description
%{summary}.

%package   devel
Summary:   Development files for %{name}
Requires:  %{name}%{?_isa} = %{version}-%{release}
# split from kf5-akonadi/kdepimlibs in 16.07
Obsoletes: kf5-akonadi-devel < 16.07
Obsoletes: kf5-akonadi-contact-devel < 16.07
Provides:  kf5-akonadi-contact-devel = %{version}-%{release}
Requires:   cmake(Qt5Widgets)
Requires:   cmake(KPim5Akonadi)
Requires:   cmake(KF5Contacts)
Requires:   cmake(KPim5GrantleeTheme)
Requires:   cmake(KPim5ContactEditor)
Recommends: cmake(KPim5CalendarCore)
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
%find_lang %{name} --all-name --with-html


%check
%if 0%{?tests}
export CTEST_OUTPUT_ON_FAILURE=1
DBUS_SESSION_BUS_ADDRESS=
xvfb-run -a \
%make_build test ARGS="--output-on-failure --timeout 30" -C %{_vpath_builddir} ||:
%endif


%ldconfig_scriptlets

%files -f %{name}.lang
%{_kf5_libdir}/libKPim5AkonadiContact.so.*
%{_kf5_libdir}/libKPim5ContactEditor.so.*
%{_kf5_qtplugindir}/akonadi_serializer_*.so
%{_kf5_datadir}/akonadi/plugins/serializer/
%{_kf5_datadir}/kf5/akonadi/contact/
%{_kf5_datadir}/qlogging-categories5/*categories

%files devel
%{_includedir}/KPim5/AkonadiContact/
%{_includedir}/KPim5/AkonadiContactEditor/
%{_kf5_libdir}/cmake/KPim5AkonadiContact/
%{_kf5_libdir}/cmake/KF5AkonadiContactEditor/
%{_kf5_libdir}/cmake/KPim5ContactEditor/
%{_kf5_libdir}/libKPim5AkonadiContact.so
%{_kf5_libdir}/libKPim5ContactEditor.so
%{_kf5_archdatadir}/mkspecs/modules/qt_AkonadiContact.pri
%{_kf5_archdatadir}/mkspecs/modules/qt_ContactEditor.pri


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

* Sat Mar 25 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.12.3-6
- rebuild

* Thu Mar 02 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.12.3-1
- 22.12.3

