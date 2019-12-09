
Name:           kde-connect
Version:        1.4
Release:        1%{?dist}
License:        GPLv2+
Summary:        KDE Connect client for communication with smartphones

Url:            https://cgit.kde.org/kdeconnect-kde.git
%if 0%{?_git_describe:1}
# using releaseme:
# ./tarme.rb kdeconnect-kde --origin trunk --version 0.8
# then rename accordingly
Source0:        kdeconnect-kde-%{_git_describe}.tar.xz
%else
Source0:        http://download.kde.org/stable/kdeconnect/%{version}/kdeconnect-kde-%{version}.tar.xz
%endif

# firewalld service definition, see https://bugzilla.redhat.com/show_bug.cgi?id=1257699#c2
Source2:        kde-connect.xml

BuildRequires:  desktop-file-utils
BuildRequires:  firewalld-filesystem
BuildRequires:  gcc-c++

BuildRequires:  extra-cmake-modules >= 5.42
BuildRequires:  kf5-rpm-macros
BuildRequires:  kf5-kcmutils-devel
BuildRequires:  kf5-kdoctools-devel
BuildRequires:  kf5-kconfigwidgets-devel
BuildRequires:  kf5-kdbusaddons-devel
BuildRequires:  kf5-ki18n-devel
BuildRequires:  kf5-kiconthemes-devel
BuildRequires:  kf5-kio-devel
BuildRequires:  kf5-knotifications-devel
BuildRequires:  kf5-kwayland-devel

BuildRequires:  qt5-qtbase-devel
BuildRequires:  qt5-qtdeclarative-devel
BuildRequires:  qt5-qtx11extras-devel

BuildRequires:  libXtst-devel
BuildRequires:  pkgconfig(libfakekey)
BuildRequires:  qca-qt5-devel >= 2.1.0-14

BuildRequires:  cmake(KF5Kirigami2) >= 5.64.0
BuildRequires:  cmake(KF5People)    >= 5.64.0

BuildRequires:  cmake(Qt5Multimedia)
BuildRequires:  cmake(KF5PeopleVCard)
BuildRequires:  cmake(KF5PulseAudioQt)

Obsoletes: kde-connect-kde4-ioslave < %{version}-%{release}
Obsoletes: kde-connect-kde4-libs < %{version}-%{release}

# upstream name
Provides:       kdeconnect-kde = %{version}-%{release}

Requires:       %{name}-libs%{?_isa} = %{version}-%{release}
Requires:       kdeconnectd = %{version}-%{release}

Requires:       fuse-sshfs
Requires:       qca-qt5-ossl%{?_isa}
# /usr/bin/plasmawindowed (make optional at least until this is split out for bug #1286431)
#Recommends:     plasma-workspace
# /usr/bin/kcmshell5
Requires:       kde-cli-tools

%description
KDE Connect adds communication between KDE and your smartphone.

Currently, you can pair with your Android devices over Wifi using the
KDE Connect 1.0 app from Albert Vaka which you can obtain via Google Play, F-Droid
or the project website.

%package -n kdeconnectd
Summary: KDE Connect service
Requires: %{name}-libs%{?_isa} = %{version}-%{release}
Requires: firewalld-filesystem
%description -n kdeconnectd
%{summary}.

%package libs
Summary: Runtime libraries for %{name}
# I think we may want to drop this, forces kdeconnectd to pull in main pkg indirectly -- rex
Requires: %{name} = %{version}-%{release}
%description libs
%{summary}.

%package devel
Summary: Development files for %{name}
Requires: %{name}-libs%{?_isa} = %{version}-%{release}
%description devel
%{summary}.

%package nautilus
Summary: KDEConnect extention for nautilus
Requires: kdeconnectd = %{version}-%{release}
Requires: nautilus-python
Supplements: (kdeconnectd and nautilus)
%description nautilus
%{summary}.


%prep
%autosetup -n kdeconnect-kde-%{version} -p1


%build
mkdir %{_target_platform}
pushd %{_target_platform}
%{cmake_kf5} ..
popd

%make_build -C %{_target_platform}


%install
make install/fast DESTDIR=%{buildroot} -C %{_target_platform}

install -m644 -p -D %{SOURCE2} %{buildroot}%{_prefix}/lib/firewalld/services/kde-connect.xml

%find_lang %{name} --all-name --with-html

# https://bugzilla.redhat.com/show_bug.cgi?id=1296523
desktop-file-edit --remove-key=OnlyShowIn %{buildroot}%{_sysconfdir}/xdg/autostart/org.kde.kdeconnect.daemon.desktop


%check
#desktop-file-validate %{buildroot}%{_datadir}/applications/*.desktop
#desktop-file-validate %{buildroot}%{_sysconfdir}/xdg/autostart/*.desktop


