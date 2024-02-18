Name:           kdevelop-php
Summary:        Php language and documentation plugins for KDevelop
Version:        23.08.5
Release:        1%{?dist}

# Most files LGPLv2+/GPLv2+
License:        GPLv2+
URL:            http://www.kde.org/
Source0:        https://download.kde.org/stable/release-service/%{version}/src/kdev-php-%{version}.tar.xz
Source1:        https://download.kde.org/stable/release-service/%{version}/src/kdev-php-%{version}.tar.xz.sig
Source2:        gpgkey-D81C0CB38EB725EF6691C385BB463350D6EF31EF.gpg

## kdevelop-pg-qt FTBFS s390x
ExcludeArch: s390x

BuildRequires:  gnupg2
BuildRequires:  gettext
BuildRequires:  kdevelop-pg-qt-devel >= 1.90.91
BuildRequires:  kdevplatform-devel >= %{version}

BuildRequires:  kf5-rpm-macros
BuildRequires:  extra-cmake-modules
BuildRequires:  grantlee-qt5-devel
BuildRequires:  kf5-knotifyconfig-devel
BuildRequires:  kf5-knewstuff-devel
BuildRequires:  kf5-kdelibs4support-devel
BuildRequires:  kf5-threadweaver-devel
BuildRequires:  kf5-ktexteditor-devel
BuildRequires:  cmake(KF5SyntaxHighlighting)
BuildRequires:  kf5-ki18n-devel
BuildRequires:  kf5-kcmutils-devel

BuildRequires:  qt5-qtbase-devel
BuildRequires:  qt5-qtwebkit-devel

%{?kdevelop_requires}

%description
%{summary}.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%setup -q -n kdev-php-%{version}


%build
%cmake_kf5
%cmake_build


%install
%cmake_install

# TODO Enable translations in stable build
%find_lang %{name} --all-name


%files -f %{name}.lang
%doc AUTHORS
%license LICENSES/*.txt
%{_datadir}/kdevappwizard/
%{_datadir}/kdevphpsupport/
%{_datadir}/kservices5/*.desktop
%{_includedir}/kdev-php/*
%{_libdir}/libkdevphp*.so
%{_libdir}/cmake/KDevPHP/*.cmake
%{_kf5_qtplugindir}/kdevplatform/
%{_datadir}/qlogging-categories5/kdevphpsupport.categories
%{_datadir}/metainfo/org.kde.kdev-php.metainfo.xml


%changelog
* Sun Feb 18 2024 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.08.5-1
- 23.08.5

* Fri Dec 08 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.08.4-1
- 23.08.4

* Fri Nov 10 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.08.3-1
- 23.08.3

* Fri Oct 13 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.08.2-1
- 23.08.2

* Thu Sep 14 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.08.1-1
- 23.08.1

* Sun Aug 27 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.08.0-1
- 23.08.0

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

