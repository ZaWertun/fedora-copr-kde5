%global framework eventviews

Name:    kf5-%{framework}
Version: 23.04.3
Release: 1%{?dist}
Summary: KDE PIM library for displaying events and calendars

License: GPLv2+
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
# available only where kf5-calendarsupport is
%{?qt5_qtwebengine_arches:ExclusiveArch: %{qt5_qtwebengine_arches}}

BuildRequires:  gnupg2
BuildRequires:  extra-cmake-modules >= 5.23.0
BuildRequires:  kf5-rpm-macros >= 5.23.0

BuildRequires:  cmake(Qt5Widgets)
BuildRequires:  cmake(Qt5Test)
BuildRequires:  cmake(Qt5UiTools)

BuildRequires:  cmake(KGantt)

BuildRequires:  cmake(KF5Akonadi)
BuildRequires:  cmake(KF5I18n)
BuildRequires:  cmake(KF5Codecs)
BuildRequires:  cmake(KF5Libkdepim)
BuildRequires:  cmake(KF5CalendarUtils)
BuildRequires:  cmake(KF5CalendarCore)
BuildRequires:  cmake(KF5CalendarSupport)
BuildRequires:  cmake(KF5AkonadiCalendar)
BuildRequires:  cmake(KF5Mime)
BuildRequires:  cmake(KF5Holidays)
BuildRequires:  cmake(KF5IdentityManagement)

%global majmin_ver %{version}
BuildRequires:  kf5-akonadi-server-devel >= %{majmin_ver}
BuildRequires:  kf5-libkdepim-devel >= %{majmin_ver}
BuildRequires:  kf5-kcalendarutils-devel >= %{majmin_ver}
BuildRequires:  kf5-kcalendarcore-devel >= %{majmin_ver}
BuildRequires:  kf5-calendarsupport-devel >= %{majmin_ver}
BuildRequires:  kf5-akonadi-calendar-devel >= %{majmin_ver}
BuildRequires:  kf5-kmime-devel >= %{majmin_ver}
BuildRequires:  kf5-kidentitymanagement-devel >= %{majmin_ver}

Requires:       kf5-filesystem

Obsoletes:      kdepim-libs < 7:16.04.0

%description
%{summary}.

%package        devel
Summary:        Development files for %{name}
Requires:       %{name}%{?_isa} = %{version}-%{release}
Requires:       cmake(KF5Akonadi)
Requires:       cmake(KF5CalendarSupport)
Requires:       cmake(KF5CalendarCore)
Requires:       cmake(KF5CalendarUtils)
Requires:       cmake(KF5AkonadiCalendar)
%description    devel
The %{name}-devel package contains libraries and header files for
developing applications that use %{name}.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -n %{framework}-%{version}


%build
%cmake_kf5
%cmake_build


%install
%cmake_install
%find_lang %{name} --all-name --with-html


%ldconfig_scriptlets

%files -f %{name}.lang
%license LICENSES/*.txt
%{_kf5_libdir}/libKPim5EventViews.so.*
%{_kf5_datadir}/qlogging-categories5/*categories

%files devel
%{_includedir}/KPim5/EventViews/
%{_kf5_libdir}/libKPim5EventViews.so
%{_kf5_libdir}/cmake/KF5EventViews/
%{_kf5_libdir}/cmake/KPim5EventViews/
%{_kf5_archdatadir}/mkspecs/modules/qt_EventViews.pri


%changelog
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

