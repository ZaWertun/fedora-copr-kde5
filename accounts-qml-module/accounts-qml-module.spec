%undefine __cmake_in_source_build
%global bazaar_rev   64
%global real_version 0.6+17.04.20170405-0ubuntu1

Name:    accounts-qml-module
Version: 0.6+17.04.20170405
Release: 1%{?dist}
Summary: Ubuntu online accounts QML module
License: GPLv2+
URL:     https://launchpad.net/accounts-qml-module
Source0: https://bazaar.launchpad.net/~online-accounts/%{name}/trunk/tarball/%{bazaar_rev}/%{name}-%{version}.tar.gz

BuildRequires: pkgconfig
BuildRequires: qt5-doctools
BuildRequires: qt5-qtbase-devel
BuildRequires: xorg-x11-server-Xvfb
BuildRequires: pkgconfig(accounts-qt5)
BuildRequires: pkgconfig(libsignon-qt5)
BuildRequires: qt5-qtdeclarative-devel
BuildRequires: qt5-qtquickcontrols2-devel

%description
%{summary}.

%prep
%autosetup -p1 -n ~online-accounts


%build
cd %{name}/trunk
%qmake_qt5 PREFIX=%{_prefix}
%cmake_build


%install
cat %{name}/trunk/Makefile
%make_install INSTALL_ROOT=%{buildroot} -C %{name}/trunk
# Used only for tests:
rm -v %{buildroot}%{_bindir}/tst_plugin
# For now we will ignore docs:
rm -rfv %{buildroot}%{_datadir}/accounts-qml-module/doc


%files
%license %{name}/trunk/COPYING
%{_qt5_qmldir}/Ubuntu/OnlineAccounts/*


%changelog
* Mon Dec 16 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 0.6+17.04.20170405-1
- version 0.6+17.04.20170405-0ubuntu1 (rev 64)

