%undefine __cmake_in_source_build

# uncomment to enable bootstrap mode
%global bootstrap 1

%if !0%{?bootstrap}
%global tests 1
%endif

Name:    korganizer
Summary: Personal Organizer
Version: 20.08.2
Release: 1%{?dist}

# code (generally) GPLv2, docs GFDL
License: GPLv2 and GFDL
URL:     https://www.kde.org/applications/utilities/korganizer/

%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0: https://download.kde.org/%{stable}/release-service/%{version}/src/%{name}-%{version}.tar.xz

# handled by qt5-srpm-macros, which defines %%qt5_qtwebengine_arches
%{?qt5_qtwebengine_arches:ExclusiveArch: %{qt5_qtwebengine_arches}}

BuildRequires: boost-devel
BuildRequires: desktop-file-utils
BuildRequires: gettext
BuildRequires: libappstream-glib
BuildRequires: perl-generators
BuildRequires: pkgconfig(phonon4qt5)

BuildRequires: cmake(Qt5DBus)
BuildRequires: cmake(Qt5Network)
BuildRequires: cmake(Qt5PrintSupport)
BuildRequires: cmake(Qt5Test)
BuildRequires: cmake(Qt5Widgets)
BuildRequires: cmake(Qt5UiTools)
BuildRequires: cmake(Qt5Xml)
BuildRequires: cmake(Qt5X11Extras)

# kf5
BuildRequires: extra-cmake-modules
BuildRequires: kf5-rpm-macros
BuildRequires: cmake(KF5Codecs)
BuildRequires: cmake(KF5Config)
BuildRequires: cmake(KF5Crash)
BuildRequires: cmake(KF5DBusAddons)
BuildRequires: cmake(KF5DocTools)
BuildRequires: cmake(KF5GlobalAccel)
BuildRequires: cmake(KF5KCMUtils)
BuildRequires: cmake(KF5NewStuff)
BuildRequires: cmake(KF5Service)

# kde-apps
%global majmin_ver %(echo %{version} | cut -d. -f1,2)
BuildRequires: kdepim-apps-libs-devel >= %{majmin_ver}
BuildRequires: kf5-akonadi-calendar-devel >= %{majmin_ver}
BuildRequires: kf5-akonadi-contacts-devel >= %{majmin_ver}
BuildRequires: kf5-akonadi-mime-devel >= %{majmin_ver}
BuildRequires: kf5-akonadi-notes-devel >= %{majmin_ver}
BuildRequires: kf5-akonadi-search-devel >= %{majmin_ver}
BuildRequires: kf5-akonadi-server-devel >= %{majmin_ver}
BuildRequires: kf5-calendarsupport-devel >= %{majmin_ver}
BuildRequires: kf5-eventviews-devel >= %{majmin_ver}
BuildRequires: kf5-incidenceeditor-devel >= %{majmin_ver}
BuildRequires: kf5-kcalendarcore-devel >= %{majmin_ver}
BuildRequires: kf5-kcalendarutils-devel >= %{majmin_ver}
BuildRequires: kf5-kcontacts-devel >= %{majmin_ver}
BuildRequires: kf5-kholidays-devel >= %{majmin_ver}
BuildRequires: kf5-kidentitymanagement-devel >= %{majmin_ver}
BuildRequires: kf5-kldap-devel >= %{majmin_ver}
BuildRequires: kf5-kontactinterface-devel >= %{majmin_ver}
BuildRequires: kf5-kmailtransport-devel >= %{majmin_ver}
BuildRequires: kf5-kmime-devel >= %{majmin_ver}
BuildRequires: kf5-kpimtextedit-devel >= %{majmin_ver}
BuildRequires: kf5-libkdepim-devel >= %{majmin_ver}
BuildRequires: kf5-mailcommon-devel >= %{majmin_ver}
BuildRequires: kf5-pimcommon-devel >= %{majmin_ver}

%if 0%{?tests}
BuildRequires: dbus-x11
BuildRequires: xorg-x11-server-Xvfb
%endif

BuildRequires: cmake(KUserFeedback)

Requires: %{name}-libs%{?_isa} = %{version}-%{release}
Requires: kdepim-runtime >= %{majmin_ver}

%description
KOrganizer is the calendar and scheduling component of the Kontact suite.
You can write journal entries, schedule appointments, events, and to-dos.

%package libs
Summary: Runtime libraries for %{name}
Requires: %{name} = %{version}-%{release}
%description libs
%{summary}.


%prep
%autosetup -n %{name}-%{version} -p1


%build
%{cmake_kf5} \
  -DBUILD_TESTING:BOOL=%{?tests:ON}%{!?tests:OFF}

%cmake_build


%install
%cmake_install

%find_lang %{name} --all-name --with-html

## unpackaged files
rm -fv %{buildroot}%{_kf5_libdir}/libkorganizer_{core,interfaces}.so


%check
desktop-file-validate %{buildroot}%{_kf5_datadir}/applications/korganizer-import.desktop
desktop-file-validate %{buildroot}%{_kf5_datadir}/applications/org.kde.%{name}.desktop
appstream-util validate-relax --nonet %{buildroot}%{_kf5_metainfodir}/org.kde.%{name}.appdata.xml
%if 0%{?tests}
export CTEST_OUTPUT_ON_FAILURE=1
xvfb-run -a \
dbus-launch --exit-with-session \
make test ARGS="--output-on-failure --timeout 20" -C %{_target_platform} ||:
%endif

