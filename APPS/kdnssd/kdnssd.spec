Name:    kdnssd
Summary: KDE Network Monitor for DNS-SD services (Zeroconf)
Version: 23.08.0
Release: 1%{?dist}

# KDE e.V. may determine that future GPL versions are accepted
License: GPLv2 or GPLv3
URL:     https://cgit.kde.org/zeroconf-ioslave.git

%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0: https://download.kde.org/%{stable}/release-service/%{version}/src/kio-zeroconf-%{version}.tar.xz
Source1: https://download.kde.org/%{stable}/release-service/%{version}/src/kio-zeroconf-%{version}.tar.xz.sig
Source2: gpgkey-D81C0CB38EB725EF6691C385BB463350D6EF31EF.gpg

# new upstream name in 4.12.95
Provides: zeroconf-ioslave = %{version}-%{release}

BuildRequires: gnupg2
BuildRequires: extra-cmake-modules
BuildRequires: cmake(KF5DBusAddons)
BuildRequires: cmake(KF5DNSSD)
BuildRequires: cmake(KF5I18n)
BuildRequires: cmake(KF5KIO)

BuildRequires: pkgconfig(avahi-compat-libdns_sd)

# when split occurred
Conflicts: kdenetwork-common < 7:4.10.80
Obsoletes: kdenetwork-kdnssd < 7:4.10.80
Provides:  kdenetwork-kdnssd = 7:%{version}-%{release}


%description
%{summary}.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -n kio-zeroconf-%{version} -p1


%build
%cmake_kf5
%cmake_build


%install
%cmake_install
%find_lang %{name} --all-name --with-html


%files -f %{name}.lang
%license LICENSES/*.txt
%{_kf5_plugindir}/kio/zeroconf.so
%{_kf5_qtplugindir}/kf5/kded/dnssdwatcher.so
%{_kf5_datadir}/dbus-1/interfaces/org.kde.kdnssd.xml
%dir %{_kf5_datadir}/remoteview/
%{_kf5_datadir}/remoteview/zeroconf.desktop
%{_kf5_metainfodir}/*.metainfo.xml


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

