## uncomment to enable bootstrap mode
#global bootstrap 1

%if !0%{?bootstrap}
%if 0%{?fedora}
%global chm 1
%global ebook 1
%endif
# uncomment to include -mobile (currently doesn't work)
# it links libokularpart.so, but fails to file/load at runtime
%global mobile 1
%endif

Name:    okular 
Summary: A document viewer
Version: 23.04.2
Release: 1%{?dist}

License: GPLv2
URL:     https://www.kde.org/applications/graphics/okular/

%global majmin_ver %(echo %{version} | cut -d. -f1,2)
%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0: http://download.kde.org/%{stable}/release-service/%{version}/src/%{name}-%{version}.tar.xz
Source1: http://download.kde.org/%{stable}/release-service/%{version}/src/%{name}-%{version}.tar.xz.sig
Source2: gpgkey-D81C0CB38EB725EF6691C385BB463350D6EF31EF.gpg

## upstream patches

BuildRequires: gnupg2
BuildRequires: desktop-file-utils
BuildRequires: libappstream-glib

BuildRequires: extra-cmake-modules
BuildRequires: kf5-rpm-macros
BuildRequires: cmake(KDEExperimentalPurpose)
BuildRequires: cmake(KF5Activities)
BuildRequires: cmake(KF5Archive)
BuildRequires: cmake(KF5Bookmarks)
BuildRequires: cmake(KF5Completion)
BuildRequires: cmake(KF5Config)
BuildRequires: cmake(KF5ConfigWidgets)
BuildRequires: cmake(KF5CoreAddons)
BuildRequires: cmake(KF5Crash)
BuildRequires: cmake(KF5DBusAddons)
BuildRequires: cmake(KF5DocTools)
BuildRequires: cmake(KF5IconThemes)
BuildRequires: cmake(KF5JS)
BuildRequires: cmake(KF5KIO)
BuildRequires: cmake(KF5Kirigami2)
BuildRequires: cmake(KF5Parts)
BuildRequires: cmake(KF5Pty)
BuildRequires: cmake(KF5ThreadWeaver)
BuildRequires: cmake(KF5Wallet)
BuildRequires: cmake(KF5KHtml)
BuildRequires: cmake(KF5WindowSystem)

BuildRequires: cmake(Qt5DBus)
BuildRequires: cmake(Qt5Test)
BuildRequires: cmake(Qt5Widgets)
BuildRequires: cmake(Qt5PrintSupport)
BuildRequires: cmake(Qt5Svg)
BuildRequires: cmake(Qt5Qml)
BuildRequires: cmake(Qt5Quick)
BuildRequires: cmake(Qt5X11Extras)
BuildRequires: qt5-qtbase-private-devel

BuildRequires: pkgconfig(phonon4qt5)
BuildRequires: cmake(Qca-qt5)

## generater/plugin deps
BuildRequires: cmake(KF5KExiv2)
BuildRequires: kdegraphics-mobipocket-devel
%if 0%{?chm}
BuildRequires: chmlib-devel
BuildRequires: pkgconfig(libzip)
%endif
%if 0%{?ebook}
BuildRequires: ebook-tools-devel
%endif
BuildRequires: libjpeg-devel
BuildRequires: libtiff-devel
BuildRequires: pkgconfig(freetype2)
BuildRequires: pkgconfig(libmarkdown)
BuildRequires: pkgconfig(libspectre)
BuildRequires: pkgconfig(poppler-qt5)
%if 0%{?fedora}
BuildRequires: pkgconfig(ddjvuapi) 
BuildRequires: pkgconfig(qca2)
BuildRequires: pkgconfig(zlib)
%endif

%if !0%{?bootstrap}
BuildRequires:  cmake(Qt5TextToSpeech)
%endif

Requires: %{name}-part%{?_isa} = %{version}-%{release}

%description
%{summary}.

%if 0%{?mobile}
%package mobile
Summary: Document viewer for plasma mobile
# included last in okular-15.12.3-1.fc23
Obsoletes: okular-active < 16.04
Requires: %{name}-part%{?_isa} = %{version}-%{release}
%description mobile
%{summary}.
%endif

%package devel
Summary:  Development files for %{name}
Provides: okular5-devel = %{version}-%{release}
Requires: %{name}-libs%{?_isa} = %{version}-%{release}
%description devel
%{summary}.

