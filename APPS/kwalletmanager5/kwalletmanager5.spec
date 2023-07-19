%global  base_name kwalletmanager

%if 0%{?fedora} > 27
# replace kde4 kwalletmanager
%global kwalletmanager 1
%endif

Name:    kwalletmanager5
Summary: Manage KDE passwords
Version: 23.04.3
Release: 1%{?dist}

License: GPLv2+
URL:     https://cgit.kde.org/%{base_name}.git

%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0:        https://download.kde.org/%{stable}/release-service/%{version}/src/%{base_name}-%{version}.tar.xz
Source1:        https://download.kde.org/%{stable}/release-service/%{version}/src/%{base_name}-%{version}.tar.xz.sig
Source2:        gpgkey-D81C0CB38EB725EF6691C385BB463350D6EF31EF.gpg

## upstreamable patches
# better/sane defaults (no autoclose mostly)
Patch1: kwalletmanager-15.12.1-defaults.patch

BuildRequires:  gnupg2
BuildRequires:  desktop-file-utils
BuildRequires:  extra-cmake-modules
BuildRequires:  kf5-kauth-devel
BuildRequires:  kf5-kcmutils-devel
BuildRequires:  kf5-kconfig-devel
BuildRequires:  kf5-kconfigwidgets-devel
BuildRequires:  kf5-kcoreaddons-devel
BuildRequires:  kf5-kdbusaddons-devel
BuildRequires:  kf5-kdelibs4support-devel
BuildRequires:  kf5-kdoctools-devel
BuildRequires:  kf5-ki18n-devel
BuildRequires:  kf5-kservice-devel
BuildRequires:  kf5-kwallet-devel
BuildRequires:  kf5-kxmlgui-devel
BuildRequires:  kf5-rpm-macros
BuildRequires:  polkit-qt5-1-devel
BuildRequires:  qt5-qtbase-devel

# translations moved here
Conflicts: kde-l10n < 17.03

%if 0%{?kwalletmanager}
Obsoletes: kwalletmanager < 15.04.3-100
Provides:  kwalletmanager = %{version}-%{release}
%endif

%description
KDE Wallet Manager is a tool to manage the passwords on your KDE system.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -n %{base_name}-%{version} -p1


%build
%cmake_kf5
%cmake_build


%install
%cmake_install
%find_lang %{name} --all-name --with-html


%check
desktop-file-validate %{buildroot}%{_kf5_datadir}/applications/org.kde.%{name}.desktop


%files -f %{name}.lang
%license LICENSES/*.txt
%{_kf5_bindir}/kwalletmanager5
%{_kf5_libexecdir}/kauth/kcm_kwallet_helper5
%{_kf5_qtplugindir}/plasma/kcms/systemsettings_qwidgets/kcm_kwallet5.so
%{_datadir}/dbus-1/system-services/org.kde.kcontrol.kcmkwallet5.service
%{_datadir}/polkit-1/actions/org.kde.kcontrol.kcmkwallet5.policy
%{_kf5_datadir}/applications/kwalletmanager5-kwalletd.desktop
%{_kf5_datadir}/dbus-1/system.d/org.kde.kcontrol.kcmkwallet5.conf
%{_kf5_datadir}/dbus-1/services/org.kde.kwalletmanager5.service
%{_kf5_datadir}/icons/hicolor/*/apps/kwalletmanager*.*
%{_kf5_datadir}/icons/hicolor/*/actions/wallet-*
%{_kf5_datadir}/kservices5/*.desktop
%{_kf5_datadir}/applications/org.kde.kwalletmanager5.desktop
%{_kf5_datadir}/dbus-1/services/
%{_kf5_datadir}/qlogging-categories5/*.categories
%{_kf5_metainfodir}/org.kde.kwalletmanager5.appdata.xml


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

