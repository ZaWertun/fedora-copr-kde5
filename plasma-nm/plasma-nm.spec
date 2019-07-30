
%global kf5_version 5.42.0

Name:    plasma-nm
Summary: Plasma for managing network connections
Version: 5.16.4
Release: 1%{?dist}

License: LGPLv2+ and GPLv2+
URL:     https://cgit.kde.org/%{name}.git

%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0: http://download.kde.org/%{stable}/plasma/%{version}/%{name}-%{version}.tar.xz

## upstream patches

# master branch

# filter plugin provides
%global __provides_exclude_from ^(%{_kf5_qtplugindir}/.*\\.so)$

BuildRequires:  gettext

BuildRequires:  kf5-rpm-macros
BuildRequires:  extra-cmake-modules
BuildRequires:  qt5-qtbase-devel
BuildRequires:  qt5-qtdeclarative-devel
BuildRequires:  qt5-qttools-devel
BuildRequires:  qt5-qttools-static

BuildRequires:  kf5-ki18n-devel
BuildRequires:  kf5-kwindowsystem-devel
BuildRequires:  kf5-kservice-devel
BuildRequires:  kf5-kcompletion-devel
BuildRequires:  kf5-kwidgetsaddons-devel
BuildRequires:  kf5-kio-devel
BuildRequires:  kf5-kcoreaddons-devel
BuildRequires:  kf5-kwallet-devel
BuildRequires:  kf5-kitemviews-devel
BuildRequires:  kf5-kxmlgui-devel
BuildRequires:  kf5-kconfigwidgets-devel
BuildRequires:  kf5-kiconthemes-devel
BuildRequires:  kf5-solid-devel
BuildRequires:  kf5-kdbusaddons-devel
BuildRequires:  kf5-knotifications-devel
BuildRequires:  kf5-plasma-devel
BuildRequires:  kf5-kdeclarative-devel
BuildRequires:  kf5-kinit-devel
BuildRequires:  kf5-kdelibs4support-devel
BuildRequires:  kf5-networkmanager-qt-devel >= %{kf5_version}
BuildRequires:  kf5-modemmanager-qt-devel >= %{kf5_version}

%if ! 0%{?bootstrap}
BuildRequires:  pkgconfig(ModemManager) >= 1.0.0
%endif
BuildRequires:  pkgconfig(libnm) >= 1.0.0
%if 0%{?fedora} || 0%{?epel}
BuildRequires:  pkgconfig(openconnect) >= 4.00
%endif

BuildRequires:  qca-qt5-devel

Requires:       NetworkManager >= 1.0.0

Obsoletes:      kde-plasma-networkmanagement < 1:0.9.1.0
Obsoletes:      kde-plasma-networkmanagement-libs < 1:0.9.1.0
Obsoletes:      kde-plasma-nm < 5.0.0-1
Provides:       kde-plasma-nm = %{version}-%{release}

%description
Plasma applet and editor for managing your network connections in KDE 4 using
the default NetworkManager service.

# Required for properly working GMS/CDMA connections
%package        mobile
Summary:        Mobile support for %{name}
Requires:       ModemManager
%if ! 0%{?bootstrap}
BuildRequires:  pkgconfig(mobile-broadband-provider-info)
%endif
Requires:       mobile-broadband-provider-info
Requires:       kf5-modemmanager-qt >= 5.0.0-1
Obsoletes:      kde-plasma-networkmanagement-mobile < 1:0.9.1.0
Obsoletes:      kde-plasma-nm-mobile < 5.0.0-1
Provides:       kde-plasma-nm-mobile = %{version}-%{release}
%description    mobile
%{summary}.

%if 0%{?fedora} || 0%{?epel}
%package        openvpn
Summary:        OpenVPN support for %{name}
Requires:       %{name}%{?_isa} = %{version}-%{release}
Requires:       NetworkManager-openvpn
Obsoletes:      kde-plasma-networkmanagement-openvpn < 1:0.9.1.0
Obsoletes:      kde-plasma-nm-openvpn < 5.0.0-1
Provides:       kde-plasma-nm-openvpn = %{version}-%{release}
%description    openvpn
%{summary}.

