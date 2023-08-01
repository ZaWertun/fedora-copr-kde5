%global framework akonadi-server

# trim changelog included in binary rpms
%global _changelog_trimtime %(date +%s -d "1 year ago")

%global mysql mysql
%if 0%{?rhel} > 6
# el7 mariadb pkgs don't have compat Provides: mysql (apparently?)
%global mysql mariadb
%endif

# uncomment to enable bootstrap mode
#global bootstrap 1

%if !0%{?bootstrap}
# skip slow(er) archs, for now -- rex
%ifnarch %{arm}
%global tests 0
%endif
%endif

Name:    kf5-%{framework}
Summary: PIM Storage Service
Version: 23.04.3
Release: 1%{?dist}

License: LGPLv2+
URL:     https://cgit.kde.org/%{framework}.git

%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0: https://download.kde.org/%{stable}/release-service/%{version}/src/akonadi-%{version}.tar.xz
Source1: https://download.kde.org/%{stable}/release-service/%{version}/src/akonadi-%{version}.tar.xz.sig
Source2: gpgkey-D81C0CB38EB725EF6691C385BB463350D6EF31EF.gpg

## mysql config
Source10:       akonadiserverrc.mysql
Source11:       akonadiserverrc.sqlite

## upstreamable patches

## upstream patches (lookaside cache)

## downstream patches

%define mysql_conf_timestamp 20170512

BuildRequires:  gnupg2
BuildRequires:  extra-cmake-modules
BuildRequires:  kf5-rpm-macros
BuildRequires:  qt5-qtbase-devel
BuildRequires:  qt5-qtxmlpatterns-devel
BuildRequires:  cmake(AccountsQt5)

BuildRequires:  cmake(KF5ItemViews)
BuildRequires:  cmake(KF5KIO)
BuildRequires:  cmake(KF5Config)
BuildRequires:  cmake(KF5I18n)
BuildRequires:  cmake(KF5DBusAddons)
BuildRequires:  cmake(KF5ItemModels)
BuildRequires:  cmake(KF5GuiAddons)
BuildRequires:  cmake(KF5IconThemes)
BuildRequires:  cmake(KF5WindowSystem)
BuildRequires:  cmake(KF5Completion)
BuildRequires:  cmake(KF5Crash)
BuildRequires:  cmake(KAccounts)

BuildRequires:  boost-devel
BuildRequires:  pkgconfig(libxslt)
BuildRequires:  pkgconfig(shared-mime-info)
BuildRequires:  pkgconfig(sqlite3) >= 3.6.23

## (some) optional deps
%if ! 0%{?bootstrap}
BuildRequires:  pkgconfig(Qt5Designer)
BuildRequires:  cmake(KF5DesignerPlugin)
%endif

# ^^ sqlite3 driver plugin needs versioned qt5 dep
BuildRequires: qt5-qtbase-private-devel

# backends, used at buildtime to query known locations of server binaries
# FIXME/TODO: set these via cmake directives, avoids needless buildroot items
BuildRequires:  mariadb-server
BuildRequires:  postgresql-server

%if 0%{?tests}
BuildRequires: dbus-x11
BuildRequires: xorg-x11-server-Xvfb
%endif

Requires(post): %{_sbindir}/update-alternatives
Requires(postun): %{_sbindir}/update-alternatives

Recommends:     %{name}-mysql = %{version}-%{release}

Conflicts:      akonadi < 1.13.0-100

# translations moved here
Conflicts: kde-l10n < 17.03

# when kf5-akonadi was split, -socialutils was dropped
Obsoletes: kf5-akonadi-socialutils < 16.07

%description
%{summary}.

%package devel
Summary:        Developer files for %{name}
Obsoletes:      kf5-akonadi-devel < 16.03
Obsoletes:      kf5-akonadi-socialutils-devel < 16.07
Requires:       %{name}%{?_isa} = %{version}-%{release}
Requires:       boost-devel
Requires:       kf5-kcompletion-devel
Requires:       kf5-kjobwidgets-devel
Requires:       kf5-kservice-devel
Requires:       kf5-solid-devel
Requires:       kf5-kxmlgui-devel
Requires:       kf5-kitemmodels-devel
Requires:       qt5-qtbase-devel
# at least dbus-1/interfaces conflict, maybe more -- rex
Conflicts:      akonadi-devel
%description devel
%{summary}.