%files -f %{name}.lang
%license COPYING*
%{_kf5_bindir}/korganizer
%{_kf5_datadir}/config.kcfg/korganizer.kcfg
%{_kf5_datadir}/kconf_update/korganizer*
%{_kf5_datadir}/kservices5/webcal.protocol
%{_datadir}/dbus-1/interfaces/org.kde.Korganizer.*.xml
%{_datadir}/dbus-1/interfaces/org.kde.korganizer.*.xml
%{_kf5_metainfodir}/org.kde.korganizer.appdata.xml
%{_kf5_datadir}/applications/org.kde.korganizer.desktop
%{_kf5_datadir}/applications/korganizer-import.desktop
%{_kf5_datadir}/applications/korganizer-view.desktop
%{_kf5_datadir}/korganizer/
%{_kf5_datadir}/icons/hicolor/*/apps/korg-journal.*
%{_kf5_datadir}/icons/hicolor/*/apps/korg-todo.*
%{_kf5_datadir}/icons/hicolor/*/apps/korganizer.*
%{_kf5_datadir}/icons/hicolor/*/apps/quickview.*
%{_kf5_datadir}/icons/oxygen/*/actions/smallclock.png
%{_kf5_datadir}/icons/oxygen/*/actions/upindicator.png
%{_kf5_datadir}/icons/oxygen/*/actions/checkmark.png
%{_kf5_datadir}/kservices5/korganizer_*.desktop
# Kontact integration
%{_kf5_datadir}/kontact/ksettingsdialog/*.setdlg
%{_kf5_datadir}/kservices5/kontact/korganizerplugin.desktop
%{_kf5_datadir}/kservices5/kontact/todoplugin.desktop
%{_kf5_datadir}/kservices5/kontact/journalplugin.desktop
%{_kf5_datadir}/kservices5/kontact/specialdatesplugin.desktop
%{_kf5_datadir}/kservices5/kcmapptsummary.desktop
%{_kf5_datadir}/kservices5/kcmsdsummary.desktop
%{_kf5_datadir}/kservices5/kcmtodosummary.desktop
# korgac
%{_kf5_bindir}/korgac
%{_sysconfdir}/xdg/autostart/org.kde.korgac.desktop
%{_kf5_datadir}/korgac
%{_kf5_datadir}/knsrcfiles/*.knsrc
%{_kf5_datadir}/kservicetypes5/*.desktop
%{_kf5_datadir}/dbus-1/services/*.service
%{_kf5_datadir}/qlogging-categories5/*categories

%ldconfig_scriptlets libs

%files libs
%{_kf5_libdir}/libkorganizer_interfaces.so.*
%{_kf5_libdir}/libkorganizer_core.so.*
%{_kf5_libdir}/libkorganizerprivate.so.*
%{_kf5_qtplugindir}/kcm_korganizer.so
%{_kf5_qtplugindir}/korganizerpart.so
# Kontact integration
%{_kf5_qtplugindir}/kcm_apptsummary.so
%{_kf5_qtplugindir}/kcm_sdsummary.so
%{_kf5_qtplugindir}/kcm_todosummary.so
%{_kf5_qtplugindir}/kontact5/kontact_korganizerplugin.so
%{_kf5_qtplugindir}/kontact5/kontact_todoplugin.so
%{_kf5_qtplugindir}/kontact5/kontact_journalplugin.so
%{_kf5_qtplugindir}/kontact5/kontact_specialdatesplugin.so


%changelog
* Fri Oct 09 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.08.2-1
- 20.08.2

* Thu Sep 03 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.08.1-1
- 20.08.1

* Fri Aug 14 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.08.0-1
- 20.08.0

* Thu Jul 09 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.04.3-1
- 20.04.3

* Sun Jul 05 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.04.2-2
- added cmake(KUserFeedback) to BuildRequires

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

* Mon Apr 29 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 19.04.0-1
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

* Wed Feb 07 2018 Fedora Release Engineering <releng@fedoraproject.org> - 17.12.2-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_28_Mass_Rebuild

* Tue Feb 06 2018 Rex Dieter <rdieter@fedoraproject.org> - 17.12.2-1
- 17.12.2

* Thu Jan 11 2018 Rex Dieter <rdieter@fedoraproject.org> - 17.12.1-1
- 17.12.1

* Thu Jan 11 2018 Igor Gnatenko <ignatenkobrain@fedoraproject.org> - 17.12.0-2
- Remove obsolete scriptlets

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

* Thu Aug 03 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.04.3-3
- rebuild

* Thu Aug 03 2017 Fedora Release Engineering <releng@fedoraproject.org> - 17.04.3-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Binutils_Mass_Rebuild

* Fri Jul 28 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.04.3-1
- 17.04.3

* Wed Jul 26 2017 Fedora Release Engineering <releng@fedoraproject.org> - 17.04.2-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Mass_Rebuild

* Thu Jun 15 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.04.2-1
- 17.04.2

* Mon May 15 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.04.1-1
- 17.04.1

* Thu Mar 09 2017 Rex Dieter <rdieter@fedoraproject.org> - 16.12.3-1
- 16.12.3

* Thu Feb 09 2017 Rex Dieter <rdieter@fedoraproject.org> - 16.12.2-1
- 16.12.2

* Thu Feb 02 2017 Rex Dieter <rdieter@fedoraproject.org> - 16.12.1-2
- Requires: kdepim-runtime

* Mon Jan 16 2017 Rex Dieter <rdieter@fedoraproject.org> - 16.12.1-1
- korganizer-16.12.1

