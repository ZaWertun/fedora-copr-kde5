Name:    kdegraphics-thumbnailers
Summary: Thumbnailers for various graphic types 
Version: 23.04.3
Release: 1%{?dist}

# most sources GPLv2+, dscparse.* GPL, gscreator.* LGPLv2+, 
License: GPLv2+
URL:     https://www.kde.org/applications/graphics/

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
BuildRequires: extra-cmake-modules
BuildRequires: kf5-rpm-macros
BuildRequires: cmake(KF5KDcraw)
BuildRequires: cmake(KF5KExiv2)
BuildRequires: cmake(KF5KIO)
BuildRequires: cmake(KF5Archive)
BuildRequires: cmake(Qt5Gui)
BuildRequires: cmake(KF5I18n)
BuildRequires: cmake(QMobipocket)

%description
%{summary}.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup


%build
%cmake_kf5
%cmake_build


%install
%cmake_install


%files
%license COPYING*
%{_kf5_qtplugindir}/gsthumbnail.so
%{_kf5_datadir}/kservices5/gsthumbnail.desktop
%{_kf5_qtplugindir}/rawthumbnail.so
%{_kf5_datadir}/kservices5/rawthumbnail.desktop
%{_kf5_qtplugindir}/blenderthumbnail.so
%{_kf5_datadir}/kservices5/blenderthumbnail.desktop
%{_kf5_qtplugindir}/mobithumbnail.so
%{_kf5_datadir}/kservices5/mobithumbnail.desktop
%{_kf5_metainfodir}/org.kde.%{name}.metainfo.xml


%changelog
* Tue Jul 18 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.04.3-1
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

