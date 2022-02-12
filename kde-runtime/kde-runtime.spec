%if 0%{?fedora} || 0%{?epel}
%global flags 1
%global webkit 1
%endif

# newer libsmbclient incompatible?
# https://bugzilla.redhat.com/show_bug.cgi?id=1604473
%if 0%{?fedora} < 28
%global smb 1
%endif

%if 0%{?fedora} > 21
%global plasma5 1
%endif

%if 0%{?plasma5} && 0%{?fedora} < 24
%global kuiserver 1
%endif

%if 0%{?fedora} < 26
%global drkonqi 1
%endif

%if 0%{?fedora} < 25
%global strigi 1
%endif

%if 0%{?fedora} < 28
# kf5-kwallet supports the same interfaces now
%global kwallet 1
%endif

Name:    kde-runtime
Summary: KDE Runtime
Version: 17.08.3
Release: 24%{?dist}

# http://techbase.kde.org/Policies/Licensing_Policy
License: LGPLv2+ and GPLv2+
URL:     https://kde.org/
%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0: http://download.kde.org/%{stable}/applications/%{version}/src/kde-runtime-%{version}.tar.xz

# add shortcuts for search provider
Patch1: kdebase-runtime-4.1.x-searchproviders-shortcuts.patch

# support kdesud -Wl,-z,relro,-z,now linker flags
Patch2: kde-runtime-kdesud_relro.patch

# add OnlyShowIn=KDE  to Desktop/Home.desktop (like trash.desktop)
Patch6: kdebase-runtime-4.3.3-home_onlyshowin_kde.patch

# correct path for htsearch
Patch7: kdebase-runtime-4.5.3-htsearch.patch

# Launch compiz via compiz-manager so we get window decorations and
# other such decadent luxuries (AdamW 2011/01)
Patch8: kdebase-runtime-4.5.95-compiz.patch

# add overrides in default manpath
Patch9: kdebase-runtime-4.3.4-man-overrides.patch

# https://bugs.kde.org/show_bug.cgi?id=310486
# revert the main part of:
# http://commits.kde.org/kde-runtime/deee161a42efda74965ca4aab7d79fb7fb375352
# (Upstream doesn't like this workaround.)
Patch10: kde-runtime-4.9.98-kde#310486.patch

# disable making files read only when moving them into trash
# (Upstream wouldn't accept this)
Patch11: kde-runtime-4.10.4-trash-readonly.patch

## upstreamable patches
# make installdbgsymbols.sh use pkexec instead of su 
# increase some timeouts in an effort to see (some) errors before close
Patch50: kde-runtime-4.9.0-installdbgsymbols.patch
# dnf-based version of patch50
Patch53:  kde-runtime-16.04.1-installdgbsymbols-dnf.patch

# use packagekit to install a possibly-missing gdb
Patch51: kde-runtime-4.11.2-install_gdb.patch

# Fix FTBFS
# workaround missing dependency on glib2 in NetworkManager.pc
Patch52: kde-runtime-15.08.0-fix-build.patch

# patch to use libtirpc for RPC, from Cygwin Ports
# should be upstreamable, considering that glibc's builtin RPC is obsolete
# backport of:
# https://github.com/cygwinports/kf5-kio-extras/blob/master/16.08.3-nfs-libtirpc.patch
# (because:
# https://github.com/cygwinports/kde-runtime/blob/master/15.04.3-libtirpc.patch
# is incomplete)
Patch54: kde-runtime-17.08.3-nfs-libtirpc.patch

# make some components optional (kwalletd)
Patch55: kde-runtime-optional_components.patch

## upstream patches

# rhel patches
Patch300: kde-runtime-4.9.2-webkit.patch

Obsoletes: kdebase-runtime < 4.7.97-10
Provides:  kdebase-runtime = %{version}-%{release}
Obsoletes: kdebase4-runtime < %{version}-%{release}
Provides:  kdebase4-runtime = %{version}-%{release}

Obsoletes: nepomukcontroller < 1:0.2

# knotify4 provides dbus service org.freedesktop.Notifications too 
Provides: desktop-notification-daemon

%{?_kde4_macros_api:Requires: kde4-macros(api) = %{_kde4_macros_api} }
# http://bugzilla.redhat.com/794958
Requires: dbus
# kdeeject
# (eject provided by util-linux that has no ExcludeArch, so not sure if that's still needed here or not )
%ifnarch s390 s390x
Requires: eject
%endif
Requires: %{name}-libs%{?_isa} = %{version}-%{release}
%if 0%{?drkonqi} && ! 0%{?plasma5}
Requires: %{name}-drkonqi = %{version}-%{release}
%endif
%if 0%{?kuiserver}
Requires: %{name}-kuiserver = %{version}-%{release}
%endif
%if ! 0%{?plasma5}
# needed by windowsexethumbnail
Requires: icoutils
%endif
%if 0%{?flags}
Requires: %{name}-flags = %{version}-%{release}
%endif

# ensure default/fallback icon theme present
# beware of bootstrapping, there be dragons
Requires: oxygen-icon-theme

BuildRequires: bzip2-devel
BuildRequires: chrpath
BuildRequires: desktop-file-utils
BuildRequires: gpgme-devel
BuildRequires: kdelibs4-devel >= 4.14.4
%if 0%{?webkit}
BuildRequires: kdelibs4-webkit-devel
%endif
BuildRequires: kactivities-devel
%if 0%{?kwallet}
BuildRequires: libgcrypt-devel >= 1.5.0
%else
Recommends: kf5-kwallet
%endif
BuildRequires: libjpeg-devel
BuildRequires: perl-generators
BuildRequires: pkgconfig
BuildRequires: pkgconfig(alsa)
BuildRequires: pkgconfig(exiv2)
# Move to openexr2 compat package in f35+
BuildRequires: pkgconfig(OpenEXR) < 3
BuildRequires: pkgconfig(openssl)
BuildRequires: pkgconfig(polkit-qt-1) 
BuildRequires: pkgconfig(libattica)
BuildRequires: pkgconfig(libcanberra)
BuildRequires: pkgconfig(libpulse)
%if 0%{?strigi}
BuildRequires: pkgconfig(libstreamanalyzer) pkgconfig(libstreams)
%endif
BuildRequires: pkgconfig(libtirpc)
BuildRequires: pkgconfig(libwebp)
BuildRequires: pkgconfig(liblzma)
BuildRequires: pkgconfig(glib-2.0)
%if 0%{?fedora} > 21
BuildRequires: pkgconfig(libnm)
%else
BuildRequires: pkgconfig(libnm-glib) pkgconfig(libnm-util)
%endif
BuildRequires: pkgconfig(qca2)
BuildRequires: pkgconfig(soprano) >= 2.6.50
BuildRequires: pkgconfig(xproto)
BuildRequires: pkgconfig(xscrnsaver)
%if 0%{?fedora}
BuildRequires: openslp-devel
%endif
%if 0%{?fedora} || 0%{?rhel} > 6
BuildRequires: libssh-devel >= 0.6
%endif
BuildRequires: zlib-devel

# some items moved -workspace -> -runtime
Conflicts: kdebase-workspace < 4.5.80
# plasmapkg moved -workspace -> -runtime
Conflicts: kde-workspace < 4.9.60

%if ! 0%{?smb}
# may need to bump this is newer builds ever done for older releases -- rex
Obsoletes: kio-smb < 17.08.3-9
%endif

%description
Core runtime for KDE 4.

%package devel
Summary:  Developer files for %{name}
Obsoletes: kdebase-runtime-devel < 4.7.97-10
Provides:  kdebase-runtime-devel = %{version}-%{release} 
Requires: %{name}-libs%{?_isa} = %{?epoch:%{epoch}:}%{version}-%{release}
%description devel
%{summary}.

%if 0%{?drkonqi}
%package drkonqi
Summary: DrKonqi crash handler for KDE4
BuildRequires: kdepimlibs-devel
Requires: %{name} = %{version}-%{release}
%if 0%{?fedora} > 23
Requires: dnf-command(debuginfo-install)
%endif
Requires: kdialog
# drkonqi patch50 uses pkexec
Requires: polkit
%description drkonqi
%{summary}.
%endif

%package kuiserver
Summary: KDE Progress Info UI server
Provides: kuiserver = %{version}-%{release}
Requires: %{name} = %{version}-%{release}
# split out at 15.08.2-1
%description kuiserver
%{summary}.

%package libs
Summary: Runtime libraries for %{name}
Obsoletes: kdebase-runtime-libs < 4.7.97-10
Provides:  kdebase-runtime-libs = %{version}-%{release}
Requires: %{name} = %{version}-%{release}
%{?kdelibs4_requires}
%description libs
%{summary}.

%package flags
Summary: Geopolitical flags
Obsoletes: kdebase-runtime-flags < 4.7.97-10
Provides:  kdebase-runtime-flags = %{version}-%{release}
Requires: %{name} = %{version}-%{release}
BuildArch: noarch
%description flags
%{summary}.

%if 0%{?smb}
%package kio-smb
Summary: Samba KIO slave
# upgrade path
Obsoletes: kde-runtime < 4.9.2-5
BuildRequires: pkgconfig(smbclient)
BuildRequires: make
Requires: %{name} = %{version}-%{release}
%description kio-smb
%{summary}.
%endif

%package -n kdesu
Summary: Runs a program with elevated privileges
# upgrade path, when kdesu was introduced
Obsoletes: kde-runtime < 14.12.3-2
# needed for non-conflicting libexec bits
Requires: %{name} = %{version}-%{release}
%description -n kdesu
%{summary}.

%package -n khelpcenter
Summary: KDE Help Center
# upgrade path
Obsoletes: kde-runtime < 4.13.3-3
Requires: %{name} = %{version}-%{release}
%description -n khelpcenter
%{summary}.

%package docs
Summary: User documentation and manuals
Epoch: 1
Obsoletes: %{name} < 4.13.3-3
Requires: %{name} = %{version}-%{release}
BuildArch: noarch
%description docs
%{summary}.


%prep
%setup -q -n kde-runtime-%{version}

## upstream patches

