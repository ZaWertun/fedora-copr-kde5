%global framework ki18n

Name:           kf5-%{framework}
Version:        5.108.0
Release:        1%{?dist}
Summary:        KDE Frameworks 5 Tier 1 addon for localization

License:        LGPLv2+
URL:            https://cgit.kde.org/%{framework}.git

%global majmin %(echo %{version} | cut -d. -f1-2)
%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0:        http://download.kde.org/%{stable}/frameworks/%{majmin}/%{framework}-%{version}.tar.xz
Source1:        http://download.kde.org/%{stable}/frameworks/%{majmin}/%{framework}-%{version}.tar.xz.sig
Source2:        gpgkey-53E6B47B45CEA3E0D5B7457758D0EE648A48B3BB.gpg

## upstream patches

# filter plugin provides
%global __provides_exclude_from ^(%{_kf5_plugindir}/.*\\.so)$

BuildRequires:  gnupg2
BuildRequires:  extra-cmake-modules >= %{majmin}
BuildRequires:  gettext
BuildRequires:  kf5-rpm-macros >= %{majmin}
BuildRequires:  perl-interpreter
BuildRequires:  python3
BuildRequires:  qt5-qtbase-devel
BuildRequires:  qt5-qtdeclarative-devel
BuildRequires:  qt5-qtscript-devel
BuildRequires:  qt5-qtbase-private-devel

Requires:       kf5-filesystem >= %{majmin}

%description
KDE Frameworks 5 Tier 1 addon for localization.

%package        devel
Summary:        Development files for %{name}
Requires:       %{name}%{?_isa} = %{version}-%{release}
Requires:       gettext
Requires:       python3
%description    devel
The %{name}-devel package contains libraries and header files for
developing applications that use %{name}.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -n %{framework}-%{version} -p1


%build
%cmake_kf5 \
   -DPYTHON_EXECUTABLE:PATH=%__python3
%cmake_build


%install
%cmake_install

%find_lang %{name} --all-name


%ldconfig_scriptlets

%files -f %{name}.lang
%doc README.md
%license LICENSES/*.txt
%{_kf5_libdir}/libKF5I18n.so.*
%{_kf5_libdir}/libKF5I18nLocaleData.so.*
%{_kf5_qmldir}/org/kde/i18n/localeData/
%{_kf5_datadir}/qlogging-categories5/*%{framework}*
%{_kf5_qtplugindir}/kf5/ktranscript.so
%lang(ca) %{_datadir}/locale/ca/LC_SCRIPTS/ki18n5/
%lang(ca@valencia) %{_datadir}/locale/ca@valencia/LC_SCRIPTS/ki18n5/
%lang(fi) %{_datadir}/locale/fi/LC_SCRIPTS/ki18n5/
%lang(gd) %{_datadir}/locale/gd/LC_SCRIPTS/ki18n5/
%lang(ja) %{_datadir}/locale/ja/LC_SCRIPTS/ki18n5/
%lang(ko) %{_datadir}/locale/ko/LC_SCRIPTS/ki18n5/
%lang(nb) %{_datadir}/locale/nb/LC_SCRIPTS/ki18n5/
%lang(nn) %{_datadir}/locale/nn/LC_SCRIPTS/ki18n5/
%lang(ru) %{_datadir}/locale/ru/LC_SCRIPTS/ki18n5/
%lang(sr) %{_datadir}/locale/sr/LC_SCRIPTS/ki18n5/
%lang(sr@ijekavian) %{_datadir}/locale/sr@ijekavian/LC_SCRIPTS/ki18n5/
%lang(sr@ijekavianlatin) %{_datadir}/locale/sr@ijekavianlatin/LC_SCRIPTS/ki18n5/
%lang(sr@latin) %{_datadir}/locale/sr@latin/LC_SCRIPTS/ki18n5/
%lang(sr) %{_datadir}/locale/uk/LC_SCRIPTS/ki18n5/

%files devel
%{_kf5_includedir}/KI18n/
%{_kf5_includedir}/KI18nLocaleData/
%{_kf5_libdir}/libKF5I18n.so
%{_kf5_libdir}/libKF5I18nLocaleData.so
%{_kf5_libdir}/cmake/KF5I18n/
%{_kf5_archdatadir}/mkspecs/modules/qt_KI18n.pri


%changelog
* Sun Jul 09 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.108.0-1
- 5.108.0

* Sat Jun 10 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.107.0-1
- 5.107.0

* Sat May 13 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.106.0-1
- 5.106.0

* Sat Apr 08 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.105.0-1
- 5.105.0

* Sat Mar 11 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.104.0-1
- 5.104.0

* Sun Feb 12 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.103.0-1
- 5.103.0

