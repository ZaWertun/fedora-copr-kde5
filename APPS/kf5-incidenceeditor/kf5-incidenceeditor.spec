%global framework incidenceeditor

Name:    kf5-%{framework}
Version: 23.08.0
Release: 1%{?dist}
Summary: KDE PIM library for creating and editing calendar incidences

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
# available only where kf5-eventviews is
%{?qt5_qtwebengine_arches:ExclusiveArch: %{qt5_qtwebengine_arches}}

BuildRequires:  gnupg2
BuildRequires:  extra-cmake-modules >= 5.23.0
BuildRequires:  kf5-rpm-macros >= 5.23.0

BuildRequires:  cmake(Qt5Widgets)
BuildRequires:  cmake(Qt5Test)
BuildRequires:  cmake(Qt5Network)
BuildRequires:  cmake(Qt5WebKitWidgets)

BuildRequires:  cmake(KGantt)

BuildRequires:  cmake(KPim5Akonadi)
BuildRequires:  cmake(KPim5AkonadiMime)
BuildRequires:  cmake(KF5I18n)
BuildRequires:  cmake(KPim5Ldap)
BuildRequires:  cmake(KF5Codecs)
BuildRequires:  cmake(KF5CalendarSupport)
BuildRequires:  cmake(KPim5EventViews)
BuildRequires:  cmake(KPim5Libkdepim)
BuildRequires:  cmake(KPim5CalendarUtils)
BuildRequires:  cmake(KF5CalendarCore)
BuildRequires:  cmake(KF5MailTransport)
BuildRequires:  cmake(Grantlee5)

BuildRequires:  cmake(KF5TextAutoCorrection)

%global majmin_ver %{version}
BuildRequires:  kf5-akonadi-mime-devel >= %{majmin_ver}
BuildRequires:  kf5-akonadi-server-devel >= %{majmin_ver}
BuildRequires:  kf5-eventviews-devel >= %{majmin_ver}
BuildRequires:  kf5-kcalendarcore-devel >= %{majmin_ver}
BuildRequires:  kf5-kcalendarutils-devel >= %{majmin_ver}
BuildRequires:  kf5-kmailtransport-devel >= %{majmin_ver}
BuildRequires:  kf5-libkdepim-devel >= %{majmin_ver}
BuildRequires:  kf5-grantleetheme-devel >= %{majmin_ver}

Requires:       kf5-filesystem

Obsoletes:      kdepim-libs < 7:16.04.0
Conflicts:      kdepim-libs < 7:16.04.0

%description
%{summary}.

%package        devel
Summary:        Development files for %{name}
Requires:       %{name}%{?_isa} = %{version}-%{release}
Requires:       cmake(KPim5EventViews)
Requires:       cmake(KF5CalendarSupport)
Requires:       cmake(KPim5CalendarUtils)
Requires:       cmake(KF5CalendarCore)
Requires:       cmake(KF5MailTransport)
Requires:       cmake(KPim5Mime)
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
%{_kf5_libdir}/libKPim5IncidenceEditor.so.*
%{_kf5_datadir}/qlogging-categories5/*categories

%files devel
%{_includedir}/KPim5/IncidenceEditor/
%{_kf5_libdir}/libKPim5IncidenceEditor.so
%{_kf5_libdir}/cmake/KPim5IncidenceEditor/
%{_kf5_archdatadir}/mkspecs/modules/qt_IncidenceEditor.pri


%changelog
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

