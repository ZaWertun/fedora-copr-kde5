#global tests 1

Name:    kio-extras
Version: 23.08.2
Release: 1%{?dist}
Summary: Additional components to increase the functionality of KIO Framework

License: GPLv2+
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

## upstramable patches

## upstream patches

# filter plugin provides
%global __provides_exclude_from ^(%{_kf5_qtplugindir}/.*\\.so)$

BuildRequires:  gnupg2
BuildRequires:  bzip2-devel
BuildRequires:  exiv2-devel
BuildRequires:  gperf

BuildRequires:  extra-cmake-modules
BuildRequires:  kf5-kactivities-devel
BuildRequires:  kf5-karchive-devel
BuildRequires:  kf5-kconfig-devel
BuildRequires:  kf5-kconfigwidgets-devel
BuildRequires:  kf5-kcoreaddons-devel
BuildRequires:  kf5-kdbusaddons-devel
BuildRequires:  kf5-kdelibs4support-devel
BuildRequires:  kf5-kdnssd-devel
BuildRequires:  kf5-kdoctools-devel
BuildRequires:  kf5-khtml-devel
BuildRequires:  kf5-ki18n-devel
BuildRequires:  kf5-kiconthemes-devel
BuildRequires:  kf5-kio-devel
BuildRequires:  kf5-kpty-devel
BuildRequires:  kf5-rpm-macros
BuildRequires:  kf5-solid-devel
BuildRequires:  cmake(KF5ActivitiesStats)
BuildRequires:  cmake(KF5SyntaxHighlighting)

BuildRequires:  libjpeg-devel
BuildRequires:  libmtp-devel
BuildRequires:  libsmbclient-devel
BuildRequires:  libssh-devel
BuildRequires:  OpenEXR-devel
BuildRequires:  openslp-devel
BuildRequires:  perl-generators
BuildRequires:  phonon-qt5-devel
BuildRequires:  pkgconfig(zlib)
BuildRequires:  pkgconfig(libtirpc)
BuildRequires:  pkgconfig(shared-mime-info)
BuildRequires:  qt5-qtbase-devel
BuildRequires:  qt5-qtsvg-devel
BuildRequires:  taglib-devel > 1.11
BuildRequires:  cmake(KDSoap)
BuildRequires:  cmake(KF5KExiv2)

%if 0%{?tests}
BuildRequires: dbus-x11
BuildRequires: time
BuildRequires: xorg-x11-server-Xvfb
%endif

# translations moved here
Conflicts: kde-l10n < 17.03

# short-lived subpkg, locale conflicts fixed in kio_mtp instead
Obsoletes: kio-extras-mtp-common < 5.2.2-3

Obsoletes: kde-runtime-docs < 5.0.0-1
# when went noarch
Obsoletes: kio-extras-doc < 5.8.0-2
# moved to main pkg
Obsoletes: kio-extras-docs < 17.03
Provides:  kio-extras-docs = %{version}-%{release}

# -htmlthumbnail removed
Obsoletes: kio-extras-htmlthumbnail < 18.08.3

# helpful for  imagethumbnail plugin
Recommends: qt5-qtimageformats%{?_isa}

# Available in RPMFusion: https://admin.rpmfusion.org/pkgdb/package/free/qt-heif-image-plugin/
Recommends: qt-heif-image-plugin%{?_isa}

# .exe/.ico previews, will limit dep to only if wine-core is installed for now -- rdieter
Recommends: (icoutils if wine-core)

# when -info was split out
Obsoletes: kio-extras < 19.04.1-1

%description
%{summary}.

%package info
Summary: Info kioslave
# when -info was split out
Obsoletes: kio-extras < 19.04.1-1
%description info
Kioslave for reading info pages.

%package        devel
Summary:        Development files for %{name}
Requires:       %{name}%{?_isa} = %{version}-%{release}
%description devel
%{summary}.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -p1


%build
%cmake_kf5 \
  %{?tests:-DBUILD_TESTING:BOOL=ON}
%cmake_build


%install
%cmake_install
%find_lang %{name} --all-name --with-html


%check
%if 0%{?tests}
export CTEST_OUTPUT_ON_FAILURE=1
xvfb-run -a dbus-launch --exit-with-session \
time make test -C %{_vpath_builddir} ARGS="--output-on-failure --timeout 30" ||:
%endif


