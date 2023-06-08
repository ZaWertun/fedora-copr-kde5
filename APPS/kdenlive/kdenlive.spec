%global _lto_cflags %{nil}
%undefine __cmake_in_source_build

Name:    kdenlive
Summary: Non-linear video editor
Version: 22.08.2
Release: 1%{?dist}

License: GPLv2+
URL:     http://www.kdenlive.org
%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0:   https://download.kde.org/%{stable}/release-service/%{version}/src/kdenlive-%{version}.tar.xz
Source1:   https://download.kde.org/%{stable}/release-service/%{version}/src/kdenlive-%{version}.tar.xz.sig
Source2:   gpgkey-D81C0CB38EB725EF6691C385BB463350D6EF31EF.gpg
Source3:   https://github.com/rttrorg/rttr/archive/v0.9.6.tar.gz
Source100: kdenlive-find-lang.sh

Patch1:   rttr_cmakelists.patch

Patch100: kdenlive-21.08.0-mlt_melt.patch

# Add support for finding html files with find-lang.sh --with-html on epel
# https://github.com/rpm-software-management/rpm/commit/0c42871ff407a3eeb1e8b8c1de9395f35659c987
%if 0%{?rhel}
# copied from openshot
# Redirect find_lang to our patched version
%global find_lang %{_sourcedir}/kdenlive-find-lang.sh %{buildroot}
%endif

BuildRequires: gnupg2
BuildRequires: desktop-file-utils
BuildRequires: extra-cmake-modules
BuildRequires: gettext
BuildRequires: cmake(KF5Archive)
BuildRequires: cmake(KF5Bookmarks)
BuildRequires: cmake(KF5Config)
BuildRequires: cmake(KF5ConfigWidgets)
BuildRequires: cmake(KF5CoreAddons)
BuildRequires: cmake(KF5DocTools)
BuildRequires: cmake(KF5DBusAddons)
BuildRequires: cmake(KF5GuiAddons)
BuildRequires: cmake(KF5I18n)
BuildRequires: cmake(KF5IconThemes)
BuildRequires: cmake(KF5ItemViews)
BuildRequires: cmake(KF5KIO)
BuildRequires: cmake(KF5JobWidgets)
BuildRequires: cmake(KF5NewStuff)
BuildRequires: cmake(KF5Notifications)
BuildRequires: cmake(KF5NotifyConfig)
BuildRequires: cmake(KF5Plotting)
BuildRequires: cmake(KF5Purpose)
BuildRequires: cmake(KF5TextWidgets)
BuildRequires: cmake(KF5XmlGui)
BuildRequires: cmake(KF5Crash)
BuildRequires: cmake(KF5FileMetaData)
BuildRequires: cmake(KF5Declarative)
BuildRequires: libappstream-glib

BuildRequires: pkgconfig(libv4l2)
BuildRequires: pkgconfig(mlt++) >= 6.16.0
%global mlt_version %(pkg-config --modversion mlt++ 2>/dev/null || echo 6.6.0)

BuildRequires: pkgconfig(Qt5Concurrent)
BuildRequires: pkgconfig(Qt5DBus)
BuildRequires: pkgconfig(Qt5OpenGL)
BuildRequires: pkgconfig(Qt5Script)
BuildRequires: pkgconfig(Qt5Svg)
BuildRequires: pkgconfig(Qt5Qml)
BuildRequires: pkgconfig(Qt5Quick)
BuildRequires: pkgconfig(Qt5Widgets)
BuildRequires: pkgconfig(Qt5WebKitWidgets)
BuildRequires: pkgconfig(Qt5Multimedia)
BuildRequires: pkgconfig(Qt5QuickControls2)
BuildRequires: cmake(Qt5NetworkAuth)

## workaround for missing dependency in kf5-kio, can remove
## once kf5-kio-5.24.0-2 (or newer is available)
BuildRequires: kf5-kinit-devel
%{?kf5_kinit_requires}
Requires: dvdauthor
Requires: dvgrab
Requires: ffmpeg
%if 0%{?fedora} >= 25 || 0%{?rhel} >= 7
Requires: mlt-freeworld%{?_isa} >= %{mlt_version}
%else
Requires: mlt%{?_isa} >= %{mlt_version}
%endif
Requires: qt5-qtquickcontrols

