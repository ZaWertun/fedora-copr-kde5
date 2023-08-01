%global tests 1

Name:    kontact
Summary: Personal Information Manager
Version: 23.04.3
Release: 1%{?dist}

# code (generally) GPLv2, docs GFDL
License: GPLv2 and GFDL
URL:     https://www.kde.org/applications/office/kontact

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

BuildRequires: cmake(Qt5DBus)
BuildRequires: cmake(Qt5Widgets)
BuildRequires: cmake(Qt5WebEngine)

# kf5
BuildRequires: extra-cmake-modules
BuildRequires: kf5-rpm-macros
BuildRequires: cmake(KF5DBusAddons)
BuildRequires: cmake(KF5DocTools)
BuildRequires: cmake(KF5KCMUtils)
BuildRequires: cmake(KF5Crash)
BuildRequires: cmake(KF5WindowSystem)
BuildRequires: cmake(KF5IconThemes)

BuildRequires: cmake(KF5TextAutoCorrection)

BuildRequires: cmake(Grantlee5)

# kde-apps
%global majmin_ver %(echo %{version} | cut -d. -f1,2)
BuildRequires: kf5-akonadi-server-devel >= %{majmin_ver}
BuildRequires: kf5-grantleetheme-devel >= %{majmin_ver}
BuildRequires: kf5-kontactinterface-devel >= %{majmin_ver}
BuildRequires: kf5-libkdepim-devel >= %{majmin_ver}
BuildRequires: kf5-mailcommon-devel >= %{majmin_ver}
BuildRequires: kf5-pimcommon-devel >= %{majmin_ver}

%if 0%{?tests}
BuildRequires: dbus-x11
BuildRequires: xorg-x11-server-Xvfb
%endif

Requires: %{name}-libs%{?_isa} = %{version}-%{release}

# core/runtime deps
Requires: kaddressbook >= %{majmin_ver}
Requires: kmail >= %{majmin_ver}
Requires: korganizer >= %{majmin_ver}

%description
Kontact is the integrated solution to your personal information management
(PIM) needs. It combines well-known KDE applications like KMail, KOrganizer
and KAddressBook into a single interface to provide easy access to mail,
scheduling, address book and other PIM functionality.

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


%check
for f in %{buildroot}%{_kf5_datadir}/applications/*.desktop ; do
  desktop-file-validate $f
done
appstream-util validate-relax --nonet %{buildroot}%{_kf5_metainfodir}/org.kde.%{name}.appdata.xml
%if 0%{?tests}
export CTEST_OUTPUT_ON_FAILURE=1
xvfb-run -a \
dbus-launch --exit-with-session \
make test ARGS="--output-on-failure --timeout 30" -C %{_vpath_builddir} ||:
%endif

%files -f %{name}.lang
%license LICENSES/*.txt
%{_kf5_bindir}/kontact
%{_kf5_metainfodir}/org.kde.kontact.appdata.xml
%{_kf5_datadir}/applications/org.kde.kontact.desktop
%{_kf5_datadir}/config.kcfg/kontact.kcfg
%{_kf5_datadir}/messageviewer/about/default/introduction_kontact.html
%{_kf5_datadir}/messageviewer/about/default/loading_kontact.html
%{_kf5_datadir}/icons/hicolor/*/apps/kontact.*
%{_kf5_datadir}/dbus-1/services/*.service
%{_kf5_datadir}/qlogging-categories5/*categories

%ldconfig_scriptlets libs

%files libs
%{_kf5_libdir}/libkontactprivate.so.*
%{_kf5_qtplugindir}/pim5/kcms/kontact/kcm_kontact.so


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

