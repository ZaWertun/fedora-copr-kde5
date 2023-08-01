## skipping tests, seem to hang indefinitely starting with 18.04.0
#global tests 1

Name:    kdepim-runtime
Summary: KDE PIM Runtime Environment
Epoch:   1
Version: 23.04.3
Release: 1%{?dist}

License: GPLv2
URL:     https://cgit.kde.org/%{framework}.git

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

## upstream patches

## upstreamable patches

Obsoletes:      kdepim-apps-libs <= 20.08.3

# nuke ill-advised -devel pkg
Obsoletes:      kdepim-runtime-devel < 1:4.7.90-3

Obsoletes:      akonadi-google < 0.4
Provides:       akonadi-google = %{version}-%{release}
Obsoletes:      akonadi-google-calendar < 0.4
Provides:       akonadi-google-calendar = %{version}-%{release}
Obsoletes:      akonadi-google-contacts < 0.4
Provides:       akonadi-google-contacts = %{version}-%{release}
Obsoletes:      akonadi-google-tasks < 0.4
Provides:       akonadi-google-tasks = %{version}-%{release}

Requires:       %{name}-libs%{?_isa} = %{epoch}:%{version}-%{release}

BuildRequires:  gnupg2
BuildRequires:  desktop-file-utils
BuildRequires:  extra-cmake-modules
BuildRequires:  gettext
BuildRequires:  kf5-rpm-macros
BuildRequires:  kf5-kdelibs4support-devel
BuildRequires:  kf5-kconfig-devel
BuildRequires:  kf5-kconfigwidgets-devel
BuildRequires:  kf5-kdav-devel
BuildRequires:  kf5-kdoctools-devel
BuildRequires:  kf5-knotifyconfig-devel
BuildRequires:  kf5-kio-devel
BuildRequires:  kf5-kitemmodels-devel
BuildRequires:  kf5-kross-devel
BuildRequires:  kf5-kcodecs-devel
BuildRequires:  kf5-kwindowsystem-devel
BuildRequires:  kf5-ktextwidgets-devel
BuildRequires:  kf5-kcmutils-devel

BuildRequires:  qt5-qtbase-devel
BuildRequires:  qt5-qtdeclarative-devel
BuildRequires:  qt5-qtlocation-devel
BuildRequires:  qt5-qtxmlpatterns-devel
BuildRequires:  qt5-qtwebchannel-devel
BuildRequires:  qt5-qtwebengine-devel
BuildRequires:  qt5-qtnetworkauth-devel
BuildRequires:  qca-qt5-devel
BuildRequires:  cmake(Qt5Keychain)

%global majmin_ver %(echo %{version} | cut -d. -f1,2)
BuildRequires:  kf5-akonadi-calendar-devel >= %{majmin_ver}
BuildRequires:  kf5-akonadi-contact-devel >= %{majmin_ver}
BuildRequires:  kf5-akonadi-mime-devel >= %{majmin_ver}
BuildRequires:  kf5-akonadi-notes-devel >= %{majmin_ver}
BuildRequires:  kf5-akonadi-server-devel >= %{majmin_ver}
BuildRequires:  kf5-kcalendarcore-devel >= %{majmin_ver}
BuildRequires:  kf5-kcalendarutils-devel >= %{majmin_ver}
BuildRequires:  kf5-kidentitymanagement-devel >= %{majmin_ver}
BuildRequires:  kf5-kimap-devel >= %{majmin_ver}
BuildRequires:  kf5-kmailtransport-devel >= %{majmin_ver}
BuildRequires:  kf5-kmbox-devel >= %{majmin_ver}
BuildRequires:  kf5-kmime-devel >= %{majmin_ver}
BuildRequires:  kf5-kpimtextedit-devel >= %{majmin_ver}
BuildRequires:  kf5-pimcommon-devel >= %{majmin_ver}
BuildRequires:  kf5-syndication-devel >= %{majmin_ver}
BuildRequires:  libkgapi-devel >= %{majmin_ver}
BuildRequires:  kf5-kdav-devel >= %{majmin_ver}
BuildRequires:  kf5-libkdepim-devel >= %{majmin_ver}
BuildRequires:  kf5-kldap-devel >= %{majmin_ver}
BuildRequires:  kf5-grantleetheme-devel >= %{majmin_ver}
BuildRequires:  cmake(KF5Holidays)
BuildRequires:  cmake(Grantlee5)

