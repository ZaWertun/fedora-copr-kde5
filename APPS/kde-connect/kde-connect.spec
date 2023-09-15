# enable experimental (default off) bluetooth support
#global bluetooth 1

%global module kdeconnect-kde

Name:           kde-connect
Version:        23.08.1
Release:        1%{?dist}
License:        GPLv2+
Summary:        KDE Connect client for communication with smartphones

%global version_major %(echo %{version} |cut -d. -f1)

Url:            https://community.kde.org/KDEConnect
#Url:            https://cgit.kde.org/kdeconnect-kde.git
%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0:        http://download.kde.org/%{stable}/release-service/%{version}/src/%{module}-%{version}.tar.xz
Source1:        http://download.kde.org/%{stable}/release-service/%{version}/src/%{module}-%{version}.tar.xz.sig
Source2:        gpgkey-D81C0CB38EB725EF6691C385BB463350D6EF31EF.gpg

# firewalld service definition, see https://bugzilla.redhat.com/show_bug.cgi?id=1257699#c2
Source3:        kde-connect.xml

## upstream patches

BuildRequires:  gnupg2
BuildRequires:  desktop-file-utils
BuildRequires:  firewalld-filesystem
BuildRequires:  libappstream-glib
BuildRequires:  gcc-c++

BuildRequires:  extra-cmake-modules >= 5.42
BuildRequires:  kf5-rpm-macros
BuildRequires:  cmake(KF5ConfigWidgets)
BuildRequires:  cmake(KF5DBusAddons)
BuildRequires:  cmake(KF5DocTools)
BuildRequires:  cmake(KF5I18n)
BuildRequires:  cmake(KF5IconThemes)
BuildRequires:  cmake(KF5KCMUtils)
BuildRequires:  cmake(KF5KIO)
BuildRequires:  cmake(KF5Kirigami2)
BuildRequires:  cmake(KF5Notifications)
BuildRequires:  cmake(KF5People)
BuildRequires:  cmake(KF5Service)
BuildRequires:  cmake(KF5Wayland)
BuildRequires:  cmake(KF5PeopleVCard)
BuildRequires:  cmake(KF5QQC2DesktopStyle)
BuildRequires:  cmake(KF5Package)
BuildRequires:  cmake(KF5GuiAddons)
BuildRequires:  cmake(KF5PulseAudioQt)
BuildRequires:  cmake(KF5ModemManagerQt)

%if 0%{?bluetooth}
BuildRequires:  cmake(Qt5Bluetooth)
%endif
BuildRequires:  cmake(Qt5DBus)
BuildRequires:  cmake(Qt5Multimedia)
BuildRequires:  cmake(Qt5Network)
BuildRequires:  cmake(Qt5Quick)
BuildRequires:  cmake(Qt5Test)
BuildRequires:  cmake(Qt5X11Extras)
BuildRequires:  cmake(Qt5QuickControls2)
BuildRequires:  cmake(Qt5WaylandClient)

BuildRequires:  qt5-qtbase-private-devel

BuildRequires:  cmake(Qca-qt5)

BuildRequires:  libXtst-devel
BuildRequires:  pkgconfig(xkbcommon)
BuildRequires:  pkgconfig(libfakekey)
BuildRequires:  pkgconfig(wayland-client)
BuildRequires:  cmake(PlasmaWaylandProtocols)
BuildRequires:  wayland-protocols-devel

Obsoletes: kde-connect-kde4-ioslave < %{version}-%{release}
Obsoletes: kde-connect-kde4-libs < %{version}-%{release}

# upstream name
Provides:       kdeconnect-kde = %{version}-%{release}

Requires:       %{name}-libs%{?_isa} = %{version}-%{release}
Requires:       kdeconnectd = %{version}-%{release}

Requires:       fuse-sshfs
Requires:       qca-qt5-ossl%{?_isa}
Requires:       kf5-kirigami2-addons%{?_isa}
# /usr/bin/plasmawindowed (make optional at least until this is split out for bug #1286431)
#Recommends:     plasma-workspace
# /usr/bin/kcmshell5
Requires:       kde-cli-tools

%description
KDE Connect adds communication between KDE and your smartphone.

Currently, you can pair with your Android devices over Wifi using the
KDE Connect 1.0 app from Albert Vaka which you can obtain via Google Play, F-Droid
or the project website.

%package -n kdeconnectd
Summary: KDE Connect service
Requires: %{name}-libs%{?_isa} = %{version}-%{release}
Requires: firewalld-filesystem
%description -n kdeconnectd
%{summary}.

%package libs
Summary: Runtime libraries for %{name}
# I think we may want to drop this, forces kdeconnectd to pull in main pkg indirectly -- rex
Requires: %{name} = %{version}-%{release}
%description libs
%{summary}.

%package devel
Summary: Development files for %{name}
Requires: %{name}-libs%{?_isa} = %{version}-%{release}
%description devel
%{summary}.

