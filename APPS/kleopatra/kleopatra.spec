Name:    kleopatra
Version: 23.08.0
Release: 1%{?dist}
Summary: KDE certificate manager and unified crypto GUI

# code: GPLv2+
# docs: GFDL
License: GPLv2+ and GFDL

URL:     http://projects.kde.org/?p=%{name}.git

%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0:        https://download.kde.org/%{stable}/release-service/%{version}/src/%{name}-%{version}.tar.xz
Source1:        https://download.kde.org/%{stable}/release-service/%{version}/src/%{name}-%{version}.tar.xz.sig
Source2:        gpgkey-D81C0CB38EB725EF6691C385BB463350D6EF31EF.gpg

## upstreamable patches

## downstream patches
# Reverting https://invent.kde.org/pim/kleopatra/-/commit/b9d9cd3ab15d1c7f5e97b80a5f19ffde2448c4cb
Patch0:         kleopatra-22.08.0-reverse-require-GpgME-version-1.16.0.patch

BuildRequires:  gnupg2
BuildRequires:  boost-devel
BuildRequires:  extra-cmake-modules >= 5.23.0
BuildRequires:  kf5-rpm-macros >= 5.23.0

BuildRequires:  cmake(Qt5Widgets)
BuildRequires:  cmake(Qt5Test)
BuildRequires:  cmake(Qt5Network)

BuildRequires:  gpgmepp-devel >= 1.7.1
BuildRequires:  cmake(QGpgme)
BuildRequires:  libassuan2-devel

BuildRequires:  cmake(KF5Codecs)
BuildRequires:  cmake(KF5DBusAddons)
BuildRequires:  cmake(KF5KCMUtils)
BuildRequires:  cmake(KF5Config)
BuildRequires:  cmake(KF5I18n)
BuildRequires:  cmake(KF5IconThemes)
BuildRequires:  cmake(KF5Notifications)
BuildRequires:  cmake(KF5XmlGui)
BuildRequires:  cmake(KF5WindowSystem)
BuildRequires:  cmake(KF5TextWidgets)
BuildRequires:  cmake(KF5DocTools)

%global majmin_ver %(echo %{version} | cut -d. -f1,2)
BuildRequires:  kf5-libkleo-devel >= %{majmin_ver} 
BuildRequires:  kf5-kmime-devel >= %{majmin_ver} 

Requires:       %{name}-libs%{?_isa} = %{version}-%{release}

%description
%{summary}.

%package        libs
Summary:        Runtime libraries for Kleopatra
Requires:       %{name} = %{version}-%{release}
%description    libs
%{summary}.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%setup -q

%if 0%{fedora} <= 35
%patch0 -p1 -b .reverse-require-GpgME-version-1.16.0
%endif


%build
%cmake_kf5
%cmake_build


%install
%cmake_install
%find_lang %{name} --all-name --with-html

# Remove non-version .so files, we don't have -devel pkg anyway
rm -fv %{buildroot}%{_kf5_libdir}/libkleopatraclientcore.so
rm -fv %{buildroot}%{_kf5_libdir}/libkleopatraclientgui.so


%files -f %{name}.lang
%license LICENSES/*.txt
%{_kf5_bindir}/kleopatra
%{_kf5_bindir}/kwatchgnupg
%{_kf5_metainfodir}/org.kde.kleopatra.appdata.xml
%{_kf5_datadir}/applications/org.kde.kleopatra.desktop
%{_kf5_datadir}/applications/kleopatra_import.desktop
%{_kf5_datadir}/kconf_update/*
%{_kf5_datadir}/kio/servicemenus/kleopatra_decryptverifyfiles.desktop
%{_kf5_datadir}/kio/servicemenus/kleopatra_decryptverifyfolders.desktop
%{_kf5_datadir}/kio/servicemenus/kleopatra_signencryptfiles.desktop
%{_kf5_datadir}/kio/servicemenus/kleopatra_signencryptfolders.desktop
%{_kf5_datadir}/kleopatra/
%{_kf5_datadir}/kwatchgnupg/
%{_kf5_datadir}/mime/packages/application-vnd-kde-%{name}.xml
%{_kf5_datadir}/icons/hicolor/*/apps/kleopatra.*
%{_kf5_datadir}/qlogging-categories5/*categories

%ldconfig_scriptlets libs

%files libs
%{_kf5_libdir}/libkleopatraclientcore.so.*
%{_kf5_libdir}/libkleopatraclientgui.so.*
%{_kf5_qtplugindir}/pim5/kcms/kleopatra/kleopatra_config_gnupgsystem.so


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

