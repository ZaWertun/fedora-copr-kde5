Name:    kompare
Summary: Diff tool
Version: 23.04.3
Release: 1%{?dist}

License: GPLv2+ and GFDL
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

BuildRequires:  gnupg2
BuildRequires:  desktop-file-utils
BuildRequires:  gcc-c++
BuildRequires:  kf5-rpm-macros
BuildRequires:  extra-cmake-modules
BuildRequires:  qt5-qtbase-devel
BuildRequires:  kf5-kcoreaddons-devel
BuildRequires:  kf5-kcodecs-devel
BuildRequires:  kf5-kdoctools-devel
BuildRequires:  kf5-kiconthemes-devel
BuildRequires:  kf5-kjobwidgets-devel
BuildRequires:  kf5-kconfig-devel
BuildRequires:  kf5-kparts-devel
BuildRequires:  kf5-ktexteditor-devel
BuildRequires:  kf5-kwidgetsaddons-devel
BuildRequires:  cmake(KF5SyntaxHighlighting)

%global majmin_ver %(echo %{version} | cut -d. -f1,2)
BuildRequires:  libkomparediff2-devel >= %{majmin_ver}

Requires:       %{name}-libs%{?_isa} = %{version}-%{release}

# translations moved here
Conflicts: kde-l10n < 17.03

Conflicts:      kdesdk-common < 4.10.80
Obsoletes:      kdesdk-kompare < 4.10.80
Provides:       kdesdk-kompare = %{version}-%{release}
Provides:       mergetool

%description
Tool to visualize changes between two versions of a file

%package libs
Summary: Runtime libraries for %{name}
Obsoletes: kdesdk-kompare-libs < 4.10.80
Provides:  kdesdk-kompare-libs = %{version}-%{release}
Requires:  %{name} = %{version}-%{release}
# help upgrade path since newer libkomparediff2 Obsoletes: kompare-libs < 4.11.80
Requires:  libkomparediff2%{?_isa} >= %{majmin_ver}
%description libs
This package contains shared libraries for %{name}.

%package devel
Summary: Developer files for %{name}
Obsoletes: kdesdk-kompare-devel < 4.10.80
Provides:  kdesdk-kompare-devel = %{version}-%{release}
Requires:  %{name}-libs%{?_isa} = %{version}-%{release}
Requires:  qt5-qtbase-devel
%description devel
The %{name}-devel package contains libraries and header files for
developing applications that use %{name}.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup


%build
%cmake_kf5
%cmake_build


%install
%cmake_install
%find_lang %{name} --all-name --with-html


%check
desktop-file-validate %{buildroot}%{_datadir}/applications/org.kde.kompare.desktop


%files -f %{name}.lang
%doc README
%license LICENSES/*.txt
%{_bindir}/kompare
%{_datadir}/applications/org.kde.kompare.desktop
%{_kf5_datadir}/kio/servicemenus/kompare.desktop
%{_kf5_metainfodir}/org.kde.kompare.appdata.xml
%{_datadir}/icons/hicolor/*/apps/kompare.*
%{_kf5_datadir}/qlogging-categories5/kompare.categories

%ldconfig_scriptlets libs

%files libs
%{_libdir}/libkomparedialogpages.so.*
%{_libdir}/libkompareinterface.so.*
%{_kf5_plugindir}/parts/komparenavtreepart.so
%{_kf5_plugindir}/parts/komparepart.so

%files devel
%{_includedir}/kompare/
%{_libdir}/libkompareinterface.so


%changelog
* Thu Jul 06 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.04.3-1
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

