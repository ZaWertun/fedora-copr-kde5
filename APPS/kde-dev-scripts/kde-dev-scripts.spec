Name:    kde-dev-scripts
Summary: KDE SDK scripts
Version: 23.08.2
Release: 1%{?dist}

License: GPLv2+ and GPLv2+ and BSD
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
BuildRequires:  extra-cmake-modules
BuildRequires:  cmake(KF5DocTools)

BuildRequires:  perl-generators
%if 0%{?fedora} >= 31
# for python3-rpm-macros
BuildRequires:  python3-devel
%else
BuildRequires:  python2-devel
%endif
# for env replacement in %%install
BuildRequires:  sed

Requires:       advancecomp
Requires:       optipng

BuildArch:      noarch

Conflicts:      kdesdk-common < 4.10.80
Provides:       kdesdk-scripts = %{version}-%{release}
Obsoletes:      kdesdk-scripts < 4.10.80

Conflicts: kde-l10n < 17.08.3-2

%description
KDE SDK scripts


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup


%build
%cmake_kf5
%cmake_build


%install
%cmake_install
%find_lang %{name} --all-name --with-html --with-man

# purge use of /usr/bin/env
sed -i \
  -e "s|^#!/usr/bin/env bash|#!/bin/bash|g" \
  -e "s|^#! /usr/bin/env bash|#!/bin/bash|g" \
  -e "s|^#!/usr/bin/env perl|#!/usr/bin/perl|g" \
  -e "s|^#! /usr/bin/env perl|#!/usr/bin/perl|g" \
  %{buildroot}%{_kf5_bindir}/*

%if 0%{?fedora} >= 31
sed -i \
  -e "s|^#!/usr/bin/env python$|#!%{__python3}|g" \
  -e "s|^#! /usr/bin/env python$|#!%{__python3}|g" \
  %{buildroot}%{_kf5_bindir}/*
%else
sed -i \
  -e "s|^#!/usr/bin/env python$|#!%{__python2}|g" \
  -e "s|^#! /usr/bin/env python$|#!%{__python2}|g" \
  %{buildroot}%{_kf5_bindir}/*
%endif

# unpackaged files
# This one fits better into krazy2 (it requires krazy2), and the version in
# kdesdk does not understand lib64.
rm -fv %{buildroot}%{_kf5_bindir}/krazy-licensecheck


%check
test -n "$(grep "/usr/bin/env" %{buildroot}%{_kf5_bindir}/* 2> /dev/null )" ||:


%files -f %{name}.lang
%doc README
%license COPYING
%{_kf5_bindir}/grantlee_strings_extractor.py
%{_kf5_bindir}/c++-copy-class-and-file
%{_kf5_bindir}/c++-rename-class-and-file
%{_kf5_bindir}/svnrevertlast
%{_kf5_bindir}/fixuifiles
%{_kf5_bindir}/cvscheck
%{_kf5_bindir}/extend_dmalloc
%{_kf5_bindir}/extractattr
%{_kf5_bindir}/noncvslist
%{_kf5_bindir}/pruneemptydirs
%{_kf5_bindir}/cvsrevertlast
%{_kf5_bindir}/create_makefile
%{_kf5_bindir}/colorsvn
%{_kf5_bindir}/cvslastchange
%{_kf5_bindir}/svngettags
%{_kf5_bindir}/create_svnignore
%{_kf5_bindir}/svnchangesince
%{_kf5_bindir}/build-progress.sh
%{_kf5_bindir}/package_crystalsvg
%{_kf5_bindir}/svnbackport
%{_kf5_bindir}/svnlastlog
%{_kf5_bindir}/cxxmetric
%{_kf5_bindir}/kdemangen.pl
%{_kf5_bindir}/cvsforwardport
%{_kf5_bindir}/includemocs
%{_kf5_bindir}/svnlastchange
%{_kf5_bindir}/wcgrep
%{_kf5_bindir}/qtdoc
%{_kf5_bindir}/nonsvnlist
%{_kf5_bindir}/svnforwardport
%{_kf5_bindir}/create_cvsignore
%{_kf5_bindir}/svnintegrate
%{_kf5_bindir}/kdekillall
%{_kf5_bindir}/create_makefiles
%{_kf5_bindir}/cvsbackport
%{_kf5_bindir}/fixkdeincludes
%{_kf5_bindir}/kde-systemsettings-tree.py
%{_kf5_bindir}/zonetab2pot.py
%{_kf5_bindir}/kde_generate_export_header
%{_kf5_bindir}/cvs-clean
%{_kf5_bindir}/kdelnk2desktop.py
%{_kf5_bindir}/findmissingcrystal
%{_kf5_bindir}/adddebug
%{_kf5_bindir}/cvsversion
%{_kf5_bindir}/cheatmake
%{_kf5_bindir}/cvsblame
%{_kf5_bindir}/optimizegraphics
%{_kf5_bindir}/cvsaddcurrentdir
%{_kf5_bindir}/fix-include.sh
%{_kf5_bindir}/kdedoc
%{_kf5_bindir}/svn-clean
%{_kf5_bindir}/png2mng.pl
%{_kf5_bindir}/extractrc
%{_kf5_bindir}/makeobj
%{_kf5_bindir}/cvslastlog
%{_kf5_bindir}/svnversions
%{_kf5_bindir}/draw_lib_dependencies
%{_kf5_bindir}/reviewboard-am
%{_kf5_bindir}/uncrustify-kf5
%{_kf5_datadir}/uncrustify/
%{_kf5_bindir}/clean-forward-declaration.sh
%{_kf5_bindir}/clean-includes.sh
%{_mandir}/man1/adddebug.1*
%{_mandir}/man1/cheatmake.1*
%{_mandir}/man1/create_cvsignore.1*
%{_mandir}/man1/create_makefile.1*
%{_mandir}/man1/create_makefiles.1*
%{_mandir}/man1/cvscheck.1*
%{_mandir}/man1/cvslastchange.1*
%{_mandir}/man1/cvslastlog.1*
%{_mandir}/man1/cvsrevertlast.1*
%{_mandir}/man1/cxxmetric.1*
%{_mandir}/man1/extend_dmalloc.1*
%{_mandir}/man1/extractrc.1*
%{_mandir}/man1/fixincludes.1*
%{_mandir}/man1/pruneemptydirs.1*
%{_mandir}/man1/qtdoc.1*
%{_mandir}/man1/zonetab2pot.py.1*


%changelog
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

