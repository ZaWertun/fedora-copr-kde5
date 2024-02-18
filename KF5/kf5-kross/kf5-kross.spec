%global framework kross

Name:    kf5-%{framework}
Version: 5.115.0
Release: 1%{?dist}
Summary: KDE Frameworks 5 Tier 3 solution for multi-language application scripting

License: LGPLv2+
URL:     https://cgit.kde.org/%{framework}.git

%global majmin %(echo %{version} | cut -d. -f1-2)
%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0: http://download.kde.org/%{stable}/frameworks/%{majmin}/portingAids/%{framework}-%{version}.tar.xz
Source1: http://download.kde.org/%{stable}/frameworks/%{majmin}/portingAids/%{framework}-%{version}.tar.xz.sig
Source2: gpgkey-53E6B47B45CEA3E0D5B7457758D0EE648A48B3BB.gpg

BuildRequires:  gnupg2
BuildRequires:  extra-cmake-modules >= %{majmin}
BuildRequires:  kf5-kcompletion-devel >= %{majmin}
BuildRequires:  kf5-kcoreaddons-devel >= %{majmin}
BuildRequires:  kf5-kdoctools-devel >= %{majmin}
BuildRequires:  kf5-ki18n-devel >= %{majmin}
BuildRequires:  kf5-kiconthemes-devel >= %{majmin}
BuildRequires:  kf5-kio-devel >= %{majmin}
BuildRequires:  kf5-kparts-devel >= %{majmin}
BuildRequires:  kf5-kwidgetsaddons-devel >= %{majmin}
BuildRequires:  kf5-kxmlgui-devel >= %{majmin}
BuildRequires:  kf5-rpm-macros
BuildRequires:  qt5-qtbase-devel
BuildRequires:  qt5-qtscript-devel
BuildRequires:  qt5-qttools-static

Requires:       %{name}-core%{_isa} = %{version}-%{release}
Requires:       %{name}-ui%{?_isa} = %{version}-%{release}

%description
Kross is a scripting bridge to embed scripting functionality into an
application. It supports QtScript as a scripting interpreter backend.

%package        devel
Summary:        Development files for %{name}
Requires:       %{name}%{?_isa} = %{version}-%{release}
Requires:       kf5-ki18n-devel >= %{majmin}
Requires:       kf5-kiconthemes-devel >= %{majmin}
Requires:       kf5-kio-devel >= %{majmin}
Requires:       kf5-kparts-devel >= %{majmin}
Requires:       kf5-kwidgetsaddons-devel >= %{majmin}
Requires:       qt5-qtbase-devel
Requires:       qt5-qtscript-devel
%description    devel
The %{name}-devel package contains libraries and header files for
developing applications that use %{name}.

%package        core
Summary:        Non-gui part of the Kross framework
%description    core
Non-gui part of the Kross framework.

%package        ui
Summary:        Gui part of the Kross framework
Requires:       %{name}-core%{?_isa} = %{version}-%{release}
%description    ui
Gui part of the Kross framework.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -n %{framework}-%{version}


%build
%cmake_kf5
%cmake_build


%install
%cmake_install
%find_lang %{name} --all-name --with-man


%files
# empty

%ldconfig_scriptlets core

%files core -f %{name}.lang
%{_kf5_bindir}/kf5kross
%{_kf5_mandir}/man1/kf5kross.1*
%{_kf5_libdir}/libKF5KrossCore.so.*
%{_kf5_qtplugindir}/krossqts.so
%{_kf5_qtplugindir}/script/krossqtsplugin.so

%ldconfig_scriptlets ui

%files ui
%{_kf5_libdir}/libKF5KrossUi.so.*
%{_kf5_qtplugindir}/krossmoduleforms.so
%{_kf5_qtplugindir}/krossmodulekdetranslation.so

%files devel
%{_kf5_includedir}/kross_version.h
%{_kf5_includedir}/KrossUi/
%{_kf5_includedir}/KrossCore/
%{_kf5_libdir}/libKF5KrossCore.so
%{_kf5_libdir}/libKF5KrossUi.so
%{_kf5_libdir}/cmake/KF5Kross/
%{_kf5_archdatadir}/mkspecs/modules/qt_KrossCore.pri
%{_kf5_archdatadir}/mkspecs/modules/qt_KrossUi.pri


%changelog
* Sun Feb 18 2024 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.115.0-1
- 5.115.0

* Fri Jan 19 2024 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.114.0-1
- 5.114.0

* Fri Dec 15 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.113.0-1
- 5.113.0

* Sun Nov 12 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.112.0-1
- 5.112.0

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

