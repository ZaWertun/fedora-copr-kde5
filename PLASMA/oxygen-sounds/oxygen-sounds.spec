%global kf5_version_min 5.94.0

Name:           oxygen-sounds
Version:        5.27.2
Release:        %autorelease
Summary:        The Oxygen Sound Theme

License:        LGPLv3+,CC0,BSD
URL:            https://invent.kde.org/plasma/oxygen-sounds

%global verdir %(echo %{version} | cut -d. -f1-3)
%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global majmin_ver %(echo %{version} | cut -d. -f1,2).50
%global stable unstable
%else
%global majmin_ver %(echo %{version} | cut -d. -f1,2)
%global stable stable
%endif
Source0:        http://download.kde.org/%{stable}/plasma/%{verdir}/%{name}-%{version}.tar.xz
Source1:        http://download.kde.org/%{stable}/plasma/%{verdir}/%{name}-%{version}.tar.xz.sig
Source2:        https://jriddell.org/esk-riddell.gpg

Provides:       oxygen-sound-theme = %{version}-%{release}
Obsoletes:      oxygen-sound-theme <= 5.24.50

BuildRequires:  gnupg2
BuildRequires:  cmake
BuildRequires:  extra-cmake-modules
BuildRequires:  kf5-rpm-macros >= %{kf5_version_min}
BuildRequires:  qt5-qtbase-devel

BuildArch:      noarch

%description
%{summary}.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup

%build
%{cmake_kf5}
%{cmake_build}

%install
%{cmake_install}


%files
%license LICENSES/*.txt
%{_kf5_datadir}/sounds/Oxygen-*


%changelog
%autochangelog

