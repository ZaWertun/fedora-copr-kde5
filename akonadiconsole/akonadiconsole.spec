
# uncomment to enable bootstrap mode
%global bootstrap 1

%if !0%{?bootstrap}
%global tests 1
%endif

Name:    akonadiconsole
Summary: Akonadi developer tool
Version: 18.12.2
Release: 1%{?dist}

# code (generally) GPLv2, docs GFDL
License: GPLv2 and GFDL
URL:     https://userbase.kde.org/Akonadi/

%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0: http://download.kde.org/%{stable}/applications/%{version}/src/%{name}-%{version}.tar.xz

# handled by qt5-srpm-macros, which defines %%qt5_qtwebengine_arches
%{?qt5_qtwebengine_arches:ExclusiveArch: %{qt5_qtwebengine_arches}}

BuildRequires: boost-devel
BuildRequires: desktop-file-utils
BuildRequires: gettext
#BuildRequires: libappstream-glib
BuildRequires: perl-generators

BuildRequires: cmake(Qt5DBus)
BuildRequires: cmake(Qt5Sql)
BuildRequires: cmake(Qt5Test)
BuildRequires: cmake(Qt5Widgets)

BuildRequires: extra-cmake-modules
BuildRequires: kf5-rpm-macros
BuildRequires: cmake(KF5Completion)
BuildRequires: cmake(KF5Config)
BuildRequires: cmake(KF5ConfigWidgets)
BuildRequires: cmake(KF5Crash)
BuildRequires: cmake(KF5DBusAddons)
BuildRequires: cmake(KF5DocTools)
BuildRequires: cmake(KF5I18n)
BuildRequires: cmake(KF5ItemModels)
BuildRequires: cmake(KF5TextWidgets)
BuildRequires: cmake(KF5WidgetsAddons)
BuildRequires: cmake(KF5XmlGui)

%global majmin_ver %(echo %{version} | cut -d. -f1,2)
BuildRequires:  kf5-akonadi-contacts-devel >= %{majmin_ver}
BuildRequires:  kf5-akonadi-mime-devel >= %{majmin_ver}
BuildRequires:  kf5-akonadi-search-devel >= %{majmin_ver}
BuildRequires:  kf5-akonadi-server-devel >= %{majmin_ver}
BuildRequires:  kf5-calendarsupport-devel >= %{majmin_ver}
BuildRequires:  kf5-kcalendarcore-devel >= %{majmin_ver}
BuildRequires:  kf5-kcontacts-devel >= %{majmin_ver}
BuildRequires:  kf5-kmime-devel >= %{majmin_ver}
BuildRequires:  kf5-kpimtextedit-devel >= %{majmin_ver}
BuildRequires:  kf5-libkdepim-devel >= %{majmin_ver}
BuildRequires:  kf5-libkleo-devel  >= %{majmin_ver}
BuildRequires:  kf5-messagelib-devel  >= %{majmin_ver}
BuildRequires:  cmake(KF5AkonadiSearch)

BuildRequires: pkgconfig(xapian-core)

%if 0%{?tests}
BuildRequires: dbus-x11
BuildRequires: xorg-x11-server-Xvfb
%endif

# upgrade path, previously included here
Requires: akonadi-calendar-tools >= %{majmin_ver}

%description
Akonadi Console is a tool for Akonadi developers that provides means of direct
interaction with the Akonadi storage, SQL debugging, protocol debugger and
other tools.


%prep
%autosetup -p1


%build
mkdir %{_target_platform}
pushd %{_target_platform}
%{cmake_kf5} .. \
  -DBUILD_TESTING:BOOL=%{?tests:ON}%{!?tests:OFF}
popd

%make_build -C %{_target_platform}


%install
make install/fast DESTDIR=%{buildroot} -C %{_target_platform}

#find_lang %{name} --all-name --with-html


%check
desktop-file-validate %{buildroot}%{_kf5_datadir}/applications/org.kde.%{name}.desktop
#appstream-util validate-relax --nonet %{buildroot}%{_kf5_metainfodir}/org.kde.%{name}.appdata.xml
%if 0%{?tests}
export CTEST_OUTPUT_ON_FAILURE=1
xvfb-run -a \
dbus-launch --exit-with-session \
make test ARGS="--output-on-failure --timeout 20" -C %{_target_platform} ||:
%endif


%ldconfig_scriptlets

%files
%license COPYING*
%{_kf5_sysconfdir}/xdg/akonadiconsole.*
%{_kf5_bindir}/akonadiconsole
%{_kf5_datadir}/kconf_update/akonadiconsole.upd
%{_kf5_datadir}/applications/org.kde.akonadiconsole.desktop
%{_kf5_datadir}/icons/hicolor/*/apps/akonadiconsole.*
%{_kf5_datadir}/kconf_update/akonadiconsole-15.08-kickoff.sh
# -libs ?
%{_kf5_libdir}/libakonadiconsole.so.*


%changelog
* Tue Feb 05 2019 Rex Dieter <rdieter@fedoraproject.org> - 18.12.2-1
- 18.12.2

* Thu Jan 31 2019 Fedora Release Engineering <releng@fedoraproject.org> - 18.12.1-2
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

* Thu Jul 12 2018 Fedora Release Engineering <releng@fedoraproject.org> - 18.04.2-2
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

* Sun Jan 07 2018 Igor Gnatenko <ignatenkobrain@fedoraproject.org> - 17.12.0-2
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

* Wed Aug 02 2017 Fedora Release Engineering <releng@fedoraproject.org> - 17.04.3-2
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

* Tue Feb 07 2017 Rex Dieter <rdieter@fedoraproject.org> - 16.12.1-3
- Requires: akonadi-calendar-tools

* Mon Jan 30 2017 Rex Dieter <rdieter@fedoraproject.org> - 16.12.1-2
- drop unused %%package libs, define majmin_ver macro

* Mon Jan 16 2017 Rex Dieter <rdieter@fedoraproject.org> - 16.12.1-1
- akonadiconsole-16.12.1

