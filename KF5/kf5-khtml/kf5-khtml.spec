%global framework khtml

Name:           kf5-%{framework}
Version: 5.111.0
Release: 1%{?dist}
Summary:        KDE Frameworks 5 Tier 4 solution with KHTML, a HTML rendering engine

License:        LGPLv2+ and GPLv3 and MIT and BSD
URL:            https://cgit.kde.org/%{framework}.git

%global versiondir %(echo %{version} | cut -d. -f1-2)
%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0:        http://download.kde.org/%{stable}/frameworks/%{versiondir}/portingAids/%{framework}-%{version}.tar.xz
Source1:        http://download.kde.org/%{stable}/frameworks/%{versiondir}/portingAids/%{framework}-%{version}.tar.xz.sig
Source2:        gpgkey-53E6B47B45CEA3E0D5B7457758D0EE648A48B3BB.gpg

# filter plugin provides
%global __provides_exclude_from ^(%{_kf5_plugindir}/.*\\.so)$

BuildRequires:  gnupg2
BuildRequires:  extra-cmake-modules >= %{version}
BuildRequires:  fdupes
BuildRequires:  giflib-devel
BuildRequires:  gperf
BuildRequires:  kf5-karchive-devel >= %{version}
BuildRequires:  kf5-kcodecs-devel >= %{version}
BuildRequires:  kf5-kglobalaccel-devel >= %{version}
BuildRequires:  kf5-ki18n-devel >= %{version}
BuildRequires:  kf5-kiconthemes-devel >= %{version}
BuildRequires:  kf5-kio-devel >= %{version}
BuildRequires:  kf5-kjs-devel >= %{version}
BuildRequires:  kf5-knotifications-devel >= %{version}
BuildRequires:  kf5-kparts-devel >= %{version}
BuildRequires:  kf5-ktextwidgets-devel >= %{version}
BuildRequires:  kf5-kwallet-devel >= %{version}
BuildRequires:  kf5-kwidgetsaddons-devel >= %{version}
BuildRequires:  kf5-kwindowsystem-devel >= %{version}
BuildRequires:  kf5-kxmlgui-devel >= %{version}
BuildRequires:  kf5-rpm-macros >= %{version}
BuildRequires:  kf5-sonnet-devel >= %{version}
BuildRequires:  libjpeg-devel
BuildRequires:  libpng-devel
BuildRequires:  openssl-devel
BuildRequires:  perl-interpreter perl(Getopt::Long)
BuildRequires:  phonon-qt5-devel
BuildRequires:  qt5-qtbase-devel
BuildRequires:  qt5-qtx11extras-devel
BuildRequires:  zlib-devel

%description
KHTML is a web rendering engine, based on the KParts technology and using KJS
for JavaScript support.

%package        devel
Summary:        Development files for %{name}
Requires:       %{name}%{?_isa} = %{version}-%{release}
Requires:       kf5-ki18n-devel >= %{version}
Requires:       kf5-kio-devel >= %{version}
Requires:       kf5-kjs-devel >= %{version}
Requires:       kf5-kparts-devel >= %{version}
Requires:       qt5-qtbase-devel
Requires:       kf5-ktextwidgets-devel >= %{version}
Requires:       kf5-kcodecs-devel >= %{version}
%description    devel
The %{name}-devel package contains libraries and header files for
developing applications that use %{name}.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -n %{framework}-%{version}

%build
%cmake_kf5 \
  -DCMAKE_SHARED_LINKER_FLAGS="-Wl,--as-needed"
%cmake_build


%install
%cmake_install

%find_lang khtml5_qt --with-qt --all-name


%ldconfig_scriptlets

%files -f khtml5_qt.lang
%doc README.md
%license COPYING.GPL3 COPYING.LIB
%{_kf5_libdir}/libKF5KHtml.so.*
%dir %{_kf5_plugindir}/parts/
%{_kf5_plugindir}/parts/*.so
%{_kf5_datadir}/kf5/khtml/
%{_kf5_datadir}/kservices5/*.desktop
%config %{_kf5_sysconfdir}/xdg/khtmlrc
%{_kf5_datadir}/qlogging-categories5/khtml.categories
%{_kf5_datadir}/qlogging-categories5/khtml.renamecategories

%files devel
%{_kf5_libdir}/libKF5KHtml.so
%{_kf5_libdir}/cmake/KF5KHtml/
%{_kf5_includedir}/KHtml/
%{_kf5_archdatadir}/mkspecs/modules/qt_KHtml.pri


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

