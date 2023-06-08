Name:    ffmpegthumbs
Version: 23.04.2
Release: 1%{?dist}
Summary: KDE ffmpegthumbnailer service

License: GPL-2.0-or-later
URL:     https://apps.kde.org/%{name}/
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
BuildRequires: gcc-c++
BuildRequires: cmake
BuildRequires: desktop-file-utils
BuildRequires: libappstream-glib

BuildRequires: extra-cmake-modules
BuildRequires: pkgconfig(libjpeg)
BuildRequires: cmake(KF5KIO)
BuildRequires: cmake(KF5I18n)
BuildRequires: ffmpeg-free-devel
BuildRequires: pkgconfig(taglib)

Provides: kffmpegthumbnailer = %{version}-%{release}
Provides: kdemultimedia-extras-freeworld = %{version}-%{release}

%description
%{summary}.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -p1


%build
%{cmake_kf5}

%cmake_build


%install
%cmake_install


%check
appstream-util validate-relax --nonet %{buildroot}%{_kf5_metainfodir}/org.kde.%{name}.metainfo.xml


%files
%license LICENSES/GPL-2.0-or-later.txt
%{_kf5_qtplugindir}/ffmpegthumbs.so
%dir %{_kf5_datadir}/kservices5
%{_kf5_datadir}/kservices5/ffmpegthumbs.desktop
%dir %{_kf5_datadir}/config.kcfg
%{_kf5_datadir}/config.kcfg/ffmpegthumbnailersettings5.kcfg
%dir %{_kf5_datadir}/qlogging-categories5
%{_kf5_datadir}/qlogging-categories5/ffmpegthumbs.categories
%{_kf5_metainfodir}/org.kde.%{name}.metainfo.xml


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

