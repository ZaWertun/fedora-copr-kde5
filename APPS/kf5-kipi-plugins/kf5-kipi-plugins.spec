%undefine __cmake_in_source_build
%global framework kipi-plugins

Name:    kf5-%{framework}
Summary: Plugins to use with kf5-libkipi applications
Version: 22.12.0
Release: 1%{?dist}

License: GPLv2+
URL:     http://www.digikam.org/

%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0:        http://download.kde.org/%{stable}/release-service/%{version}/src/%{framework}-%{version}.tar.xz

## upstream patches

## upstreamable patches

BuildRequires: desktop-file-utils

BuildRequires: extra-cmake-modules
BuildRequires: gettext
BuildRequires: gcc-c++

BuildRequires: cmake(KF5Config)
BuildRequires: cmake(KF5WindowSystem)
BuildRequires: cmake(KF5XmlGui)
BuildRequires: cmake(KF5I18n)
BuildRequires: cmake(KF5Archive)
BuildRequires: cmake(KF5KIO)

BuildRequires: pkgconfig(Qt5Svg)
BuildRequires: pkgconfig(Qt5XmlPatterns)
BuildRequires: pkgconfig(Qt5X11Extras)

BuildRequires: cmake(KF5Kipi)

Requires: %{name}-libs%{?_isa} = %{version}-%{release}
Requires: hicolor-icon-theme

# upgrade path for eol'd kde4-based kipi-plugins
Obsoletes: kipi-plugins < 5.0.0

%description
This package contains plugins to use with Kipi, the KDE Image Plugin
Interface.

%package libs
Summary: Runtime libraries for %{name}
# upgrade path
Obsoletes: kipi-plugins-libs < 5.0.0
Requires: %{name} = %{version}-%{release}
%description libs
%{summary}.


%prep
%autosetup -p1 -n kipi-plugins-%{version}

# fix ChagngeLog encoding
mv ChangeLog ChangeLog.orig
iconv -f iso-8859-1 -t utf-8 ChangeLog.orig -o ChangeLog


%build
%cmake_kf5

%cmake_build


%install
%cmake_install

%find_lang all --all-name --with-html


%check
desktop-file-validate %{buildroot}%{_kf5_datadir}/applications/kipiplugins.desktop


%files -n kf5-kipi-plugins -f all.lang
%doc AUTHORS ChangeLog
%doc README TODO NEWS
%license COPYING
%{_kf5_metainfodir}/org.kde.kipi_plugins.metainfo.xml
%{_kf5_datadir}/applications/kipiplugins.desktop
%{_kf5_datadir}/kxmlgui5/kipi/
%{_kf5_datadir}/icons/hicolor/*/apps/kipi-*
%{_kf5_datadir}/kservices5/kipiplugin_*.desktop
%{_kf5_datadir}/kipiplugin_*/

%ldconfig_scriptlets libs

%files libs
%{_kf5_libdir}/libKF5kipiplugins.so*
%{_kf5_qtplugindir}/kipiplugin_*.so
%{_kf5_metainfodir}/org.kde.kipi_plugins.metainfo.xml


%changelog
* Thu Dec 08 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.12.0-1
- 22.12.0

* Fri Nov 04 2022 Marc Deop i Argemí (Private) <marc@marcdeop.com> - 22.08.3-1
- 22.08.3

* Fri Oct 14 2022 Marc Deop <marcdeop@fedoraproject.org> - 22.08.2-1
- 22.08.2

* Thu Sep 08 2022 Marc Deop <marcdeop@fedoraproject.org> - 22.08.1-1
- 22.08.1

* Fri Aug 19 2022 Marc Deop <marcdeop@fedoraproject.org> - 22.08.0-1
- 22.08.0

* Thu Jul 21 2022 Fedora Release Engineering <releng@fedoraproject.org> - 22.04.3-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_37_Mass_Rebuild

* Mon Jul 18 2022 Than Ngo <than@redhat.com> - 22.04.3-1
- 22.04.3

* Thu May 12 2022 Justin Zobel <justin@1707.io> - 22.04.1-1
- Update to 22.04.1

* Mon May 09 2022 Justin Zobel <justin@1707.io> - 22.04.0-1
- Update to 22.04.0

* Wed Mar 02 2022 Marc Deop <marcdeop@fedoraproject.org> - 21.12.3-1
- 21.12.3

* Fri Feb 04 2022 Rex Dieter <rdieter@fedoraproject.org> - 21.12.2-1
- 21.12.2

* Thu Jan 20 2022 Fedora Release Engineering <releng@fedoraproject.org> - 21.12.1-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_36_Mass_Rebuild

* Mon Jan 17 2022 Rex Dieter <rdieter@fedoraproject.org> - 21.12.1-1
- 21.12.1

* Mon Dec 27 2021 Rex Dieter <rdieter@fedoraproject.org> - 21.12.0-1
- 21.12.0

* Tue Nov 02 2021 Rex Dieter <rdieter@fedoraproject.org> - 21.08.3-1
- 21.08.3

* Wed Oct 20 2021 Rex Dieter <rdieter@fedoraproject.org> - 21.08.2-1
- 21.08.2

* Thu Jul 22 2021 Fedora Release Engineering <releng@fedoraproject.org> - 21.04.2-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_35_Mass_Rebuild

* Thu Jun 10 2021 Rex Dieter <rdieter@fedoraproject.org> - 21.04.2-1
- 21.04.2

* Tue May 11 2021 Rex Dieter <rdieter@fedoraproject.org> - 21.04.1-1
- 21.04.1

* Sun Apr 18 2021 Rex Dieter <rdieter@fedoraproject.org> - 21.04.0-1
- 21.04.0

* Wed Mar 03 2021 Rex Dieter <rdieter@fedoraproject.org> - 20.12.3-1
- 20.12.3

* Wed Feb 03 2021 Rex Dieter <rdieter@fedoraproject.org> - 20.12.2-1
- 20.12.2

* Tue Jan 26 2021 Fedora Release Engineering <releng@fedoraproject.org> - 20.08.3-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_34_Mass_Rebuild

* Fri Nov  6 14:42:46 CST 2020 Rex Dieter <rdieter@fedoraproject.org> - 20.08.3-1
- 20.08.3

* Tue Sep 15 2020 Rex Dieter <rdieter@fedoraproject.org> - 20.08.1-1
- 20.08.1

* Tue Aug 18 2020 Rex Dieter <rdieter@fedoraproject.org> - 20.08.0-1
- 20.08.0

* Tue Jul 28 2020 Fedora Release Engineering <releng@fedoraproject.org> - 20.04.3-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_33_Mass_Rebuild

* Fri Jul 17 2020 Rex Dieter <rdieter@fedoraproject.org> - 20.04.3-1
- 20.04.3

* Sat Jun 13 2020 Marie Loise Nolden <loise@kde.org> - 20.04.2-1
- 20.04.2

* Wed Jan 29 2020 Fedora Release Engineering <releng@fedoraproject.org> - 5.9.1-4
- Rebuilt for https://fedoraproject.org/wiki/Fedora_32_Mass_Rebuild

* Thu Jul 25 2019 Fedora Release Engineering <releng@fedoraproject.org> - 5.9.1-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_31_Mass_Rebuild

* Fri May 10 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.9.1-2
- Requires: hicolor-icon-theme
- fix ChangeLog encoding
- validate kipiplugins.desktop
- add .spec comment clarifying purpose of Obsoletes: kipi-plugins (upgrade path only)

* Thu May 09 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.9.1-1
- kipi-plugis-5.9.1