%package nautilus
Summary: KDEConnect extention for nautilus
Requires: kdeconnectd = %{version}-%{release}
Requires: nautilus-python
Supplements: (kdeconnectd and nautilus)
%description nautilus
%{summary}.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -n %{module}-%{version} -p1


%build
%cmake_kf5 \
  %{?bluetooth:-DBLUETOOTH_ENABLED:BOOL=ON}
%cmake_build


%install
%cmake_install

# firewalld as shipped in f31+ provides it's own kdeconnect.xml
%if 0%{?fedora} && 0%{?fedora} < 31
install -m644 -p -D %{SOURCE3} %{buildroot}%{_prefix}/lib/firewalld/services/kde-connect.xml
%endif

%find_lang %{name} --all-name --with-html

# https://bugzilla.redhat.com/show_bug.cgi?id=1296523
desktop-file-edit --remove-key=OnlyShowIn %{buildroot}%{_sysconfdir}/xdg/autostart/org.kde.kdeconnect.daemon.desktop


%check
appstream-util validate-relax --nonet %{buildroot}%{_kf5_metainfodir}/org.kde.kdeconnect.appdata.xml ||:
appstream-util validate-relax --nonet %{buildroot}%{_kf5_metainfodir}/org.kde.kdeconnect.metainfo.xml
desktop-file-validate %{buildroot}%{_datadir}/applications/kcm_kdeconnect.desktop
for i in %{buildroot}%{_datadir}/applications/org.kde.kdeconnect*.desktop ; do
desktop-file-validate $i ||:
done


%files -f %{name}.lang
%license LICENSES/*.txt
%{_kf5_bindir}/kdeconnect-*
%dir %{_kf5_datadir}/kdeconnect/
%{_kf5_datadir}/kdeconnect/kdeconnect_clipboard_config.qml
%{_kf5_datadir}/plasma/plasmoids/org.kde.kdeconnect/
%{_kf5_datadir}/knotifications5/*
%{_kf5_datadir}/kservices5/*.desktop
%{_kf5_datadir}/qlogging-categories5/kdeconnect-kde.categories
%{_qt5_plugindir}/plasma/kcms/systemsettings_qwidgets/kcm_kdeconnect.so
%{_kf5_plugindir}/kfileitemaction/kdeconnectfileitemaction.so
%{_kf5_plugindir}/kio/kdeconnect.so
%{_datadir}/icons/hicolor/*/apps/kdeconnect*
%{_datadir}/icons/hicolor/*/status/{laptop,smartphone,tablet,tv}{connected,disconnected,trusted}.svg
%{_kf5_metainfodir}/org.kde.kdeconnect.appdata.xml
%{_kf5_metainfodir}/org.kde.kdeconnect.metainfo.xml
%{_datadir}/applications/kcm_kdeconnect.desktop
%{_datadir}/applications/org.kde.kdeconnect*.desktop
%{_qt5_archdatadir}/qml/org/kde/kdeconnect/
%{_datadir}/contractor/
%{_datadir}/Thunar/
%{_datadir}/zsh/
%{_datadir}/deepin/

%post -n kdeconnectd
%{?firewalld_reload}

%postun -n kdeconnectd
if [ $1 -eq 0 ] ; then
%{?firewalld_reload}
fi

%files -n kdeconnectd
%{_sysconfdir}/xdg/autostart/org.kde.kdeconnect.daemon.desktop
%{_datadir}/applications/org.kde.kdeconnect.daemon.desktop
%caps(cap_net_admin=pe) %{_libexecdir}/kdeconnectd
 %{_datadir}/dbus-1/services/org.kde.kdeconnect.service
# firewalld as shipped in f31+ provides it's own kdeconnect.xml
%if 0%{?fedora} && 0%{?fedora} < 31
%{_prefix}/lib/firewalld/services/kde-connect.xml
%endif

%ldconfig_scriptlets libs

%files libs
%{_kf5_libdir}/libkdeconnectpluginkcm.so.%{version_major}*
%{_kf5_libdir}/libkdeconnectinterfaces.so.%{version_major}*
%{_kf5_libdir}/libkdeconnectcore.so.%{version_major}*
%{_qt5_plugindir}/kdeconnect/
%{_kf5_datadir}/kdeconnect/kdeconnect_findthisdevice_config.qml
%{_kf5_datadir}/kdeconnect/kdeconnect_pausemusic_config.qml
%{_kf5_datadir}/kdeconnect/kdeconnect_runcommand_config.qml
%{_kf5_datadir}/kdeconnect/kdeconnect_sendnotifications_config.qml
%{_kf5_datadir}/kdeconnect/kdeconnect_share_config.qml

%files nautilus
%{_datadir}/nautilus-python/extensions/kdeconnect-share.py*


%changelog
* Thu Sep 14 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.08.1-1
- 23.08.1

* Sun Aug 27 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.08.0-1
- 23.08.0

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

