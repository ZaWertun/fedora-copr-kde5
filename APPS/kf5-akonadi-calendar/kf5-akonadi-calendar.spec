%global framework akonadi-calendar
%global tests 1

Name:    kf5-%{framework}
Version: 23.08.3
Release: 1%{?dist}
Summary: The Akonadi Calendar Library

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
BuildRequires:  cyrus-sasl-devel
BuildRequires:  extra-cmake-modules
BuildRequires:  kf5-rpm-macros
%global kf5_ver 5.23.0
BuildRequires:  kf5-kdelibs4support-devel >= %{kf5_ver}
BuildRequires:  kf5-kio-devel >= %{kf5_ver}
BuildRequires:  kf5-kwallet-devel >= %{kf5_ver}
BuildRequires:  kf5-kcodecs-devel >= %{kf5_ver}
#global majmin_ver %(echo %{version} | cut -d. -f1,2)
%global majmin_ver %{version}
BuildRequires:  kf5-kmailtransport-devel >= %{majmin_ver}
BuildRequires:  kf5-kcontacts-devel >= %{majmin_ver}
BuildRequires:  kf5-kidentitymanagement-devel >= %{majmin_ver}
BuildRequires:  kf5-kcalendarcore-devel >= %{majmin_ver}
BuildRequires:  kf5-kcalendarutils-devel >= %{majmin_ver}
# kf5-akonadi-contacts available only where qt5-qtwebengine is
%{?qt5_qtwebengine_arches:ExclusiveArch: %{qt5_qtwebengine_arches}}
BuildRequires:  kf5-akonadi-contacts-devel >= %{majmin_ver}
BuildRequires:  kf5-akonadi-server-devel >= %{majmin_ver}
BuildRequires:  kf5-grantleetheme-devel >= %{majmin_ver}
BuildRequires:  kf5-messagelib-devel >= %{majmin_ver}
BuildRequires:  kf5-libkdepim-devel >= %{majmin_ver}
BuildRequires:  qt5-qtbase-devel
BuildRequires:  cmake(Grantlee5)
BuildRequires:  cmake(KF5TextAutoCorrectionCore)
%if 0%{?tests}
BuildRequires: kf5-akonadi-server >= %{majmin_ver}
BuildRequires: kf5-akonadi-server-mysql
BuildRequires: dbus-x11
BuildRequires: xorg-x11-server-Xvfb
%endif


%description
%{summary}.

%package        devel
Summary:        Development files for %{name}
Requires:       %{name}%{?_isa} = %{version}-%{release}
Requires:       kf5-akonadi-contacts-devel
Requires:       kf5-akonadi-server-devel
Requires:       kf5-kcalendarcore-devel
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
xvfb-run -a \
dbus-launch --exit-with-session \
make test ARGS="--output-on-failure --timeout 30" -C %{_vpath_builddir} ||:
%endif


%ldconfig_scriptlets

%files -f %{name}.lang
%license LICENSES/*.txt
%{_kf5_bindir}/kalendarac
%{_kf5_libdir}/libKPim5AkonadiCalendar.so.*
%{_kf5_plugindir}/org.kde.kcalendarcore.calendars/libakonadicalendarplugin.so
%{_kf5_qtplugindir}/akonadi_serializer_kcalcore.so
%{_kf5_datadir}/akonadi/plugins/serializer/
%{_kf5_datadir}/qlogging-categories5/*categories
%{_kf5_datadir}/knotifications5/kalendarac.notifyrc
%{_kf5_datadir}/dbus-1/services/org.kde.kalendarac.service
%{_sysconfdir}/xdg/autostart/org.kde.kalendarac.desktop

%files devel
%{_includedir}/KPim5/AkonadiCalendar/
%{_kf5_libdir}/libKPim5AkonadiCalendar.so
%{_kf5_libdir}/cmake/KF5AkonadiCalendar/
%{_kf5_libdir}/cmake/KPim5AkonadiCalendar/
%{_kf5_archdatadir}/mkspecs/modules/qt_AkonadiCalendar.pri


%changelog
* Fri Nov 10 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.08.3-1
- 23.08.3

* Sun Oct 15 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.08.2-5
- rebuilt

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

* Sat Mar 25 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.12.3-5
- rebuild

* Thu Mar 02 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.12.3-1
- 22.12.3

