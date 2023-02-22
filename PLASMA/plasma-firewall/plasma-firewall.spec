%global kf5_version_min 5.78

# Disable ufw for RHEL
%if 0%{?rhel}
%bcond_with ufw
%else
%bcond_without ufw
%endif

Name:    plasma-firewall
Version: 5.27.1
Release: 1%{?dist}
Summary: Control Panel for your system firewall

License: BSD
URL:     https://invent.kde.org/plasma/%{name}

%global verdir %(echo %{version} | cut -d. -f1-3)
%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global majmin_ver %(echo %{version} | cut -d. -f1,2).50
%global stable unstable
%else
%global majmin_ver %(echo %{version} | cut -d. -f1,2)
%global stable stable
%endif
Source0: http://download.kde.org/%{stable}/plasma/%{version}/%{name}-%{version}.tar.xz
Source1: http://download.kde.org/%{stable}/plasma/%{version}/%{name}-%{version}.tar.xz.sig
Source2: https://jriddell.org/esk-riddell.gpg

BuildRequires: gnupg2
BuildRequires: gcc-c++
BuildRequires: make
BuildRequires: cmake

BuildRequires: extra-cmake-modules >= %{kf5_version_min}
BuildRequires: kf5-rpm-macros
BuildRequires: kf5-kcmutils-devel >= %{kf5_version_min}
BuildRequires: kf5-kcoreaddons-devel >= %{kf5_version_min}
BuildRequires: kf5-kdeclarative-devel >= %{kf5_version_min}
BuildRequires: kf5-ki18n-devel >= %{kf5_version_min}
BuildRequires: kf5-plasma-devel >= %{kf5_version_min}

BuildRequires: qt5-qtbase-devel
BuildRequires: qt5-qtx11extras-devel

# Owns KCM directories
Requires: kf5-kcmutils%{?_isa} >= %{kf5_version_min}

Requires: %{name}-backend = %{version}-%{release}
Suggests: %{name}-firewalld

%description
%{summary}.

%package firewalld
Summary: FirewallD backend for Plasma Firewall
Requires: %{name}%{?_isa} = %{version}-%{release}
Provides: %{name}-backend = %{version}-%{release}
Conflicts: %{name}-backend
Requires: firewalld

%description firewalld
This package provides the backend code for Plasma Firewall
to interface with FirewallD.

%if %{with ufw}
%package ufw
Summary: UFW backend for Plasma Firewall
Requires: %{name}%{?_isa} = %{version}-%{release}
Provides: %{name}-backend = %{version}-%{release}
Conflicts: %{name}-backend
Requires: ufw
# For dbus directories
Requires: dbus-common
# For polkit directories
Requires: polkit

%description ufw
This package provides the backend code for Plasma Firewall
to interface with the Uncomplicated Firewall (UFW).
%endif

%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -n %{name}-%{version} -p1

%build
%cmake_kf5
%cmake_build

%install
%cmake_install

%find_lang %{name} --all-name --with-html

%if ! %{with ufw}
# Delete ufw stuff when we don't need it
rm -rfv %{buildroot}%{_qt5_plugindir}/kf5/plasma_firewall/ufwbackend.so
rm -rfv %{buildroot}%{_libexecdir}/kde_ufw_plugin_helper.py
rm -rfv %{buildroot}%{_datadir}/dbus-1/system-services/org.kde.ufw.service
rm -rfv %{buildroot}%{_datadir}/dbus-1/system.d/org.kde.ufw.conf
rm -rfv %{buildroot}%{_datadir}/kcm_ufw/defaults
rm -rfv %{buildroot}%{_datadir}/polkit-1/actions/org.kde.ufw.policy
%endif

