%global framework kded

Name:    kf5-%{framework}
Version: 5.115.0
Release: 1%{?dist}
Summary: KDE Frameworks 5 Tier 3 addon with extensible daemon for system-level services

License: LGPLv2+
URL:     https://cgit.kde.org/%{framework}.git

%global majmin %(echo %{version} | cut -d. -f1-2)
%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0: http://download.kde.org/%{stable}/frameworks/%{majmin}/%{framework}-%{version}.tar.xz
Source1: http://download.kde.org/%{stable}/frameworks/%{majmin}/%{framework}-%{version}.tar.xz.sig
Source2: gpgkey-53E6B47B45CEA3E0D5B7457758D0EE648A48B3BB.gpg

BuildRequires:  gnupg2
BuildRequires:  extra-cmake-modules >= %{majmin}
BuildRequires:  kf5-kconfig-devel >= %{majmin}
BuildRequires:  kf5-kcoreaddons-devel >= %{majmin}
BuildRequires:  kf5-kcrash-devel >= %{majmin}
BuildRequires:  kf5-kdbusaddons-devel >= %{majmin}
BuildRequires:  kf5-kdoctools-devel >= %{majmin}
BuildRequires:  kf5-kservice-devel >= %{majmin}
BuildRequires:  kf5-rpm-macros
BuildRequires:  systemd-rpm-macros

BuildRequires:  qt5-qtbase-devel

%description
KDED stands for KDE Daemon which isn't very descriptive. KDED runs
in the background and performs a number of small tasks. Some of these
tasks are built in, others are started on demand.

Custom KDED modules can be provided by 3rd party frameworks and
applications.

%package        devel
Summary:        Development files for %{name}
Requires:       %{name}%{?_isa} = %{version}-%{release}
%description    devel
The %{name}-devel package contains libraries and header files for
developing applications that use %{name}.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -n %{framework}-%{version}


%build
%cmake_kf5
%cmake_build


%install
%cmake_install
%find_lang kded5 --with-man --without-mo

# create/own this
mkdir -p %{buildroot}%{_kf5_plugindir}/kded


%post
%systemd_user_post plasma-%{framework}.service


%preun
%systemd_user_preun plasma-%{framework}.service


%files -f kded5.lang
%doc README.md
%license LICENSES/*.txt
%{_kf5_datadir}/qlogging-categories5/%{framework}.*
%{_kf5_bindir}/kded5
# fake, nodisplay, placeholder mostly
%{_kf5_datadir}/applications/org.kde.kded5.desktop
%{_kf5_datadir}/dbus-1/services/*.service
%{_kf5_datadir}/kservicetypes5/*.desktop
%{_kf5_mandir}/man8/kded5.8*
%dir %{_kf5_plugindir}/kded/
%{_userunitdir}/plasma-%{framework}.service

%files devel
%{_kf5_libdir}/cmake/KDED/
%{_kf5_datadir}/dbus-1/interfaces/*.xml


%changelog
* Sun Feb 18 2024 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.115.0-1
- 5.115.0

* Fri Jan 19 2024 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.114.0-1
- 5.114.0

* Fri Dec 15 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.113.0-1
- 5.113.0

* Sun Nov 12 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.112.0-1
- 5.112.0

* Thu Oct 19 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.111.0-1
- 5.111.0

* Sun Sep 10 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.110.0-1
- 5.110.0

* Fri Aug 18 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.109.0-1
- 5.109.0

* Sun Jul 09 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.108.0-1
- 5.108.0

* Sat Jun 10 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.107.0-1
- 5.107.0

* Sat May 13 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.106.0-1
- 5.106.0

* Sat Apr 08 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.105.0-1
- 5.105.0

* Sat Mar 11 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.104.0-1
- 5.104.0

* Sun Feb 12 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.103.0-1
- 5.103.0

