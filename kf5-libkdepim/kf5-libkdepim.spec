%global framework libkdepim

Name:    kf5-%{framework}
Version: 19.04.2
Release: 1%{?dist}
Summary: Library for common kdepim apps

License: GPLv2+
URL:     http://cgit.kde.org/%{framework}.git

%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0:        http://download.kde.org/%{stable}/applications/%{version}/src/%{framework}-%{version}.tar.xz

BuildRequires:  boost-devel
%global kf5_ver 5.23
BuildRequires:  extra-cmake-modules >= %{kf5_ver}
BuildRequires:  kf5-rpm-macros >= %{kf5_ver} 
BuildRequires:  cmake(KF5I18n)
BuildRequires:  cmake(KF5Completion)
BuildRequires:  cmake(KF5KCMUtils)
BuildRequires:  cmake(KF5Codecs)
BuildRequires:  cmake(KF5Wallet)
BuildRequires:  cmake(KF5IconThemes)
BuildRequires:  cmake(KF5ItemViews)
BuildRequires:  cmake(Qt5Designer)
BuildRequires:  cmake(Qt5UiTools)
BuildRequires:  cmake(Qt5Widgets)
#global majmin_ver %(echo %{version} | cut -d. -f1,2)
%global majmin_ver %{version}
# kf5-akonadi-contacts available only where qt5-qtwebengine is
%{?qt5_qtwebengine_arches:ExclusiveArch: %{qt5_qtwebengine_arches}}
BuildRequires:  kf5-akonadi-contacts-devel >= %{majmin_ver}
BuildRequires:  kf5-akonadi-search-devel >= %{majmin_ver}
BuildRequires:  kf5-akonadi-server-devel >= %{majmin_ver}
BuildRequires:  kf5-kcontacts-devel >= %{majmin_ver}
BuildRequires:  kf5-kimap-devel >= %{majmin_ver}
BuildRequires:  kf5-kldap-devel >= %{majmin_ver}
BuildRequires:  kf5-kmime-devel >= %{majmin_ver}

Requires:       kf5-filesystem

Obsoletes:      kdepim-libs < 7:16.04.0
Conflicts:      kdepim-libs < 7:16.04.0
# kdepimwidgets designer plugin moved here
Conflicts:      kdepim-common < 16.04.0
# kcm_ldap moved here
Conflicts:      kaddressbook < 16.04.0

%description
%{summary}.

%package        akonadi
Summary:        The LibkdepimAkonadi runtime library
Requires:       %{name}%{?_isa} = %{version}-%{release}
%description    akonadi
%{summary}.

%package        devel
Summary:        Development files for %{name}
Requires:       %{name}%{?_isa} = %{version}-%{release}
# -akonadi deps
Requires:       %{name}-akonadi%{?_isa} = %{version}-%{release}
Requires:       cmake(KF5Akonadi)
Requires:       cmake(KF5AkonadiContact)
%description    devel
The %{name}-devel package contains libraries and header files for
developing applications that use %{name}.


%prep
%autosetup -n %{framework}-%{version}


%build
mkdir %{_target_platform}
pushd %{_target_platform}
%{cmake_kf5} ..
popd

make %{?_smp_mflags} -C %{_target_platform}


%install
make install/fast DESTDIR=%{buildroot} -C %{_target_platform}

%find_lang %{name} --all-name --with-html


%ldconfig_scriptlets

%files -f %{name}.lang
%license COPYING*
%{_kf5_sysconfdir}/xdg/libkdepim.*
%{_kf5_libdir}/libKF5Libkdepim.so.5*
%{_kf5_datadir}/kdepimwidgets/
%{_qt5_plugindir}/designer/kdepimwidgets.so
%{_kf5_datadir}/kservices5/kcmldap.desktop
%{_qt5_plugindir}/kcm_ldap.so

%ldconfig_scriptlets akonadi

%files akonadi
%{_kf5_libdir}/libKF5LibkdepimAkonadi.so.5*
%{_qt5_plugindir}/designer/kdepimakonadiwidgets.so

