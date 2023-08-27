Name:    gwenview
Summary: An image viewer
Epoch:   1
Version: 23.08.0
Release: 1%{?dist}

# app: GPLv2+
# lib:  IJG and (LGPLv2 or LGPLv3 or LGPLv3+ (KDE e.V.)) and LGPLv2+ and GPLv2+
License: GPLv2+
URL:     https://www.kde.org/applications/graphics/gwenview/
%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif 
Source0: https://download.kde.org/%{stable}/release-service/%{version}/src/%{name}-%{version}.tar.xz
Source1: https://download.kde.org/%{stable}/release-service/%{version}/src/%{name}-%{version}.tar.xz.sig
Source2: gpgkey-D81C0CB38EB725EF6691C385BB463350D6EF31EF.gpg

%global majmin_ver %(echo %{version} | cut -d. -f1,2)

## upstream patches (master branch)

## upstreamable patches

BuildRequires: gnupg2
BuildRequires: desktop-file-utils
BuildRequires: extra-cmake-modules
BuildRequires: gettext
BuildRequires: kf5-rpm-macros
BuildRequires: kf5-baloo-devel
BuildRequires: kf5-kactivities-devel
BuildRequires: kf5-kdelibs4support-devel
BuildRequires: kf5-kio-devel
BuildRequires: kf5-libkdcraw-devel
BuildRequires: kf5-libkipi-devel
BuildRequires: kf5-purpose-devel
BuildRequires: libappstream-glib
BuildRequires: libjpeg-devel
BuildRequires: libtiff-devel
BuildRequires: cfitsio-devel
BuildRequires: pkgconfig(exiv2)
BuildRequires: pkgconfig(lcms2)
BuildRequires: pkgconfig(libpng)
BuildRequires: pkgconfig(phonon4qt5)
BuildRequires: pkgconfig(Qt5DBus) pkgconfig(Qt5Widgets) pkgconfig(Qt5Script) pkgconfig(Qt5Test)
BuildRequires: pkgconfig(Qt5Concurrent) pkgconfig(Qt5Svg) pkgconfig(Qt5OpenGL)
BuildRequires: pkgconfig(Qt5X11Extras)
BuildRequires: cmake(kImageAnnotator)
BuildRequires: cmake(kColorPicker)

BuildRequires: wayland-protocols-devel
BuildRequires: cmake(Qt5WaylandClient)
BuildRequires: pkgconfig(wayland-client)
BuildRequires: qt5-qtbase-private-devel

Requires: %{name}-libs%{?_isa} = %{epoch}:%{version}-%{release}

# support for more formats, e.g. jp2, tiff, webp
Recommends: qt5-qtimageformats%{?_isa}
# eps, etc...
Recommends: kf5-kimageformats%{?_isa}

# when split occurred
Conflicts: kdegraphics < 7:4.6.95-10

# translations moved here
Conflicts: kde-l10n < 17.03

%description
%{summary}.

%package  libs 
Summary:  Runtime files for %{name} 
# wrt (LGPLv2 or LGPLv3), KDE e.V. may determine that future GPL versions are accepted 
License:  IJG and LGPLv2+ and GPLv2+ and LGPLv2 or LGPLv3
Requires: %{name} = %{epoch}:%{version}-%{release}
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
%find_lang %{name} --all-name --with-html

# Add JPEG JXL image support
sed -e '/MimeType=/s/$/image\/jxl;/' -i %{buildroot}%{_kf5_datadir}/applications/org.kde.gwenview.desktop


%check
appstream-util validate-relax --nonet %{buildroot}%{_kf5_metainfodir}/org.kde.gwenview.appdata.xml
desktop-file-validate %{buildroot}%{_kf5_datadir}/applications/org.kde.gwenview.desktop


%files -f %{name}.lang
%license COPYING
%{_kf5_bindir}/gwenview
%{_kf5_bindir}/gwenview_importer
%{_kf5_datadir}/applications/org.kde.gwenview.desktop
%{_kf5_datadir}/applications/org.kde.gwenview_importer.desktop
%{_kf5_datadir}/icons/hicolor/*/*/*
%{_kf5_datadir}/gwenview/
%{_kf5_datadir}/kconf_update/gwenview*
%{_kf5_datadir}/solid/actions/gwenview_importer*.desktop
%{_kf5_metainfodir}/org.kde.gwenview.appdata.xml

%ldconfig_scriptlets libs

%files libs
%{_kf5_libdir}/libgwenviewlib.so.*
%{_kf5_qtplugindir}/kf5/parts/gvpart.so
%{_kf5_datadir}/kservices5/gvpart.desktop
%{_kf5_datadir}/qlogging-categories5/%{name}.categories
%{_kf5_plugindir}/kfileitemaction/slideshowfileitemaction.so


%changelog
* Sun Aug 27 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 1:23.08.0-1
- 23.08.0

* Thu Jul 06 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 1:23.04.3-1
- 23.04.3

* Wed Jun 14 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 1:23.04.2-5
- rebuild

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