%patch1 -p1 -b .searchproviders-shortcuts
%patch6 -p1 -b .home_onlyshowin_kde
%patch7 -p1 -b .htsearch
%patch8 -p1 -b .config
%patch9 -p1 -b .man-overrides
%if 0%{?fedora} < 19 && 0%{?rhel} < 7
%patch10 -p1 -b .kde310486
%endif
%patch11 -p1 -b .trash-readonly
%if 0%{?fedora} > 22
%patch53 -p1 -b .installdgbsymbols-dnf
%else
%patch50 -p1 -b .installdgbsymbols
%endif
%patch51 -p1 -b .install_gdb
%patch52 -p1 -b .fixbuild
%patch54 -p1 -b .libtirpc
%patch55 -p1 -b .optional_components

%if ! 0%{?webkit}
%patch300 -p1 -b .webkit
%global no_webkit -DKDERUNTIME_NO_WEBKIT:BOOL=ON -DPLASMA_NO_KDEWEBKIT:BOOL=ON
%endif


%build
mkdir %{_target_platform}
pushd %{_target_platform}
%{cmake_kde4} .. \
  -DKDE4_ENABLE_FPIE:BOOL=ON \
  %{?no_webkit} \
%if 0%{?plasma5}
  -DBUILD_khelpcenter:BOOL=OFF \
  %{?!drkonqi:-DBUILD_drkonqi:BOOL=OFF} \
  %{?!kuiserver:-DBUILD_kuiserver:BOOL=OFF} \
  %{?!kwallet:-DBUILD_kwalletd:BOOL=OFF}
%endif

popd

%make_build -C %{_target_platform}


%install
make install/fast DESTDIR=%{buildroot} -C %{_target_platform}

# fix documentation multilib conflict in index.cache
for f in kioslave/nepomuksearch kcontrol/spellchecking kcontrol/performance \
   kcontrol/kcmnotify kcontrol/kcmcss kcontrol/ebrowsing; do
   bunzip2 %{buildroot}%{_kde4_docdir}/HTML/en/$f/index.cache.bz2
   sed -i -e 's!name="id[a-z]*[0-9]*"!!g' %{buildroot}%{_kde4_docdir}/HTML/en/$f/index.cache
   sed -i -e 's!#id[a-z]*[0-9]*"!!g' %{buildroot}%{_kde4_docdir}/HTML/en/$f/index.cache
   bzip2 -9 %{buildroot}%{_kde4_docdir}/HTML/en/$f/index.cache
done

# kdesu symlink
ln -s %{_kde4_libexecdir}/kdesu %{buildroot}%{_kde4_bindir}/kdesu

# omit hicolor index.theme, use one from hicolor-icon-theme
rm -f %{buildroot}%{_kde4_iconsdir}/hicolor/index.theme

# remove country flags because some people/countries forbid some other
# people/countries' flags :-(
%{!?flags:rm -f %{buildroot}%{_kde4_datadir}/locale/l10n/*/flag.png}

%if 0%{?drkonqi}
# installdbgsymbols script
install -p -D -m755 drkonqi/doc/examples/installdbgsymbols_fedora.sh \
  %{buildroot}%{_kde4_libexecdir}/installdbgsymbols.sh
%endif

# rpaths
# use chrpath hammer for now, find better patching solutions later -- Rex
chrpath --list   %{buildroot}%{_libdir}/kde4/plugins/phonon_platform/kde.so ||:
chrpath --delete %{buildroot}%{_libdir}/kde4/plugins/phonon_platform/kde.so

## unpackaged files
# FIXME: -devel type files, omit for now
rm -vf  %{buildroot}%{_kde4_libdir}/lib{kwalletbackend,molletnetwork}.so
%if 0%{?rhel}
rm -fv %{buildroot}%{_kde4_datadir}/kde4/services/searchproviders/fedora.desktop
%endif

%if 0%{?plasma5}
rm -fv  %{buildroot}%{_kde4_bindir}/{kdesu,khelpcenter}
rm -fv  %{buildroot}%{_kde4_libexecdir}/khc_*
rm -fv  %{buildroot}%{_kde4_libdir}/libkdeinit4_{khelpcenter}.so
rm -frv %{buildroot}%{_kde4_docdir}/HTML/en/{kcontrol,kdesu,khelpcenter,knetattach,fundamentals,onlinehelp}
rm -frv %{buildroot}%{_kde4_appsdir}/khelpcenter/
rm -fv  %{buildroot}%{_kde4_datadir}/services/{khelpcenter}.desktop
rm -fv  %{buildroot}%{_kde4_datadir}/config.kcfg/khelpcenter.kcfg
rm -fv  %{buildroot}%{_mandir}/man1/kdesu.1*
# now provided by kde-cli-tools >= 5.23.90
# https://phabricator.kde.org/T14763
# https://invent.kde.org/plasma/kde-cli-tools/-/merge_requests/23
rm -fv  %{buildroot}%{_bindir}/{kde-open,keditfiletype,kioclient,kmimetypefinder,kstart,ksvgtopng}
%else
# install this service for KDE 3 applications too
mkdir %{buildroot}%{_datadir}/services
ln -s %{_kde4_datadir}/kde4/services/khelpcenter.desktop \
      %{buildroot}%{_datadir}/services/khelpcenter.desktop
%endif


%check
for f in %{buildroot}%{_kde4_datadir}/applications/kde4/*.desktop ; do
  desktop-file-validate $f
done


%files
%license COPYING COPYING.LIB
%if !0%{?plasma5}
# new conflicts with kde-cli-tools
%{_kde4_bindir}/kde-open
%{_kde4_bindir}/keditfiletype
%{_kde4_bindir}/kioclient
%{_kde4_bindir}/kmimetypefinder
%{_kde4_bindir}/kstart
%{_kde4_bindir}/ksvgtopng
%endif
%{_kde4_bindir}/kcmshell4
%{_kde4_bindir}/kde-cp
%{_kde4_bindir}/kde-mv
%{_kde4_bindir}/kde4
%{_kde4_bindir}/kde4-menu
%{_kde4_bindir}/kdebugdialog
%{_kde4_bindir}/kfile4
%{_kde4_bindir}/kglobalaccel
%{_kde4_bindir}/khotnewstuff-upload
%{_kde4_bindir}/khotnewstuff4
%{_kde4_bindir}/kiconfinder
%{_kde4_bindir}/knotify4
%{_kde4_bindir}/kquitapp
%{_kde4_bindir}/kreadconfig
%{_kde4_bindir}/ktraderclient
%{_kde4_bindir}/ktrash
%if 0%{?kwallet}
%{_kde4_bindir}/kwalletd
%{_kde4_libdir}/libkdeinit4_kwalletd.so
%{_kde4_appsdir}/kwalletd/
%endif
%{_kde4_bindir}/kwriteconfig
%{_kde4_bindir}/plasma-remote-helper
%{_kde4_bindir}/plasmapkg
%{_mandir}/man1/plasmapkg.1*
%{_kde4_bindir}/solid-hardware
%{_kde4_appsdir}/desktoptheme/
%{_kde4_appsdir}/hardwarenotifications/
%{_kde4_appsdir}/kcm_componentchooser/
%{_kde4_appsdir}/kcmlocale/
%{_kde4_appsdir}/kcm_phonon/
%{_kde4_appsdir}/kconf_update/*
%{_kde4_appsdir}/kde/
%{_kde4_appsdir}/kglobalaccel/
%{_kde4_appsdir}/kio_bookmarks/
%{_kde4_appsdir}/kio_desktop/
%{_kde4_appsdir}/kio_docfilter/
%{_kde4_appsdir}/kio_finger/
%{_kde4_appsdir}/kio_info/
%{_kde4_appsdir}/konqsidebartng/
%{_kde4_appsdir}/ksmserver/
%{_kde4_appsdir}/libphonon/
%{_kde4_appsdir}/phonon/
%dir %{_kde4_appsdir}/remoteview/
%{_kde4_appsdir}/remoteview/network.desktop
%{_kde4_configdir}/*.knsrc
%{_kde4_datadir}/config.kcfg/jpegcreatorsettings.kcfg
%{_datadir}/dbus-1/services/org.kde.knotify.service
%{_datadir}/dbus-1/system-services/org.kde.kcontrol.kcmremotewidgets.service
%{_kde4_datadir}/kde4/services/*.desktop
%{_kde4_datadir}/kde4/services/qimageioplugins/webp.desktop
%{_kde4_datadir}/kde4/services/*.protocol
%{_kde4_datadir}/kde4/services/kded/
%{_kde4_datadir}/kde4/services/searchproviders/
%{_kde4_datadir}/kde4/servicetypes/*
%{_kde4_datadir}/mime/packages/network.xml
%{_kde4_datadir}/mime/packages/webp.xml
%{_kde4_datadir}/sounds/*
%{_kde4_iconsdir}/default.kde4
%{_kde4_libdir}/kconf_update_bin/*
%{_kde4_libdir}/libkdeinit4_kcmshell4.so
%{_kde4_libdir}/libkdeinit4_kglobalaccel.so
%{_kde4_libdir}/kde4/platformimports/
%{_kde4_libdir}/kde4/kcm_*.so
%{_kde4_libdir}/kde4/kded_*.so
%{_kde4_libexecdir}/kcmremotewidgetshelper
%{_kde4_libexecdir}/kdeeject
%{_kde4_libexecdir}/kdesu
%attr(2755,root,nobody) %{_kde4_libexecdir}/kdesud
%{_kde4_libexecdir}/kdontchangethehostname
%{_kde4_libexecdir}/kioexec
%{_kde4_libexecdir}/knetattach
%{_kde4_iconsdir}/hicolor/*/*/*
%{_kde4_sysconfdir}/xdg/menus/kde-information.menu
%{_kde4_datadir}/applications/kde4/knetattach.desktop
%{_kde4_configdir}/kshorturifilterrc
%{_kde4_datadir}/desktop-directories/*.directory
%exclude %{_kde4_datadir}/desktop-directories/kde-information.directory
%{_kde4_datadir}/emoticons/kde4/
%{_kde4_datadir}/locale/l10n/
%{_kde4_datadir}/locale/currency/
%{?flags:%exclude %{_kde4_datadir}/locale/l10n/*/flag.png}
%{_polkit_qt_policydir}/*.policy
%{_sysconfdir}/dbus-1/system.d/*

%if 0%{?smb}
#files kio-smb
%dir %{_kde4_appsdir}/konqueror/dirtree/
%dir %{_kde4_appsdir}/konqueror/dirtree/remote/
%{_kde4_appsdir}/konqueror/dirtree/remote/smb-network.desktop
%{_kde4_appsdir}/remoteview/smb-network.desktop
%{_kde4_datadir}/kde4/services/smb.protocol
# dup'd in -libs glob
#{_kde4_libdir}/kde4/kio_smb.so
%endif

%files devel
%{_kde4_includedir}/*
%{_kde4_appsdir}/cmake/modules/*.cmake
%{_datadir}/dbus-1/interfaces/*.xml

%if 0%{?drkonqi}
%if 0%{?fedora} > 16 || 0%{?rhel} > 6
%post drkonqi
# make DrKonqi work by default by taming SELinux enough (suggested by dwalsh)
# if KDE_DEBUG is set, DrKonqi is disabled, so do nothing
# if it is unset (or empty), check if deny_ptrace is already disabled
# if not, disable it
if [ -z "$KDE_DEBUG" ] ; then
  if [ "`getsebool deny_ptrace 2>/dev/null`" == 'deny_ptrace --> on' ] ; then
    setsebool -P deny_ptrace off &> /dev/null || :
  fi
fi
%endif

%files drkonqi
%{_kde4_libexecdir}/drkonqi
%{_kde4_libexecdir}/installdbgsymbols.sh
%{_kde4_appsdir}/drkonqi/
%endif

%ldconfig_scriptlets libs

%files libs
# unversioned plugin:
%{_kde4_libdir}/attica_kde.so
%{_kde4_libdir}/libknotifyplugin.so
%if 0%{?kwallet}
%{_kde4_libdir}/libkwalletbackend.so.*
%endif
%{_kde4_libdir}/libmolletnetwork.so.*
%{_kde4_libdir}/kde4/*.so
%{_kde4_libdir}/kde4/imports/
# FIXME: Is this a good idea? Won't multilib apps need KCMs, too?
%exclude %{_kde4_libdir}/kde4/kcm_*.so
%exclude %{_kde4_libdir}/kde4/kded_*.so
%{_kde4_libdir}/kde4/plugins/phonon_platform/
%{_kde4_libdir}/kde4/plugins/imageformats/kimg_webp.so

%if 0%{?flags}
%files flags
%{_kde4_datadir}/locale/l10n/*/flag.png
%endif

