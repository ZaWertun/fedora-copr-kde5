%global tests 1

Name:    kalarm
Summary: Personal Alarm Scheduler
Version: 23.08.0
Release: 1%{?dist}

# code (generally) GPLv2, docs GFDL
License: GPLv2 and GFDL
URL:     https://www.kde.org/applications/utilities/kalarm

%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0: https://download.kde.org/%{stable}/release-service/%{version}/src/%{name}-%{version}.tar.xz
Source1: https://download.kde.org/%{stable}/release-service/%{version}/src/%{name}-%{version}.tar.xz.sig
Source2: gpgkey-D81C0CB38EB725EF6691C385BB463350D6EF31EF.gpg

# handled by qt5-srpm-macros, which defines %%qt5_qtwebengine_arches
%{?qt5_qtwebengine_arches:ExclusiveArch: %{qt5_qtwebengine_arches}}

BuildRequires: gnupg2
BuildRequires: boost-devel
BuildRequires: desktop-file-utils
BuildRequires: gettext
BuildRequires: libappstream-glib
BuildRequires: perl-generators

BuildRequires: cmake(Qt5DBus)
BuildRequires: cmake(Qt5Gui)
BuildRequires: cmake(Qt5Network)
BuildRequires: cmake(Qt5X11Extras)

# kf5
BuildRequires: extra-cmake-modules
BuildRequires: kf5-rpm-macros
BuildRequires: cmake(KF5Auth)
BuildRequires: cmake(KF5Codecs)
BuildRequires: cmake(KF5Completion)
BuildRequires: cmake(KF5Config)
BuildRequires: cmake(KF5ConfigWidgets)
BuildRequires: cmake(KF5DBusAddons)
BuildRequires: cmake(KF5DocTools)
BuildRequires: cmake(KF5GlobalAccel)
BuildRequires: cmake(KF5GuiAddons)
BuildRequires: cmake(KF5I18n)
BuildRequires: cmake(KF5IconThemes)
BuildRequires: cmake(KF5JobWidgets)
BuildRequires: cmake(KF5KCMUtils)
BuildRequires: cmake(KF5KDELibs4Support)
BuildRequires: cmake(KF5KIO)
BuildRequires: cmake(KF5Notifications)
BuildRequires: cmake(KF5Service)
BuildRequires: cmake(KF5WidgetsAddons)
BuildRequires: cmake(KF5WindowSystem)
BuildRequires: cmake(KF5XmlGui)
BuildRequires: cmake(KF5IdleTime)
BuildRequires: cmake(KF5NotifyConfig)
BuildRequires: cmake(Grantlee5)

BuildRequires: pkgconfig(phonon4qt5)

# kde-apps
%global majmin_ver %(echo %{version} | cut -d. -f1,2)
BuildRequires: kf5-akonadi-contacts-devel >= %{majmin_ver}
BuildRequires: kf5-akonadi-mime-devel >= %{majmin_ver}
BuildRequires: kf5-akonadi-server-devel >= %{majmin_ver}
BuildRequires: kf5-kcalendarcore-devel >= %{majmin_ver}
BuildRequires: kf5-kcalendarutils-devel >= %{majmin_ver}
BuildRequires: kf5-kholidays-devel >= %{majmin_ver}
BuildRequires: kf5-kidentitymanagement-devel >= %{majmin_ver}
BuildRequires: kf5-kmailtransport-devel >= %{majmin_ver}
BuildRequires: kf5-kmime-devel >= %{majmin_ver}
BuildRequires: kf5-kpimtextedit-devel >= %{majmin_ver}
BuildRequires: kf5-libkdepim-devel >= %{majmin_ver}
BuildRequires: kf5-mailcommon-devel >= %{majmin_ver}
BuildRequires: kf5-pimcommon-devel >= %{majmin_ver}
BuildRequires: kf5-grantleetheme-devel >= %{majmin_ver}

%if 0%{?tests}
BuildRequires: dbus-x11
BuildRequires: xorg-x11-server-Xvfb
%endif

Requires: kdepim-runtime >= %{majmin_ver}

Obsoletes: kf5-kalarmcal < 22.04.0
Provides:  kf5-kalarmcal = %{version}-%{release}

%description
KAlarm is a personal alarm message, command and email scheduler.

%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -p1


%build
%cmake_kf5 \
  -DBUILD_TESTING:BOOL=%{?tests:ON}%{!?tests:OFF}
%cmake_build


%install
%cmake_install
%find_lang %{name} --all-name --with-html


%check
desktop-file-validate %{buildroot}%{_kf5_datadir}/applications/org.kde.%{name}.desktop
appstream-util validate-relax --nonet %{buildroot}%{_kf5_metainfodir}/org.kde.%{name}.appdata.xml ||:
%if 0%{?tests}
export CTEST_OUTPUT_ON_FAILURE=1
xvfb-run -a \
dbus-launch --exit-with-session \
make test ARGS="--output-on-failure --timeout 30" -C %{_vpath_builddir} ||:
%endif


%files -f %{name}.lang
%license LICENSES/*.txt
%{_kf5_bindir}/kalarm
%{_kf5_bindir}/kalarmautostart
%{_kf5_libdir}/libkalarmplugin.so.5*
%{_kf5_libdir}/libkalarmcalendar.so.5*
%{_kf5_libexecdir}/kauth/kalarm_helper
%{_kf5_qtplugindir}/pim5/kalarm/
%{_kf5_metainfodir}/org.kde.kalarm.appdata.xml
%{_kf5_datadir}/applications/org.kde.kalarm.desktop
%{_sysconfdir}/xdg/autostart/kalarm.autostart.desktop
%{_kf5_datadir}/config.kcfg/kalarmconfig.kcfg
%{_kf5_datadir}/kalarm/
%{_kf5_datadir}/dbus-1/system.d/org.kde.kalarm.rtcwake.conf
%{_datadir}/dbus-1/system-services/org.kde.kalarm.rtcwake.service
%{_datadir}/polkit-1/actions/org.kde.kalarm.rtcwake.policy
%{_datadir}/dbus-1/interfaces/org.kde.kalarm.kalarm.xml
%{_kf5_datadir}/icons/hicolor/*/apps/kalarm.*
%{_kf5_datadir}/icons/breeze/*/actions/show-today.svg
%{_kf5_datadir}/icons/breeze-dark/*/actions/show-today.svg
%{_kf5_datadir}/kxmlgui5/kalarm/kalarmui.rc
%{_kf5_datadir}/qlogging-categories5/*categories
%{_kf5_datadir}/knotifications5/kalarm.notifyrc


%changelog
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

