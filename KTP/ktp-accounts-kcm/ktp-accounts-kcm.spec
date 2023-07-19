Name:    ktp-accounts-kcm
Summary: KDE Configuration Module for Telepathy Instant Messaging Accounts
Version: 23.04.3
Release: 1%{?dist}

License: LGPLv2+
URL:     https://cgit.kde.org/%{name}.git

%global majmin_ver %(echo %{version} | cut -d. -f1,2)
%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0: http://download.kde.org/stable/release-service/%{version}/src/%{name}-%{version}.tar.xz
Source1: http://download.kde.org/stable/release-service/%{version}/src/%{name}-%{version}.tar.xz.sig
Source2: gpgkey-D81C0CB38EB725EF6691C385BB463350D6EF31EF.gpg

# due to kaccounts-providers
# handled by qt5-srpm-macros, which defines %%qt5_qtwebengine_arches
%{?qt5_qtwebengine_arches:ExclusiveArch: %{qt5_qtwebengine_arches}}

BuildRequires:  gnupg2
BuildRequires:  extra-cmake-modules
BuildRequires:  intltool
BuildRequires:  kf5-kcmutils-devel
BuildRequires:  kf5-kcodecs-devel
BuildRequires:  kf5-kconfigwidgets-devel
BuildRequires:  kf5-kcoreaddons-devel
BuildRequires:  kf5-ki18n-devel
BuildRequires:  kf5-kiconthemes-devel
BuildRequires:  kf5-kio-devel
BuildRequires:  kf5-kitemviews-devel
BuildRequires:  kf5-kwidgetsaddons-devel
BuildRequires:  kf5-rpm-macros
BuildRequires:  libaccounts-glib-devel
BuildRequires:  qt5-qtbase-devel
BuildRequires:  telepathy-qt5-devel

BuildRequires:  kaccounts-integration-devel >= %{majmin_ver}
BuildRequires:  ktp-common-internals-devel >= %{majmin_ver}

Obsoletes:      telepathy-kde-accounts-kcm < 0.3.0
Provides:       telepathy-kde-accounts-kcm = %{version}-%{release}

Obsoletes:      telepathy-kde-accounts-kcm-plugins < 0.2.0
Provides:       telepathy-kde-accounts-kcm-plugins = %{version}-%{release}

Obsoletes:      telepathy-kde-accounts-kcm-devel < 0.2.0

# Headers are not installed since in 15.04.0-1
Obsoletes:      %{name}-devel < 15.04.0-1

# see patch1
Requires: kaccounts-integration%{?_isa} >= %{majmin_ver} 
# can't find a a better place to put this -- rex
Requires: signon-kwallet-extension >= %{majmin_ver}

# various protocol handlers
## msn (old)
#Requires: telepathy-butterfly
## xmpp/jabber
Requires:       telepathy-gabble
## msn (newer, libpurple)
## retired f31+
%if 0%{?fedora} < 31
Recommends:       telepathy-haze
%endif
## irc
#Requires: telepathy-idle
## audio calls
#Requires:       telepathy-rakia >= 0.7.4
## local xmpp
Requires:       telepathy-salut
## gadu/gadu
#Requires: telepathy-sunshine

%description
This is a KControl Module which handles adding/editing/removing Telepathy
Accounts. It interacts with any Telepathy Spec compliant AccountManager
to manipulate the accounts.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -p1


%build
%cmake_kf5
%cmake_build


%install
%cmake_install

%find_lang %{name} --all-name --with-html

## unpackaged files
rm -fv %{buildroot}%{_kf5_libdir}/libktpaccountskcminternal.so


%ldconfig_scriptlets

%files -f %{name}.lang
%license COPYING*
%doc README
%{_kf5_libdir}/libktpaccountskcminternal.so.*
%{_kf5_datadir}/kservicetypes5/ktpaccountskcminternal-accountuiplugin.desktop
%{_datadir}/telepathy/profiles/*.profile
%{_datadir}/accounts/services/kde/
%{_datadir}/accounts/providers/kde/
%{_kf5_qtplugindir}/*.so
%{_kf5_qtplugindir}/kaccounts/ui/ktpaccountskcm_plugin_kaccounts.so
%{_kf5_datadir}/kservices5/*.desktop


%changelog
* Tue Jul 18 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.04.3-1
- 23.04.3

* Thu Jun 08 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.04.2-1
- 23.04.2

* Thu May 11 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.04.1-1
- 23.04.1

* Fri Apr 21 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.04.0-1
- 23.04.0

* Thu Mar 02 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.12.3-1
- 22.12.3

* Thu Jan 05 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.12.1-1
- 22.12.1