%files devel
%{_kf5_libdir}/libKF5Libkdepim.so
%{_kf5_libdir}/cmake/KF5Libkdepim/
%{_kf5_includedir}/libkdepim_version.h
%{_kf5_includedir}/libkdepim/
%{_kf5_includedir}/Libkdepim/
%{_kf5_archdatadir}/mkspecs/modules/qt_Libkdepim.pri
%{_kf5_libdir}/cmake/MailTransportDBusService/
%{_kf5_datadir}/dbus-1/interfaces/org.kde.addressbook.service.xml
%{_kf5_datadir}/dbus-1/interfaces/org.kde.mailtransport.service.xml
# -akonadi
%{_kf5_libdir}/libKF5LibkdepimAkonadi.so
%{_kf5_libdir}/cmake/KF5LibkdepimAkonadi/
%{_kf5_includedir}/libkdepimakonadi_version.h
%{_kf5_includedir}/libkdepimakonadi/
%{_kf5_includedir}/LibkdepimAkonadi/
%{_kf5_archdatadir}/mkspecs/modules/qt_LibkdepimAkonadi.pri


%changelog
* Thu Jun 06 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 19.04.2-1
- 19.04.2

* Thu May 09 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 19.04.1-1
- 19.04.1

* Sun Apr 28 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 19.04.0-1
- 19.04.0

* Tue Feb 05 2019 Rex Dieter <rdieter@fedoraproject.org> - 18.12.2-1
- 18.12.2

* Fri Feb 01 2019 Fedora Release Engineering <releng@fedoraproject.org> - 18.12.1-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_30_Mass_Rebuild

* Tue Jan 08 2019 Rex Dieter <rdieter@fedoraproject.org> - 18.12.1-1
- 18.12.1

* Fri Dec 14 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.12.0-1
- 18.12.0

* Tue Nov 06 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.08.3-1
- 18.08.3

* Wed Oct 10 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.08.2-1
- 18.08.2

* Mon Oct 01 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.08.1-1
- 18.08.1

* Fri Jul 13 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.04.3-1
- 18.04.3

* Fri Jul 13 2018 Fedora Release Engineering <releng@fedoraproject.org> - 18.04.2-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_29_Mass_Rebuild

* Wed Jun 06 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.04.2-1
- 18.04.2

* Wed May 09 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.04.1-1
- 18.04.1

* Fri Apr 20 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.04.0-1
- 18.04.0

* Tue Mar 06 2018 Rex Dieter <rdieter@fedoraproject.org> - 17.12.3-1
- 17.12.3

* Tue Feb 06 2018 Rex Dieter <rdieter@fedoraproject.org> - 17.12.2-1
- 17.12.2

* Thu Jan 11 2018 Rex Dieter <rdieter@fedoraproject.org> - 17.12.1-1
- 17.12.1

* Tue Dec 12 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.12.0-1
- 17.12.0

* Wed Dec 06 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.11.90-1
- 17.11.90

* Wed Nov 22 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.11.80-1
- 17.11.80

* Wed Nov 08 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.08.3-1
- 17.08.3

* Mon Sep 25 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.08.1-1
- 17.08.1

* Fri Jul 28 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.04.3-1
- 17.04.3

* Wed Jul 26 2017 Fedora Release Engineering <releng@fedoraproject.org> - 17.04.2-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Mass_Rebuild

* Thu Jun 15 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.04.2-1
- 17.04.2

* Sat May 13 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.04.1-1
- 17.04.1

* Thu Mar 09 2017 Rex Dieter <rdieter@fedoraproject.org> - 16.12.3-1
- 16.12.3

* Thu Feb 09 2017 Rex Dieter <rdieter@fedoraproject.org> - 16.12.2-1
- 16.12.2

* Mon Jan 16 2017 Rex Dieter <rdieter@fedoraproject.org> - 16.12.1-1
- 16.12.1

* Mon Dec 05 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.08.3-1
- 16.08.3

* Thu Oct 13 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.08.2-1
- 16.08.2

* Thu Sep 08 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.08.1-1
- 16.08.1

* Sun Sep 04 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.08.0-1
- 16.08.0

* Sun Jul 10 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.04.3-1
- 16.04.3

* Sun Jun 12 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.04.2-1
- 16.04.2

* Thu May 26 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.04.1-2
- fix Conflicts/Obsoletes versioning

* Tue May 24 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.04.1-1
- first try