%package        vpnc
Summary:        Vpnc support for %{name}
Requires:       %{name}%{?_isa} = %{version}-%{release}
Requires:       NetworkManager-vpnc
Obsoletes:      kde-plasma-networkmanagement-vpnc < 1:0.9.1.0
Obsoletes:      kde-plasma-nm-vpnc < 5.0.0-1
Provides:       kde-plasma-nm-vpnc = %{version}-%{release}
%description    vpnc
%{summary}.

%package        openconnect
Summary:        OpenConnect support for %{name}
Requires:       %{name}%{?_isa} = %{version}-%{release}
Requires:       NetworkManager-openconnect
Obsoletes:      kde-plasma-networkmanagement-openconnect < 1:0.9.1.0
Obsoletes:      kde-plasma-nm-openconnect < 5.0.0-1
Provides:       kde-plasma-nm-openconnect = %{version}-%{release}
%description    openconnect
%{summary}.

%package        openswan
Summary:        Openswan support for %{name}
Requires:       %{name}%{?_isa} = %{version}-%{release}
Requires:       NetworkManager-openswan
Obsoletes:      kde-plasma-nm-openswan < 5.0.0-1
Provides:       kde-plasma-nm-openswan = %{version}-%{release}
%description    openswan
%{summary}.

%package        strongswan
Summary:        Strongswan support for %{name}
Requires:       %{name}%{?_isa} = %{version}-%{release}
Requires:       strongswan
Obsoletes:      kde-plasma-nm-strongswan < 5.0.0-1
Provides:       kde-plasma-nm-strongswan = %{version}-%{release}
%description    strongswan
%{summary}.

%package        l2tp
Summary:        L2TP support for %{name}
Requires:       %{name}%{?_isa} = %{version}-%{release}
Requires:       NetworkManager-l2tp
Obsoletes:      kde-plasma-nm-l2tp < 5.0.0-1
Provides:       kde-plasma-nm-l2tp = %{version}-%{release}
%description    l2tp
%{summary}.

%package        pptp
Summary:        PPTP support for %{name}
Requires:       %{name}%{?_isa} = %{version}-%{release}
Requires:       NetworkManager-pptp
Obsoletes:      kde-plasma-networkmanagement-pptp < 1:0.9.1.0
Obsoletes:      kde-plasma-nm-pptp < 5.0.0-1
Provides:       kde-plasma-nm-pptp = %{version}-%{release}
%description    pptp
%{summary}.

%package        ssh
Summary:        SSH suppor for %{name}
Requires:       %{name}%{?_isa} = %{version}-%{release}
Requires:       NetworkManager-ssh
%description    ssh
%{summary}.

%package        sstp
Summary:        SSTP support for %{name}
Requires:       %{name}%{?_isa} = %{version}-%{release}
%description    sstp
%{summary}.

%package        iodine
Summary:        Iodine support for %{name}
Requires:       %{name}%{?_isa} = %{version}-%{release}
Requires:       NetworkManager-iodine
%description    iodine
%{summary}.

%package        fortisslvpn
Summary:        Fortigate SSL VPN support for %{name}
Requires:       %{name}%{?_isa} = %{version}-%{release}
Requires:       NetworkManager-fortisslvpn
%description    fortisslvpn
%{summary}.

%package        wireguard
Summary:        Wireguard support for %{name}
Requires:       %{name}%{?_isa} = %{version}-%{release}
Requires:       NetworkManager-wireguard
%description    wireguard
%{summary}.
%endif


%prep
%autosetup -p1 -n %{name}-%{version}


%build
mkdir %{_target_platform}
pushd %{_target_platform}
%{cmake_kf5} ..
popd

%make_build -C %{_target_platform}


%install
make install/fast  DESTDIR=%{buildroot} -C %{_target_platform}

%find_lang kcm_mobile_broadband
%find_lang kcm_mobile_wifi
%find_lang plasma_applet_org.kde.plasma.networkmanagement
%find_lang plasmanetworkmanagement-kded
%find_lang plasmanetworkmanagement-kcm
%find_lang plasmanetworkmanagement-libs
%find_lang plasmanetworkmanagement_vpncui
%find_lang plasmanetworkmanagement_openvpnui
%find_lang plasmanetworkmanagement_openconnectui
%find_lang plasmanetworkmanagement_openswanui
%find_lang plasmanetworkmanagement_strongswanui
%find_lang plasmanetworkmanagement_l2tpui
%find_lang plasmanetworkmanagement_pptpui
%find_lang plasmanetworkmanagement_sshui
%find_lang plasmanetworkmanagement_sstpui
%find_lang plasmanetworkmanagement_iodineui
%find_lang plasmanetworkmanagement_fortisslvpnui