%files -f %{name}.lang
%license COPYING
%{_kf5_bindir}/kdeconnect-*
%{_kf5_datadir}/plasma/plasmoids/org.kde.kdeconnect/
%{_kf5_datadir}/knotifications5/*
%{_kf5_datadir}/kservices5/*.desktop
%{_kf5_datadir}/kservicetypes5/*.desktop
%{_kf5_datadir}/Thunar/sendto/kdeconnect-thunar.desktop
%{_qt5_plugindir}/kcm_kdeconnect.so
%{_kf5_plugindir}/kio/kdeconnect.so
%{_datadir}/icons/hicolor/*/apps/kdeconnect*
%{_datadir}/icons/hicolor/*/status/*
%{_datadir}/contractor/kdeconnect.contract
%{_datadir}/zsh/site-functions/_kdeconnect
%{_kf5_metainfodir}/org.kde.kdeconnect.kcm.appdata.xml
%{_datadir}/applications/*.desktop
%{_qt5_archdatadir}/qml/org/kde/kdeconnect/

%post -n kdeconnectd
%{?firewalld_reload}

%postun -n kdeconnectd
if [ $1 -eq 0 ] ; then
%{?firewalld_reload}
fi

%files -n kdeconnectd
%{_sysconfdir}/xdg/autostart/org.kde.kdeconnect.daemon.desktop
%{_libexecdir}/kdeconnectd
%{_datadir}/dbus-1/services/org.kde.kdeconnect.service
%{_prefix}/lib/firewalld/services/kde-connect.xml

%ldconfig_scriptlets libs

%files libs
%{_libdir}/libkdeconnectsmshelper.so.*
%{_kf5_libdir}/libkdeconnectpluginkcm.so.1*
%{_kf5_libdir}/libkdeconnectinterfaces.so.1*
%{_kf5_libdir}/libkdeconnectcore.so.1*
%{_qt5_plugindir}/kdeconnect*.so
%{_qt5_plugindir}/kdeconnect/
%{_kf5_plugindir}/kfileitemaction/kdeconnectfileitemaction.so

%files nautilus
%{_datadir}/nautilus-python/extensions/kdeconnect-share.py*


%changelog
* Mon Dec 09 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 1.4-1
- 1.4

* Mon Sep 30 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 1.3.5-1
- 1.3.5

* Wed Jun 26 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 1.3.4-3
- 1.3.4

* Fri Feb 01 2019 Fedora Release Engineering <releng@fedoraproject.org> - 1.3.3-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_30_Mass_Rebuild

* Sat Nov 10 2018 Rex Dieter <rdieter@fedoraproject.org> - 1.3.3-1
- 1.3.3

* Fri Jul 13 2018 Fedora Release Engineering <releng@fedoraproject.org> - 1.3.1-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_29_Mass_Rebuild

* Thu May 31 2018 Rex Dieter <rdieter@fedoraproject.org> - 1.3.1-1
- 1.3.1

* Mon Apr 09 2018 Rex Dieter <rdieter@fedoraproject.org> - 1.3.0-1
- 1.3.0
- -nautilus subpkg (extention for nautilus)

* Sun Mar 04 2018 Rex Dieter <rdieter@fedoraproject.org> - 1.2.1-3
- use %%make_build %%ldconfig_scriptlets
- BR: gcc-c++

* Wed Feb 07 2018 Fedora Release Engineering <releng@fedoraproject.org> - 1.2.1-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_28_Mass_Rebuild

* Sat Jan 27 2018 Rex Dieter <rdieter@fedoraproject.org> - 1.2.1-1
- 1.2.1, update url

* Thu Jan 11 2018 Igor Gnatenko <ignatenkobrain@fedoraproject.org>
- Remove obsolete scriptlets

* Sat Oct 07 2017 Rex Dieter <rdieter@fedoraproject.org> - 1.2-2
- fix typo in Obsoletes

* Fri Oct 06 2017 Rex Dieter <rdieter@fedoraproject.org> - 1.2-1
- 1.2

* Thu Aug 03 2017 Fedora Release Engineering <releng@fedoraproject.org> - 1.0.3-4
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Binutils_Mass_Rebuild

* Wed Jul 26 2017 Fedora Release Engineering <releng@fedoraproject.org> - 1.0.3-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Mass_Rebuild

* Fri Feb 10 2017 Fedora Release Engineering <releng@fedoraproject.org> - 1.0.3-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_26_Mass_Rebuild

* Tue Dec 27 2016 Rex Dieter <rdieter@math.unl.edu> - 1.0.3-1
- kdeconnect-1.0.3 (#1408570), drop kde4 (compat) kioslave 

* Wed Oct 05 2016 Rex Dieter <rdieter@fedoraproject.org> - 1.0.1-2
- fix _with_kde4 conditional

* Wed Oct 05 2016 Rex Dieter <rdieter@fedoraproject.org> - 1.0.1-1.1
- -kde4-libs: inflate soname to avoid collisions (#1374869)
- fix Obsoletes

* Wed Sep 21 2016 Rex Dieter <rdieter@fedoraproject.org> - 1.0.1-1
- 1.0.1

* Thu Sep 01 2016 Rex Dieter <rdieter@fedoraproject.org> 1.0-2
- update URL (#1325177)

* Sun Aug 28 2016 Rex Dieter <rdieter@fedoraproject.org> - 1.0-1
- kde-connect-1.0

* Sun Jun 05 2016 Rex Dieter <rdieter@fedoraproject.org> - 0.9-7
- prep git snapshot (for 1.0 compatibility), but don't use yet
- kdeconnectd subpkg (#1324214)
- kdeconnectd does not autostart on MATE (#1296523)

* Fri Feb 19 2016 Rex Dieter <rdieter@fedoraproject.org> 0.9-6
- drop kde4 support (f24+)

* Thu Feb 04 2016 Fedora Release Engineering <releng@fedoraproject.org> - 0.9-5
- Rebuilt for https://fedoraproject.org/wiki/Fedora_24_Mass_Rebuild

* Sat Jan 09 2016 Rex Dieter <rdieter@fedoraproject.org> 0.9-4
- kde-connect-0.9g

* Tue Dec 01 2015 Rex Dieter <rdieter@fedoraproject.org> 0.9-3
- make plasma-workspace a soft dependency (#1286431)

* Thu Nov 19 2015 Rex Dieter <rdieter@fedoraproject.org> 0.9-2
- respin kde-connect-0.9f, includes translations

* Mon Nov 16 2015 Rex Dieter <rdieter@fedoraproject.org> 0.9-1
- kde-connect-0.9 (missing translations?)

* Tue Nov 10 2015 Rex Dieter <rdieter@fedoraproject.org> 0.8-10
- Requires: plasma-workspace kde-cli-tools (#1280078)

* Wed Sep 23 2015 Rex Dieter <rdieter@fedoraproject.org> 0.8-9
- include kde-connect firewalld service (#1115547)

* Thu Aug 27 2015 Helio Chissini de Castro <helio@kde.org> - 0.8-8
- Added buildreq for specific qca version that has proper headers

* Wed Aug 26 2015 Rex Dieter <rdieter@fedoraproject.org> - 0.8-7
- fresh snapshot, use releaseme to include translations
- tighten subpkg deps
- .spec cosmetics

* Fri Aug 07 2015 Helio Chissini de Castro <helio@kde.org> - 0.8-6
- Added missing requires, qca-qt5-ossl. Thanks to Stefano Cavallari <spiky.kiwi@gmail.com>

* Wed Aug 05 2015 Helio Chissini de Castro <helio@kde.org> - 0.8-5
- Update the KF5 snapshot.
- Added b revision for 0.8 KDE 4
- Added requires for fuse-ssh ( thanks to Sudhir Khanger )

* Wed Jun 17 2015 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 0.8-4
- Rebuilt for https://fedoraproject.org/wiki/Fedora_23_Mass_Rebuild

* Fri May 01 2015 Helio Chissini de Castro <helio@kde.org> - 0.8.3
- Added some missing buildrequires for rawhide

* Mon Apr 20 2015 Helio Chissini de Castro <helio@kde.org> - 0.8-2
- KDE Connect KF5 snapshot based on 0.8 and kioslave for KDE 4

* Sun Feb 22 2015 Rex Dieter <rdieter@fedoraproject.org> 0.8-1
- KDE Connect 0.8 available (#1195011)
- use %%{?_kde_runtime_requires} (instead of %%_kf5_version macro)

* Thu Oct 16 2014 Rex Dieter <rdieter@fedoraproject.org> - 0.7.3-1
- kde-connect-0.7.3
- BR: libfakekey-devel (and switch other BR's to pkgconfig style)

* Sat Aug 16 2014 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 0.7.2-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_21_22_Mass_Rebuild

* Sun Jul 06 2014 Rex Dieter <rdieter@fedoraproject.org> 0.7.2-1
- kde-connect-0.7.2 (#1116448)

* Sun Jun 29 2014 Rex Dieter <rdieter@fedoraproject.org> 0.7.1-1
- 0.7.1

* Sat Jun 28 2014 Rex Dieter <rdieter@fedoraproject.org> - 0.7-1
- kde-connect-0.7 (#1114196)
- Requires: fuse-sshfs (#1114197)
- Requires: qca-ossl
- -libs, -devel subpkgs

* Sun Jun 08 2014 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 0.6-0.3.20140305git52901898
- Rebuilt for https://fedoraproject.org/wiki/Fedora_21_Mass_Rebuild

* Wed Mar 05 2014 Martin Briza <mbriza@redhat.com> - 0.6-0.2.20140305git52901898
- Include the translations too

* Wed Mar 05 2014 Martin Briza <mbriza@redhat.com> - 0.6-0.1.20140305git52901898
- Updated to the latest upstream git to match the mobile app release

* Mon Feb 24 2014 Martin Briza <mbriza@redhat.com> - 0.5-1
- New release

* Thu Jan 02 2014 Martin Briza <mbriza@redhat.com> - 0.4.2-1
- Initial package
