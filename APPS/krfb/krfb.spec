Name:    krfb
Summary: Desktop sharing
Version: 23.08.2
Release: 1%{?dist}

License: GPLv2+ and GFDL
URL:     https://www.kde.org/applications/system/krfb/

%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0: https://download.kde.org/%{stable}/release-service/%{version}/src/%{name}-%{version}.tar.xz
Source1: https://download.kde.org/%{stable}/release-service/%{version}/src/%{name}-%{version}.tar.xz.sig
Source2: gpgkey-D81C0CB38EB725EF6691C385BB463350D6EF31EF.gpg

BuildRequires: gnupg2
BuildRequires: desktop-file-utils
BuildRequires: extra-cmake-modules
BuildRequires: gettext
BuildRequires: kf5-kcompletion-devel
BuildRequires: kf5-kconfig-devel
BuildRequires: kf5-kcoreaddons-devel
BuildRequires: kf5-kcrash-devel
BuildRequires: kf5-kdoctools-devel
BuildRequires: kf5-kdbusaddons-devel
BuildRequires: kf5-kdnssd-devel
BuildRequires: kf5-ki18n-devel
BuildRequires: kf5-knotifications-devel
BuildRequires: kf5-kwallet-devel
BuildRequires: kf5-kwidgetsaddons
BuildRequires: kf5-kxmlgui-devel

BuildRequires: cmake(Qt5DBus)
BuildRequires: cmake(Qt5X11Extras)
BuildRequires: cmake(Qt5WaylandClient)
BuildRequires: cmake(Qt5XkbCommonSupport)

BuildRequires: cmake(KF5WindowSystem)
BuildRequires: cmake(KF5Wayland)
BuildRequires: cmake(PlasmaWaylandProtocols)

BuildRequires: qt5-qtbase-static
BuildRequires: pkgconfig(libvncserver)
BuildRequires: pkgconfig(x11)
BuildRequires: pkgconfig(xcb)
BuildRequires: pkgconfig(xcb-damage)
BuildRequires: pkgconfig(xcb-image)
BuildRequires: pkgconfig(xcb-render)
BuildRequires: pkgconfig(xcb-shape)
BuildRequires: pkgconfig(xcb-shm)
BuildRequires: pkgconfig(xcb-xfixes)
BuildRequires: pkgconfig(xdamage)
BuildRequires: pkgconfig(wayland-client)
BuildRequires: pkgconfig(gbm)
BuildRequires: pkgconfig(epoxy)

BuildRequires: pkgconfig(xtst)
BuildRequires: libjpeg-devel
BuildRequires: pipewire-devel
BuildRequires: zlib-devel
BuildRequires: libvncserver-devel

BuildRequires: cmake(KPipeWire)

Requires: %{name}-libs%{?_isa} = %{version}-%{release}

# when split occurred
Conflicts: kdenetwork-common < 7:4.10.80
Obsoletes: kdenetwork-krfb < 7:4.10.80
Provides:  kdenetwork-krfb = 7:%{version}-%{release}

# translations moved here
Conflicts: kde-l10n < 17.03

%description
%{summary}.

%package libs
Summary: Runtime libraries for %{name}
Requires: %{name} = %{version}-%{release}
Obsoletes: kdenetwork-krfb-libs < 7:4.10.80
Provides:  kdenetwork-krfb-libs = 7:%{version}-%{release}
%description libs
%{summary}.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -p1


%build
%cmake_kf5
%cmake_build


%install
%cmake_install
%find_lang %{name} --all-name --with-html


%check
desktop-file-validate %{buildroot}%{_kf5_datadir}/applications/org.kde.krfb.desktop
desktop-file-validate %{buildroot}%{_kf5_datadir}/applications/org.kde.krfb.virtualmonitor.desktop


%files -f %{name}.lang
%license COPYING*
%doc README AUTHORS
%{_kf5_bindir}/krfb
%{_kf5_bindir}/krfb-virtualmonitor
%{_kf5_datadir}/krfb/
%{_kf5_datadir}/applications/org.kde.krfb.desktop
%{_kf5_datadir}/applications/org.kde.krfb.virtualmonitor.desktop
%{_kf5_metainfodir}/org.kde.krfb.appdata.xml
%{_datadir}/icons/hicolor/*/apps/krfb.*
%{_kf5_datadir}/qlogging-categories5/%{name}.categories

%ldconfig_scriptlets libs

%files libs
%{_kf5_libdir}/libkrfbprivate.so.5*
%{_kf5_qtplugindir}/krfb/


%changelog
* Fri Oct 13 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.08.2-1
- 23.08.2

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

