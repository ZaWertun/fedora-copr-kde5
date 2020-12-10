%undefine __cmake_in_source_build
%global min_qt_version 5.14.0

Name:    disman
Version: 0.520.1
Release: 1%{?dist}
Summary: Qt/C++ display management library

%global  real_version %(echo %{version} |sed 's/~/-/')
License: LGPLv2+
URL:     https://gitlab.com/kwinft/%{name}
Source0: %{url}/-/archive/%{name}@%{real_version}/%{name}-%{name}@%{real_version}.tar.bz2

# filter plugin provides
%global __provides_exclude_from ^(%{_kf5_qtplugindir}/.*\\.so)$

BuildRequires:  extra-cmake-modules
BuildRequires:  kf5-rpm-macros

BuildRequires:  qt5-qtbase-devel        >= %{min_qt_version}
BuildRequires:  qt5-qtx11extras-devel   >= %{min_qt_version}
BuildRequires:  qt5-qtdeclarative-devel >= %{min_qt_version}
BuildRequires:  cmake(Qt5Sensors)       >= %{min_qt_version}

BuildRequires:  kf5-ki18n-devel
BuildRequires:  kf5-kcoreaddons-devel
BuildRequires:  kf5-kconfigwidgets-devel
BuildRequires:  kf5-kdbusaddons-devel
BuildRequires:  kf5-kxmlgui-devel
BuildRequires:  kf5-kglobalaccel-devel
BuildRequires:  cmake(KF5Declarative)
BuildRequires:  cmake(KF5IconThemes)
BuildRequires:  cmake(KF5Plasma)
BuildRequires:  cmake(KF5Wayland)
BuildRequires:  cmake(KF5KCMUtils)

BuildRequires:  wrapland-client-devel >= 0.520.0
BuildRequires:  wrapland-server-devel >= 0.520.0

Requires:       kf5-filesystem
Requires:       qt5-qtgraphicaleffects

%description
%{summary}.

%package        devel
Summary:        Development files for %{name}
Requires:       %{name}%{?_isa} = %{version}-%{release}
%description    devel
Headers, development libraries and documentation for %{name}.

%prep
%autosetup -p1 -n %{name}-%{name}@%{real_version}


%build
%{cmake_kf5}
%cmake_build


%install
%cmake_install
%find_lang %{name} --with-kde --with-qt --all-name ||:
echo >> %{name}.lang


%files -f %{name}.lang
%license COPYING.LIB
%{_bindir}/dismanctl
%{_libdir}/libDisman.so.*
%{_libdir}/libDismanWayland.so.*
%{_datadir}/dbus-1/services/org.kwinft.disman.service
%{_libexecdir}/disman_backend_launcher
%{_kf5_qtplugindir}/disman/*.so
%{_kf5_qtplugindir}/disman/wayland/*.so
%{_kf5_datadir}/qlogging-categories5/Disman.categories


%files devel
%{_libdir}/libDisman.so
%{_libdir}/libDismanWayland.so
%{_libdir}/cmake/Disman/Disman*.cmake
%{_libdir}/cmake/DismanWayland/DismanWayland*.cmake
%{_libdir}/pkgconfig/disman.pc
%{_libdir}/pkgconfig/disman-wayland.pc
%{_includedir}/disman/
%{_includedir}/disman_version.h
%{_includedir}/disman_wayland_version.h
%{_kf5_archdatadir}/mkspecs/modules/qt_Disman.pri


%changelog
* Thu Dec 10 17:00:46 MSK 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 0.520.1-1
- version 0.520.1

* Wed Oct 14 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 0.520.0-1
- 0.520.0

* Tue Jun 16 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 0.519.0-1
- version 0.519.0

* Mon May 25 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 0.519.0~beta.0-1
- first spec for version 0.519.0-beta.0

