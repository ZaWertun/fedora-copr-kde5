%global tests 1

Name:    korganizer
Summary: Personal Organizer
Version: 23.08.1
Release: 1%{?dist}

# code (generally) GPLv2, docs GFDL
License: GPLv2 and GFDL
URL:     https://www.kde.org/applications/utilities/korganizer/

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
BuildRequires: pkgconfig(phonon4qt5)

BuildRequires: cmake(Qt5DBus)
BuildRequires: cmake(Qt5Network)
BuildRequires: cmake(Qt5PrintSupport)
BuildRequires: cmake(Qt5Test)
BuildRequires: cmake(Qt5Widgets)
BuildRequires: cmake(Qt5UiTools)
BuildRequires: cmake(Qt5Xml)
BuildRequires: cmake(Qt5X11Extras)
BuildRequires: cmake(Grantlee5)

# kf5
BuildRequires: extra-cmake-modules
BuildRequires: kf5-rpm-macros
BuildRequires: cmake(KF5Codecs)
BuildRequires: cmake(KF5Config)
BuildRequires: cmake(KF5Crash)
BuildRequires: cmake(KF5DBusAddons)
BuildRequires: cmake(KF5DocTools)
BuildRequires: cmake(KF5GlobalAccel)
BuildRequires: cmake(KF5KCMUtils)
BuildRequires: cmake(KF5NewStuff)
BuildRequires: cmake(KF5Service)

BuildRequires: cmake(KF5TextAutoCorrection)

# kde-apps
%global majmin_ver %(echo %{version} | cut -d. -f1,2)
BuildRequires: kf5-akonadi-calendar-devel >= %{majmin_ver}
BuildRequires: kf5-akonadi-contacts-devel >= %{majmin_ver}
BuildRequires: kf5-akonadi-mime-devel >= %{majmin_ver}
BuildRequires: kf5-akonadi-notes-devel >= %{majmin_ver}
BuildRequires: kf5-akonadi-search-devel >= %{majmin_ver}
BuildRequires: kf5-akonadi-server-devel >= %{majmin_ver}
BuildRequires: kf5-calendarsupport-devel >= %{majmin_ver}
BuildRequires: kf5-eventviews-devel >= %{majmin_ver}
BuildRequires: kf5-incidenceeditor-devel >= %{majmin_ver}
BuildRequires: kf5-kcalendarcore-devel >= %{majmin_ver}
BuildRequires: kf5-kcalendarutils-devel >= %{majmin_ver}
BuildRequires: kf5-kcontacts-devel >= %{majmin_ver}
BuildRequires: kf5-kholidays-devel >= %{majmin_ver}
BuildRequires: kf5-kidentitymanagement-devel >= %{majmin_ver}
BuildRequires: kf5-kldap-devel >= %{majmin_ver}
BuildRequires: kf5-kontactinterface-devel >= %{majmin_ver}
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

BuildRequires: cmake(KUserFeedback)

Requires: %{name}-libs%{?_isa} = %{version}-%{release}
Requires: kdepim-runtime >= %{majmin_ver}

%description
KOrganizer is the calendar and scheduling component of the Kontact suite.
You can write journal entries, schedule appointments, events, and to-dos.

%package libs
Summary: Runtime libraries for %{name}
Requires: %{name} = %{version}-%{release}
%description libs
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
%find_lang %{name} --all-name --with-html

## unpackaged files
rm -fv %{buildroot}%{_kf5_libdir}/libkorganizer_{core,interfaces}.so


%check
desktop-file-validate %{buildroot}%{_kf5_datadir}/applications/korganizer-import.desktop
desktop-file-validate %{buildroot}%{_kf5_datadir}/applications/org.kde.%{name}.desktop
appstream-util validate-relax --nonet %{buildroot}%{_kf5_metainfodir}/org.kde.%{name}.appdata.xml
%if 0%{?tests}
export CTEST_OUTPUT_ON_FAILURE=1
xvfb-run -a \
dbus-launch --exit-with-session \
make test ARGS="--output-on-failure --timeout 30" -C %{_vpath_builddir} ||:
%endif


%files -f %{name}.lang
%license LICENSES/*.txt
%{_kf5_bindir}/korganizer
%{_kf5_datadir}/config.kcfg/korganizer.kcfg
%{_datadir}/dbus-1/interfaces/org.kde.Korganizer.*.xml
%{_datadir}/dbus-1/interfaces/org.kde.korganizer.*.xml
%{_kf5_metainfodir}/org.kde.korganizer.appdata.xml
%{_kf5_datadir}/applications/org.kde.korganizer.desktop
%{_kf5_datadir}/applications/korganizer-import.desktop
%{_kf5_datadir}/applications/korganizer-view.desktop
%{_kf5_datadir}/korganizer/
%{_kf5_datadir}/icons/hicolor/*/apps/korg-journal.*
%{_kf5_datadir}/icons/hicolor/*/apps/korg-todo.*
%{_kf5_datadir}/icons/hicolor/*/apps/korganizer.*
%{_kf5_datadir}/icons/hicolor/*/apps/quickview.*
%{_kf5_datadir}/knsrcfiles/*.knsrc
#{_kf5_datadir}/kservicetypes5/*.desktop
%{_kf5_datadir}/dbus-1/services/*.service
%{_kf5_datadir}/qlogging-categories5/*categories

%ldconfig_scriptlets libs

%files libs
%{_kf5_libdir}/libkorganizer_interfaces.so.*
%{_kf5_libdir}/libkorganizer_core.so.*
%{_kf5_libdir}/libkorganizerprivate.so.*
%{_kf5_qtplugindir}/korganizerpart.so
%{_kf5_qtplugindir}/pim5/kcms/korganizer/
# Kontact integration
%{_kf5_qtplugindir}/pim5/kcms/summary/kcmapptsummary.so
%{_kf5_qtplugindir}/pim5/kcms/summary/kcmsdsummary.so
%{_kf5_qtplugindir}/pim5/kcms/summary/kcmtodosummary.so
%{_kf5_qtplugindir}/pim5/kontact/kontact_korganizerplugin.so
%{_kf5_qtplugindir}/pim5/kontact/kontact_todoplugin.so
%{_kf5_qtplugindir}/pim5/kontact/kontact_journalplugin.so
%{_kf5_qtplugindir}/pim5/kontact/kontact_specialdatesplugin.so


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

