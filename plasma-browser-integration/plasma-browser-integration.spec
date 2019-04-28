%global base_name    plasma-browser-integration

Name:    plasma-browser-integration
Summary: %{base_name} provides components necessary to integrate browsers into the Plasma Desktop
Version: 5.15.4
Release: 1%{?dist}

License: GPLv2+
URL:     https://cgit.kde.org/%{base_name}.git

%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0:        http://download.kde.org/%{stable}/plasma/%{version}/%{base_name}-%{version}.tar.xz

## downstream patches

## upstream patches

## upstreamable patches

BuildRequires:  extra-cmake-modules
BuildRequires:  kf5-rpm-macros
BuildRequires:  cmake(Qt5Gui)
BuildRequires:  cmake(Qt5DBus)
BuildRequires:  cmake(Qt5Widgets)
BuildRequires:  cmake(KF5KIO)
BuildRequires:  cmake(KF5I18n)
BuildRequires:  cmake(KF5Config)
BuildRequires:  cmake(KF5DBusAddons)
BuildRequires:  cmake(KF5Notifications)
BuildRequires:  cmake(KF5Runner)
BuildRequires:  cmake(KF5WindowSystem)
BuildRequires:  cmake(KF5Activities)

Supplements: (plasma-workspace and chromium)
Supplements: (plasma-workspace and firefox)

%description
%{base_name} coupled with a browser plugin provides integration of the browser in the desktop.

For more information, see
https://community.kde.org/Plasma/Browser_Integration


%prep
%autosetup -n %{base_name}-%{version} -p1


%build
mkdir %{_target_platform}
pushd %{_target_platform}
%{cmake_kf5} .. \
  -DMOZILLA_DIR:PATH=%{_libdir}/mozilla
popd

%make_build -C %{_target_platform}


%install
make install/fast DESTDIR=%{buildroot} -C %{_target_platform}

%find_lang %{name} --all-name


%files -f %{name}.lang
%license COPYING-GPL3
%config %{_sysconfdir}/chromium/native-messaging-hosts/org.kde.plasma.browser_integration.json
%config %{_sysconfdir}/opt/chrome/native-messaging-hosts/org.kde.plasma.browser_integration.json
%{_bindir}/plasma-browser-integration-host
%{_libdir}/mozilla/native-messaging-hosts/org.kde.plasma.browser_integration.json
%{_libdir}/qt5/plugins/kf5/kded/browserintegrationreminder.so
%{_libdir}/qt5/plugins/krunner_browsertabs.so
%{_datadir}/kservices5/plasma-runner-browsertabs.desktop


%changelog
* Sun Apr 28 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.15.4-1
- 5.15.4

* Tue Feb 19 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.14.5-1
- 5.14.5

* Tue Nov 27 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.14.4-1
- 5.14.4

* Thu Nov 08 2018 Martin Kyral <martin.kyral@gmail.com> - 5.14.3-1
- 5.14.3

* Wed Oct 24 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.14.2-1
- 5.14.2

* Tue Oct 16 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.14.1-1
- 5.14.1

* Fri Oct 05 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.14.0-1
- 5.14.0

* Mon Oct 01 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.13.90-2
- add kde wiki link to %%description

* Fri Sep 14 2018 Martin Kyral <martin.kyral@gmail.com> - 5.13.90-1
- 5.13.90

* Tue Sep 04 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.13.5-1
- 5.13.5

* Mon Aug 20 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.13.4-2
- Supplements: (plasma-workspace and (chromium or firefox))

* Thu Aug 02 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.13.4-1
- 5.13.4

* Fri Jul 13 2018 Fedora Release Engineering <releng@fedoraproject.org> - 5.13.3-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_29_Mass_Rebuild

* Wed Jul 11 2018 Martin Kyral <martin.kyral@gmail.com> - 5.13.3-1
- 5.13.3

* Mon Jul 09 2018 Martin Kyral <martin.kyral@gmail.com> - 5.13.2-1
- 5.13.2

* Tue Jun 19 2018 Martin Kyral <martin.kyral@gmail.com> - 5.13.1-1
- 5.13.1

* Sat Jun 09 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.13.0-1
- 5.13.0

* Fri May 18 2018 Martin Kyral <martin.kyral@gmail.com> - 5.12.90-1
- 5.12.90 (new package)