%files docs
%{_kde4_docdir}/HTML/en/kdebugdialog/
%{_kde4_docdir}/HTML/en/kioslave/

%if 0%{?kuiserver}
%files kuiserver
%{_kde4_bindir}/kuiserver
%{_kde4_libdir}/libkdeinit4_kuiserver.so
%{_datadir}/dbus-1/services/org.kde.kuiserver.service
%{_kde4_datadir}/kde4/services/kuiserver.desktop
%endif

%if ! 0%{?plasma5}
%files -n khelpcenter
%{_kde4_bindir}/khelpcenter
%{_kde4_libexecdir}/khc_*
%{_kde4_libdir}/libkdeinit4_khelpcenter.so
%{_kde4_docdir}/HTML/en/khelpcenter/
%{_kde4_docdir}/HTML/en/fundamentals/
%{_kde4_docdir}/HTML/en/onlinehelp/
%{_kde4_appsdir}/khelpcenter/
%{_kde4_datadir}/kde4/services/khelpcenter.desktop
%{_kde4_datadir}/services/khelpcenter.desktop
%{_kde4_datadir}/config.kcfg/khelpcenter.kcfg
%{_kde4_datadir}/applications/kde4/Help.desktop
%{_kde4_datadir}/desktop-directories/kde-information.directory

%files -n kdesu
%{_kde4_bindir}/kdesu
%{_kde4_docdir}/HTML/en/kdesu/
%{_mandir}/man1/kdesu.1*
## include non-conflicting libexec bits here too ? -- rex
%endif


%changelog
* Thu Jan 20 2022 Fedora Release Engineering <releng@fedoraproject.org> - 17.08.3-24
- Rebuilt for https://fedoraproject.org/wiki/Fedora_36_Mass_Rebuild

* Tue Jan 18 2022 Rex Dieter <rdieter@fedoraproject.org> - 17.08.3-23
- fix conflicts with kde-cli-tools >= 5.23.90
- update URL

* Sun Aug 01 2021 Richard Shaw <hobbes1069@gmail.com> - 17.08.3-22
- Move to openexr2 compat package.

* Thu Jul 22 2021 Fedora Release Engineering <releng@fedoraproject.org> - 17.08.3-21
- Rebuilt for https://fedoraproject.org/wiki/Fedora_35_Mass_Rebuild