%description
Kdenlive is an intuitive and powerful multi-track video editor, including most
recent video technologies.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -p1


%build
mkdir -p %{__cmake_builddir}/rttr/src
cp -v %{SOURCE3} %{__cmake_builddir}/rttr/src
%cmake_kf5 \
  -DKDE_INSTALL_USE_QT_SYS_PATHS:BOOL=ON
%cmake_build


%install
%cmake_install

## unpackaged files
rm -rfv  %{buildroot}%{_datadir}/doc/Kdenlive/

%find_lang %{name} --with-html --all-name


%check
appstream-util validate-relax --nonet %{buildroot}%{_kf5_metainfodir}/org.kde.%{name}.appdata.xml ||:
desktop-file-validate %{buildroot}%{_kf5_datadir}/applications/org.kde.%{name}.desktop


%if 0%{?rhel} && 0%{?rhel} < 8
%post
/usr/bin/update-desktop-database &> /dev/null || :
/bin/touch --no-create %{_kf5_datadir}/icons/hicolor &> /dev/null || :
/bin/touch --no-create %{_kf5_datadir}/mime/packages &> /dev/null || :

%postun
if [ $1 -eq 0 ] ; then
  /bin/touch --no-create %{_kf5_datadir}/icons/hicolor &>/dev/null
  /usr/bin/gtk-update-icon-cache %{_kf5_datadir}/icons/hicolor &>/dev/null || :
  /bin/touch --no-create %{_kf5_datadir}/mime/packages &> /dev/null || :
  /usr/bin/update-mime-database %{_kf5_datadir}/mime &> /dev/null || :
  /usr/bin/update-desktop-database &> /dev/null || :
fi

%posttrans
/usr/bin/gtk-update-icon-cache %{_kf5_datadir}/icons/hicolor &> /dev/null || :
/usr/bin/update-mime-database %{?fedora:-n} %{_kf5_datadir}/mime &> /dev/null || :
%endif

%files -f %{name}.lang
%doc AUTHORS README*
%license COPYING
%{_kf5_bindir}/kdenlive_render
%{_kf5_bindir}/%{name}
%{_kf5_datadir}/applications/org.kde.%{name}.desktop
%{_kf5_metainfodir}/org.kde.%{name}.appdata.xml
%{_kf5_datadir}/kdenlive/
%{_kf5_datadir}/mime/packages/org.kde.kdenlive.xml
%{_kf5_datadir}/mime/packages/westley.xml
%{_kf5_datadir}/icons/hicolor/*/*/*
%{_kf5_datadir}/config.kcfg/kdenlivesettings.kcfg
%{_kf5_datadir}/knotifications5/kdenlive.notifyrc
%{_kf5_datadir}/kservices5/mltpreview.desktop
%{_kf5_datadir}/kxmlgui5/kdenlive/
%{_kf5_datadir}/knsrcfiles/kdenlive_keyboardschemes.knsrc
%{_kf5_datadir}/knsrcfiles/kdenlive_renderprofiles.knsrc
%{_kf5_datadir}/knsrcfiles/kdenlive_titles.knsrc
%{_kf5_datadir}/knsrcfiles/kdenlive_wipes.knsrc
%{_kf5_datadir}/knsrcfiles/kdenlive_luts.knsrc
%{_kf5_mandir}/man1/kdenlive.1*
%{_kf5_mandir}/man1/kdenlive_render.1*
# consider subpkg for multilib
%{_kf5_qtplugindir}/mltpreview.so
%{_kf5_datadir}/qlogging-categories5/*categories


%changelog
* Thu Oct 13 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.08.2-1
- 22.08.2

* Thu Sep 08 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.08.1-1
- 22.08.1

* Fri Aug 19 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.08.0-1
- 22.08.0

* Thu Jul 07 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.04.3-1
- 22.04.3

* Thu Jun 09 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.04.2-1
- 22.04.2

