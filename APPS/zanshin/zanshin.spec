Name:           zanshin
Version:        23.08.4
Release:        1%{?dist}
Summary:        Todo/action management software

License:        GPLv2
URL:            http://zanshin.kde.org/
Source0:        https://download.kde.org/stable/release-service/%{version}/src/%{name}-%{version}.tar.xz
Source1:        https://download.kde.org/stable/release-service/%{version}/src/%{name}-%{version}.tar.xz.sig
Source2:        gpgkey-D81C0CB38EB725EF6691C385BB463350D6EF31EF.gpg

## upstream patches

# handled by qt5-srpm-macros, which defines %%qt5_qtwebengine_arches
%{?qt5_qtwebengine_arches:ExclusiveArch: %{qt5_qtwebengine_arches}}

BuildRequires:  gnupg2
BuildRequires:  extra-cmake-modules
BuildRequires:  gettext
BuildRequires:  intltool
## kf5
BuildRequires:  cmake(KPim5Akonadi)
BuildRequires:  cmake(KPim5AkonadiContact)
BuildRequires:  cmake(KPim5AkonadiCalendar)
BuildRequires:  cmake(KPim5AkonadiNotes)
BuildRequires:  cmake(KPim5AkonadiSearch)
BuildRequires:  cmake(KF5IdentityManagement)
BuildRequires:  cmake(KPim5KontactInterface)
BuildRequires:  cmake(KPim5Ldap)
BuildRequires:  cmake(KF5Runner)
BuildRequires:  cmake(KF5Wallet)
BuildRequires:  cmake(Qt5Gui)
BuildRequires:  cmake(Qt5Widgets)
BuildRequires:  cmake(Qt5Qml)
BuildRequires:  cmake(Qt5Test)
## %%check
BuildRequires:  desktop-file-utils
BuildRequires:  libappstream-glib
%if 0%{?tests}
BuildRequires:  boost-devel
BuildRequires:  dbus-x11
BuildRequires:  kf5-akonadi-server
BuildRequires:  kf5-akonadi-server-devel
BuildRequires:  time
BuildRequires:  xorg-x11-server-Xvfb
%endif

Provides: zanshin-frontend = %{version}-%{release}
Requires: zanshin-common = %{version}-%{release}
Obsoletes: renku < %{version}
Provides: zanshin = %{version}-%{release}
# https://bugzilla.redhat.com/show_bug.cgi?id=1602214
Requires: kdepim-runtime

%description
Zanshin Todo is a powerful yet simple application for managing your day to day
actions. It helps you organize and reduce the cognitive pressure of what one has
to do in his job and personal life. You'll never forget anything anymore,
getting your mind like water.

%package common
Summary: common files for %{name}
Requires: zanshin-frontend = %{version}-%{release}
BuildArch: noarch
%description common
%{summary}.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -p1


%build
%cmake_kf5 \
  -DBUILD_TESTING:BOOL=%{?tests:ON}%{!?tests:OFF}
%cmake_build


%install
%cmake_install
%find_lang %{name} --all-name


%check
appstream-util validate-relax --nonet %{buildroot}%{_kf5_metainfodir}/org.kde.%{name}.metainfo.xml
desktop-file-validate %{buildroot}%{_datadir}/applications/org.kde.%{name}.desktop
%if 0%{?tests}
export CTEST_OUTPUT_ON_FAILURE=1
xvfb-run -a \
dbus-launch --exit-with-session \
time \
%ctest --timeout 30
%endif

%files common -f %{name}.lang
%doc AUTHORS HACKING TODO
%license LICENSES/*.txt
%{_kf5_datadir}/icons/hicolor/*/*/zanshin.*

%files
%{_kf5_bindir}/zanshin*
%{_kf5_metainfodir}/org.kde.zanshin.metainfo.xml
%{_kf5_datadir}/applications/org.kde.zanshin.desktop
%{_kf5_datadir}/kxmlgui5/zanshin/
%{_kf5_plugindir}/krunner/org.kde.%{name}.so
%{_qt5_plugindir}/zanshin_part.so
%{_kf5_qtplugindir}/pim5/kontact/kontact_zanshinplugin.so

%changelog
* Fri Dec 08 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.08.4-1
- 23.08.4

* Fri Nov 10 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.08.3-1
- 23.08.3

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