%ldconfig_scriptlets

%files -f plasma_applet_org.kde.plasma.networkmanagement.lang -f plasmanetworkmanagement-kded.lang -f plasmanetworkmanagement-libs.lang -f plasmanetworkmanagement-kcm.lang -f kcm_mobile_broadband.lang -f kcm_mobile_wifi.lang
%{_libdir}/libplasmanm_internal.so
%{_libdir}/libplasmanm_editor.so
# plasma-nm applet
%{_qt5_qmldir}/org/kde/plasma/networkmanagement/
%{_kf5_datadir}/plasma/plasmoids/org.kde.plasma.networkmanagement/
%{_kf5_datadir}/kservices5/plasma-applet-org.kde.plasma.networkmanagement.desktop
#{_datadir}/plasma/updates/*.js
# plasma-nm notifications
%{_kf5_datadir}/knotifications5/networkmanagement.notifyrc
# plasma-nm kded
%{_kf5_plugindir}/kded/networkmanagement.so
# plasma-nm other
%{_kf5_datadir}/kservicetypes5/plasma-networkmanagement-vpnuiplugin.desktop
# appdata
%{_kf5_metainfodir}/org.kde.plasma.networkmanagement.appdata.xml
# kcm
%{_qt5_plugindir}/kcm_networkmanagement.so
%{_datadir}/kcm_networkmanagement/qml/
%{_kf5_datadir}/kservices5/kcm_networkmanagement.desktop

%files mobile

%if 0%{?fedora} || 0%{?epel}
%files openvpn -f plasmanetworkmanagement_openvpnui.lang
%{_kf5_qtplugindir}/libplasmanetworkmanagement_openvpnui.so
%{_kf5_datadir}/kservices5/plasmanetworkmanagement_openvpnui.desktop

%files vpnc -f plasmanetworkmanagement_vpncui.lang
%{_kf5_qtplugindir}/libplasmanetworkmanagement_vpncui.so
%{_kf5_datadir}/kservices5/plasmanetworkmanagement_vpncui.desktop

%files openconnect -f plasmanetworkmanagement_openconnectui.lang
%{_kf5_qtplugindir}/libplasmanetworkmanagement_openconnectui.so
%{_kf5_datadir}/kservices5/plasmanetworkmanagement_openconnect*.desktop
%{_kf5_datadir}/kservices5/plasmanetworkmanagement_openconnect_juniperui.desktop

%files openswan -f plasmanetworkmanagement_openswanui.lang
%{_kf5_qtplugindir}/libplasmanetworkmanagement_openswanui.so
%{_kf5_datadir}/kservices5/plasmanetworkmanagement_openswanui.desktop

%files strongswan -f plasmanetworkmanagement_strongswanui.lang
%{_kf5_qtplugindir}/libplasmanetworkmanagement_strongswanui.so
%{_kf5_datadir}/kservices5/plasmanetworkmanagement_strongswanui.desktop

%files l2tp -f plasmanetworkmanagement_l2tpui.lang
%{_kf5_qtplugindir}/libplasmanetworkmanagement_l2tpui.so
%{_kf5_datadir}/kservices5/plasmanetworkmanagement_l2tpui.desktop

%files pptp -f plasmanetworkmanagement_pptpui.lang
%{_kf5_qtplugindir}/libplasmanetworkmanagement_pptpui.so
%{_kf5_datadir}/kservices5/plasmanetworkmanagement_pptpui.desktop

%files ssh -f plasmanetworkmanagement_sshui.lang
%{_kf5_qtplugindir}/libplasmanetworkmanagement_sshui.so
%{_kf5_datadir}/kservices5/plasmanetworkmanagement_sshui.desktop

%files sstp -f plasmanetworkmanagement_sstpui.lang
%{_kf5_qtplugindir}/libplasmanetworkmanagement_sstpui.so
%{_kf5_datadir}/kservices5/plasmanetworkmanagement_sstpui.desktop

%files iodine -f plasmanetworkmanagement_iodineui.lang
%{_kf5_qtplugindir}/libplasmanetworkmanagement_iodineui.so
%{_kf5_datadir}/kservices5/plasmanetworkmanagement_iodineui.desktop

