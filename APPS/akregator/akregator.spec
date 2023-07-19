# uncomment to enable bootstrap mode
%global bootstrap 1

%if !0%{?bootstrap}
%global tests 1
%endif

Name:    akregator
Summary: Feed Reader
Version: 23.04.3
Release: 1%{?dist}

# code (generally) GPLv2, docs GFDL
License: GPLv2 and GFDL
URL:     https://www.kde.org/applications/internet/akregator/

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

BuildRequires: cmake(Qt5PrintSupport)
BuildRequires: cmake(Qt5Test)
BuildRequires: cmake(Qt5Widgets)
BuildRequires: cmake(Qt5WebEngine)
BuildRequires: cmake(Qt5WebEngineWidgets)

BuildRequires: cmake(Grantlee5)

# kf5
BuildRequires: extra-cmake-modules
BuildRequires: kf5-rpm-macros
BuildRequires: cmake(KF5Crash)
BuildRequires: cmake(KF5DocTools)
BuildRequires: cmake(KF5KCMUtils)
BuildRequires: cmake(KF5NotifyConfig)
BuildRequires: cmake(KF5Parts)
BuildRequires: cmake(KF5TextEditor)
BuildRequires: cmake(KF5XmlGui)
BuildRequires: cmake(KF5IconThemes)
BuildRequires: cmake(KF5Notifications)
BuildRequires: cmake(KF5WindowSystem)
BuildRequires: cmake(KF5SyntaxHighlighting)

BuildRequires: cmake(KF5TextAutoCorrection)

# kde-apps
%global majmin_ver %(echo %{version} | cut -d. -f1,2)
BuildRequires: kf5-akonadi-mime-devel >= %{majmin_ver}
BuildRequires: kf5-grantleetheme-devel >= %{majmin_ver}
BuildRequires: kf5-kontactinterface-devel >= %{majmin_ver}
BuildRequires: kf5-kpimtextedit-devel >= %{majmin_ver}
BuildRequires: kf5-libkdepim-devel >= %{majmin_ver}
BuildRequires: kf5-libkleo-devel >= %{majmin_ver}
BuildRequires: kf5-messagelib-devel >= %{majmin_ver}
BuildRequires: kf5-pimcommon-devel >= %{majmin_ver}
# in kf5 since 5.50.0
BuildRequires: kf5-syndication-devel >= %{majmin_ver}

%if 0%{?tests}
BuildRequires: dbus-x11
BuildRequires: xorg-x11-server-Xvfb
%endif

BuildRequires: cmake(KUserFeedback)

Requires: %{name}-libs%{?_isa} = %{version}-%{release}

%description
Akregator is a news feed reader. It enables you to follow news sites,
blogs and other RSS/Atom-enabled websites without the need to manually
check for updates using a web browser.

%package libs
Summary: Runtime libraries for %{name}
Requires: %{name} = %{version}-%{release}
%description libs
%{summary}.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -p1 -n %{name}-%{version} -p1


%build
%cmake_kf5 \
  -DBUILD_TESTING:BOOL=%{?tests:ON}%{!?tests:OFF}
%cmake_build


%install
%cmake_install
%find_lang %{name} --all-name --with-html

## unpackaged files
rm -fv %{buildroot}%{_kf5_libdir}/libakregatorinterfaces.so


%check
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
%{_kf5_bindir}/akregatorstorageexporter
%{_kf5_bindir}/akregator
%{_kf5_datadir}/config.kcfg/akregator.kcfg
%{_kf5_metainfodir}/org.kde.akregator.appdata.xml
%{_kf5_datadir}/applications/org.kde.akregator.desktop
%{_kf5_datadir}/icons/hicolor/*/apps/akregator.*
%{_kf5_datadir}/icons/hicolor/*/apps/akregator_empty.png
%{_kf5_datadir}/akregator/
%{_datadir}/dbus-1/interfaces/org.kde.akregator.part.xml
# Kontact integration
%{_kf5_datadir}/knotifications5/akregator.notifyrc
%{_kf5_datadir}/qlogging-categories5/*categories

%ldconfig_scriptlets libs

%files libs
%{_kf5_libdir}/libakregatorinterfaces.so.*
%{_kf5_libdir}/libakregatorprivate.so.*
%{_kf5_qtplugindir}/akregatorpart.so
%{_kf5_qtplugindir}/pim5/kcms/akregator/
# Kontact integration
%{_kf5_qtplugindir}/pim5/kontact/kontact_akregatorplugin.so


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

