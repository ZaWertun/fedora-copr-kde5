# uncomment to enable bootstrap mode
#global bootstrap 1

%if !0%{?bootstrap}
%global docs 1
%global tests 1
%endif

Name:    extra-cmake-modules
Summary: Additional modules for CMake build system
Version: 5.114.0
Release: 1%{?dist}

License: BSD
URL:     https://api.kde.org/ecm/
#URL:    https://cgit.kde.org/extra-cmake-modules.git/

%global versiondir %(echo %{version} | cut -d. -f1-2)
%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0:        http://download.kde.org/%{stable}/frameworks/%{versiondir}/%{name}-%{version}.tar.xz
Source1:        http://download.kde.org/%{stable}/frameworks/%{versiondir}/%{name}-%{version}.tar.xz.sig
Source2:        gpgkey-53E6B47B45CEA3E0D5B7457758D0EE648A48B3BB.gpg
BuildArch:      noarch

## bundle clang python bindings here, at least until they are properly packaged elsewhere, see:
## https://bugzilla.redhat.com/show_bug.cgi?id=1490997
#Source1: clang-python-4.0.1.tar.gz
#if 0%{?fedora} && 0%{?fedora} < 27
%if 0
%global clang 1
Provides: bundled(python2-clang) = 4.0.1
%if 0%{?tests}
BuildRequires: python2-PyQt5-devel
%endif
%endif

## upstreamable patches
# do not unconditionally link in base/core libpoppler library
Patch2: extra-cmake-modules-5.39.0-poppler_overlinking.patch
# https://bugzilla.redhat.com/1435525
Patch3: extra-cmake-modules-5.93.0-qt_prefix.patch

BuildRequires: gnupg2
BuildRequires: kf5-rpm-macros
%if 0%{?docs}
# qcollectiongenerator
BuildRequires: qt5-qttools-devel
# sphinx-build
%if 0%{?fedora} || 0%{?rhel} > 7
BuildRequires: python3-sphinx
%global sphinx_build -DSphinx_BUILD_EXECUTABLE:PATH=%{_bindir}/sphinx-build-3
%else
BuildRequires: python2-sphinx
%endif
%endif

Requires: kf5-rpm-macros
%if 0%{?fedora}
# /usr/share/ECM/kde-modules/appstreamtest.cmake references appstreamcli
# hard vs soft dep?  --rex
Requires: appstream
%endif
# /usr/share/ECM/modules/ECMPoQmTools.cmake
%if 0%{?fedora} || 0%{?rhel} > 7
Requires: cmake(Qt5LinguistTools)
%else
# use pkgname instead of cmake since el7 qt5 pkgs currently do not include cmake() provides
Requires: qt5-linguist
%endif

%description
Additional modules for CMake build system needed by KDE Frameworks.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -p1 %{?clang:-a1}


%build

%if 0%{?clang}
PYTHONPATH=`pwd`/python
export PYTHONPATH
%endif

%cmake_kf5 \
  -DBUILD_HTML_DOCS:BOOL=%{?docs:ON}%{!?docs:OFF} \
  -DBUILD_MAN_DOCS:BOOL=%{?docs:ON}%{!?docs:OFF} \
  -DBUILD_TESTING:BOOL=%{?tests:ON}%{!?tests:OFF} \
  %{?sphinx_build}
%cmake_build


%install
%cmake_install

%if 0%{?clang}
# hack clang-python install
mkdir -p %{buildroot}%{_datadir}/ECM/python/clang
install -m644 -p python/clang/* %{buildroot}%{_datadir}/ECM/python/clang/
%endif


%check
%if 0%{?tests}
%if 0%{?clang}
PYTHONPATH=`pwd`/python
export PYTHONPATH
%endif
export CTEST_OUTPUT_ON_FAILURE=1
make test ARGS="--output-on-failure --timeout 300" -C %{_target_platform} ||:
%endif


%files
%doc README.rst
%license COPYING-CMAKE-SCRIPTS
%{_datadir}/ECM/
%if 0%{?docs}
%{_kf5_docdir}/ECM/html/
%{_kf5_mandir}/man7/ecm*.7*
%endif


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

