Name:    poxml
Summary: Text utilities from kdesdk
Version: 23.08.4
Release: 1%{?dist}

License: GPLv2+
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

BuildRequires: gnupg2
BuildRequires: extra-cmake-modules
BuildRequires: gettext-devel
BuildRequires: kf5-kdoctools-devel
BuildRequires: kf5-rpm-macros
BuildRequires: pkgconfig(libxslt)
BuildRequires: pkgconfig(Qt5Xml)

# translations moved here
Conflicts: kde-l10n < 17.03

Conflicts:     kdesdk-common < 4.10.80
Provides:      kdesdk-poxml = %{version}-%{release}
Obsoletes:     kdesdk-poxml < 4.10.80

%description
Text utilities from kdesdk, including
po2xml
split2po
swappo
xml2pot


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup


%build
%cmake_kf5
# seeing failures, appear to be parallel-build races
%cmake_build


%install
%cmake_install

%find_lang %{name} --all-name --with-html --with-man


%files -f %{name}.lang
%license COPYING*
%{_kf5_bindir}/po2xml
%{_kf5_bindir}/split2po
%{_kf5_bindir}/swappo
%{_kf5_bindir}/xml2pot
%{_mandir}/man1/po2xml*
%{_mandir}/man1/split2po*
%{_mandir}/man1/swappo*
%{_mandir}/man1/xml2pot*


%changelog
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

