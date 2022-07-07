# uncomment to enable bootstrap mode
#global bootstrap 1

%if !0%{?bootstrap}
## * significantly increases build-time and there at least several failures
##   ping upstream if this is expected
## * FTBFS on arm (disable for now)
#ifnarch %{arm}
#global tests 1
#endif
%endif

Name:           zanshin
Version:        22.04.3
Release:        1%{?dist}
Summary:        Todo/action management software

License:        GPLv2
URL:            http://zanshin.kde.org/
Source0:        https://download.kde.org/stable/release-service/%{version}/src/%{name}-%{version}.tar.xz

## upstream patches

# handled by qt5-srpm-macros, which defines %%qt5_qtwebengine_arches
%{?qt5_qtwebengine_arches:ExclusiveArch: %{qt5_qtwebengine_arches}}

BuildRequires:  extra-cmake-modules
BuildRequires:  gettext
BuildRequires:  intltool
## kf5
BuildRequires:  cmake(KF5Akonadi)
BuildRequires:  cmake(KF5AkonadiCalendar) cmake(KF5AkonadiContact)
BuildRequires:  cmake(KF5AkonadiNotes)
BuildRequires:  cmake(KF5AkonadiSearch)
BuildRequires:  cmake(KF5IdentityManagement)
BuildRequires:  cmake(KF5KontactInterface)
BuildRequires:  cmake(KF5Ldap)
BuildRequires:  cmake(KF5Runner)
BuildRequires:  cmake(KF5Wallet)
BuildRequires:  cmake(Qt5Gui)
BuildRequires:  cmake(Qt5Widgets)
BuildRequires:  cmake(Qt5Qml)
BuildRequires:  cmake(Qt5Test)
## %%check
BuildRequires:  desktop-file-utils
BuildRequires:  libappstream-glib
%if 0%{?tests}
BuildRequires:  boost-devel
BuildRequires:  dbus-x11
BuildRequires:  kf5-akonadi-server
BuildRequires:  kf5-akonadi-server-devel
BuildRequires:  time
BuildRequires:  xorg-x11-server-Xvfb
%endif

Provides: zanshin-frontend = %{version}-%{release}
Requires: zanshin-common = %{version}-%{release}
Obsoletes: renku < %{version}
Provides: zanshin = %{version}-%{release}
# https://bugzilla.redhat.com/show_bug.cgi?id=1602214
Requires: kdepim-runtime

%description
Zanshin Todo is a powerful yet simple application for managing your day to day
actions. It helps you organize and reduce the cognitive pressure of what one has
to do in his job and personal life. You'll never forget anything anymore,
getting your mind like water.

%package common
Summary: common files for %{name}
Requires: zanshin-frontend = %{version}-%{release}
BuildArch: noarch
%description common
%{summary}.


%prep
%autosetup -p1


%build
%cmake_kf5 \
  -DBUILD_TESTING:BOOL=%{?tests:ON}%{!?tests:OFF}
%cmake_build


%install
%cmake_install
%find_lang %{name} --all-name


%check
appstream-util validate-relax --nonet %{buildroot}%{_kf5_metainfodir}/org.kde.%{name}.metainfo.xml
desktop-file-validate %{buildroot}%{_datadir}/applications/org.kde.%{name}.desktop
%if 0%{?tests}
export CTEST_OUTPUT_ON_FAILURE=1
xvfb-run -a \
dbus-launch --exit-with-session \
time \
%ctest --timeout 30
%endif

%files common -f %{name}.lang
%doc AUTHORS HACKING TODO
%license LICENSES/*.txt
%{_kf5_datadir}/icons/hicolor/*/*/zanshin.*
%dir %{_kf5_datadir}/kservices5/kontact/

%files
%{_kf5_bindir}/zanshin*
%{_kf5_metainfodir}/org.kde.zanshin.metainfo.xml
%{_kf5_datadir}/applications/org.kde.zanshin.desktop
%{_kf5_datadir}/kservices5/kontact/zanshin_plugin.desktop
%{_kf5_datadir}/kservices5/plasma-runner-zanshin.desktop
%{_kf5_datadir}/kservices5/zanshin_part.desktop
%{_kf5_datadir}/kxmlgui5/zanshin/
%{_qt5_plugindir}/kontact5/kontact_zanshinplugin.so
%{_qt5_plugindir}/krunner_zanshin.so
%{_qt5_plugindir}/zanshin_part.so

%changelog
* Thu Jul 07 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.04.3-1
- 22.04.3

* Thu Jun 09 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.04.2-1
- 22.04.2

* Thu May 12 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.04.1-1
- 22.04.1

* Thu Apr 21 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.04.0-1
- 22.04.0

* Thu Mar 03 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 21.12.3-1
- 21.12.3

* Thu Feb 03 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 21.12.2-1
- 21.12.2

* Fri Jul 23 2021 Fedora Release Engineering <releng@fedoraproject.org> - 0.5.71-5
- Rebuilt for https://fedoraproject.org/wiki/Fedora_35_Mass_Rebuild

* Mon May  3 2021 José Matos <jamatos@fedoraproject.org> - 0.5.71-4
- Add patch to support KDEPIM >= 20.08

* Thu Jan 28 2021 Fedora Release Engineering <releng@fedoraproject.org> - 0.5.71-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_34_Mass_Rebuild

* Wed Jul 29 2020 Fedora Release Engineering <releng@fedoraproject.org> - 0.5.71-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_33_Mass_Rebuild

