Name:    kdeedu-data
Summary: Shared icons, artwork and data files for educational applications
Version: 23.08.2
Release: 1%{?dist}

License: GPLv2
URL:     https://cgit.kde.org/%{name}.git

%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0: https://download.kde.org/%{stable}/release-service/%{version}/src/%{name}-%{version}.tar.xz
Source1: https://download.kde.org/%{stable}/release-service/%{version}/src/%{name}-%{version}.tar.xz.sig
Source2: gpgkey-D81C0CB38EB725EF6691C385BB463350D6EF31EF.gpg
BuildArch: noarch

BuildRequires: gnupg2
BuildRequires: kde-filesystem
BuildRequires: extra-cmake-modules
BuildRequires: kf5-rpm-macros
# ECM macro used in kdeedu-data needs qmake
BuildRequires: qt5-qtbase-devel

# when kdeedu-data was split out from libkdeedu, upgrade path
Obsoletes: libkdeedu < 14

Requires: hicolor-icon-theme
Requires: kde-filesystem

%description
%{summary}.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%setup -q


%build
%cmake_kf5
%cmake_build


%install
%cmake_install

# previous packages shipped %%{_kde4_appsdir}/kvtml , so let's
# (hard)link data in in both places
mkdir -p %{buildroot}%{_kde4_appsdir}
cp -alf \
   %{buildroot}%{_kf5_datadir}/apps/kvtml/ \
   %{buildroot}%{_kde4_appsdir}


%files
%license COPYING
%{_kde4_appsdir}/kvtml/
%{_kf5_datadir}/apps/kvtml/
%{_datadir}/icons/hicolor/*/*/*


%changelog
* Fri Oct 13 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.08.2-1
- 23.08.2

* Thu Sep 14 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.08.1-1
- 23.08.1

* Mon Aug 28 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.08.0-1
- 23.08.0

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

