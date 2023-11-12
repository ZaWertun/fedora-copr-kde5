Name:       kdevelop-python
Version:    23.08.3
%global py3_suffix -py3
%global py3_tag .py3
Release:    1%{?dist}
License:    GPLv2
Source0:    http://download.kde.org/stable/release-service/%{version}/src/kdev-python-%{version}.tar.xz
Source1:    http://download.kde.org/stable/release-service/%{version}/src/kdev-python-%{version}.tar.xz.sig
Source2:    gpgkey-D81C0CB38EB725EF6691C385BB463350D6EF31EF.gpg

## kdevelop-pg-qt FTBFS s390x
ExcludeArch: s390x

# ensure the installed Python 3 scripts have #!/usr/bin/env python3
Patch0:     kdev-python-1.7.0-py3-shebang.patch

# remove bogus autogenerated set"other methods from
# documentation_files/__builtin_types__.py that are actually the < and >
# operators, not methods - the bogus syntax (quote in identifier) makes the
# brp-python-bytecompile script choke
Patch1:     kdev-python-1.7.0-py3-doc-syntax.patch

# Fix syntax error due to async being a reserved keyword in python3.7
Patch2:     kdev-python_async.patch

Summary:    Python 3 Plugin for KDevelop
URL:        https://www.kdevelop.org/

BuildRequires:  gnupg2
BuildRequires:  kdevelop-devel >= %{version}
BuildRequires:  python3-devel >= 3.10
BuildRequires:  python-rpm-macros
BuildRequires:  gettext

BuildRequires:  kf5-rpm-macros
BuildRequires:  extra-cmake-modules
BuildRequires:  grantlee-qt5-devel
BuildRequires:  kf5-ki18n-devel
BuildRequires:  kf5-knotifyconfig-devel
BuildRequires:  kf5-knewstuff-devel
BuildRequires:  kf5-kcmutils-devel
BuildRequires:  kf5-ktexteditor-devel
BuildRequires:  cmake(KF5SyntaxHighlighting)
BuildRequires:  kf5-threadweaver-devel
BuildRequires:  kf5-kitemmodels-devel
BuildRequires:  kf5-kdelibs4support-devel

BuildRequires:  qt5-qtbase-devel
BuildRequires:  qt5-qtwebkit-devel

# force using Python 3 for bytecompiling
%global __python %{__python3}

%{?kdevelop_requires}

%description
Python 3 language support for the KDevelop Integrated Development Environment.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%setup -qn kdev-python-%{version}

# don't use backups because CMakeLists.txt installs the whole directory
# shebang
%patch0 -p1
# doc-syntax
%patch1 -p1
# async
%patch2 -p1

%build
%cmake_kf5
%cmake_build

%py_byte_compile %{__python3} %{buildroot}%{_datadir}/kdevpythonsupport

%install
%cmake_install

# don't ship this Python 2 script, it is only used to generate the
# documentation_files, it is not needed at runtime
rm -f %{buildroot}%{_datadir}/kdevpythonsupport/documentation_files/PyKDE4/parse_xml.py

# TODO Enable translations in stable build
%find_lang kdevpython

%ldconfig_scriptlets

%files -f kdevpython.lang
%doc DESIGN README
%{_libdir}/libkdevpythonduchain.so
%{_libdir}/libkdevpythonparser.so
%{_libdir}/libkdevpythoncompletion.so
%{_datadir}/kdevpythonsupport
%{_datadir}/kdevappwizard/templates/*
%{_kf5_qtplugindir}/kdevplatform/
%{_datadir}/qlogging-categories5/kdevpythonsupport.categories
%{_datadir}/metainfo/org.kde.kdev-python.metainfo.xml

%changelog
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

