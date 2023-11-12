%global framework kapidox

Name:    kf5-%{framework}
Version: 5.112.0
Release: 1%{?dist}
Summary: KDE Frameworks 5 Tier 4 scripts and data for building API documentation

License: BSD
URL:     https://cgit.kde.org/%{framework}.git

%global majmin %(echo %{version} | cut -d. -f1-2)
%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0: http://download.kde.org/%{stable}/frameworks/%{majmin}/%{framework}-%{version}.tar.xz
Source1: http://download.kde.org/%{stable}/frameworks/%{majmin}/%{framework}-%{version}.tar.xz.sig
Source2: gpgkey-53E6B47B45CEA3E0D5B7457758D0EE648A48B3BB.gpg

## upstream patches

# make sure BuildArch comes *after* patches, to ensure %%autosetup works right
BuildArch:      noarch

BuildRequires:  gnupg2
BuildRequires:  python3
BuildRequires:  python3-devel
BuildRequires:  python3-setuptools
BuildRequires:  kf5-rpm-macros >= %{majmin}

Requires:       python3-jinja2
Requires:       python3-PyYAML
Requires:       kf5-filesystem >= %{majmin}

%description
Scripts and data for building API documentation (dox) in a standard format and
style.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -n %{framework}-%{version} -p1


%build
%py3_build


%install
%py3_install


%files
%license LICENSES/*.txt
%{python3_sitelib}/kapidox
%{python3_sitelib}/kapidox-%{version}-py*.egg-info
%{_kf5_bindir}/depdiagram_generate_all
%{_kf5_bindir}/%{framework}-depdiagram-generate
%{_kf5_bindir}/%{framework}-depdiagram-prepare
%{_kf5_bindir}/%{framework}-generate


%changelog
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

