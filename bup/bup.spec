Name:           bup
Version:        0.33
Release:        1%{?dist}
Summary:        Very efficient backup system based on the git packfile format

License:        GPLv2
URL:            https://github.com/%{name}/%{name}
Source0:        https://github.com/%{name}/%{name}/archive/refs/tags/%{version}.tar.gz#/%{name}-%{version}.tar.gz

%global git_min_ver 1.5.6

BuildRequires:  gcc
BuildRequires:  sed
BuildRequires:  make
BuildRequires:  pandoc
BuildRequires:  git-core >= %{git_min_ver}

BuildRequires:  perl-Time-HiRes
BuildRequires:  python3-devel
BuildRequires:  python3-fuse
BuildRequires:  python3-pylibacl
BuildRequires:  python3-pyxattr
BuildRequires:  python3-tornado

# For tests:
BuildRequires:  acl
BuildRequires:  attr
BuildRequires:  kmod
BuildRequires:  rsync
BuildRequires:  man-db
BuildRequires:  par2cmdline
BuildRequires:  python3-devel
BuildRequires:  python3-pytest
BuildRequires:  python3-pytest-xdist

Requires:       git-core >= %{git_min_ver}
Requires:       par2cmdline
Requires:       python3
Requires:       python3-fuse
Requires:       python3-pylibacl
Requires:       python3-pyxattr
Requires:       python3-tornado

%description
Very efficient backup system based on the git packfile format,
providing fast incremental saves and global deduplication
(among and within files, including virtual machine images).


%prep
%autosetup


%build
%{__sed} -i 's|export LC_CTYPE=iso-8859-1||' dev/install-python-script
# Macros `%%configure` can't be used - it's not configure from autotools
./configure
%make_build


%install
%make_install PREFIX=%{_prefix}
%{__sed} -i 's|#!/bin/sh|#!/usr/bin/sh|' %{buildroot}%{_prefix}/lib/%{name}/cmd/%{name}-*


%check
# Removing `test-meta` - it fails inside mock
%{__rm} -v test/ext/test-meta
%{__make} %{?_smp_mflags} check


%files
%license LICENSE
%doc README.md
%doc %{_datadir}/doc/%{name}/
%{_bindir}/%{name}
%{_prefix}/lib/%{name}/
%{_mandir}/man1/%{name}*.1.gz



%changelog
* Wed Mar 29 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 0.33-1
- version 0.33

* Tue Dec 07 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 0.32-2
- added par2cmdline to Requires & BuildRequires

* Tue Dec 07 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 0.32-1
- first spec for version 0.32
