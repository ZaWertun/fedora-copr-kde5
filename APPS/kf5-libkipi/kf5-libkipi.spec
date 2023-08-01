%global framework libkipi

Name:    kf5-libkipi
Summary: Common plugin infrastructure for KDE image applications
Version: 23.04.3
Release: 1%{?dist}

License: GPLv2+
URL:     https://cgit.kde.org/%{framework}.git
%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0: https://download.kde.org/%{stable}/release-service/%{version}/src/%{framework}-%{version}.tar.xz
Source1: https://download.kde.org/%{stable}/release-service/%{version}/src/%{framework}-%{version}.tar.xz.sig
Source2: gpgkey-D81C0CB38EB725EF6691C385BB463350D6EF31EF.gpg

BuildRequires: gnupg2
BuildRequires: extra-cmake-modules
BuildRequires: gettext
BuildRequires: kf5-kconfig-devel
BuildRequires: kf5-ki18n-devel
BuildRequires: kf5-kxmlgui-devel
BuildRequires: kf5-kservice-devel
BuildRequires: kf5-rpm-macros
BuildRequires: pkgconfig(Qt5Gui)

%if 0%{?fedora} > 23
Recommends: kf5-kipi-plugins
%endif

%description
Kipi (KDE Image Plugin Interface) is an effort to develop a common plugin
structure (for Digikam, Gwenview, etc.). Its aim is to share
image plugins among graphic applications.

%package devel
Summary:  Development files for %{name}
Requires: %{name}%{?_isa} = %{version}-%{release}
Requires: cmake(Qt5Gui)
Requires: cmake(KF5Config)
Requires: cmake(KF5Service)
Requires: cmake(KF5XmlGui)
%description devel
%{summary}.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup  -n %{framework}-%{version} -p1


%build
%cmake_kf5
%cmake_build


%install
%cmake_install


%ldconfig_scriptlets

%files
%doc AUTHORS README
%license LICENSES/*.txt
%{_kf5_libdir}/libKF5Kipi.so.*
%{_kf5_datadir}/icons/hicolor/*/*/*
%{_kf5_datadir}/kservicetypes5/kipiplugin.desktop
%{_kf5_datadir}/qlogging-categories5/kipi.categories

%files devel
%{_kf5_libdir}/libKF5Kipi.so
#{_kf5_includedir}/libkipi_version.h
%{_kf5_includedir}/KIPI/
%{_kf5_libdir}/cmake/KF5Kipi/


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