* Wed Mar 11 2020 Than Ngo <than@redhat.com> - 0.5.71-1
- update to 0.5.71

* Fri Jan 31 2020 Fedora Release Engineering <releng@fedoraproject.org> - 0.5.0-6
- Rebuilt for https://fedoraproject.org/wiki/Fedora_32_Mass_Rebuild

* Sat Jul 27 2019 Fedora Release Engineering <releng@fedoraproject.org> - 0.5.0-5
- Rebuilt for https://fedoraproject.org/wiki/Fedora_31_Mass_Rebuild

* Sun Feb 03 2019 Fedora Release Engineering <releng@fedoraproject.org> - 0.5.0-4
- Rebuilt for https://fedoraproject.org/wiki/Fedora_30_Mass_Rebuild

* Tue Jul 24 2018 Rex Dieter <rdieter@fedoraproject.org> - 0.5.0-3
- Requires: kdepim-runtime (#1602214)

* Sat Jul 14 2018 Fedora Release Engineering <releng@fedoraproject.org> - 0.5.0-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_29_Mass_Rebuild

* Sun Mar 04 2018 Rex Dieter <rdieter@fedoraproject.org> - 0.5.0-1
- zanshin-0.5.0 (#1551299)
- use %%make_build

* Fri Feb 09 2018 Fedora Release Engineering <releng@fedoraproject.org> - 0.4.1-10.20171205
- Rebuilt for https://fedoraproject.org/wiki/Fedora_28_Mass_Rebuild

* Thu Jan 18 2018 Igor Gnatenko <ignatenkobrain@fedoraproject.org> - 0.4.1-9.20171205
- Remove obsolete scriptlets

* Thu Dec 21 2017 Rex Dieter <rdieter@fedoraproject.org> - 0.4.1-8.20171205
- 20171205 snapshot (rolls in many FTBFS fixes)

* Tue Dec 19 2017 Rex Dieter <rdieter@fedoraproject.org> - 0.4.1-7
- rebuild (kde-apps-17.12)

* Thu Aug 03 2017 Fedora Release Engineering <releng@fedoraproject.org> - 0.4.1-6
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Binutils_Mass_Rebuild

* Thu Jul 27 2017 Fedora Release Engineering <releng@fedoraproject.org> - 0.4.1-5
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Mass_Rebuild

* Mon Feb 27 2017 Rex Dieter <rdieter@fedoraproject.org> - 0.4.1-4
- rebuild (kde-apps-16.12)

* Sat Feb 11 2017 Fedora Release Engineering <releng@fedoraproject.org> - 0.4.1-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_26_Mass_Rebuild

* Sun Jan 29 2017 Rex Dieter <rdieter@fedoraproject.org> - 0.4.1-2
- main/renku: Requires: -common

* Wed Nov 09 2016 Rex Dieter <rdieter@fedoraproject.org> - 0.4.1-1
- zanshin-0.4.1

* Wed Nov 09 2016 Rex Dieter <rdieter@fedoraproject.org> - 0.4.0-3
- ExclusiveArch: %%{qt5_qtwebengine_arches}

* Wed Jul 06 2016 Rex Dieter <rdieter@fedoraproject.org> - 0.4.0-2
- fix icon scriptlets
- -common, renku subpkgs

* Wed Jul 06 2016 Rex Dieter <rdieter@fedoraproject.org> - 0.4-1
- zanshin-0.4 (#1335615)

* Fri Feb 05 2016 Fedora Release Engineering <releng@fedoraproject.org> - 0.2.1-9
- Rebuilt for https://fedoraproject.org/wiki/Fedora_24_Mass_Rebuild

* Fri Jun 19 2015 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 0.2.1-8
- Rebuilt for https://fedoraproject.org/wiki/Fedora_23_Mass_Rebuild

* Sat May 02 2015 Kalev Lember <kalevlember@gmail.com> - 0.2.1-7
- Rebuilt for GCC 5 C++11 ABI change

* Mon Aug 18 2014 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 0.2.1-6
- Rebuilt for https://fedoraproject.org/wiki/Fedora_21_22_Mass_Rebuild

* Sat Jun 07 2014 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 0.2.1-5
- Rebuilt for https://fedoraproject.org/wiki/Fedora_21_Mass_Rebuild

* Sun Aug 04 2013 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 0.2.1-4
- Rebuilt for https://fedoraproject.org/wiki/Fedora_20_Mass_Rebuild

* Fri Feb 15 2013 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 0.2.1-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_19_Mass_Rebuild

* Sun Jul 22 2012 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 0.2.1-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_18_Mass_Rebuild

* Sat Feb 18 2012 Christoph Wickert <cwickert@fedoraproject.org> - 0.2.1-1
- Update to 0.2.1

* Wed Nov 30 2011 Christoph Wickert <cwickert@fedoraproject.org> - 0.2.0-1
- Update to 0.2.0

* Wed Oct 05 2011 Christoph Wickert <cwickert@fedoraproject.org> - 0.1.91-1
- Update to 0.2 rc1

* Thu Sep 01 2011 Christoph Wickert <cwickert@fedoraproject.org> - 0.1.82-1
- Update to 0.2 Beta2

* Tue Aug 09 2011 Christoph Wickert <cwickert@fedoraproject.org> - 0.1.81-1
- Initial package (0.2. beta1)
