# trim changelog included in binary rpms
%global _changelog_trimtime %(date +%s -d "1 year ago")

## allow building with an older extra-cmake-modules
%global kf5_version 5.33.0

Name:    breeze-icon-theme
Summary: Breeze icon theme
Version: 5.114.0
Release: 1%{?dist}

# http://techbase.kde.org/Policies/Licensing_Policy
License: LGPLv3+
URL:     https://api.kde.org/frameworks-api/frameworks-apidocs/frameworks/breeze-icons/html/

%global versiondir %(echo %{version} | cut -d. -f1-2)
%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0: http://download.kde.org/%{stable}/frameworks/%{versiondir}/breeze-icons-%{version}.tar.xz
Source1: http://download.kde.org/%{stable}/frameworks/%{versiondir}/breeze-icons-%{version}.tar.xz.sig
Source2: gpgkey-53E6B47B45CEA3E0D5B7457758D0EE648A48B3BB.gpg

## upstream patches (lookaside cache)

## upstreamable patches

# must come *after* patches or %%autosetup sometimes doesn't work right -- rex
BuildArch: noarch

BuildRequires:  gnupg2
BuildRequires:  extra-cmake-modules
BuildRequires:  kf5-rpm-macros
BuildRequires:  qt5-qtbase-devel

# icon optimizations
BuildRequires: hardlink
# for optimizegraphics
BuildRequires: time
# for generate-24px-versions.py
BuildRequires: python3-lxml

# inheritance, though could consider Recommends: if needed -- rex
Requires: hicolor-icon-theme

# Needed for proper Fedora logo
Requires: system-logos

# upstream name
Provides:       breeze-icons = %{version}-%{release}
Provides:       kf5-breeze-icons = %{version}-%{release}

# upgrade path, since this no longer includes cursors since 5.16.0
Obsoletes:      breeze-icon-theme < 5.17.0

%description
%{summary}.

%package rcc
Summary: breeze Qt resource files
# when split out
#Conflicts: breeze-icon-theme < 5.33.0-2
Requires: %{name} = %{version}-%{release}
%description rcc
%{summary}.

%package devel
Summary: Development files for %{name}
Requires: %{name} = %{version}-%{release}
%description devel
Development files for %{name}.

%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -n breeze-icons-%{version} -p1

%if 0%{?kf5_version:1}
sed -i -e "s|%{version}|%{kf5_version}|g" CMakeLists.txt
%endif


%build
%cmake_kf5
%cmake_build


%install
%cmake_install

# Do not use Fedora logo from upstream
rm -rf %{buildroot}%{_datadir}/icons/breeze-dark/apps/48/org.fedoraproject.AnacondaInstaller.svg
rm -rf %{buildroot}%{_datadir}/icons/breeze/apps/48/org.fedoraproject.AnacondaInstaller.svg
# Use copy found in fedora-logos
pushd %{buildroot}%{_datadir}/icons/breeze-dark/apps/48/
ln -s ../../../hicolor/48x48/apps/org.fedoraproject.AnacondaInstaller.svg org.fedoraproject.AnacondaInstaller.svg
popd
pushd %{buildroot}%{_datadir}/icons/breeze/apps/48/
ln -s ../../../hicolor/48x48/apps/org.fedoraproject.AnacondaInstaller.svg org.fedoraproject.AnacondaInstaller.svg
popd

## icon optimizations
#du -s  .
#time optimizegraphics ||:
du -hs .
hardlink -c -v %{buildroot}%{_datadir}/icons/
du -hs .

# %%ghost icon.cache
touch  %{buildroot}%{_kf5_datadir}/icons/{breeze,breeze-dark}/icon-theme.cache


%check
%if 0%{?fedora} > 25 || 0%{?rhel} > 7
## trigger-based scriptlets
%transfiletriggerin -- %{_datadir}/icons/breeze
gtk-update-icon-cache --force %{_datadir}/icons/breeze &>/dev/null || :

%transfiletriggerin -- %{_datadir}/icons/breeze-dark
gtk-update-icon-cache --force %{_datadir}/icons/breeze-dark &>/dev/null || :

%transfiletriggerpostun -- %{_datadir}/icons/breeze
gtk-update-icon-cache --force %{_datadir}/icons/breeze &>/dev/null || :

%transfiletriggerpostun -- %{_datadir}/icons/breeze-dark
gtk-update-icon-cache --force %{_datadir}/icons/breeze-dark &>/dev/null || :

%else
## classic scriptlets
%post
touch --no-create %{_datadir}/icons/breeze &> /dev/null || :
touch --no-create %{_datadir}/icons/breeze-dark &> /dev/null || :

%posttrans
gtk-update-icon-cache %{_datadir}/icons/breeze &> /dev/null || :
gtk-update-icon-cache %{_datadir}/icons/breeze-dark &> /dev/null || :

%postun
if [ $1 -eq 0 ] ; then
  touch --no-create %{_datadir}/icons/breeze &> /dev/null || :
  gtk-update-icon-cache %{_datadir}/icons/breeze &> /dev/null || :
  touch --no-create %{_datadir}/icons/breeze-dark &> /dev/null || :
  gtk-update-icon-cache %{_datadir}/icons/breze-dark &> /dev/null || :
fi
%endif

%files
%license COPYING-ICONS
%doc README.md
%ghost %{_datadir}/icons/breeze/icon-theme.cache
%ghost %{_datadir}/icons/breeze-dark/icon-theme.cache
%{_datadir}/icons/breeze/
%{_datadir}/icons/breeze-dark/
%exclude %{_datadir}/icons/breeze/breeze-icons.rcc
%exclude %{_datadir}/icons/breeze-dark/breeze-icons-dark.rcc


%files rcc
%{_datadir}/icons/breeze/breeze-icons.rcc
%{_datadir}/icons/breeze-dark/breeze-icons-dark.rcc


%files devel
%{_kf5_libdir}/cmake/KF5BreezeIcons/


%changelog
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

