# uncomment to enable bootstrap mode
#global bootstrap 1

%if !0%{?bootstrap}
#global tests 1
%endif

Name:    dolphin
Summary: KDE File Manager
Version: 23.04.3
Release: 1%{?dist}

License: GPLv2+
URL:     https://cgit.kde.org/%{name}.git/
%global revision %(echo %{version} | cut -d. -f3)
%global majmin_ver %(echo %{version} | cut -d. -f1,2)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0: https://download.kde.org/%{stable}/release-service/%{majmin_ver}.%{revision}/src/%{name}-%{version}.tar.xz
Source1: https://download.kde.org/%{stable}/release-service/%{majmin_ver}.%{revision}/src/%{name}-%{version}.tar.xz.sig
Source2: gpgkey-D81C0CB38EB725EF6691C385BB463350D6EF31EF.gpg

## upstream patches

BuildRequires:  gnupg2
BuildRequires:  systemd-rpm-macros
BuildRequires:  desktop-file-utils
BuildRequires:  qt5-qtbase-devel

BuildRequires:  extra-cmake-modules
BuildRequires:  kf5-rpm-macros
BuildRequires:  cmake(KF5DocTools)
BuildRequires:  cmake(KF5Init)
BuildRequires:  cmake(KF5KCMUtils)
BuildRequires:  cmake(KF5NewStuff)
BuildRequires:  cmake(KF5CoreAddons)
BuildRequires:  cmake(KF5I18n)
BuildRequires:  cmake(KF5DBusAddons)
BuildRequires:  cmake(KF5Bookmarks)
BuildRequires:  cmake(KF5Config)
BuildRequires:  cmake(KF5KIO)
BuildRequires:  cmake(KF5Parts)
BuildRequires:  cmake(KF5Solid)
BuildRequires:  cmake(KF5IconThemes)
BuildRequires:  cmake(KF5Completion)
BuildRequires:  cmake(KF5TextWidgets)
BuildRequires:  cmake(KF5Notifications)
BuildRequires:  cmake(KF5Crash)
BuildRequires:  cmake(KF5WindowSystem)

BuildRequires:  phonon-qt5-devel
BuildRequires:  cmake(Qt5X11Extras)

%if ! 0%{?bootstrap}
BuildRequires:  kf5-kactivities-devel
BuildRequires:  kf5-baloo-devel
BuildRequires:  kf5-kfilemetadata-devel
BuildRequires:  baloo-widgets-devel >= %{majmin_ver}
%endif

%if 0%{?tests}
BuildRequires: xorg-x11-server-Xvfb
# for %%check
BuildRequires:  libappstream-glib
%endif

Requires:       %{name}-libs%{?_isa} = %{version}-%{release}
%{?kf5_kinit_requires}

Recommends:     kio-fuse%{?_isa}
Recommends:     kio-extras%{?_isa}

# translations moved here
Conflicts: kde-l10n < 17.03

%description
%{summary}.

%package        libs
Summary:        Dolphin runtime libraries
Requires:       %{name} = %{version}-%{release}
%description    libs
%{summary}.

%package        devel
Summary:        Developer files for %{name}
Requires:       %{name}-libs%{?_isa} = %{version}-%{release}
Requires:       qt5-qtbase-devel%{?_isa}
Requires:       kf5-kio-devel%{?_isa}
%description    devel
%{summary}.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -n %{name}-%{version} -p1


%build
%cmake_kf5 \
  -DBUILD_TESTING:BOOL=%{?tests:ON}%{!?tests:OFF}
%cmake_build


%install
%cmake_install

%find_lang dolphin --with-html
%find_lang dolphin_servicemenuinstaller -f dolphin_servicemenuinstaller.lang


%check
appstream-util validate-relax --nonet %{buildroot}%{_kf5_metainfodir}/org.kde.%{name}.appdata.xml ||:
desktop-file-validate %{buildroot}%{_kf5_datadir}/applications/org.kde.%{name}.desktop ||:
%if 0%{?tests}
export CTEST_OUTPUT_ON_FAILURE=1
xvfb-run -a \
make test ARGS="--output-on-failure --timeout 10" -C %{_vpath_builddir} ||:
%endif


%post
%systemd_user_post plasma-%{name}.service


%preun
%systemd_user_preun plasma-%{name}.service


%files -f dolphin.lang -f dolphin_servicemenuinstaller.lang
%license COPYING*
%doc README.md
%{_kf5_bindir}/dolphin
%{_kf5_bindir}/servicemenuinstaller
%dir %{_kf5_datadir}/%{name}/
%{_kf5_datadir}/%{name}/dolphinpartactions.desktop
%{_kf5_datadir}/config.kcfg/dolphin_*
%{_kf5_datadir}/kglobalaccel/*.desktop
%{_datadir}/dbus-1/services/org.kde.dolphin.FileManager1.service
%{_kf5_metainfodir}/org.kde.%{name}.appdata.xml
%{_kf5_datadir}/applications/org.kde.%{name}.desktop
%lang(fi) %{_datadir}/locale/fi/LC_SCRIPTS/dolphin/
%{_kf5_datadir}/qlogging-categories5/*.categories
%{_kf5_datadir}/knsrcfiles/*.knsrc
%{_userunitdir}/plasma-%{name}.service
%{_kf5_datadir}/kconf_update/dolphin_detailsmodesettings.upd
%{_kf5_datadir}/zsh/site-functions/_%{name}

%ldconfig_scriptlets libs

%files libs
%{_kf5_libdir}/libdolphinprivate.so.*
%{_kf5_libdir}/libdolphinvcs.so.*
%{_kf5_plugindir}/parts/%{name}part.so
%{_kf5_qtplugindir}/dolphin/kcms/

%files devel
%{_includedir}/Dolphin/
%{_includedir}/dolphin*_export.h
%{_kf5_libdir}/cmake/DolphinVcs/
%{_kf5_libdir}/libdolphinvcs.so
%{_datadir}/dbus-1/interfaces/org.freedesktop.FileManager1.xml


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

