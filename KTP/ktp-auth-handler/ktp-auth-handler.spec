Name:    ktp-auth-handler
Summary: Provide UI/KWallet Integration
Version: 23.04.3
Release: 1%{?dist}

License: LGPLv2+
URL:     https://cgit.kde.org/%{name}.git

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
BuildRequires:  gettext
BuildRequires:  kf5-ki18n-devel
BuildRequires:  kf5-kio-devel
BuildRequires:  kf5-kwallet-devel
BuildRequires:  kf5-kwidgetsaddons-devel
BuildRequires:  kf5-rpm-macros
BuildRequires:  libaccounts-qt5-devel
BuildRequires:  qca-qt5-devel
BuildRequires:  qt5-qtbase-devel
BuildRequires:  signon-devel
BuildRequires:  telepathy-qt5-devel

%global majmin_ver %(echo %{version} | cut -d. -f1,2)
BuildRequires:  kaccounts-integration-devel
BuildRequires:  ktp-common-internals-devel >= %{majmin_ver}

# runtime dep checked during build, mostly harmless
BuildRequires:  qca-qt5-ossl
Requires:       qca-qt5-ossl

%description
Provide UI/KWallet Integration For Passwords and SSL Errors on Account Connect.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -p1


%build
%cmake_kf5
%cmake_build


%install
%cmake_install

%find_lang %{name} --all-name --with-html


%files -f %{name}.lang
%license COPYING*
%{_libexecdir}/ktp-auth-handler
%{_datadir}/dbus-1/services/org.freedesktop.Telepathy.Client.KTp.ConfAuthObserver.service
%{_datadir}/dbus-1/services/org.freedesktop.Telepathy.Client.KTp.SASLHandler.service
%{_datadir}/dbus-1/services/org.freedesktop.Telepathy.Client.KTp.TLSHandler.service
%{_datadir}/telepathy/clients/KTp.ConfAuthObserver.client
%{_datadir}/telepathy/clients/KTp.SASLHandler.client
%{_datadir}/telepathy/clients/KTp.TLSHandler.client


%changelog
* Thu Jul 06 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.04.3-1
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

