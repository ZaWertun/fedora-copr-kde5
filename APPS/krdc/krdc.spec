%if 0%{?fedora} > 20
%global freerdp11 1
%global freerdp_ver 1:1.1.0
%else
%global freerdp_ver 1:1.0.2
%endif

Name:    krdc
Summary: Remote desktop client
Version: 23.08.0
Release: 1%{?dist}

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

BuildRequires: gnupg2
BuildRequires: desktop-file-utils
BuildRequires: libappstream-glib

BuildRequires: extra-cmake-modules
BuildRequires: gettext
BuildRequires: kf5-kbookmarks-devel
BuildRequires: kf5-kcompletion-devel
BuildRequires: kf5-kconfig-devel
BuildRequires: kf5-kcoreaddons-devel
BuildRequires: kf5-kcmutils-devel
BuildRequires: kf5-kcrash-devel
BuildRequires: kf5-kdoctools-devel
BuildRequires: kf5-kdbusaddons-devel
BuildRequires: kf5-kdnssd-devel
BuildRequires: kf5-ki18n-devel
BuildRequires: kf5-kiconthemes-devel
BuildRequires: kf5-knotifications-devel
BuildRequires: kf5-knotifyconfig-devel
BuildRequires: kf5-kwallet-devel
BuildRequires: kf5-kwidgetsaddons
BuildRequires: kf5-kxmlgui-devel
BuildRequires: kf5-rpm-macros
BuildRequires: kf5-kwindowsystem-devel

BuildRequires: freerdp >= %{freerdp_ver}
BuildRequires: zlib-devel
BuildRequires: libvncserver-devel
BuildRequires: libssh-devel
# for hicolor icon hack in %%prep
BuildRequires: oxygen-icon-theme

Requires: %{name}-libs%{?_isa} = %{version}-%{release}
Requires: freerdp >= %{freerdp_ver}
%if ! 0%{?freerdp11}
Requires: freerdp < 1:1.1.0
%endif

# when split occurred
Conflicts: kdenetwork-common < 7:4.10.80
Obsoletes: kdenetwork-krdc < 7:4.10.80
Provides:  kdenetwork-krdc = 7:%{version}-%{release}

# translations moved here
Conflicts: kde-l10n < 17.03

%description
%{summary}.

%package libs
Summary: Runtime libraries for %{name}
Requires: %{name} = %{version}-%{release}
Obsoletes: kdenetwork-krdc-libs < 7:4.10.80
Provides:  kdenetwork-krdc-libs = 7:%{version}-%{release}
%description libs
%{summary}.

%package devel
Summary: Developer files for %{name}
Requires: %{name}-libs%{?_isa} = %{version}-%{release}
Obsoletes: kdenetwork-krdc-devel < 7:4.10.80
Provides:  kdenetwork-krdc-devel = 7:%{version}-%{release}
%description devel
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

# workaround https://bugs.kde.org/show_bug.cgi?id=365986
mkdir -p %{buildroot}%{_datadir}/icons/hicolor
pushd %{_datadir}/icons/oxygen/
for icon in $(find */apps -name krdc.*) $(find base/*/apps -name krdc.*); do
cp -v --parents -n ${icon} %{buildroot}%{_datadir}/icons/hicolor/
done
mv %{buildroot}%{_datadir}/icons/hicolor/base/* %{buildroot}%{_datadir}/icons/hicolor/ ||:


%check
appstream-util validate-relax --nonet %{buildroot}%{_kf5_metainfodir}/org.kde.%{name}.appdata.xml
desktop-file-validate %{buildroot}%{_kf5_datadir}/applications/org.kde.%{name}.desktop


%files -f %{name}.lang
%license LICENSES/*
%{_kf5_bindir}/krdc
%{_kf5_datadir}/kio/servicemenus/smb2rdc.desktop
%{_kf5_datadir}/applications/org.kde.krdc.desktop
%{_kf5_datadir}/icons/hicolor/*/apps/krdc.*
%{_kf5_datadir}/config.kcfg/krdc.kcfg
%{_kf5_datadir}/qlogging-categories5/*categories
%{_kf5_metainfodir}/org.kde.%{name}.appdata.xml

%ldconfig_scriptlets libs

%files libs
%{_kf5_libdir}/libkrdccore.so.5*
%{_kf5_libdir}/libkrdccore.so.%{version}
%{_kf5_qtplugindir}/krdc/

%files devel
%{_includedir}/krdc/
%{_includedir}/krdccore_export.h
%{_kf5_libdir}/libkrdccore.so


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