%files fortisslvpn -f plasmanetworkmanagement_fortisslvpnui.lang
%{_kf5_qtplugindir}/libplasmanetworkmanagement_fortisslvpnui.so
%{_kf5_datadir}/kservices5/plasmanetworkmanagement_fortisslvpnui.desktop
%endif


%changelog
* Tue Jul 30 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.16.4-1
- 5.16.4

* Tue Jul 09 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.16.3-1
- 5.16.3

* Tue Jun 25 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.16.2-1
- 5.16.2

* Tue Jun 18 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.16.1-1
- 5.16.1

* Tue Jun 11 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.16.0-1
- 5.16.0

* Tue May 07 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.15.5-1
- 5.15.5

* Sun Apr 28 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.15.4-1
- 5.15.4

* Tue Feb 19 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.14.5-1
- 5.14.5

* Tue Nov 27 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.14.4-1
- 5.14.4

* Thu Nov 08 2018 Martin Kyral <martin.kyral@gmail.com> - 5.14.3-1
- 5.14.3

* Wed Oct 24 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.14.2-1
- 5.14.2

* Tue Oct 16 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.14.1-1
- 5.14.1

* Sat Oct 06 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.14.0-1
- 5.14.0

* Fri Sep 14 2018 Martin Kyral <martin.kyral@gmail.com> - 5.13.90-1
- 5.13.90

* Tue Sep 04 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.13.5-1
- 5.13.5

* Thu Aug 02 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.13.4-1
- 5.13.4

* Fri Jul 20 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.13.3-2
- use %%_qt5_qmldir

* Wed Jul 11 2018 Martin Kyral <martin.kyral@gmail.com> - 5.13.3-1
- 5.13.3

* Mon Jul 09 2018 Martin Kyral <martin.kyral@gmail.com> - 5.13.2-1
- 5.13.2

* Tue Jun 19 2018 Martin Kyral <martin.kyral@gmail.com> - 5.13.1-1
- 5.13.1

* Sat Jun 09 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.13.0-1
- 5.13.0

* Fri May 18 2018 Martin Kyral <martin.kyral@gmail.com> - 5.12.90-1
- 5.12.90

* Sun May 06 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.12.5-2
- pull in upstream fixes
- cleanup

* Tue May 01 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.12.5-1
- 5.12.5

* Tue Mar 27 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.12.4-1
- 5.12.4

* Tue Mar 06 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.12.3-1
- 5.12.3

* Wed Feb 21 2018 Jan Grulich <jgrulich@redhat.com> - 5.12.2-1
- 5.12.2

* Tue Feb 13 2018 Jan Grulich <jgrulich@redhat.com> - 5.12.1-1
- 5.12.1

* Fri Feb 09 2018 Fedora Release Engineering <releng@fedoraproject.org> - 5.12.0-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_28_Mass_Rebuild

* Fri Feb 02 2018 Jan Grulich <jgrulich@redhat.com> - 5.12.0-1
- 5.12.0

* Mon Jan 15 2018 Jan Grulich <jgrulich@redhat.com> - 5.11.95-1
- 5.11.95

* Tue Jan 02 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.11.5-1
- 5.11.5

* Thu Nov 30 2017 Martin Kyral <martin.kyral@gmail.com> - 5.11.4-1
- 5.11.4

* Wed Nov 08 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.11.3-1
- 5.11.3

* Wed Oct 25 2017 Martin Kyral <martin.kyral@gmail.com> - 5.11.2-1
- 5.11.2

* Tue Oct 17 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.11.1-1
- 5.11.1

* Wed Oct 11 2017 Martin Kyral <martin.kyral@gmail.com> - 5.11.0-1
- 5.11.0

* Thu Aug 24 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.10.5-1
- 5.10.5

* Thu Aug 03 2017 Fedora Release Engineering <releng@fedoraproject.org> - 5.10.4-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Binutils_Mass_Rebuild

* Thu Jul 27 2017 Fedora Release Engineering <releng@fedoraproject.org> - 5.10.4-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Mass_Rebuild

* Fri Jul 21 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.10.4-1
- 5.10.4

* Tue Jun 27 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.10.3-1
- 5.10.3

* Thu Jun 15 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.10.2-1
- 5.10.2

* Tue Jun 06 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.10.1-1
- 5.10.1

