%undefine __cmake_in_source_build
%global framework kipi-plugins

Name:    kf5-%{framework}
Summary: Plugins to use with kf5-libkipi applications
Version: 23.08.5
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
* Sun Feb 18 2024 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.08.5-1
- 23.08.5

* Fri Dec 08 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.08.4-1
- 23.08.4

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

* Thu Mar 02 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.12.3-1
- 22.12.3

* Thu Feb 02 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.12.2-1
- 22.12.2

