%global framework kdoctools

Name:    kf5-%{framework}
Version: 5.108.0
Release: 1%{?dist}
Summary: KDE Frameworks 5 Tier 2 addon for generating documentation

License: GPLv2+ and MIT
URL:     https://cgit.kde.org/%{framework}.git

%global majmin %(echo %{version} | cut -d. -f1-2)
%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0:        http://download.kde.org/%{stable}/frameworks/%{majmin}/%{framework}-%{version}.tar.xz
Source1:        http://download.kde.org/%{stable}/frameworks/%{majmin}/%{framework}-%{version}.tar.xz.sig
Source2:        gpgkey-53E6B47B45CEA3E0D5B7457758D0EE648A48B3BB.gpg

BuildRequires:  gnupg2
BuildRequires:  docbook-dtds
BuildRequires:  docbook-style-xsl

BuildRequires:  kf5-rpm-macros
BuildRequires:  extra-cmake-modules >= %{majmin}
BuildRequires:  kf5-karchive-devel >= %{majmin}
BuildRequires:  kf5-ki18n-devel >= %{majmin}

BuildRequires:  libxml2-devel
BuildRequires:  libxslt-devel
%if 0%{?fedora} || 0%{?rhel} > 7
%global _with_html --with-html
BuildRequires:  perl-generators
BuildRequires:  perl(Any::URI::Escape)
%else
BuildRequires:  perl(URI::Escape)
%endif
BuildRequires:  qt5-qtbase-devel

Requires:       docbook-dtds
Requires:       docbook-style-xsl

%description
Provides tools to generate documentation in various format from DocBook files.

%package        devel
Summary:        Development files for %{name}
Requires:       %{name}%{?_isa} = %{version}-%{release}
Provides:       kf5-kdoctools-static = %{version}-%{release}
Requires:       qt5-qtbase-devel
%if 0%{?fedora} || 0%{?rhel} > 7
Requires:       perl(Any::URI::Escape)
%else
Requires:       perl(URI::Escape)
%endif
%description    devel
The %{name}-devel package contains libraries and header files for
developing applications that use %{name}.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -n %{framework}-%{version} -p1


%build
%cmake_kf5
%cmake_build


%install
%cmake_install

%find_lang %{name} --all-name --with-man %{?_with_html}


%ldconfig_scriptlets

%files -f %{name}.lang
%doc README.md
%license LICENSES/*.txt
%{_kf5_libdir}/libKF5DocTools.so.5*
## FIXME/TODO: which of these to move to -devel -- rex
%{_kf5_bindir}/checkXML5
%{_kf5_bindir}/meinproc5
%{_kf5_mandir}/man1/*.1*
%{_kf5_mandir}/man7/*.7*
%{_kf5_datadir}/kf5/kdoctools/
%if !0%{?_with_html:1}
%{_kf5_docdir}/HTML/*/kdoctools5-common/
%endif

%files devel
%{_kf5_includedir}/KDocTools/
%{_kf5_libdir}/libKF5DocTools.so
%{_kf5_libdir}/cmake/KF5DocTools/


%changelog
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

