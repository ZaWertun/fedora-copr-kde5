%global framework kitemviews

Name:           kf5-%{framework}
Version: 5.107.0
Release: 1%{?dist}
Summary:        KDE Frameworks 5 Tier 1 addon with item views

License:        GPLv2+ and LGPLv2+
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

BuildRequires:  gnupg2
BuildRequires:  extra-cmake-modules >= %{majmin}
BuildRequires:  kf5-rpm-macros >= %{majmin}
BuildRequires:  qt5-qtbase-devel
BuildRequires:  qt5-qttools-devel

Requires:       kf5-filesystem >= %{majmin}

%description
KDE Frameworks 5 Tier 1 addon with item views.


%package        devel
Summary:        Development files for %{name}
Requires:       %{name}%{?_isa} = %{version}-%{release}
Requires:       qt5-qtbase-devel
%description    devel
The %{name}-devel package contains libraries and header files for
developing applications that use %{name}.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -n %{framework}-%{version}


%build
mkdir -p %{_target_platform}
%cmake_kf5
%cmake_build


%install
%cmake_install

%find_lang_kf5 kitemviews5_qt


%ldconfig_scriptlets

%files -f kitemviews5_qt.lang
%doc README.md
%license LICENSES/*.txt
%{_kf5_libdir}/libKF5ItemViews.so.*
%{_kf5_qtplugindir}/designer/*.so
%{_kf5_datadir}/qlogging-categories5/kitemviews.categories

%files devel
%{_kf5_includedir}/KItemViews/
%{_kf5_libdir}/libKF5ItemViews.so
%{_kf5_libdir}/cmake/KF5ItemViews/
%{_kf5_archdatadir}/mkspecs/modules/qt_KItemViews.pri


%changelog
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

