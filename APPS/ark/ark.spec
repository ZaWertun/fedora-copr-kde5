%if 0%{?fedora}
%global p7zip 1
%endif

Name:    ark
Summary: Archive manager
Version: 23.04.2
Release: 1%{?dist}

License: GPLv2+
URL:     https://www.kde.org/applications/utilities/ark/

%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0: https://download.kde.org/%{stable}/release-service/%{version}/src/%{name}-%{version}.tar.xz
Source1: https://download.kde.org/%{stable}/release-service/%{version}/src/%{name}-%{version}.tar.xz.sig
Source2: gpgkey-D81C0CB38EB725EF6691C385BB463350D6EF31EF.gpg

## upstreamable patches

## upstream patches

BuildRequires: gnupg2
BuildRequires: bzip2-devel
BuildRequires: desktop-file-utils

BuildRequires: extra-cmake-modules
BuildRequires: kf5-rpm-macros
BuildRequires: kf5-karchive-devel
BuildRequires: kf5-kconfig-devel
BuildRequires: kf5-kcrash-devel
BuildRequires: kf5-kdbusaddons-devel
BuildRequires: kf5-kdoctools-devel
BuildRequires: kf5-khtml-devel
BuildRequires: kf5-ki18n-devel
BuildRequires: kf5-kiconthemes-devel
BuildRequires: kf5-kio-devel
BuildRequires: kf5-kpty-devel
BuildRequires: kf5-kservice-devel
BuildRequires: kf5-kwidgetsaddons-devel
BuildRequires: cmake(KF5ItemModels)
BuildRequires: cmake(KF5Parts)
BuildRequires: cmake(KF5WindowSystem)
BuildRequires: cmake(KF5KIO)

BuildRequires: libappstream-glib
BuildRequires: pkgconfig(libarchive)
BuildRequires: pkgconfig(liblzma)
BuildRequires: pkgconfig(libzip)
BuildRequires: qt5-qtbase-devel
BuildRequires: zlib-devel

BuildRequires: /usr/bin/7z
BuildRequires: /usr/bin/lzop
BuildRequires: /usr/bin/unar

# when split occured
Conflicts: kdeutils-common < 6:4.7.80

# translations moved here
Conflicts: kde-l10n < 17.03

Obsoletes: kdeutils-ark < 6:4.7.80
Provides:  kdeutils-ark = 6:%{version}-%{release}

Requires: %{name}-libs%{?_isa} = %{version}-%{release}

# Dependencies for archive plugins.
# could split .desktop like okular to support these via
# TryExec=<foo> instead someday -- Rex
Requires: bzip2
Requires: gzip
%if 0%{?p7zip}
Requires: p7zip-plugins
%endif
Requires: unzip
# optional/soft dependencies
%if 0%{?fedora} > 23
Suggests: lha
Recommends: unar
%endif

Requires: /usr/bin/7z
Requires: /usr/bin/lzop
Requires: /usr/bin/unar

%description
Ark is a program for managing various archive formats.

Archives can be viewed, extracted, created and modified from within Ark.
The program can handle various formats such as tar, gzip, bzip2, zip,
rar and lha (if appropriate command-line programs are installed).

%package libs
Summary: Runtime libraries for %{name}
# libkerfuffle is BSD, plugins are mix of BSD and GPLv2+
License: BSD and GPLv2+
Requires: %{name} = %{version}-%{release}
Obsoletes: kdeutils-ark-libs < 6:4.7.80
Provides:  kdeutils-ark-libs = 6:%{version}-%{release}
Provides: ark-part = %{version}-%{release}
Provides: ark-part%{?_isa} = %{version}-%{release}
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
%find_lang %{name} --all-name --with-html --with-man


%check
appstream-util validate-relax --nonet %{buildroot}%{_kf5_metainfodir}/org.kde.ark.appdata.xml ||:
desktop-file-validate %{buildroot}%{_kf5_datadir}/applications/org.kde.ark.desktop


%files -f %{name}.lang
%license LICENSES/*.txt
%{_kf5_sysconfdir}/xdg/arkrc
%{_kf5_bindir}/ark
%{_kf5_datadir}/config.kcfg/ark.kcfg
%{_kf5_datadir}/kconf_update/ark.upd
%{_kf5_datadir}/kconf_update/ark_add_hamburgermenu_to_toolbar.sh
%{_kf5_metainfodir}/org.kde.ark.appdata.xml
%{_kf5_datadir}/applications/org.kde.ark.desktop
%{_kf5_datadir}/icons/hicolor/*/apps/ark.*
%{_kf5_datadir}/qlogging-categories5/*.categories
%{_mandir}/man1/ark.1*

%ldconfig_scriptlets

%files libs
%{_kf5_libdir}/libkerfuffle.so.*
%{_kf5_qtplugindir}/kerfuffle/
%{_kf5_qtplugindir}/kf5/parts/%{name}part.so
%{_kf5_plugindir}/kio_dnd/extracthere.so
%{_kf5_plugindir}/kfileitemaction/compressfileitemaction.so
%{_kf5_plugindir}/kfileitemaction/extractfileitemaction.so


%changelog
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

