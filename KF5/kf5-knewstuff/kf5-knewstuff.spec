%global framework knewstuff

Name:    kf5-%{framework}
Version: 5.111.0
Release: 1%{?dist}
Summary: KDE Frameworks 5 Tier 3 module for downloading application assets

License: LGPLv2+
URL:     https://cgit.kde.org/%{framework}.git

%global majmin %(echo %{version} | cut -d. -f1-2)
%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0: http://download.kde.org/%{stable}/frameworks/%{majmin}/%{framework}-%{version}.tar.xz
Source1: http://download.kde.org/%{stable}/frameworks/%{majmin}/%{framework}-%{version}.tar.xz.sig
Source2: gpgkey-53E6B47B45CEA3E0D5B7457758D0EE648A48B3BB.gpg

## upstream patches

# filter qml provides
%global __provides_exclude_from ^%{_kf5_qmldir}/.*\\.so$

BuildRequires:  gnupg2
BuildRequires:  extra-cmake-modules >= %{majmin}
BuildRequires:  kf5-attica-devel >= %{majmin}
BuildRequires:  kf5-karchive-devel >= %{majmin}
BuildRequires:  kf5-kcompletion-devel >= %{majmin}
BuildRequires:  kf5-kconfig-devel >= %{majmin}
BuildRequires:  kf5-kcoreaddons-devel >= %{majmin}
BuildRequires:  kf5-ki18n-devel >= %{majmin}
BuildRequires:  kf5-kiconthemes-devel >= %{majmin}
BuildRequires:  kf5-kio-devel >= %{majmin}
BuildRequires:  kf5-kitemviews-devel >= %{majmin}
BuildRequires:  kf5-kservice-devel >= %{majmin}
BuildRequires:  kf5-ktextwidgets-devel >= %{majmin}
BuildRequires:  kf5-kwidgetsaddons-devel >= %{majmin}
BuildRequires:  kf5-kwindowsystem-devel >= %{majmin}
BuildRequires:  kf5-kxmlgui-devel >= %{majmin}
BuildRequires:  kf5-kpackage-devel >= %{majmin}
BuildRequires:  kf5-rpm-macros
BuildRequires:  qt5-qtbase-devel
BuildRequires:  qt5-qtdeclarative-devel

BuildRequires:  cmake(Qt5UiPlugin)
BuildRequires:  cmake(KF5Syndication)

%description
KDE Frameworks 5 Tier 3 module for downloading and sharing additional
application data like plugins, themes, motives, etc.

%package        devel
Summary:        Development files for %{name}
Requires:       %{name}%{?_isa} = %{version}-%{release}
Requires:       kf5-attica-devel >= %{majmin}
Requires:       kf5-kservice-devel >= %{majmin}
Requires:       kf5-kxmlgui-devel >= %{majmin}
Requires:       qt5-qtbase-devel
%description    devel
The %{name}-devel package contains libraries and header files for
developing applications that use %{name}.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -n %{framework}-%{version} -p1


%build
%cmake_kf5
%cmake_build


%install
%cmake_install

%find_lang %{name} --all-name


%ldconfig_scriptlets

%files -f %{name}.lang
%doc README.md
%license LICENSES/*.txt
%{_kf5_bindir}/knewstuff-dialog
%{_kf5_libdir}/libKF5NewStuff.so.*
%{_kf5_libdir}/libKF5NewStuffCore.so.*
%{_kf5_libdir}/libKF5NewStuffWidgets.so.*
%{_kf5_qtplugindir}/designer/knewstuffwidgets.so
%{_kf5_datadir}/kf5/kmoretools/
%{_kf5_datadir}/qlogging-categories5/%{framework}*
%{_kf5_datadir}/applications/org.kde.knewstuff-dialog.desktop
%dir %{_kf5_qmldir}/org/
%dir %{_kf5_qmldir}/org/kde
%{_kf5_qmldir}/org/kde/newstuff/

%files devel
%{_kf5_includedir}/KNewStuff3/
%{_kf5_includedir}/KMoreTools/
%{_kf5_libdir}/libKF5NewStuff.so
%{_kf5_libdir}/libKF5NewStuffCore.so
%{_kf5_libdir}/libKF5NewStuffWidgets.so
%{_kf5_libdir}/cmake/KF5NewStuff/
%{_kf5_libdir}/cmake/KF5NewStuffCore/
%{_kf5_libdir}/cmake/KF5NewStuffQuick/
%{_kf5_archdatadir}/mkspecs/modules/qt_KNewStuff.pri
%{_kf5_archdatadir}/mkspecs/modules/qt_KNewStuffCore.pri


%changelog
* Thu Oct 19 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.111.0-1
- 5.111.0

* Sun Sep 10 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.110.0-1
- 5.110.0

* Fri Aug 18 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.109.0-1
- 5.109.0

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