%package mysql
Summary:        Akonadi MySQL backend support
# upgrade path
Obsoletes:      akonadi < 1.7.90-2
Obsoletes:      akonadi-mysql < 15.08.0
Provides:       akonadi-mysql = %{version}-%{release}
Requires:       %{name}%{?_isa} = %{version}-%{release}
Requires:       %{mysql}-server
%if "%{?mysql}" != "mariadb" && 0%{?fedora} > 20
Recommends:     mariadb-server
%endif
Requires:       qt5-qtbase-mysql%{?_isa}
Requires(post): %{_sbindir}/update-alternatives
Requires(postun): %{_sbindir}/update-alternatives
%description mysql
Configures akonadi to use mysql backend by default.

Requires an available instance of mysql server at runtime.
Akonadi can spawn a per-user one automatically if the mysql-server
package is installed on the machine.
See also: %{_sysconfdir}/akonadi/mysql-global.conf


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -n akonadi-%{version} -p1


%build
%cmake_kf5 \
  %{?database_backend:-DDATABASE_BACKEND=%{database_backend}} \
  -DBUILD_TESTING:BOOL=%{?tests:ON}%{!?tests:OFF}
%cmake_build


%install
%cmake_install

%find_lang libakonadi5
%find_lang akonadi_knut_resource
cat akonadi_knut_resource.lang >> libakonadi5.lang

install -p -m644 -D %{SOURCE10} %{buildroot}%{_sysconfdir}/xdg/akonadi/akonadiserverrc.mysql
install -p -m644 -D %{SOURCE11} %{buildroot}%{_sysconfdir}/xdg/akonadi/akonadiserverrc.sqlite

mkdir -p %{buildroot}%{_datadir}/akonadi/agents

touch -d %{mysql_conf_timestamp} \
  %{buildroot}%{_sysconfdir}/xdg/akonadi/mysql-global*.conf \
  %{buildroot}%{_sysconfdir}/xdg/akonadi/mysql-local.conf

# create/own these dirs
mkdir -p %{buildroot}%{_kf5_datadir}/akonadi/plugins
mkdir -p %{buildroot}%{_kf5_libdir}/akonadi

# %%ghost'd global akonadiserverrc
touch akonadiserverrc
install -p -m644 -D akonadiserverrc %{buildroot}%{_sysconfdir}/xdg/akonadi/akonadiserverrc

