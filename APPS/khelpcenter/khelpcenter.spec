Name:    khelpcenter
Summary: Show documentation for KDE applications
# Override khelpcenter subpackage from kde-runtime-15.04 (no longer built)
Epoch:   1
Version: 23.08.5
Release: 1%{?dist}

License: GPLv2 or GPLv3
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
BuildRequires:  extra-cmake-modules
BuildRequires:  grantlee-qt5-devel
BuildRequires:  kf5-karchive-devel
BuildRequires:  kf5-kbookmarks-devel
BuildRequires:  kf5-kcmutils-devel
BuildRequires:  kf5-kconfig-devel
BuildRequires:  kf5-kcoreaddons-devel
BuildRequires:  kf5-kdbusaddons-devel
BuildRequires:  kf5-kdoctools-devel
BuildRequires:  kf5-khtml-devel
BuildRequires:  kf5-kinit-devel >= 5.10.0-3
BuildRequires:  kf5-ki18n-devel
BuildRequires:  kf5-kservice-devel
BuildRequires:  kf5-kwindowsystem-devel
BuildRequires:  kf5-rpm-macros
BuildRequires:  libappstream-glib
BuildRequires:  libxml2-devel
BuildRequires:  perl-interpreter
BuildRequires:  perl-generators
BuildRequires:  qt5-qtbase-devel
BuildRequires:  qt5-qtscript-devel
BuildRequires:  xapian-core-devel

# translations moved here
Conflicts: kde-l10n < 17.04.1-2

Requires:       kf5-filesystem

# libkdeinit5_*
%{?kf5_kinit_requires}

%description
%{summary}.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup


%build
%cmake_kf5
%cmake_build


%install
%cmake_install

# Provide khelpcenter service for KDE 3 applications
mkdir -p %{buildroot}%{_datadir}/services
cp -alf \
  %{buildroot}%{_datadir}/kservices5/khelpcenter.desktop \
  %{buildroot}%{_datadir}/services/

%find_lang %{name} --all-name --with-html


%check
desktop-file-validate %{buildroot}%{_datadir}/applications/org.kde.Help.desktop ||:
appstream-util validate-relax --nonet %{buildroot}%{_kf5_metainfodir}/org.kde.khelpcenter.metainfo.xml


%files -f %{name}.lang
%doc README.metadata
%license LICENSES/*.txt
%{_bindir}/khelpcenter
%{_libexecdir}/khc_mansearch.pl
%{_libexecdir}/khc_xapianindexer
%{_libexecdir}/khc_xapiansearch
%{_kf5_datadir}/khelpcenter/
%{_kf5_datadir}/dbus-1/services/org.kde.khelpcenter.service
%{_kf5_metainfodir}/org.kde.khelpcenter.metainfo.xml
%{_datadir}/applications/org.kde.khelpcenter.desktop
%{_datadir}/config.kcfg/khelpcenter.kcfg
%{_datadir}/kservices5/khelpcenter.desktop
%{_datadir}/services/khelpcenter.desktop
%{_datadir}/kde4/services/khelpcenter.desktop
%{_kf5_datadir}/qlogging-categories5/*.categories


%changelog
* Sun Feb 18 2024 Yaroslav Sidlovsky <zawertun@gmail.com> - 1:23.08.5-1
- 23.08.5

* Fri Dec 08 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 1:23.08.4-1
- 23.08.4

* Fri Nov 10 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 1:23.08.3-1
- 23.08.3

* Fri Oct 13 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 1:23.08.2-1
- 23.08.2

* Thu Sep 14 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 1:23.08.1-1
- 23.08.1

* Sun Aug 27 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 1:23.08.0-1
- 23.08.0

* Thu Jul 06 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 1:23.04.3-1
- 23.04.3

* Thu Jun 08 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 1:23.04.2-1
- 23.04.2

* Thu May 11 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 1:23.04.1-1
- 23.04.1

* Thu Apr 20 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 1:23.04.0-1
- 23.04.0

* Sat Mar 25 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 1:22.12.3-6
- rebuild

* Sat Mar 25 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 1:22.12.3-5
- rebuild