# https://bugzilla.redhat.com/show_bug.cgi?id=1662756
Requires: libkgapi%{?_isa} >= %{majmin_ver}
BuildRequires:  cmake(KF5PimCommon)

## bundled patched version, as stock one FTBFS at the moment
#BuildRequires:  libkolab-devel >= 1.0
Provides: bundled(libkolab) = 1.0.2

BuildRequires:  libkolabxml-devel >= 1.1

# needed by google calendar resource
BuildRequires:  pkgconfig(libical)
BuildRequires:  pkgconfig(libxslt) pkgconfig(libxml-2.0)

BuildRequires:  pkgconfig(shared-mime-info)
BuildRequires:  pkgconfig(zlib)

%if !0%{?bootstrap}
BuildRequires:  cmake(Qt5TextToSpeech)
%endif

%if 0%{?tests}
BuildRequires: dbus-x11
BuildRequires: kf5-akonadi-server-mysql >= %{majmin_ver}
BuildRequires: xorg-x11-server-Xvfb
%endif

%description
%{summary}.

%package libs
Summary: %{name} runtime libraries
# some plugins moved here 16.04.0-1
Obsoletes: kdepim-runtime < 1:16.04
Requires: %{name} = %{epoch}:%{version}-%{release}
Requires: kf5-akonadi-server%{?_isa} >= %{version}
%description libs
%{summary}.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -n kdepim-runtime-%{version}%{?pre} -p1


%build
%cmake_kf5 \
  -DBUILD_TESTING:BOOL=%{?tests:ON}%{!?tests:OFF}
%cmake_build


%install
%cmake_install
%find_lang %{name} --all-name --with-html

# unpackaged files
rm -fv %{buildroot}%{_kf5_libdir}/lib{akonadi-filestore,folderarchivesettings,libakonadi-singlefileresource,kmindexreader,maildir}.so


%check
%if 0%{?tests}
export CTEST_OUTPUT_ON_FAILURE=1
xvfb-run -a \
dbus-launch --exit-with-session \
make test ARGS="--output-on-failure --timeout 30" -C %{_vpath_builddir} ||:
%endif

%files -f %{name}.lang
%license LICENSES/*.txt
%{_kf5_bindir}/akonadi_*
%{_kf5_bindir}/gidmigrator
%{_kf5_datadir}/akonadi/accountwizard/*
%{_kf5_datadir}/akonadi/agents/*
%{_kf5_datadir}/akonadi/firstrun/*
%{_kf5_datadir}/knotifications5/*
%{_kf5_datadir}/kservices5/akonadi/davgroupware-providers/*
%{_kf5_datadir}/mime/packages/kdepim-mime.xml
%{_kf5_datadir}/icons/hicolor/*/apps/*
%{_kf5_datadir}/dbus-1/interfaces/*.xml
%{_kf5_datadir}/qlogging-categories5/*categories
%{_kf5_datadir}/applications/org.kde.akonadi_*.desktop

%ldconfig_scriptlets libs

%files libs
%{_kf5_libdir}/libakonadi-filestore.so.5*
%{_kf5_libdir}/libfolderarchivesettings.so.5*
%{_kf5_libdir}/libakonadi-singlefileresource.so.5*
%{_kf5_libdir}/libkmindexreader.so.5*
%{_kf5_libdir}/libmaildir.so.5*
%{_kf5_plugindir}/kio/akonadi.so
%{_kf5_qtplugindir}/pim5/akonadi/config/*.so
%{_kf5_qtplugindir}/pim5/kcms/kaddressbook/kcm_ldap.so


%changelog
* Thu Jul 06 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 1:23.04.3-1
- 23.04.3

* Thu Jun 08 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 1:23.04.2-1
- 23.04.2

* Thu May 11 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 1:23.04.1-1
- 23.04.1

* Thu Apr 20 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 1:23.04.0-1
- 23.04.0

* Thu Mar 02 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 1:22.12.3-1
- 22.12.3

* Thu Feb 02 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 1:22.12.2-1
- 22.12.2