## unpackaged files
# omit mysql-global-mobile.conf
rm -fv %{buildroot}%{_sysconfdir}/xdg/akonadi/mysql-global-mobile.conf
# part of omitting exceptions header hack, drop the custom (no-longer-used) header itself
rm -fv %{buildroot}%{_kf5_includedir}/AkonadiCore/std_exception.h
# apparmor rules
rm -fv %{buildroot}%{_sysconfdir}/apparmor.d/*

%check
%if 0%{?tests}
export CTEST_OUTPUT_ON_FAILURE=1
xvfb-run -a \
dbus-launch --exit-with-session \
make test ARGS="--output-on-failure --timeout 300" -C %{_vpath_builddir} ||:
%endif


%post
%{?ldconfig}
%{_sbindir}/update-alternatives \
  --install %{_sysconfdir}/xdg/akonadi/akonadiserverrc \
  akonadiserverrc \
  %{_sysconfdir}/xdg/akonadi/akonadiserverrc.sqlite \
  8

%postun
%{?ldconfig}
if [ $1 -eq 0 ] ; then
%{_sbindir}/update-alternatives \
  --remove akonadiserverrc \
  %{_sysconfdir}/xdg/akonadi/akonadiserverrc.sqlite
fi


%files -f libakonadi5.lang
%doc AUTHORS README.md
%license LICENSES/*.txt
%dir %{_sysconfdir}/xdg/akonadi/
%ghost %config(missingok,noreplace) %{_sysconfdir}/xdg/akonadi/akonadiserverrc
%config(noreplace) %{_sysconfdir}/xdg/akonadi/akonadiserverrc.sqlite
%{_kf5_bindir}/akonadi_agent_launcher
%{_kf5_bindir}/akonadi_agent_server
%{_kf5_bindir}/akonadi_control
%{_kf5_bindir}/akonadi_rds
%{_kf5_bindir}/akonadictl
%{_kf5_bindir}/akonadiserver
%{_kf5_libdir}/akonadi/
%{_kf5_datadir}/dbus-1/services/org.freedesktop.Akonadi.*.service
%{_kf5_datadir}/mime/packages/akonadi-mime.xml
%{_kf5_datadir}/akonadi/
%{_kf5_datadir}/config.kcfg/resourcebase.kcfg
%{_kf5_datadir}/kf5/akonadi/
%if ! 0%{?bootstrap}
%{_kf5_qtplugindir}/designer/akonadi5widgets.so
%endif
%{_kf5_qtplugindir}/sqldrivers/libqsqlite3.so
%{_kf5_qtplugindir}/pim5/akonadi/akonadi_test_searchplugin.so
%{_kf5_libdir}/libKPim5AkonadiAgentBase.so.5*
%{_kf5_libdir}/libKPim5AkonadiCore.so.5*
%{_kf5_libdir}/libKPim5AkonadiPrivate.so.5*
%{_kf5_libdir}/libKPim5AkonadiWidgets.so.5*
%{_kf5_libdir}/libKPim5AkonadiXml.so.5*
# let newer %%trigger-based scriptlets catch this -- rex
%{_kf5_datadir}/icons/hicolor/*/apps/akonadi.*
%{_kf5_datadir}/qlogging-categories5/*categories

# akonadi_knut_resource
%{_kf5_bindir}/akonadi_knut_resource
%{_kf5_datadir}/kf5/akonadi_knut_resource/

%files devel
%{_kf5_bindir}/akonadi2xml
%{_kf5_bindir}/akonadiselftest
%{_kf5_bindir}/akonaditest
%{_kf5_bindir}/asapcat
%{_kf5_datadir}/dbus-1/interfaces/org.freedesktop.Akonadi.*.xml
%{_kf5_datadir}/kdevappwizard/templates/akonadiresource.tar.bz2
%{_kf5_datadir}/kdevappwizard/templates/akonadiserializer.tar.bz2
%{_includedir}/KPim5/Akonadi/
%{_includedir}/KPim5/AkonadiXml/
%{_includedir}/KPim5/AkonadiCore/
%{_includedir}/KPim5/AkonadiWidgets/
%{_includedir}/KPim5/AkonadiAgentBase/
%{_kf5_libdir}/libKPim5AkonadiAgentBase.so
%{_kf5_libdir}/libKPim5AkonadiCore.so
%{_kf5_libdir}/libKPim5AkonadiPrivate.so
%{_kf5_libdir}/libKPim5AkonadiWidgets.so
%{_kf5_libdir}/libKPim5AkonadiXml.so
%{_kf5_libdir}/cmake/KF5Akonadi/
%{_kf5_libdir}/cmake/KPim5Akonadi/
%{_kf5_archdatadir}/mkspecs/modules/qt_Akonadi*.pri

%post mysql
%{_sbindir}/update-alternatives \
  --install %{_sysconfdir}/xdg/akonadi/akonadiserverrc \
  akonadiserverrc \
  %{_sysconfdir}/xdg/akonadi/akonadiserverrc.mysql \
  10

%postun mysql
if [ $1 -eq 0 ]; then
%{_sbindir}/update-alternatives \
  --remove akonadiserverrc \
  %{_sysconfdir}/xdg/akonadi/akonadiserverrc.mysql
fi

%files mysql
%config(noreplace) %{_sysconfdir}/xdg/akonadi/akonadiserverrc.mysql
%config(noreplace) %{_sysconfdir}/xdg/akonadi/mysql-global.conf
%config(noreplace) %{_sysconfdir}/xdg/akonadi/mysql-local.conf


%changelog
* Thu Jul 06 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.04.3-1
- 23.04.3

* Thu Jun 08 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.04.2-1
- 23.04.2

* Thu May 11 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.04.1-1
- 23.04.1

* Thu Apr 20 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.04.0-1
- 23.04.0

* Tue Apr 18 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.12.3-5
- rebuild

* Thu Mar 02 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.12.3-1
- 22.12.3