%files -f %{name}.lang
%license LICENSES/*.txt
%{_libdir}/libkcm_firewall_core.so
%{_qt5_plugindir}/plasma/kcms/systemsettings/kcm_firewall.so
%dir %{_qt5_plugindir}/kf5/plasma_firewall
%{_datadir}/kpackage/kcms/kcm_firewall
%{_datadir}/applications/kcm_firewall.desktop
%{_metainfodir}/org.kde.plasma.firewall.metainfo.xml

%files firewalld
%{_qt5_plugindir}/kf5/plasma_firewall/firewalldbackend.so

%if %{with ufw}
%files ufw
%{_qt5_plugindir}/kf5/plasma_firewall/ufwbackend.so
%{_libexecdir}/kde_ufw_plugin_helper.py
%{_kf5_libexecdir}/kauth/kde_ufw_plugin_helper
%{_datadir}/dbus-1/system-services/org.kde.ufw.service
%{_datadir}/dbus-1/system.d/org.kde.ufw.conf
%dir %{_datadir}/kcm_ufw
%{_datadir}/kcm_ufw/defaults
%{_datadir}/polkit-1/actions/org.kde.ufw.policy
%endif


%changelog
* Tue Feb 21 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.27.1-1
- 5.27.1

* Tue Feb 14 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.27.0-1
- 5.27.0

* Wed Jan 04 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.26.5-1
- 5.26.5

* Tue Nov 29 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.26.4-1
- 5.26.4

* Tue Nov 08 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.26.3-1
- 5.26.3

* Wed Oct 26 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.26.2-1
- 5.26.2

* Tue Oct 18 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.26.1-1
- 5.26.1

* Tue Oct 11 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.26.0-1
- 5.26.0

* Tue Sep 06 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.25.5-1
- 5.25.5

* Tue Aug 02 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.25.4-1
- 5.25.4

* Wed Jul 13 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.25.3-1
- 5.25.3

* Tue Jun 28 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.25.2-1
- 5.25.2

* Tue Jun 21 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.25.1-1
- 5.25.1

* Tue Jun 14 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.25.0-1
- 5.25.0

* Tue May 03 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.24.5-1
- 5.24.5

* Tue Mar 29 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.24.4-1
- 5.24.4

* Tue Mar 08 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.24.3-1
- 5.24.3

* Tue Feb 22 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.24.2-1
- 5.24.2

* Tue Feb 15 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.24.1-1
- 5.24.1

* Tue Feb 08 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.24.0-1
- 5.24.0

* Tue Jan 04 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.23.5-1
- 5.23.5

* Tue Nov 30 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.23.4-1
- 5.23.4

* Tue Nov 09 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.23.3-1
- 5.23.3

* Tue Oct 26 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.23.2-1
- 5.23.2

* Tue Oct 19 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.23.1-1
- 5.23.1

* Thu Oct 14 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.23.0-1
- 5.23.0

* Tue Aug 31 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.22.5-1
- 5.22.5

* Tue Jul 27 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.22.4-1
- 5.22.4

* Thu Jul 08 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.22.3-1
- 5.22.3

* Wed Jun 23 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.22.2.1-1
- 5.22.2.1

* Tue Jun 22 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.22.2-1
- 5.22.2

* Tue Jun 15 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.22.1-1
- 5.22.1

* Tue Jun 08 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.22.0-1
- 5.22.0

* Tue May 04 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.21.5-1
- 5.21.5

* Tue Apr 06 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.21.4-1
- 5.21.4

* Tue Mar 16 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.21.3-1
- 5.21.3

* Wed Mar 03 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.21.2-1
- 5.21.2

* Tue Feb 23 13:50:07 MSK 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.21.1-1
- 5.21.1

* Thu Feb 11 2021 Jan Grulich <jgrulich@redhat.com> - 5.21.0-1
- 5.21.0

* Fri Jan 29 2021 Neal Gompa <ngompa13@gmail.com> - 5.20.90-4
- Subpackage firewall backends
- Ensure directory ownership is correct

* Thu Jan 28 2021 Marc Deop <marcdeop@fedoraproject.org> - 5.20.90-3
- Add BuildRequires for gcc-c++, make and cmake

* Wed Jan 27 2021 Marc Deop <marcdeop@fedoraproject.org> - 5.20.90-2
- Fix License

* Wed Jan 27 2021 Marc Deop <marcdeop@fedoraproject.org> - 5.20.90-1
- 5.20.90 (beta)