* Wed May 31 2017 Jan Grulich <jgrulich@redhat.com> - 5.10.0-1
- 5.10.0

* Wed Apr 26 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.9.5-1
- 5.9.5

* Mon Apr 10 2017 Jan Grulich <jgrulich@redhat.com> - 5.9.4-2
- Set auto-negotiate to true for new wired connections
  Resolves: bz#1440583

* Thu Mar 23 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.9.4-1
- 5.9.4

* Sat Mar 04 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.9.3-2
- rebuild

* Wed Mar 01 2017 Jan Grulich <jgrulich@redhat.com> - 5.9.3-1
- 5.9.3

* Tue Feb 21 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.8.6-1
- 5.8.6

* Sat Feb 11 2017 Fedora Release Engineering <releng@fedoraproject.org> - 5.8.5-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_26_Mass_Rebuild

* Mon Jan 02 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.8.5-2
- filter plugin provides

* Wed Dec 28 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.8.5-1
- 5.8.5

* Tue Nov 22 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.8.4-1
- 5.8.4

* Tue Nov 01 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.8.3-1
- 5.8.3

* Tue Oct 18 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.8.2-1
- 5.8.2

* Tue Oct 11 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.8.1-1
- 5.8.1

* Thu Sep 29 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.8.0-1
- 5.8.0

* Thu Sep 22 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.7.95-1
- 5.7.95

* Tue Sep 13 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.7.5-1
- 5.7.5

* Sun Sep 11 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.7.4-2
- BR: mobile-broadband-provider-info (to make optional dep cmake report happy)

* Tue Aug 23 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.7.4-1
- 5.7.4

* Wed Aug 10 2016 Jan Grulich <jgrulich@redhat.com> - 5.7.3-2
- Add more choices to --comp-lzo option

* Tue Aug 02 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.7.3-1
- 5.7.3

* Tue Jul 19 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.7.2-1
- 5.7.2

* Tue Jul 12 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.7.1-1
- 5.7.1

* Thu Jun 30 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.7.0-1
- 5.7.0

* Sat Jun 25 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.6.95-1
- 5.6.95

