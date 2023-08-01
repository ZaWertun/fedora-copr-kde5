Name:    kdenetwork-filesharing
Summary: Network filesharing
Version: 23.04.3
Release: 1%{?dist}

# KDE e.V. may determine that future GPL versions are accepted
License: GPLv2 or GPLv3
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
BuildRequires: extra-cmake-modules
BuildRequires: gettext

BuildRequires: kf5-kcoreaddons-devel
BuildRequires: kf5-kdoctools-devel
BuildRequires: kf5-ki18n-devel
BuildRequires: kf5-kio-devel
BuildRequires: kf5-kwidgetsaddons-devel
BuildRequires: kf5-rpm-macros
BuildRequires: cmake(KF5Declarative)

BuildRequires: pkgconfig(packagekitqt5)

BuildRequires: pkgconfig(Qt5Widgets)
BuildRequires: cmake(Qt5Qml)
BuildRequires: cmake(QCoro5)


# or gets pulled in via PK at runtime
Recommends: samba

# when split occurred
Conflicts: kdenetwork-common < 7:4.10.80
Obsoletes: kdenetwork-fileshare-samba < 7:4.10.80
Provides:  kdenetwork-fileshare-samba = 7:%{version}-%{release}

# translations moved here
Conflicts: kde-l10n < 17.03

%description
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


%files -f %{name}.lang
%license LICENSES/*.txt
%dir %{_kf5_plugindir}/propertiesdialog/
%{_kf5_plugindir}/propertiesdialog/SambaAcl.so
%{_kf5_plugindir}/propertiesdialog/sambausershareplugin.so
%{_kf5_metainfodir}/*.metainfo.xml
%{_kf5_libexecdir}/kauth/authhelper
%{_kf5_datadir}/dbus-1/system-services/org.kde.filesharing.samba.service
%{_kf5_datadir}/dbus-1/system.d/org.kde.filesharing.samba.conf
%{_kf5_datadir}/polkit-1/actions/org.kde.filesharing.samba.policy


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

