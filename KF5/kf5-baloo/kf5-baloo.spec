%global         framework baloo

# uncomment to enable bootstrap mode
#global bootstrap 1

%if !0%{?bootstrap}
%global tests 1
%endif

Name:    kf5-%{framework}
Summary: A Tier 3 KDE Frameworks 5 module that provides indexing and search functionality
Version: 5.109.0
Release: 1%{?dist}

# libs are LGPL, tools are GPL
# KDE e.V. may determine that future LGPL/GPL versions are accepted
License: (LGPLv2 or LGPLv3) and (GPLv2 or GPLv3)
URL:     https://community.kde.org/Baloo

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

Source3:        97-kde-baloo-filewatch-inotify.conf
# shutdown script to explictly stop baloo_file on logout
Source4:        baloo_file_shutdown.sh

## upstreamable patches
Patch100:       kf5-baloo-5.98.0-disable-content-indexing-by-default.patch
# https://invent.kde.org/frameworks/baloo/-/merge_requests/131
Patch101:       use_the_FSID_as_the_device_identifier_where_possible.patch 

## upstream patches

BuildRequires:  gnupg2
BuildRequires:  extra-cmake-modules >= %{majmin}
BuildRequires:  kf5-kconfig-devel >= %{majmin}
BuildRequires:  kf5-kcoreaddons-devel >= %{majmin}
BuildRequires:  kf5-kcrash-devel >= %{majmin}
BuildRequires:  kf5-kdbusaddons-devel >= %{majmin}
BuildRequires:  kf5-kfilemetadata-devel >= %{majmin}
BuildRequires:  kf5-ki18n-devel >= %{majmin}
BuildRequires:  kf5-kidletime-devel >= %{majmin}
BuildRequires:  kf5-kio-devel >= %{majmin}
BuildRequires:  kf5-rpm-macros >= %{majmin}
BuildRequires:  kf5-solid-devel >= %{majmin}

BuildRequires:  lmdb-devel
BuildRequires:  qt5-qtbase-devel
BuildRequires:  qt5-qtdeclarative-devel

BuildRequires:  systemd-rpm-macros

%if 0%{?tests}
BuildRequires: dbus-x11
BuildRequires: time
BuildRequires: xorg-x11-server-Xvfb
%endif

Obsoletes:      kf5-baloo-tools < 5.5.95-1
Provides:       kf5-baloo-tools = %{version}-%{release}

%if 0%{?fedora}
Obsoletes:      baloo < 5
Provides:       baloo = %{version}-%{release}
%else
Conflicts:      baloo < 5
%endif

# main pkg accidentally multilib'd prior to 5.21.0-4
Obsoletes:      kf5-baloo < 5.21.0-4

%description
%{summary}.

%package        devel
Summary:        Development files for %{name}
# KDE e.V. may determine that future LGPL versions are accepted
License:        LGPLv2 or LGPLv3
Requires:       %{name}-libs%{?_isa} = %{version}-%{release}
Requires:       kf5-kcoreaddons-devel >= %{majmin}
Requires:       kf5-kfilemetadata-devel >= %{majmin}
Requires:       qt5-qtbase-devel

%description    devel
The %{name}-devel package contains libraries and header files for
developing applications that use %{name}.

%package        file
Summary:        File indexing and search for Baloo
# KDE e.V. may determine that future LGPL versions are accepted
License:        LGPLv2 or LGPLv3
%if 0%{?fedora}
Obsoletes:      baloo-file < 5.0.1-2
Provides:       baloo-file = %{version}-%{release}
%else
Conflicts:      baloo-file < 5
%endif
Requires:       %{name}-libs%{?_isa} = %{version}-%{release}
%description    file
%{summary}.

%package        libs
Summary:        Runtime libraries for %{name}
# KDE e.V. may determine that future LGPL versions are accepted
License:        LGPLv2 or LGPLv3
%description    libs
%{summary}.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -n %{framework}-%{version} -p1


%build
%cmake_kf5 \
  -DBUILD_TESTING:BOOL=%{?tests:ON}%{!?tests:OFF}
%cmake_build


%install
%cmake_install

# baloodb not installed unless BUILD_EXPERIMENTAL is enabled, so omit translations
rm -fv %{buildroot}%{_datadir}/locale/*/LC_MESSAGES/baloodb5.*

install -p -m644 -D %{SOURCE3} %{buildroot}%{_prefix}/lib/sysctl.d/97-kde-baloo-filewatch-inotify.conf
install -p -m755 -D %{SOURCE4} %{buildroot}%{_sysconfdir}/xdg/plasma-workspace/shutdown/baloo_file.sh

%find_lang %{name} --all-name --with-kde


%check
%if 0%{?tests}
export CTEST_OUTPUT_ON_FAILURE=1
xvfb-run -a \
dbus-launch --exit-with-session \
time \
make test ARGS="--output-on-failure --timeout 300" -C %{_target_platform} ||:
%endif


%post
%systemd_user_post kde-%{framework}.service


%preun
%systemd_user_preun kde-%{framework}.service


%files -f %{name}.lang
%license LICENSES/*.txt
%{_kf5_bindir}/baloosearch
%{_kf5_bindir}/balooshow
%{_kf5_bindir}/balooctl
%{_kf5_datadir}/qlogging-categories5/%{framework}*

%files file
%{_prefix}/lib/sysctl.d/97-kde-baloo-filewatch-inotify.conf
%config(noreplace) %{_sysconfdir}/xdg/plasma-workspace/shutdown/baloo_file.sh
%{_kf5_bindir}/baloo_file
%{_kf5_bindir}/baloo_file_extractor
%config(noreplace) %{_kf5_sysconfdir}/xdg/autostart/baloo_file.desktop
%{_userunitdir}/kde-%{framework}.service
%{_libexecdir}/baloo_file
%{_libexecdir}/baloo_file_extractor

%ldconfig_scriptlets libs

%files libs
%license LICENSES/*.txt
%{_kf5_libdir}/libKF5Baloo.so.*
%{_kf5_libdir}/libKF5BalooEngine.so.*
# multilib'd plugins and friends
%{_kf5_plugindir}/kio/baloosearch.so
%{_kf5_plugindir}/kio/tags.so
%{_kf5_plugindir}/kio/timeline.so
%{_kf5_plugindir}/kded/baloosearchmodule.so
%{_kf5_qmldir}/org/kde/baloo

%files devel
%{_kf5_libdir}/libKF5Baloo.so
%{_kf5_libdir}/cmake/KF5Baloo/
%{_kf5_libdir}/pkgconfig/Baloo.pc
%{_kf5_includedir}/Baloo/
%{_kf5_archdatadir}/mkspecs/modules/qt_Baloo.pri
%{_kf5_datadir}/dbus-1/interfaces/org.kde.baloo.*.xml
%{_kf5_datadir}/dbus-1/interfaces/org.kde.Baloo*.xml


%changelog
* Fri Aug 18 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.109.0-1
- 5.109.0

* Sun Jul 09 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.108.0-1
- 5.108.0

* Wed Jun 14 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.107.0-2
- merged changes from official package

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

