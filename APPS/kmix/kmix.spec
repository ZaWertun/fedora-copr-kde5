Name:    kmix 
Summary: KDE volume control 
Version: 23.04.3
Release: 1%{?dist}

%global version_major %(echo %{version} |cut -d. -f1)

# code is LGPLv2+ except for gui/osdwidget.* which is GPLv2+
# docs GFDL
License: GPLv2+ and GFDL
URL:     https://cgit.kde.org/%{name}.git

%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0: https://download.kde.org/%{stable}/release-service/%{version}/src/%{name}-%{version}.tar.xz
Source1: https://download.kde.org/%{stable}/release-service/%{version}/src/%{name}-%{version}.tar.xz.sig
Source2: gpgkey-D81C0CB38EB725EF6691C385BB463350D6EF31EF.gpg

## upstream patches

## upstreamable patches
# disable autostart by default (on newer plasma releases that use plasma-pa)
Patch2:  kmix-21.04.0-autostart_disable.patch

BuildRequires: gnupg2
BuildRequires: desktop-file-utils
BuildRequires: libappstream-glib

BuildRequires: pkgconfig(alsa)
# FIXME/TODO: kf5 build seems to expects libcanberra cmake support, update? -- rex
BuildRequires: pkgconfig(libcanberra)
BuildRequires: pkgconfig(libpulse) pkgconfig(libpulse-mainloop-glib)

BuildRequires: extra-cmake-modules
BuildRequires: kf5-rpm-macros
BuildRequires: kf5-kconfigwidgets-devel
BuildRequires: kf5-kglobalaccel-devel
BuildRequires: kf5-ki18n-devel
BuildRequires: kf5-kcmutils-devel
BuildRequires: kf5-kdbusaddons-devel
BuildRequires: kf5-kiconthemes-devel
BuildRequires: kf5-kxmlgui-devel
BuildRequires: kf5-kdelibs4support-devel
BuildRequires: kf5-kinit-devel >= 5.10.0-3
%{?kf5_kinit_requires}
BuildRequires: kf5-plasma-devel

BuildRequires: pkgconfig(Qt5Gui)

# when split occurred
Obsoletes: kdemultimedia-kmix < 6:4.8.80
Provides:  kdemultimedia-kmix = 6:%{version}-%{release}

# translations moved here
Conflicts: kde-l10n < 17.03

%description
%{summary}.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%setup -q

%patch2 -p1 -b .autostart_disable


%build
%cmake_kf5
%cmake_build


%install
%cmake_install

%find_lang %{name} --all-name --with-html --with-man


%check
appstream-util validate-relax --nonet %{buildroot}%{_kf5_metainfodir}/org.kde.kmix.appdata.xml
desktop-file-validate %{buildroot}%{_datadir}/applications/org.kde.kmix.desktop


%files -f %{name}.lang
%doc AUTHORS ChangeLog TODO
%license COPYING
%{_datadir}/dbus-1/interfaces/org.kde.kmix.control.xml
%{_datadir}/dbus-1/interfaces/org.kde.kmix.mixer.xml
%{_datadir}/dbus-1/interfaces/org.kde.kmix.mixset.xml
%{_datadir}/icons/hicolor/*/*/kmix.*
%{_kf5_bindir}/kmix
%{_kf5_bindir}/kmixctrl
%{_kf5_bindir}/kmixremote
%{_kf5_datadir}/applications/org.kde.kmix.desktop
%{_kf5_metainfodir}/org.kde.kmix.appdata.xml
%{_kf5_datadir}/kmix/
%{_kf5_datadir}/kxmlgui5/kmix/
%{_sysconfdir}/xdg/autostart/restore_kmix_volumes.desktop
%{_sysconfdir}/xdg/autostart/kmix_autostart.desktop
%{_kf5_datadir}/knotifications5/kmix.notifyrc
%{_kf5_datadir}/kservices5/kmixctrl_restore.desktop
%{_kf5_datadir}/config.kcfg/%{name}settings.kcfg
%{_kf5_datadir}/qlogging-categories5/%{name}.categories
# -libs subpkg?
%{_kf5_libdir}/libkmixcore.so.5*
%{_kf5_libdir}/libkmixcore.so.%{version_major}*
%{_kf5_plugindir}/kded/kmixd.so


%changelog
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

