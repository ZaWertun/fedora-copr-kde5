%global framework libkcddb

Name:    kf5-%{framework}
Version: 23.04.3
Release: 1%{?dist}
Summary: CDDB retrieval library

License: LGPLv2+ and GPLv2+
URL:     https://www.kde.org/applications/multimedia/

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
BuildRequires: kf5-rpm-macros
BuildRequires: cmake(KF5Config)
BuildRequires: cmake(KF5DocTools)
BuildRequires: cmake(KF5Codecs)
BuildRequires: cmake(KF5I18n)
BuildRequires: cmake(KF5KIO)
BuildRequires: cmake(KF5WidgetsAddons)
BuildRequires: cmake(KF5KCMUtils)

BuildRequires: pkgconfig(Qt5Network)
BuildRequires: pkgconfig(Qt5Widgets)

BuildRequires: pkgconfig(libmusicbrainz5)

Requires:  %{name}-doc = %{version}-%{release}

%if 0%{?fedora} || 0%{?rhel} > 7
# kcmshell5
Recommends:   kde-cli-tools
%endif

# translations moved here
Conflicts: kde-l10n < 17.03

%description
%{summary}.

%package devel
Summary:  Development files for %{name}
Requires: %{name}%{?_isa} = %{version}-%{release}
%description devel
%{summary}.

%package doc
Summary: Documentation for %{name}
License: GFDL
Requires:  %{name} = %{version}-%{release}
BuildArch: noarch
%description doc
Documentation for %{name}.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -n %{framework}-%{version} -p1


%build
%if 0%{?rhel} && 0%{?rhel} < 8
CXXFLAGS="%{optflags} $(pkg-config --cflags libmusicbrainz5)"
export CXXFLAGS
%endif

%cmake_kf5
%cmake_build


%install
%cmake_install
%find_lang %{name} --all-name --with-man

%if 0%{?fedora} || 0%{?rhel} > 7
%find_lang %{name}-doc --all-name --with-html --without-mo
%else
echo '%{_kf5_docdir}/HTML/*/kcontrol' > %{name}-doc.lang
%endif

%ldconfig_scriptlets

%files -f %{name}.lang
%license LICENSES/*.txt
%{_kf5_libdir}/libKF5Cddb.so.5*
%{_qt5_plugindir}/plasma/kcms/systemsettings_qwidgets/kcm_cddb.so
%{_kf5_datadir}/config.kcfg/libkcddb5.kcfg
%{_kf5_datadir}/applications/kcm_cddb.desktop
%{_kf5_datadir}/qlogging-categories5/%{framework}.categories

%files devel
%{_kf5_libdir}/libKF5Cddb.so
%{_includedir}/KCddb5/
%{_kf5_includedir}/KCddb/
%{_kf5_libdir}/cmake/KF5Cddb/
%{_qt5_archdatadir}/mkspecs/modules/qt_KCddb.pri

%files doc -f %{name}-doc.lang


%changelog
* Tue Jul 18 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.04.3-1
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