* Thu Jun 16 2016 Rex Dieter <rdieter@fedoraproject.org> 5.6.5-2
- backport some master/ branch fixes, including plasma crasher (kde#346118,#1259472)

* Tue Jun 14 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.6.5-1
- 5.6.5

* Sat May 14 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.6.4-1
- 5.6.4

* Thu May 12 2016 Jan Grulich <jgrulich@redhat.com> - 5.6.3-2
- Filter out tun connections (changed by NM 1.2.0)

* Tue Apr 19 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.6.3-1
- 5.6.3

* Sat Apr 09 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.6.2-1
- 5.6.2

* Fri Apr 08 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.6.1-1
- 5.6.1

* Tue Mar 01 2016 Daniel Vrátil <dvratil@fedoraproject.org> - 5.5.5-1
- Plasma 5.5.5

* Thu Feb 04 2016 Fedora Release Engineering <releng@fedoraproject.org> - 5.5.4-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_24_Mass_Rebuild

* Wed Jan 27 2016 Daniel Vrátil <dvratil@fedoraproject.org> - 5.5.4-1
- Plasma 5.5.4

* Thu Jan 07 2016 Daniel Vrátil <dvratil@fedoraproject.org> - 5.5.3-1
- Plasma 5.5.3

* Thu Dec 31 2015 Rex Dieter <rdieter@fedoraproject.org> - 5.5.2-1
- 5.5.2

* Fri Dec 18 2015 Daniel Vrátil <dvratil@fedoraproject.org> - 5.5.1-1
- Plasma 5.5.1

* Mon Dec 14 2015 Jan Grulich <jgrulich@redhat.com> - 5.5.0-2
- Fix openconnect dialog layout (kdebz#356587, kdebz#356622)

* Thu Dec 03 2015 Daniel Vrátil <dvratil@fedoraproject.org> - 5.5.0-1
- Plasma 5.5.0

* Wed Dec 02 2015 Jan Grulich <jgrulich@redhat.com> - 5.4.95-2
- Fix openconnect failure when accepting certificate
  Resolves: bz#1285482

* Wed Nov 25 2015 Daniel Vrátil <dvratil@fedoraproject.org> - 5.4.95-1
- Plasma 5.4.95

* Thu Nov 19 2015 Rex Dieter <rdieter@fedoraproject.org> - 5.4.3-2
- drop updates script, it's (probably) no longer needed, and doesn't work where it is anyway

* Thu Nov 05 2015 Daniel Vrátil <dvratil@redhat.com> - 5.4.3-1
- Plasma 5.4.3

* Tue Oct 27 2015 Rex Dieter <rdieter@fedoraproject.org> 5.4.2-2
- backport 'make bluez calls async' patch

* Thu Oct 01 2015 Rex Dieter <rdieter@fedoraproject.org> - 5.4.2-1
- 5.4.2

* Wed Sep 09 2015 Rex Dieter <rdieter@fedoraproject.org> - 5.4.1-1
- 5.4.1

* Fri Aug 21 2015 Daniel Vrátil <dvratil@redhat.com> - 5.4.0-1
- Plasma 5.4.0

* Thu Aug 13 2015 Daniel Vrátil <dvratil@redhat.com> - 5.3.95-1
- Plasma 5.3.95

* Thu Jun 25 2015 Daniel Vrátil <dvratil@redhat.com> - 5.3.2-1
- Plasma 5.3.2

* Wed Jun 17 2015 Jan Grulich <jgrulich@redhat.com> - 5.3.1-3
- OpenVPN: Do not overwrite file dialog modes set by default

* Mon Jun 15 2015 Jan Grulich <jgrulich@redhat.com> - 5.3.1-2
- OpenVPN: Do not insert translated value for remote-cert-tls

* Tue May 26 2015 Daniel Vrátil <dvratil@redhat.com> - 5.3.1-1
- Plasma 5.3.1

* Mon Apr 27 2015 Daniel Vrátil <dvratil@redhat.com> - 5.3.0-1
- Plasma 5.3.0

* Wed Apr 22 2015 Daniel Vrátil <dvratil@redhat.com> - 5.2.95-1
- Plasma 5.2.95

* Thu Apr 09 2015 Jan Grulich <jgrulich@redhat.com> - 5.2.2-2
- Rebuild (kf5-modemmanager-qt)

* Fri Mar 20 2015 Daniel Vrátil <dvratil@redhat.com> - 5.2.2-1
- Plasma 5.2.2

* Fri Feb 27 2015 Daniel Vrátil <dvratil@redhat.com> - 5.2.1-2
- Rebuild (GCC 5)

* Tue Feb 24 2015 Daniel Vrátil <dvratil@redhat.com> - 5.2.1-1
- Plasma 5.2.1

* Wed Jan 28 2015 Daniel Vrátil <dvratil@redhat.com> - 5.2.0-3
- BR kf5-modemmanger-qt instead of kf5-libmm-qt

* Tue Jan 27 2015 Daniel Vrátil <dvratil@redhat.com> - 5.2.0-2
- fix dependencies on rawhide (with NM >= 1.0.0)

* Mon Jan 26 2015 Daniel Vrátil <dvratil@redhat.com> - 5.2.0-1
- Plasma 5.2.0

* Tue Jan 13 2015 Daniel Vrátil <dvratil@redhat.com> - 5.1.95-1.beta
- Plasma 5.1.95 Beta

* Wed Dec 17 2014 Daniel Vrátil <dvratil@redhat.com> - 5.1.2-2
- Plasma 5.1.2

* Fri Nov 07 2014 Daniel Vrátil <dvratil@redhat.com> - 5.1.1-1
- Plasma 5.1.1

* Tue Oct 14 2014 Daniel Vrátil <dvratil@redhat.com> - 5.1.0.1-1
- Plasma 5.1.0.1

* Thu Oct 09 2014 Daniel Vrátil <dvratil@redhat.com> - 5.1.0-1
- Plasma 5.1.0

* Tue Sep 16 2014 Daniel Vrátil <dvratil@redhat.com> - 5.0.2-1
- Plasma 5.0.2

* Sun Aug 10 2014 Daniel Vrátil <dvratil@redhat.com> - 5.0.1-1
- Plasma 5.0.1

* Thu Jul 17 2014 Daniel Vrátil <dvratil@redhat.com> - 5.0.0-1
- Plasma 5.0.0-1

* Thu Apr 03 2014 Daniel Vrátil <dvratil@redhat.com> - 4.96.0-1.20140515git9cc2530
- fork into kde5-plasma-nm
