# trim changelog included in binary rpms
%global _changelog_trimtime %(date +%s -d "1 year ago")

%if ! 0%{?bootstrap}
## enable icon optimizations, takes awhile
%global optimize 1
%endif

## allow building with an older extra-cmake-modules
%global kf5_version 5.27.0

Name:    oxygen-icon-theme
Summary: Oxygen icon theme
Epoch:   1
Version: 5.111.0
Release: 1%{?dist}

# http://techbase.kde.org/Policies/Licensing_Policy
License: LGPLv3+
URL:     https://techbase.kde.org/Projects/Oxygen

%global versiondir %(echo %{version} | cut -d. -f1-2)
%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0: http://download.kde.org/%{stable}/frameworks/%{versiondir}/oxygen-icons5-%{version}.tar.xz
Source1: http://download.kde.org/%{stable}/frameworks/%{versiondir}/oxygen-icons5-%{version}.tar.xz.sig
Source2: gpgkey-53E6B47B45CEA3E0D5B7457758D0EE648A48B3BB.gpg
BuildArch: noarch

# we are noarch, skip trying to find debuginfo
%global debug_package   %{nil}

## upstreamable patches

BuildRequires:  gnupg2
BuildRequires:  kf5-rpm-macros
BuildRequires:  extra-cmake-modules >= %{?kf5_version}%{!?kf5_version:%{version}}
BuildRequires:  qt5-qtbase-devel

BuildRequires:  hardlink
%if 0%{?optimize}
# for optimizegraphics
BuildRequires:  kde-dev-scripts
%endif
BuildRequires:  kde-filesystem
BuildRequires:  time

# inheritance, though could consider Recommends: if needed -- rex
Requires: hicolor-icon-theme

# upstream names
Provides:       oxygen-icons5 = %{epoch}:%{version}-%{release}
Provides:       oxygen-icons = %{epoch}:%{version}-%{release}
Provides:       kf5-oxygen-icons = %{epoch}:%{version}-%{release}

# some icons moved here from kdepim, add explicit Conflicts to help dep solvers
%if 0%{?fedora} < 24
# http://bugzilla.redhat.com/1308475
Conflicts: kmail < 4.14.10-10
%else
# http://bugzilla.redhat.com/1308358
Conflicts: kmail < 15.12.2
%endif

%description
%{summary}.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -n oxygen-icons5-%{version} -p1

%if 0%{?kf5_version:1}
sed -i -e "s|%{version}|%{kf5_version}|g" CMakeLists.txt
%endif


%build
%cmake_kf5
%cmake_build


%install
%cmake_install

# optimize
pushd %{buildroot}%{_kf5_datadir}/icons/oxygen

du -s  .

hardlink -c -v %{buildroot}%{_kf5_datadir}/icons/oxygen

du -s .

%if 0%{?optimize}
time optimizegraphics

du -s .

## As of 15.04.3, hardlink reports
#Directories 78
#Objects 6926
#IFREG 6848
#Comparisons 901
#Linked 901
#saved 7737344
hardlink -c -v %{buildroot}%{_kf5_datadir}/icons/oxygen

du -s .
%endif

# create/own all potential dirs
mkdir -p %{buildroot}%{_kf5_datadir}/icons/oxygen/{16x16,22x22,24x24,32x32,36x36,48x48,64x64,96x96,128x128,512x512,scalable}/{actions,apps,devices,mimetypes,places}


%if 0%{?fedora} > 25
## trigger-based scriptlets
%transfiletriggerin -- %{_datadir}/icons/oxygen
gtk-update-icon-cache --force %{_datadir}/icons/oxygen &>/dev/null || :

%transfiletriggerpostun -- %{_datadir}/icons/oxygen
gtk-update-icon-cache --force %{_datadir}/icons/oxygen &>/dev/null || :

%else
# classic scriptlets
%post
touch --no-create %{_kf5_datadir}/icons/oxygen &> /dev/null || :

%posttrans
gtk-update-icon-cache %{_kf5_datadir}/icons/oxygen &> /dev/null || :

%postun
if [ $1 -eq 0 ] ; then
touch --no-create %{_kf5_datadir}/icons/oxygen &> /dev/null || :
gtk-update-icon-cache %{_kf5_datadir}/icons/oxygen &> /dev/null || :
fi
%endif

%files
%doc AUTHORS CONTRIBUTING
%license COPYING
%dir %{_datadir}/icons/oxygen/
%{_datadir}/icons/oxygen/index.theme
%{_datadir}/icons/oxygen/base/
%{_datadir}/icons/oxygen/*x*/
%{_datadir}/icons/oxygen/scalable/


%changelog
* Thu Oct 19 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 1:5.111.0-1
- 5.111.0

* Sun Sep 10 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 1:5.110.0-1
- 5.110.0

* Fri Aug 18 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 1:5.109.0-1
- 5.109.0

* Sun Jul 09 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 1:5.108.0-1
- 5.108.0

* Sat Jun 10 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 1:5.107.0-1
- 5.107.0

* Sat May 13 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 1:5.106.0-1
- 5.106.0

* Sat Apr 08 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 1:5.105.0-1
- 5.105.0

* Sat Mar 11 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 1:5.104.0-1
- 5.104.0

* Sun Feb 12 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 1:5.103.0-1
- 5.103.0