%package  libs 
Summary:  Runtime files for %{name} 
%if 0%{?fedora}
# use Recommends to avoid hard deps -- rex
## lpr
Recommends: cups-client
## ps2pdf,pdf2ps
Recommends: ghostscript-core
%endif
%description libs 
%{summary}.

%package part
Summary: Okular kpart plugin
Provides: okular5-part = %{version}-%{release}
Provides: okular5-part%{?_isa} = %{version}-%{release}
Requires: %{name}-libs%{?_isa} = %{version}-%{release}
# translations moved here
Conflicts: kde-l10n < 17.03
%description part
%{summary}.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -p1

%if ! 0%{?mobile}
# disable/omit mobile, it doesn't work -- rex
sed -i -e 's|^add_subdirectory( mobile )|#add_subdirectory( mobile )|' CMakeLists.txt
%endif


%build
%cmake_kf5 \
%if 0%{?mobile}
  -DOKULAR_UI=both
%else
  -DOKULAR_UI=desktop
%endif
%cmake_build


%install
%cmake_install

%find_lang all --all-name --with-html --with-man
grep -v \
  -e %{_mandir} \
  -e %{_kf5_docdir} \
  all.lang > okular-part.lang
cat all.lang okular-part.lang | sort | uniq -u > okular.lang

# unpackaged files
%if ! 0%{?mobile}
rm -fv %{buildroot}%{_kf5_datadir}/applications/org.kde.mobile.okular_*.desktop
%endif


%check
desktop-file-validate %{buildroot}%{_kf5_datadir}/applications/org.kde.okular.desktop
appstream-util validate-relax --nonet %{buildroot}%{_kf5_metainfodir}/org.kde.okular.appdata.xml
%if 0%{?mobile}
desktop-file-validate %{buildroot}%{_kf5_datadir}/applications/org.kde.okular.kirigami.desktop
appstream-util validate-relax --nonet %{buildroot}%{_kf5_metainfodir}/org.kde.okular.kirigami.appdata.xml
%endif

%files -f okular.lang
%license LICENSES/*.txt
%{_kf5_bindir}/okular
%{_kf5_datadir}/applications/org.kde.okular.desktop
%{_kf5_metainfodir}/org.kde.okular.appdata.xml
%{_kf5_datadir}/applications/okularApplication_*.desktop
%{_kf5_metainfodir}/org.kde.okular-*.metainfo.xml
%{_kf5_datadir}/okular/
%{_kf5_datadir}/icons/hicolor/*/*/*
%{_kf5_datadir}/kconf_update/okular.upd
%{_kf5_datadir}/qlogging-categories5/%{name}*
%{_mandir}/man1/okular.1*

%if 0%{?mobile}
%files mobile
%{_bindir}/okularkirigami
%{_qt5_qmldir}/org/kde/okular/
%{_kf5_metainfodir}/org.kde.okular.kirigami.appdata.xml
%{_kf5_datadir}/applications/org.kde.okular.kirigami.desktop
%{_kf5_datadir}/applications/org.kde.mobile.okular_*.desktop
%endif

%files devel
%{_includedir}/okular/
%{_libdir}/libOkular5Core.so
%{_libdir}/cmake/Okular5/

%ldconfig_scriptlets libs

%files libs
%{_libdir}/libOkular5Core.so.10*

%files part -f okular-part.lang
%if 0%{?fedora}
%{_kf5_plugindir}/kio/kio_msits.so
%endif
%{_kf5_datadir}/config.kcfg/*.kcfg
%{_kf5_datadir}/kservices5/okular[A-Z]*.desktop
%{_kf5_datadir}/kservices5/okular_part.desktop
%{_kf5_datadir}/kservicetypes5/okularGenerator.desktop
%{_kf5_datadir}/kxmlgui5/okular/
%dir %{_qt5_plugindir}/okular/
%dir %{_qt5_plugindir}/okular/generators/
%{_qt5_plugindir}/okular/generators/okularGenerator_*.so
%{_qt5_plugindir}/okularpart.so


%changelog
* Thu Jun 08 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.04.2-1
- 23.04.2

* Thu May 11 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.04.1-1
- 23.04.1

* Thu Apr 20 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.04.0-1
- 23.04.0

* Wed Apr 05 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.12.3-2
- build okular kirigami

* Thu Mar 02 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.12.3-1
- 22.12.3