* Mon Mar 01 2021 Kevin Kofler <Kevin@tigcc.ticalc.org> - 17.08.3-20
- Remove bogus BuildRequires: xorg-x11-font-utils (#1933544)

* Tue Jan 26 2021 Fedora Release Engineering <releng@fedoraproject.org> - 17.08.3-19
- Rebuilt for https://fedoraproject.org/wiki/Fedora_34_Mass_Rebuild

* Tue Jan 12 2021 Mamoru TASAKA <mtasaka@fedoraproject.org> - 17.08.3-18
- rebuild against New OpenEXR properly

* Fri Jan 01 2021 Richard Shaw <hobbes1069@gmail.com> - 17.08.3-17
- Rebuild for OpenEXR 2.5.3.

* Tue Dec 22 2020 Rex Dieter <rdieter@fedoraproject.org> - 17.08.3-16
- Requires: -dbus-x11 +dbus

* Tue Jul 28 2020 Fedora Release Engineering <releng@fedoraproject.org> - 17.08.3-15
- Rebuilt for https://fedoraproject.org/wiki/Fedora_33_Mass_Rebuild

* Wed Jan 29 2020 Fedora Release Engineering <releng@fedoraproject.org> - 17.08.3-14
- Rebuilt for https://fedoraproject.org/wiki/Fedora_32_Mass_Rebuild

* Thu Jul 25 2019 Fedora Release Engineering <releng@fedoraproject.org> - 17.08.3-13
- Rebuilt for https://fedoraproject.org/wiki/Fedora_31_Mass_Rebuild

* Thu Apr 11 2019 Richard Shaw <hobbes1069@gmail.com> - 17.08.3-12
- Rebuild for OpenEXR 2.3.0.

* Fri Feb 01 2019 Fedora Release Engineering <releng@fedoraproject.org> - 17.08.3-11
- Rebuilt for https://fedoraproject.org/wiki/Fedora_30_Mass_Rebuild

* Wed Jan 30 2019 Rex Dieter <rdieter@fedoraproject.org> - 17.08.3-10
- rebuild (exiv2)

* Fri Jul 20 2018 Rex Dieter <rdieter@fedoraproject.org> - 17.08.3-9
- disable kio-smb on f29+ (#1604473)

* Fri Jul 13 2018 Fedora Release Engineering <releng@fedoraproject.org> - 17.08.3-8
- Rebuilt for https://fedoraproject.org/wiki/Fedora_29_Mass_Rebuild

* Wed Mar 28 2018 Rex Dieter <rdieter@fedoraproject.org> - 17.08.3-7
- omit kwallet on f28+

* Fri Mar 02 2018 Rex Dieter <rdieter@fedoraproject.org> - 17.08.3-6
- -docs: omit Conflicts with plasma-desktop-doc, Epoch++ (#1199720)
- use %%make_build %%ldconfig_scriptlets
- re-enable webkit support

* Fri Feb 09 2018 Igor Gnatenko <ignatenkobrain@fedoraproject.org> - 17.08.3-5
- Escape macros in %%changelog

* Wed Feb 07 2018 Fedora Release Engineering <releng@fedoraproject.org> - 17.08.3-4
- Rebuilt for https://fedoraproject.org/wiki/Fedora_28_Mass_Rebuild

* Thu Jan 11 2018 Igor Gnatenko <ignatenkobrain@fedoraproject.org> - 17.08.3-3
- Remove obsolete scriptlets

* Wed Jan 10 2018 Kevin Kofler <Kevin@tigcc.ticalc.org> - 17.08.3-2
- Build against libtirpc (#1532943), patch from Cygwin Ports

* Wed Nov 08 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.08.3-1
- 17.08.3

* Wed Oct 11 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.08.2-1
- 17.08.2

* Tue Sep 05 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.08.1-1
- 17.08.1

* Sat Aug 26 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.08.0-1
- 17.08.0

* Fri Jul 28 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.04.3-1
- 17.04.3

* Wed Jul 26 2017 Fedora Release Engineering <releng@fedoraproject.org> - 17.04.2-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Mass_Rebuild

* Thu Jun 15 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.04.2-1
- 17.04.2

* Wed May 10 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.04.1-1
- 17.04.1

* Tue May 02 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.04.0-2
- rebuild (exiv2)

* Fri Apr 14 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.04.0-1
- 17.04.0, update URL

* Wed Mar 08 2017 Rex Dieter <rdieter@fedoraproject.org> - 16.12.3-1
- 16.12.3

* Thu Feb 09 2017 Rex Dieter <rdieter@fedoraproject.org> - 16.12.2-2
- make Requires: eject less conditional

* Wed Feb 08 2017 Rex Dieter <rdieter@fedoraproject.org> - 16.12.2-1
- 16.12.2

* Wed Feb 01 2017 Sandro Mani <manisandro@gmail.com> - 16.12.1-2
- Rebuild (libwebp)

* Tue Jan 10 2017 Rex Dieter <rdieter@fedoraproject.org> - 16.12.1-1
- 16.12.1, drop -drkonqi (f26+)

* Thu Dec 22 2016 Rex Dieter <rdieter@math.unl.edu> - 16.12.0-1
- 16.12.0

* Sat Dec 10 2016 Igor Gnatenko <i.gnatenko.brain@gmail.com> - 16.08.3-4
- Rebuild for gpgme 1.18

* Wed Dec 07 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.08.3-3
- (re)enable smb, use %%global for macros

* Mon Dec 05 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.08.3-2
- disable smb/webkit support (for bootstrapping)

* Wed Nov 30 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.08.3-1
- 16.08.3

* Thu Oct 13 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.08.2-1
- 16.08.2

* Tue Sep 06 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.08.1-1
- 16.08.1

* Tue Aug 16 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.08.0-3
- rebuild

* Tue Aug 16 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.08.0-2
- rebuild

* Fri Aug 12 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.08.0-1
- 16.08.0

* Sat Aug 06 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.07.90-1
- 16.07.90

* Fri Jul 29 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.07.80-1
- 16.07.80

* Fri Jul 08 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.04.3-1
- 16.04.3

* Sun Jun 12 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.04.2-1
- 16.04.2

* Sun Jun 05 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.04.1-3
- -drkonqi: Requires: kdialog, dnf-command(debuginfo-install) (f24+)

* Wed Jun 01 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.04.1-2
- disable strigi support (f25+)

* Sun May 08 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.04.1-1
- 16.04.1

* Wed Apr 20 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.04.0-3
- rebuild (qt)

* Mon Apr 18 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.04.0-2
- rebuild (qt)

* Fri Apr 15 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.04.0-1
- 16.04.0

* Sat Mar 12 2016 Rex Dieter <rdieter@fedoraproject.org> 15.12.3-1
- 15.12.3, use %%license, drop some kde-workspace-only deps

* Fri Feb 12 2016 Rex Dieter <rdieter@fedoraproject.org> - 15.12.2-1
- 15.12.2

* Thu Feb 04 2016 Fedora Release Engineering <releng@fedoraproject.org> - 15.12.1-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_24_Mass_Rebuild

* Fri Jan 08 2016 Rex Dieter <rdieter@fedoraproject.org> 15.12.1-1
- 15.12.1, update URL

* Mon Dec 28 2015 Igor Gnatenko <i.gnatenko.brain@gmail.com> - 15.12.0-2
- Rebuilt for libwebp soname bump

* Fri Dec 18 2015 Rex Dieter <rdieter@fedoraproject.org> 15.12.0-1
- 15.12.0, - restore -kuiserver (#1249157)

* Tue Nov 17 2015 Rex Dieter <rdieter@fedoraproject.org> 15.08.3-2
- Unhelpful summary/description for drkonqi packages (#1282810)

* Sat Nov 07 2015 Rex Dieter <rdieter@fedoraproject.org> 15.08.3-1
- 15.08.3

* Thu Oct 29 2015 Rex Dieter <rdieter@fedoraproject.org> 15.08.2-3
- Recommends: -drkonqi (f22+)

* Tue Oct 13 2015 Rex Dieter <rdieter@fedoraproject.org> 15.08.2-2
- use upstreamed version of patch

* Mon Oct 12 2015 Rex Dieter <rdieter@fedoraproject.org> - 15.08.2-1
- kde-apps-15.08.2
- -kuiserver subpkg, omitted for plasma5 (#1249157), Provides: kuiserver
- -devel: move dbus-1 xml interface files here

* Fri Sep 25 2015 Rex Dieter <rdieter@fedoraproject.org> 15.08.1-2
- Drop Requires: kde-runtime-drkonqi (#1264486)

* Mon Sep 14 2015 Rex Dieter <rdieter@fedoraproject.org> - 15.08.1-1
- 15.08.1

* Sat Aug 29 2015 Rex Dieter <rdieter@fedoraproject.org> 15.08.0-2
- make oxygen-icon-theme dep unversioned

* Thu Aug 20 2015 Than Ngo <than@redhat.com> - 15.08.0-1
- 15.08.0

* Sun Jun 28 2015 Rex Dieter <rdieter@fedoraproject.org> - 15.04.3-1
- 15.04.3

* Wed Jun 24 2015 Rex Dieter <rdieter@fedoraproject.org> - 15.04.2-3
- rebuild (exiv2)

* Wed Jun 17 2015 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 15.04.2-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_23_Mass_Rebuild

* Tue Jun 02 2015 Rex Dieter <rdieter@fedoraproject.org> - 15.04.2-1
- 15.04.2

* Sat May 30 2015 Rex Dieter <rdieter@fedoraproject.org> 15.04.1-2
- omit kdesu,khelpcenter where plasma5 is present (f22+)

* Thu May 14 2015 Rex Dieter <rdieter@fedoraproject.org> 15.04.1-1
- 15.04.1

* Mon Apr 13 2015 Rex Dieter <rdieter@fedoraproject.org> 15.04.0-1
- 15.04.0

* Sat Mar 07 2015 Rex Dieter <rdieter@fedoraproject.org> - 14.12.3-2
- kdesu subpkg (#1199720)
- move plasmapkg.1 manpage to main pkg (it's safe)

* Sat Feb 28 2015 Rex Dieter <rdieter@fedoraproject.org> 14.12.3-1
- 14.12.3

* Tue Feb 24 2015 Than Ngo <than@redhat.com> - 14.12.2-1
- 14.12.2

* Thu Jan 29 2015 Daniel Vrátil <dvratil@redhat.com> 14.12.1-3
- don't provide kdesu on F22+ (provided by kde-cli-tools)

* Tue Jan 20 2015 Rex Dieter <rdieter@fedoraproject.org> 14.12.1-2
- tarball respin

* Sat Jan 10 2015 Rex Dieter <rdieter@fedoraproject.org> 14.12.1-1
- 14.12.1

* Fri Dec 19 2014 Rex Dieter <rdieter@fedoraproject.org> 14.11.97-3
- Provides: kdesu

* Mon Dec 08 2014 Rex Dieter <rdieter@fedoraproject.org> 14.11.97-2
- -libs: fix kdelibs dep (version != _kde4_version)

* Fri Dec 05 2014 Rex Dieter <rdieter@fedoraproject.org> 14.11.97-1
- 14.11.97

* Tue Nov 25 2014 Rex Dieter <rdieter@fedoraproject.org> 4.14.3-3
- rebuild (openexr)

* Mon Nov 17 2014 Than Ngo <than@redhat.com> - 4.14.3-2
- fix bz#1164609, CVE-2014-8600, Insufficient Input Validation By IO Slaves

* Sat Nov 08 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.14.3-1
- 4.14.3

* Sat Oct 11 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.14.2-1
- 4.14.2

* Tue Oct 07 2014 Rex Dieter <rdieter@fedoraproject.org> 4.14.1-2
- drop mouseeventlistener.patch, upstream fixed

* Mon Sep 15 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.14.1-1
- 4.14.1

* Sat Aug 16 2014 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 4.14.0-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_21_22_Mass_Rebuild

* Thu Aug 14 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.14.0-1
- 4.14.0

* Tue Aug 05 2014 Daniel Vrátil <dvratil@redhat.com> - 4.13.97-2
- Split khelpcenter and -docs into subpackages to improve coinstallability with Plasma 5

* Tue Aug 05 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.13.97-1
- 4.13.97

* Thu Jul 17 2014 Rex Dieter <rdieter@fedoraproject.org> 4.13.3-2
- rebuild (for pulseaudio, bug #1117683)

* Mon Jul 14 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.13.3-1
- 4.13.3

* Tue Jul 08 2014 Rex Dieter <rdieter@fedoraproject.org> 4.13.2-4
- scriptlet polish

* Thu Jul 03 2014 Rex Dieter <rdieter@fedoraproject.org> 4.13.2-3
- optimize mimeinfo scriptlet

* Thu Jun 19 2014 Rex Dieter <rdieter@fedoraproject.org> 4.13.2-2
- BR: s/qtwebkit-devel/kdelibs4-webkit-devel/

* Mon Jun 09 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.13.2-1
- 4.13.2

* Sun Jun 08 2014 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 4.13.1-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_21_Mass_Rebuild

* Fri May 16 2014 Rex Dieter <rdieter@fedoraproject.org> 4.13.1-2
- respin

* Sat May 10 2014 Rex Dieter <rdieter@fedoraproject.org> 4.13.1-1
- 4.13.1

* Thu May 01 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.13.0-3
- upstream phonon kcm fix (pulseaudio card profiles)
- drop unused passwordless-kwallet patch (now that we have pam-kwallet to play with)

* Mon Apr 14 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.13.0-2
- add option to enable optional compat nepomuk stuff (default off)
- drop: Requires: nepomuk-core

* Sat Apr 12 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.13.0-1
- 4.13.0

* Thu Apr 03 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.12.97-1
- 4.12.97

* Sat Mar 22 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.12.95-1
- 4.12.95

* Tue Mar 18 2014 Rex Dieter <rdieter@fedoraproject.org> 4.12.90-1
- 4.12.90

* Sat Mar 01 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.12.3-1
- 4.12.3

* Fri Jan 31 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.12.2-1
- 4.12.2

* Fri Jan 10 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.12.1-1
- 4.12.1

* Fri Jan 10 2014 Kevin Kofler <Kevin@tigcc.ticalc.org> - 4.12.0-3
- backport support for libssh 0.6 and ECDSA keys (kde#327024) from master/4.13

* Wed Jan 01 2014 Rex Dieter <rdieter@fedoraproject.org> 4.12.0-2
- rebuild (libwebp)

* Thu Dec 19 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.12.0-1
- 4.12.0

* Fri Dec 13 2013 Rex Dieter <rdieter@fedoraproject.org> 4.11.97-3
- %%build: -DKDE4_ENABLE_FPIE:BOOL=ON
- kdesud -Wl,-z,relro,-z,now linker flags

* Tue Dec 03 2013 Rex Dieter <rdieter@fedoraproject.org> 4.11.97-2
- rebuild (exiv2)

* Sun Dec 01 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.11.97-1
- 4.11.97

* Wed Nov 27 2013 Rex Dieter <rdieter@fedoraproject.org> 4.11.95-3
- rebuild (openexr)

* Sat Nov 23 2013 Rex Dieter <rdieter@fedoraproject.org> 4.11.95-2
- rebase passwordless kwallet support

* Thu Nov 21 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.11.95-1
- 4.11.95

* Tue Nov 19 2013 Rex Dieter <rdieter@fedoraproject.org> 4.11.90-2
- Volume gets restored to 100%% after each knotify event (#324975)

* Sat Nov 16 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.11.90-1
- 4.11.90

* Sat Nov 02 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.11.3-1
- 4.11.3

* Fri Nov 01 2013 Rex Dieter <rdieter@fedoraproject.org> 4.11.2-3
- unversioned BR: kdepimlibs-devel

* Sun Oct 20 2013 Rex Dieter <rdieter@fedoraproject.org> 4.11.2-2
- -drkonqi: use PK to install gdb at runtime, if missing

* Sat Sep 28 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.11.2-1
- 4.11.2

* Wed Sep 25 2013 Rex Dieter <rdieter@fedoraproject.org> 4.11.1-3
- Fix automount when explicit Unknown Mount removable option is selected (#1012080)

* Mon Sep 09 2013 Rex Dieter <rdieter@fedoraproject.org> 4.11.1-2
- rebuild (openexr)

* Tue Sep 03 2013 Rex Dieter <rdieter@fedoraproject.org> 4.11.1-1
- 4.11.1
- backport kwallet synchronous mode fix (kde#254198)

* Thu Aug 08 2013 Than Ngo <than@redhat.com> - 4.11.0-1
- 4.11.0

* Wed Aug 07 2013 Petr Pisar <ppisar@redhat.com> - 4.10.97-2
- Perl 5.18 rebuild

* Thu Jul 25 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.10.97-1
- 4.10.97

* Wed Jul 24 2013 Rex Dieter <rdieter@fedoraproject.org> 4.10.95-2
- fix initial_passwordless_kwallet option (kde review#110328)

* Tue Jul 23 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.10.95-1
- 4.10.95

* Thu Jul 18 2013 Petr Pisar <ppisar@redhat.com> - 4.10.90-4
- Perl 5.18 rebuild

* Sat Jun 29 2013 Rex Dieter <rdieter@fedoraproject.org> 4.10.90-3
- support initial_passwordless_kwallet option (kde review#110328)

* Sat Jun 29 2013 Rex Dieter <rdieter@fedoraproject.org> 4.10.90-2
- pull in upstream kwallet-related work

* Thu Jun 27 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.10.90-1
- 4.10.90

* Tue Jun 18 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.10.4-3
- avoid/workaround X3 plasma mouse scroll events (kde#316546)
- prune/fix changelog

* Wed Jun 12 2013 Martin Briza <mbriza@redhat.com> - 4.10.4-2
- Do not make deleted (moved to trash) files read only (thanks ltink for the patch) (#921735)

* Sat Jun 01 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.10.4-1
- 4.10.4

* Sat May 18 2013 Jan Grulich <jgrulich@redhat.com> - 4.10.3-3
- fix: phonon systemsettings module gets 'modified' without any changes

* Mon May 13 2013 Than Ngo <than@redhat.com> - 4.10.3-2
- add mssing buildroot

* Mon May 06 2013 Than Ngo <than@redhat.com> - 4.10.3-1
- 4.10.3

* Mon Apr 29 2013 Rex Dieter <rdieter@fedoraproject.org> 4.10.2-3
- move drkonqi-related scriptlet to -drkonqi subpkg

* Mon Apr 29 2013 Than Ngo <than@redhat.com> - 4.10.2-2
- fix multilib issue

* Sun Mar 31 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.10.2-1
- 4.10.2

* Thu Mar 21 2013 Rex Dieter <rdieter@fedoraproject.org> 4.10.1-3
- Conflicts: kde-workspace < 4.9.60 (#924905)

* Sun Mar 10 2013 Rex Dieter <rdieter@fedoraproject.org> 4.10.1-2
- rebuild (OpenEXR)

* Sat Mar 02 2013 Rex Dieter <rdieter@fedoraproject.org> 4.10.1-1
- 4.10.1

* Thu Feb 21 2013 Rex Dieter <rdieter@fedoraproject.org> 4.10.0-3
- Requires: icoutils

* Sat Feb 02 2013 Kevin Kofler <Kevin@tigcc.ticalc.org> - 4.10.0-2
- ktimezoned: watch /etc/localtime if it doesn't exist yet (#906972)

* Thu Jan 31 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.10.0-1
- 4.10.0

* Mon Jan 28 2013 Rex Dieter <rdieter@fedoraproject.org> 4.9.98-3
- make upgrade hack apply on <f19 only

* Sun Jan 27 2013 Kevin Kofler <Kevin@tigcc.ticalc.org> 4.9.98-2
- Unable to logout after KDE upgrade (kde#310486)

* Sun Jan 20 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.9.98-1
- 4.9.98

* Fri Jan 18 2013 Adam Tkac <atkac redhat com> - 4.9.97-2
- rebuild due to "jpeg8-ABI" feature drop

* Fri Jan 04 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.9.97-1
- 4.9.97

* Thu Dec 20 2012 Rex Dieter <rdieter@fedoraproject.org> - 4.9.95-1
- 4.9.95

* Mon Dec 03 2012 Rex Dieter <rdieter@fedoraproject.org> 4.9.90-1
- 4.9.90 (4.10 beta2)

* Mon Dec 03 2012 Than Ngo <than@redhat.com> - 4.9.4-1
- 4.9.4

* Mon Nov 19 2012 Than Ngo <than@redhat.com> - 4.9.3-2
- fedora/rhel condition

* Fri Nov 02 2012 Rex Dieter <rdieter@fedoraproject.org> 4.9.3-1
- 4.9.3
- drop -kio-smb subpkg

* Mon Oct 29 2012 Rex Dieter <rdieter@fedoraproject.org> 4.9.2-6
- drop -desktoptheme subpkg, that one is a bad idea :(
- Requires: drkonqi

* Mon Oct 29 2012 Rex Dieter <rdieter@fedoraproject.org> - 4.9.2-5
- -desktoptheme, -drkonqi, -kio-smb subpkgs (#855930)
- drop hard Requires: htdig (#855930)
- remove .spec cruft

* Wed Oct 24 2012 Rex Dieter <rdieter@fedoraproject.org> 4.9.2-4
- rebuild (libjpeg-turbo v8)

* Thu Oct 11 2012 Than Ngo <than@redhat.com> - 4.9.2-3
- update webkit patch

* Tue Oct 02 2012 Rex Dieter <rdieter@fedoraproject.org> 4.9.2-2
- kde-runtime-4.9.2 is missing kio_smb (#862169) 

* Fri Sep 28 2012 Rex Dieter <rdieter@fedoraproject.org> - 4.9.2-1
- 4.9.2

* Mon Sep 03 2012 Than Ngo <than@redhat.com> - 4.9.1-1
- 4.9.1

* Tue Aug 07 2012 Rex Dieter <rdieter@fedoraproject.org> 4.9.0-4
- improve installdbgsymbols script

* Sat Aug 04 2012 Than Ngo <than@redhat.com> - 4.9.0-3
- update webkit patch

* Thu Aug 02 2012 Rex Dieter <rdieter@fedoraproject.org> 4.9.0-2
- respin

* Thu Jul 26 2012 Lukas Tinkl <ltinkl@redhat.com> - 4.9.0-1
- 4.9.0

* Thu Jul 19 2012 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 4.8.97-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_18_Mass_Rebuild

* Wed Jul 11 2012 Rex Dieter <rdieter@fedoraproject.org> - 4.8.97-1
- 4.8.97

* Wed Jun 27 2012 Jaroslav Reznik <jreznik@redhat.com> - 4.8.95-1
- 4.8.95

* Wed Jun 20 2012 Rex Dieter <rdieter@fedoraproject.org> 4.8.90-3
- rebuild (attica)

* Tue Jun 12 2012 Rex Dieter <rdieter@fedoraproject.org> 4.8.90-2
- BR: nepomuk-core-devel pkgconfig(NetworkManager) pkgconfig(qca2)

* Sat Jun 09 2012 Rex Dieter <rdieter@fedoraproject.org> - 4.8.90-1
- 4.8.90

* Fri Jun 01 2012 Jaroslav Reznik <jreznik@redhat.com> - 4.8.80-2
- respin

* Sat May 26 2012 Jaroslav Reznik <jreznik@redhat.com> - 4.8.80-1
- 4.8.80
- add kactivities BR

* Fri May 04 2012 Than Ngo <than@redhat.com> - 4.8.3-3
- add rhel/fedora condition

* Wed May 02 2012 Rex Dieter <rdieter@fedoraproject.org> - 4.8.3-2
- rebuild (exiv2)

* Mon Apr 30 2012 Jaroslav Reznik <jreznik@redhat.com> - 4.8.3-1
- 4.8.3

* Fri Mar 30 2012 Jaroslav Reznik <jreznik@redhat.com> - 4.8.2-1
- 4.8.2

* Thu Mar 08 2012 Rex Dieter <rdieter@fedoraproject.org> 4.8.1-2
- upstream listview margins patch

* Mon Mar 05 2012 Jaroslav Reznik <jreznik@redhat.com> - 4.8.1-1
- 4.8.1
- fix libnepomukdatamanagement (-devel, -libs)

* Tue Feb 28 2012 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 4.8.0-6
- Rebuilt for c++ ABI breakage

* Mon Feb 27 2012 Rex Dieter <rdieter@fedoraproject.org> 4.8.0-5
- Requires: dbus-x11 (#794958)

* Wed Feb 08 2012 Kevin Kofler <Kevin@tigcc.ticalc.org> - 4.8.0-4
- fix quoting in SELinux scriptlet (#788309)

* Tue Feb 07 2012 Kevin Kofler <Kevin@tigcc.ticalc.org> - 4.8.0-3
- add kconf_update script to migrate nepomukserverrc config group (#771053)

* Mon Feb 06 2012 Kevin Kofler <Kevin@tigcc.ticalc.org> - 4.8.0-2
- make DrKonqi work by default by taming SELinux enough (suggested by dwalsh)

* Fri Jan 20 2012 Jaroslav Reznik <jreznik@redhat.com> - 4.8.0-1
- 4.8.0

* Sat Jan 14 2012 Bruno Wolff III 4.7.97-12
- Fix typo in kde-runtime-flags obsolete that prevents updates

* Fri Jan 13 2012 Rex Dieter <rdieter@fedoraproject.org> 4.7.97-11
- %%check: desktop-file-validate
- %%doc COPYING COPYING.LIB

* Wed Jan 04 2012 Rex Dieter <rdieter@fedoraproject.org> 4.7.97-10
- kdebase-runtime -> kde-runtime rename

* Wed Jan 04 2012 Rex Dieter <rdieter@fedoraproject.org> 4.7.97-1
- 4.7.97

* Sat Dec 31 2011 Rex Dieter <rdieter@fedoraproject.org> 4.7.95-2
- rebuild (attica)

* Wed Dec 21 2011 Radek Novacek <rnovacek@redhat.com> - 4.7.95-1
- 4.7.95

* Fri Dec 09 2011 Rex Dieter <rdieter@fedoraproject.org> 4.7.90-2
- make nepomuk menu items (with only oxygen icons) OnlyShowIn=KDE (#682055)

* Sat Dec 03 2011 Rex Dieter <rdieter@fedoraproject.org> 4.7.90-1
- 4.7.90

* Fri Nov 18 2011 Jaroslav Reznik <jreznik@redhat.com> 4.7.80-1
- 4.7.80 (beta 1)

* Thu Nov 17 2011 Rex Dieter <rdieter@fedoraproject.org> 4.7.3-12
- active-development-4.7-diff.patch
- omit activitymanger unconditionally
- drop dep on libkactivities

* Tue Nov 15 2011 Rex Dieter <rdieter@fedoraproject.org> 4.7.3-11
- BR: libjpeg-devel

* Fri Nov 04 2011 Rex Dieter <rdieter@fedoraproject.org> 4.7.3-10
- no_activitymanager

* Thu Nov 03 2011 Lukas Tinkl <ltinkl@redhat.com> - 4.7.3-3
- require poppler-utils for Nepomuk (PDF file indexing)

* Mon Oct 31 2011 Rex Dieter <rdieter@fedoraproject.org> 4.7.3-2
- drop activitymanager/zeitgiest support

* Sat Oct 29 2011 Rex Dieter <rdieter@fedoraproject.org> 4.7.3-1
- 4.7.3
- drop QUrl hack/workaround

* Wed Oct 26 2011 Rex Dieter <rdieter@fedoraproject.org> 4.7.2-6
- fix knotify, workaround Qt 4.8 QUrl.toLocalFile behavior change (#749213)

* Tue Oct 25 2011 Rex Dieter <rdieter@fedoraproject.org> 4.7.2-5
- include toggle to omit activitymanager

* Mon Oct 24 2011 Rex Dieter <rdieter@fedoraproject.org> 4.7.2-4
- pkgconfig-style deps

* Fri Oct 14 2011 Rex Dieter <rdieter@fedoraproject.org> - 4.7.2-3
- rebuild (exiv2)

* Wed Oct 05 2011 Rex Dieter <rdieter@fedoraproject.org> 4.7.2-2
- drop hard cagibi dep (add to comps instead)

* Tue Oct 04 2011 Rex Dieter <rdieter@fedoraproject.org> 4.7.2-1
- 4.7.2

* Tue Sep 27 2011 Kevin Kofler <Kevin@tigcc.ticalc.org> 4.7.1-2
- rebuild once again for fixed RPM dependency generators for Plasma

* Fri Sep 02 2011 Than Ngo <than@redhat.com> - 4.7.1-1
- 4.7.1

* Sun Aug 21 2011 Kevin Kofler <Kevin@tigcc.ticalc.org> 4.7.0-5
- rebuild again for the fixed RPM dependency generators for Plasma (#732271)

* Sun Aug 21 2011 Kevin Kofler <Kevin@tigcc.ticalc.org> 4.7.0-4
- rebuild for the RPM dependency generators for Plasma (GSoC 2011)

* Tue Jul 26 2011 Jaroslav Reznik <jreznik@redhat.com> - 4.7.0-3
- respin

* Tue Jul 26 2011 Jaroslav Reznik <jreznik@redhat.com> 4.7.0-2
- BR libqzeitgeist

* Tue Jul 26 2011 Jaroslav Reznik <jreznik@redhat.com> 4.7.0-1
- 4.7.0
- Provides: kde-runtime (matching new upstream tarball)

* Thu Jul 21 2011 Rex Dieter <rdieter@fedoraproject.org> 4.6.95-2
- rebuild (qt48)

* Fri Jul 08 2011 Jaroslav Reznik <jreznik@redhat.com> 4.6.95-1
- 4.6.95 (rc2)
- Nepomuk IndexCleaner throttling (kdebz#276593)

* Tue Jun 28 2011 Rex Dieter <rdieter@fedoraproject.org> 4.6.90-2
- move oxygen-icons-theme dep (back) here from kdelibs 

* Mon Jun 27 2011 Than Ngo <than@redhat.com> - 4.6.90-1
- 4.6.90 (rc1)

* Tue Jun 14 2011 Rex Dieter <rdieter@fedoraproject.org> 4.6.80-2
- drop xine support (f16+, #723933)
- drop references to crystalsvg icons

* Fri May 27 2011 Jaroslav Reznik <jreznik@redhat.com> - 4.6.80-1
- 4.6.80 (beta1)
- upstream tarball is now kde-runtime

* Thu Apr 28 2011 Rex Dieter <rdieter@fedoraproject.org> - 4.6.3-1
- 4.6.3 

* Wed Apr 06 2011 Than Ngo <than@redhat.com> - 4.6.2-1
- 4.6.2

* Thu Mar 24 2011 Kevin Kofler <Kevin@tigcc.ticalc.org> 4.6.1-3
- rebuild in the main F15 buildroot now that NM 0.9 has been tagged into it

* Wed Mar 23 2011 Rex Dieter <rdieter@fedoraproject.org> 4.6.1-2
- nm-0.9 support patch

* Sat Feb 26 2011 Rex Dieter <rdieter@fedoraproject.org> 4.6.1-1
- 4.6.1

* Tue Feb 08 2011 Than Ngo <than@redhat.com> - 4.6.0-2
- add overrides in default manpath

* Fri Jan 21 2011 Jaroslav Reznik <jreznik@redhat.com> - 4.6.0-1
- 4.6.0

* Thu Jan 20 2011 Adam Williamson <awilliam@redhat.com> - 4.5.95-3
- update compiz patch to launch ccsm for configuration, not simple-ccsm
  (we don't package it)

* Thu Jan 20 2011 Adam Williamson <awilliam@redhat.com> - 4.5.95-2
- fix up compiz launching from systemsettings (by calling compiz-manager)

* Wed Jan 05 2011 Jaroslav Reznik <jreznik@redhat.com> - 4.5.95-1
- 4.5.95 (4.6rc2)

* Sat Jan 01 2011 Rex Dieter <rdieter@fedoraproject.org> - 4.5.90-2
- rebuild (exiv2)

* Wed Dec 22 2010 Rex Dieter <rdieter@fedoraproject.org> 4.5.90-1
- 4.5.90 (4.6rc)

* Sat Dec 04 2010 Thomas Janssen <thomasj@fedoraproject.org> 4.5.85-1
- 4.5.85 (4.6beta2)

* Tue Nov 23 2010 Rex Dieter <rdieter@fedoraproject.org> 4.5.80-3
- -libs: move libknotifyplugin.so here

* Sun Nov 21 2010 Rex Dieter <rdieter@fedoraproject.org> - 4.5.80-2
- Conflicts: kdebase-workspace < 4.5.80

* Sat Nov 20 2010 Rex Dieter <rdieter@fedoraproject.org> - 4.5.80-1
- 4.5.80 (4.6beta1)

* Fri Nov 12 2010 Rex Dieter <rdieter@fedoraproject.org> - 4.5.4-4
- fix build with newer phonon(4.4.3+)

* Fri Nov 12 2010 Rex Dieter <rdieter@fedoraproject.org> - 4.5.3-3
- kdesu password-keeping does not work (#650630)
- htsearch patch

* Thu Nov 11 2010 Rex Dieter <rdieter@fedoraproject.org> - 4.5.3-2
- Crash after visiting Phonon setting (#652409, kde#255736)

* Fri Oct 29 2010 Than Ngo <than@redhat.com> - 4.5.3-1
- 4.5.3

* Fri Oct 15 2010 Rex Dieter <rdieter@fedoraproject.org> - 4.5.2-3
- pa speakersetup backport (courtesy of coling/mandriva)
- hammer rpath from phonon_platform/kde.so

* Fri Oct 15 2010 Rex Dieter <rdieter@fedoraproject.org> - 4.5.2-2
- use kde4's copy of khelpcenter.desktop

* Fri Oct 01 2010 Rex Dieter <rdieter@fedoraproject.org> -  4.5.2-1
- 4.5.2

* Fri Aug 27 2010 Jaroslav Reznik <jreznik@redhat.com> - 4.5.1-1
- 4.5.1

* Tue Aug 10 2010 Than Ngo <than@redhat.com> - 4.5.0-3
- backport to fix the freeze of kded on first use of the network:/kio-slave

* Fri Aug 06 2010 Jaroslav Reznik <jreznik@redhat.com> - 4.5.0-2
- requires cagibi

* Tue Aug 03 2010 Than Ngo <than@redhat.com> - 4.5.0-1
- 4.5.0

* Sun Jul 25 2010 Rex Dieter <rdieter@fedoraproject.org> - 4.4.95-1
- 4.5 RC3 (4.4.95)

* Fri Jul 16 2010 Rex Dieter <rdieter@fedoraproject.org> - 4.4.92-2
- move oxygen-icon-theme (back) to kdelibs

* Wed Jul 07 2010 Rex Dieter <rdieter@fedoraproject.org> - 4.4.92-1
- 4.5 RC2 (4.4.92)
- Requires: -flags
- drop virtuosoconverter patch

* Fri Jun 25 2010 Jaroslav Reznik <jreznik@redhat.com> - 4.4.90-1
- 4.5 RC1 (4.4.90)

* Mon Jun 07 2010 Jaroslav Reznik <jreznik@redhat.com> - 4.4.85-1
- 4.5 Beta 2 (4.4.85)
- added remote widgets service and policy (from kdelibs)

* Mon May 31 2010 Rex Dieter <rdieter@fedoraproject.org> - 4.4.80-2 
- rebuild (exiv2)

* Fri May 21 2010 Jaroslav Reznik <jreznik@redhat.com> - 4.4.80-1
- 4.5 Beta 1 (4.4.80)
- install dbus .service files

* Fri Apr 30 2010 Jaroslav Reznik <jreznik@redhat.com> - 4.4.3-1
- 4.4.3

* Wed Apr 21 2010 Rex Dieter <rdieter@fedoraproject.org> - 4.4.2-3
- kio_sftp does not work - "Error. Out of memory"  (#582968)
- BR: libssh-devel >= 0.4.2

* Sat Apr 17 2010 Rex Dieter <rdieter@fedoraproject.org> - 4.4.2-2
- Requires: hal-storage-addon for f13 too (#579256)

* Mon Mar 29 2010 Lukas Tinkl <ltinkl@redhat.com> - 4.4.2-1
- 4.4.2

* Tue Mar 16 2010 Rex Dieter <rdieter@fedoraproject.org> - 4.4.1-2
- Requires: hal-storage-addon (f14+)
- drop <f11 Conflicts baggage
- plasma-scriptengine-javascript-4.4-errata

* Sat Feb 27 2010 Rex Dieter <rdieter@fedoraproject.org> - 4.4.1-1
- 4.4.1

* Mon Feb 08 2010 Than Ngo <than@redhat.com> - 4.4.0-3
- respin

* Sun Feb 07 2010 Rex Dieter <rdieter@fedoraproject.org> - 4.4.0-2
- nepomuk_fulltextindex patch
- virtuosoconverter patch

* Fri Feb 05 2010 Than Ngo <than@redhat.com> - 4.4.0-1
- 4.4.0

* Sun Jan 31 2010 Rex Dieter <rdieter@fedoraproject.org> - 4.3.98-1
- KDE 4.3.98 (4.4rc3)
- notifications for new global shortcuts should be silenced (#549784)

* Wed Jan 20 2010 Lukas Tinkl <ltinkl@redhat.com> - 4.3.95-1
- KDE 4.3.95 (4.4rc2)

* Tue Jan 19 2010 Kevin Kofler <Kevin@tigcc.ticalc.org> - 4.3.90-2
- install installdbgsymbols_fedora.sh as %%{_kde4_libexecdir}/installdbgsymbols.sh

* Wed Jan 06 2010 Rex Dieter <rdieter@fedoraproject.org> - 4.3.90-1
- kde-4.3.85 (4.4rc1)

* Sun Jan 03 2010 Rex Dieter <rdieter@fedoraproject.org> - 4.3.85-3
- rebuild (exiv2)

* Sat Jan 02 2010 Rex Dieter <rdieter@fedoraproject.org> - 4.3.85-2
- Provides: desktop-notification-daemon

* Fri Dec 18 2009 Rex Dieter <rdieter@fedoraproject.org> - 4.3.85-1
- kde-4.3.85 (4.4beta2)

* Wed Dec 16 2009 Jaroslav Reznik <jreznik@redhat.com> - 4.3.80-4
- Repositioning the KDE Brand (#547361)

* Wed Dec 09 2009 Rex Dieter <rdieter@fedoraproject.org> - 4.3.80-3
- BR: attica-devel shared-desktop-ontologies-devel

* Sat Dec 05 2009 Kevin Kofler <Kevin@tigcc.ticalc.org> - 4.3.80-2
- BR exiv2-devel

* Tue Dec 01 2009 Lukáš Tinkl <ltinkl@redhat.com> - 4.3.80-1
- KDE 4.4 beta1 (4.3.80)

* Mon Nov 30 2009 Kevin Kofler <Kevin@tigcc.ticalc.org> - 4.3.75-0.2.svn1048496
- BR libssh-devel >= 0.3.92

* Sat Nov 21 2009 Ben Boeckel <MathStuf@gmail.com> - 4.3.75-0.1.svn1048496
- update to 4.3.75 snapshot

* Wed Nov 18 2009 Rex Dieter <rdieter@fedoraproject.org> - 4.3.3-6
- rebuild (qt-4.6.0-rc1, fc13+)

* Sat Nov 14 2009 Rex Dieter <rdieter@fedoraproject.org> - 4.3.3-5
- disable manpath patch for now, does more harm than good (#532071)

* Fri Nov 13 2009 Than Ngo <than@redhat.com> - 4.3.3-4
- rhel cleanup, fix conditional for RHEL

* Wed Nov 11 2009 Than Ngo <than@redhat.com> - 4.3.3-3
- rhel cleanup, drop BR on openslp-devel

* Thu Nov 05 2009 Rex Dieter <rdieter@fedoraproject.org> 4.3.3-2
- add OnlyShowIn=KDE to Desktop/Home.desktop (like trash.desktop)

* Sat Oct 31 2009 Rex Dieter <rdieter@fedoraproject.org> 4.3.3-1
- 4.3.3

* Thu Oct 15 2009 Rex Dieter <rdieter@fedoraproject.org> 4.3.2-4
- Conflicts: kdebase4 < 4.3.0 instead

* Wed Oct 14 2009 Rex Dieter <rdieter@fedoraproject.org> 4.3.2-3
- Conflicts: kdebase < 6:4.3.0
- Requires: oxygen-icon-theme >= %%{version}

* Tue Oct 06 2009 Rex Dieter <rdieter@fedoraproject.org> 4.3.2-2
- BR: bzip2-devel xz-devel
- -libs: move Requires: kdepimlibs... here

* Sun Oct 04 2009 Than Ngo <than@redhat.com> - 4.3.2-1
- 4.3.2

* Wed Sep 30 2009 Nils Philippsen <nils@redhat.com> - 4.3.1-4
- fix manpath patch (spotted by Kevin Kofler)

* Wed Sep 30 2009 Nils Philippsen <nils@redhat.com> - 4.3.1-3
- if available, use the "manpath" command in the man kioslave to determine man
  page file locations

* Tue Sep 15 2009 Rex Dieter <rdieter@fedorproject.org> - 4.3.1-2
- restore some previously inadvertantly omitted nepomuk ontologies

* Fri Aug 28 2009 Than Ngo <than@redhat.com> - 4.3.1-1
- 4.3.1

* Wed Aug 12 2009 Lukáš Tinkl <ltinkl@redhat.com> - 4.3.0-4
- unbreak fish kioslave protocol (#516416)

* Mon Aug 10 2009 Lukáš Tinkl <ltinkl@redhat.com> - 4.3.0-3
- fix Oxygen comboboxes' text being garbled (drawn twice); fixes kdebug:202701
- fix Locale control module crashing when dragging languages around (kdebug:201578)

* Tue Aug 04 2009 Than Ngo <than@redhat.com> - 4.3.0-2
- respin

* Thu Jul 30 2009 Than Ngo <than@redhat.com> - 4.3.0-1
- 4.3.0

* Fri Jul 24 2009 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 4.2.98-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_12_Mass_Rebuild

* Wed Jul 22 2009 Than Ngo <than@redhat.com> - 4.2.98-1
- 4.3rc3

* Thu Jul 16 2009 Rex Dieter <rdieter@fedoraproject.org> - 4.2.96-2
- respin (soprano-2.3.0)
- License: LGPLv2+

* Thu Jul 09 2009 Than Ngo <than@redhat.com> - 4.2.96-1
- 4.3rc2

* Thu Jul 02 2009 Rex Dieter <rdieter@fedoraproject.org> 4.2.95-3
- drop unneeded BR: ImageMagick (#509241)

* Mon Jun 29 2009 Lukáš Tinkl <ltinkl@redhat.com> - 4.2.95-2
- don't start nepomuk server unconditionally (#487322)

* Thu Jun 25 2009 Than Ngo <than@redhat.com> - 4.2.95-1
- 4.3rc1

* Wed Jun 03 2009 Rex Dieter <rdieter@fedoraproject.org> - 4.2.90-1
- KDE-4.3 beta2 (4.2.90)

* Tue Jun 02 2009 Lorenzo Villani <lvillani@binaryhelix.net> - 4.2.85-3
- Drop old Fedora < 8 conditionals

* Tue May 19 2009 Than Ngo <than@redhat.com> - 4.2.85-2
- file conflicts with kdepim

* Wed May 13 2009 Lukáš Tinkl <ltinkl@redhat.com> - 4.2.85-1
- KDE 4.3 beta 1

* Thu Apr 16 2009 Rex Dieter <rdieter@fedoraproject.org> - 4.2.2-4
- fix persistent systray notifications (#485796)

* Wed Apr 01 2009 Rex Dieter <rdieter@fedoraproject.org> - 4.2.2-3
- -flags subpkg
- koji/noarch hacks dropped

* Wed Apr 01 2009 Than Ngo <than@redhat.com> - 4.2.2-2
- drop kdebase-runtime-4.2.1-pulseaudio-cmake.patch

* Mon Mar 30 2009 Lukáš Tinkl <ltinkl@redhat.com> - 4.2.2-1
- KDE 4.2.2

* Fri Mar 27 2009 Rex Dieter <rdieter@fedoraproject.org> - 4.2.1-3
- flags subpkg (not enabled)
- optimize scriptlets

* Tue Mar  3 2009 Lukáš Tinkl <ltinkl@redhat.com> - 4.2.1-2
- fix PulseAudio cmake detection

* Fri Feb 27 2009 Than Ngo <than@redhat.com> - 4.2.1-1
- 4.2.1

* Wed Feb 25 2009 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 4.2.0-8
- Rebuilt for https://fedoraproject.org/wiki/Fedora_11_Mass_Rebuild

* Wed Feb 18 2009 Lukáš Tinkl <ltinkl@redhat.com> - 4.2.0-7
- #486059 -  missing dependency on htdig

* Thu Feb 12 2009 Rex Dieter <rdieter@fedoraproject.org> - 4.2.0-6
- -libs: include %%{_kde4_libdir}/libkwalletbackend.so.* here

* Thu Feb 12 2009 Rex Dieter <rdieter@fedoraproject.org> - 4.2.0-5
- Req: %%{name}-libs%%{?_isa} for multilib sanity (#456926)

* Mon Feb 02 2009 Rex Dieter <rdieter@fedoraproject.org> - 4.2.0-4
- own %%_kde4_datadir/locale/l10n/

* Mon Jan 26 2009 Rex Dieter <rdieter@fedoraproject.org> - 4.2.0-3
- respun tarball

* Mon Jan 26 2009 Rex Dieter <rdieter@fedoraproject.org> - 4.2.0-2
- Files not trashed to the correct location in Dolphin/Konqueror
  (#481598, kdebug#178479)
- omit --with-samba crud

* Thu Jan 22 2009 Than Ngo <than@redhat.com> - 4.2.0-1
- 4.2.0
- +BR: pulseaudio-libs-devel xine-lib-devel
- -BR: giflib-devel pcre-devel

* Tue Jan 13 2009 Rex Dieter <rdieter@fedoraproject.org> 4.1.96-2
- tarball respin
- drop extraneous deps (that are in kdelibs)

* Wed Jan 07 2009 Than Ngo <than@redhat.com> - 4.1.96-1
- 4.2rc1

* Mon Dec 22 2008 Rex Dieter <rdieter@fedoraproject.org> 4.1.85-2
- include %%_bindir/kdesu symlink

* Thu Dec 11 2008 Than Ngo <than@redhat.com> 4.1.85-1
- 4.2beta2

* Mon Dec 01 2008 Kevin Kofler <Kevin@tigcc.ticalc.org> 4.1.80-5
- don't ship libkwalletbackend.so devel symlink (conflicts with kdelibs3-devel,
  and should be in a -devel package if it gets shipped)

* Thu Nov 27 2008 Kevin Kofler <Kevin@tigcc.ticalc.org> 4.1.80-4
- BR strigi-devel >= 0.5.11.1 because 0.5.11 is broken

* Thu Nov 20 2008 Kevin Kofler <Kevin@tigcc.ticalc.org> 4.1.80-3
- readd still relevant part of the Phonon PulseAudio patch (for the KCM)

* Wed Nov 19 2008 Than Ngo <than@redhat.com> 4.1.80-2
- drop kdebase-runtime-4.0.72-pulseaudio.patch/icons, it's part of phonon

* Wed Nov 19 2008 Lorenzo Villani <lvillani@binaryhelix.net> - 4.1.80-1
- 4.1.80
- Drop upstreamed patch kdebase-runtime-4.1.2-kioexec.patch
- BR cmake >= 2.6.2
- Use 'make install/fast'
- Drop subpkg phonon-backend-xine and related file entries: this has to be
  part of phonon now that it moved there
- Drop xine-lib-devel BR
- Add libkwalletbackend to files list
- Drop _default_patch_fuzz 2

* Thu Nov 13 2008 Than Ngo <than@redhat.com> 4.1.3-5
- apply upstream patch to fix X crash when disabling compositing

* Wed Nov 12 2008 Than Ngo <than@redhat.com> 4.1.3-1
- 4.1.3

* Tue Oct 14 2008 Than Ngo <than@redhat.com> 4.1.2-5
- apply upstream patch, kioexec processes never terminate

* Tue Sep 30 2008 Than Ngo <than@redhat.com> 4.1.2-4
- fix broken audio-backend-jack.svgz

* Tue Sep 30 2008 Than Ngo <than@redhat.com> 4.1.2-3
- add missing icons

* Sun Sep 28 2008 Rex Dieter <rdieter@fedoraproject.org> 4.1.2-2
- make VERBOSE=1
- respin against new(er) kde-filesystem
- grow -libs, kde4 styles are unavailable for i386 applications (#456826)

* Fri Sep 26 2008 Rex Dieter <rdieter@fedoraproject.org. 4.1.2-1
- 4.1.2

* Tue Sep 16 2008 Than Ngo <than@redhat.com> 4.1.1-3
- fix inherit issue in iconthemes, preview icons
  do not show

* Mon Sep 01 2008 Than Ngo <than@redhat.com> 4.1.1-2
- fix #460710, knetattach is kio_remote's wizard program, don't show
  it in the menu.

* Thu Aug 28 2008 Than Ngo <than@redhat.com> 4.1.1-1
- 4.1.1

* Wed Aug 13 2008 Kevin Kofler <Kevin@tigcc.ticalc.org> 4.1.0-3
- fix PA not being default in the Xine backend (KCM part, see phonon-4.2.0-4)

* Tue Aug 12 2008 Than Ngo <than@redhat.com> 4.1.0-2
- crash fix when stopping a service that is not yet initialized

* Fri Jul 25 2008 Kevin Kofler <Kevin@tigcc.ticalc.org> 4.1.0-1.1
- don't remove autostart directory on F8- (does not conflict, fixes build
  failure due to nepomukserver.desktop listed in filelist but not found)

* Wed Jul 23 2008 Than Ngo <than@redhat.com> 4.1.0-1
- 4.1.0

* Wed Jul 23 2008 Rex Dieter <rdieter@fedoraproject.org> 4.0.99-2
- phonon-backend-xine: drop Obsoletes/Requires upgrade hack

* Fri Jul 18 2008 Rex Dieter <rdieter@fedoraproject.org> 4.0.99-1
- 4.0.99

* Mon Jul 14 2008 Rex Dieter <rdieter@fedoraproject.org> 4.0.98-4
- respin

* Mon Jul 14 2008 Rex Dieter <rdieter@fedoraproject.org> 4.0.98-3
- -phonon-backend-xine: new subpkg

* Thu Jul 10 2008 Rex Dieter <rdieter@fedoraproject.org> 4.0.98-1
- 4.0.98

* Sun Jul 06 2008 Rex Dieter <rdieter@fedoraproject.org> 4.0.85-1
- 4.0.85

* Fri Jun 27 2008 Rex Dieter <rdieter@fedoraproject.org> 4.0.84-1
- 4.0.84

* Thu Jun 19 2008 Than Ngo <than@redhat.com> 4.0.83-1
- 4.0.83 (beta2)

* Sat Jun 14 2008 Rex Dieter <rdieter@fedoraproject.org> 4.0.82-1
- 4.0.82

* Thu Jun 05 2008 Than Ngo <than@redhat.com> 4.0.80-2
- add searchproviders-shortcuts for redhat bugzilla

* Mon May 26 2008 Than Ngo <than@redhat.com> 4.0.80-1
- 4.1 beta 1

* Tue May 06 2008 Kevin Kofler <Kevin@tigcc.ticalc.org> 4.0.72-2
- BR new minimum version of soprano-devel

* Tue May 06 2008 Kevin Kofler <Kevin@tigcc.ticalc.org> 4.0.72-1
- update to 4.0.72 (4.1 alpha 1)
- drop upstreamed deinterlace-crash patch
- drop khelpcenter patch (fixed upstream)
- update Phonon PulseAudio patch
- drop Fedora 7 support
- update file list

* Mon Apr 28 2008 Rex Dieter <rdieter@fedoraproject.org> 4.0.3-10.1
- omit conflicting icons (kde3_desktop=1 case)

* Thu Apr 17 2008 Rex Dieter <rdieter@fedoraproject.org> 4.0.3-10
- oxygen-icon-theme: build noarch

* Thu Apr 17 2008 Rex Dieter <rdieter@fedoraproject.org> 4.0.3-9
- %%post/%%postun: hicolor icon theme scriptlets

* Thu Apr 17 2008 Than Ngo <than@redhat.com> 4.0.3-8
- only omit hicolor index.theme (#439374)

* Thu Apr 17 2008 Than Ngo <than@redhat.com> 4.0.3-7
- fix khelpcenter, search plugins/settings in correct path (#443016)

* Tue Apr 15 2008 Rex Dieter <rdieter@fedoraproject.org> 4.0.3-6
- respin (at f13's request)

* Mon Apr 07 2008 Rex Dieter <rdieter@fedoraproject.org> 4.0.3-5
- pulseaudio patch (use as default, if available)

* Sat Apr 05 2008 Kevin Kofler <Kevin@tigcc.ticalc.org> 4.0.3-4
- don't crash if we don't have deinterlacing support in xine-lib (#440299)

* Thu Apr 03 2008 Kevin Kofler <Kevin@tigcc.ticalc.org> 4.0.3-3
- rebuild for the new %%{_kde4_buildtype}

* Mon Mar 31 2008 Kevin Kofler <Kevin@tigcc.ticalc.org> 4.0.3-2
- update file list for _kde4_libexecdir

* Fri Mar 28 2008 Than Ngo <than@redhat.com> 4.0.3-1
- 4.0.3

* Thu Mar 20 2008 Rex Dieter <rdieter@fedoraproject.org> 4.0.2-5
- don't own %%_kde4_docdir/HTML/en/

* Thu Mar 20 2008 Rex Dieter <rdieter@fedoraproject.org> 4.0.2-4
- oxygen-icon-theme, oxygen-icon-theme-scalable pkgs
- include noarch build hooks (not enabled)

* Fri Mar 07 2008 Kevin Kofler <Kevin@tigcc.ticalc.org> 4.0.2-3
- BR libxcb-devel everywhere (including F7)

* Fri Mar 07 2008 Kevin Kofler <Kevin@tigcc.ticalc.org> 4.0.2-2
- if building for a KDE 4 desktop, include the khelpcenter.desktop service
  description for KDE 3 here so help works in KDE 3 apps

* Fri Feb 29 2008 Than Ngo <than@redhat.com> 4.0.2-1
- 4.0.2

* Mon Feb 25 2008 Rex Dieter <rdieter@fedoraproject.org> 4.0.1-3
- %%files: don't own %%_kde4_libdir/kde4/plugins (thanks wolfy!)

* Sat Feb 23 2008 Kevin Kofler <Kevin@tigcc.ticalc.org> 4.0.1-2
- reenable kio_smb everywhere (including F9) now that we have a GPLv3 qt4
  (kio_smb itself is already GPLv2+)

* Wed Jan 30 2008 Rex Dieter <rdieter@fedoraproject.org> 4.0.1-1
- 4.0.1

* Tue Jan 08 2008 Rex Dieter <rdieter[AT]fedoraproject.org> 4.0.0-2
- respun tarball

* Mon Jan 07 2008 Kevin Kofler <Kevin@tigcc.ticalc.org> 4.0.0-1
- update to 4.0.0
- update file list, don't remove renamed khotnewstuff.knsrc for KDE 3 desktop