%ldconfig_scriptlets

%files -f %{name}.lang
# include *a* copy, others are in mtp/
%license LICENSES/*.txt
%{_kf5_libdir}/libkioarchive.so.5*
%dir %{_kf5_plugindir}/kded
%{_kf5_plugindir}/kded/filenamesearchmodule.so
%{_kf5_plugindir}/kded/recentdocumentsnotifier.so
%dir %{_kf5_plugindir}/kio/
%dir %{_kf5_plugindir}/kiod/
%{_kf5_plugindir}/kio/kio_filenamesearch.so
%{_kf5_plugindir}/kio/activities.so
%{_kf5_plugindir}/kio/archive.so
%{_kf5_plugindir}/kio/bookmarks.so
%{_kf5_plugindir}/kio/filter.so
%{_kf5_plugindir}/kio/fish.so
%{_kf5_plugindir}/kio/man.so
%{_kf5_plugindir}/kiod/kmtpd.so
%{_kf5_plugindir}/kio/mtp.so
%{_kf5_plugindir}/kio/nfs.so
%{_kf5_plugindir}/kio/recentdocuments.so
%{_kf5_plugindir}/kio/sftp.so
%{_kf5_plugindir}/kio/smb.so
%{_kf5_plugindir}/kio/thumbnail.so
%{_kf5_plugindir}/kio/recentlyused.so
%{_kf5_plugindir}/thumbcreator/audiothumbnail.so
%{_kf5_plugindir}/thumbcreator/comicbookthumbnail.so
%{_kf5_plugindir}/thumbcreator/djvuthumbnail.so
%{_kf5_plugindir}/thumbcreator/ebookthumbnail.so
%{_kf5_plugindir}/thumbcreator/exrthumbnail.so
%{_kf5_plugindir}/thumbcreator/imagethumbnail.so
%{_kf5_plugindir}/thumbcreator/jpegthumbnail.so
%{_kf5_plugindir}/thumbcreator/kraorathumbnail.so
%{_kf5_plugindir}/thumbcreator/opendocumentthumbnail.so
%{_kf5_plugindir}/thumbcreator/svgthumbnail.so
%{_kf5_plugindir}/thumbcreator/textthumbnail.so
%{_kf5_plugindir}/thumbcreator/windowsexethumbnail.so
%{_kf5_plugindir}/thumbcreator/windowsimagethumbnail.so
%{_kf5_plugindir}/kfileitemaction/forgetfileitemaction.so
%{_kf5_plugindir}/kfileitemaction/kactivitymanagerd_fileitem_linking_plugin.so
%{_kf5_qtplugindir}/kfileaudiopreview.so
%{_datadir}/kio_docfilter/
%{_datadir}/kio_bookmarks/
%dir %{_datadir}/konqueror/
%dir %{_datadir}/konqueror/dirtree/
%dir %{_datadir}/konqueror/dirtree/remote/
%{_datadir}/konqueror/dirtree/remote/mtp-network.desktop
%{_datadir}/konqueror/dirtree/remote/smb-network.desktop
%{_datadir}/remoteview/
%{_kf5_datadir}/mime/packages/org.kde.kio.smb.xml
%{_kf5_datadir}/solid/actions/solid_mtp.desktop
%{_kf5_datadir}/dbus-1/services/org.kde.kmtpd5.service
%{_kf5_datadir}/kservices5/directorythumbnail.desktop
%{_kf5_datadir}/kservicetypes5/thumbcreator.desktop
%{_datadir}/config.kcfg/jpegcreatorsettings5.kcfg
%{_kf5_datadir}/qlogging-categories5/*categories
%{_kf5_libexecdir}/smbnotifier
%{_kf5_plugindir}/kded/smbwatcher.so


%files info
%{_kf5_plugindir}/kio/info.so
# perl deps, but required at runtime for the info kioslave to actually work:
%dir %{_datadir}/kio_info/
%{_datadir}/kio_info/kde-info2html*

%files devel
%{_includedir}/KioArchive/
# no soname symlink? --rex
%{_kf5_libdir}/cmake/KioArchive/


%changelog
* Fri Oct 13 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.08.2-1
- 23.08.2

* Thu Sep 14 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.08.1-1
- 23.08.1

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

