Name:    kf5
Version: 5.114.0
Release: 1%{?dist}
Summary: Filesystem and RPM macros for KDE Frameworks 5
License: BSD
URL:     http://www.kde.org

Source0: macros.kf5

%description
Filesystem and RPM macros for KDE Frameworks 5

%package filesystem
Summary: Filesystem for KDE Frameworks 5
# noarch -> arch transition
Obsoletes: kf5-filesystem < 5.10.0-2
%{?_qt5_version:Requires: qt5-qtbase%{?_isa} >= %{_qt5_version}}
%description filesystem
Filesystem for KDE Frameworks 5.

%package rpm-macros
Summary: RPM macros for KDE Frameworks 5
%if 0%{?fedora} || 0%{?rhel} > 7
Requires: cmake >= 3
Requires: qt5-rpm-macros >= 5.11
%else
Requires: cmake3
Requires: qt5-qtbase-devel >= 5.11
%endif
# misc build environment dependencies
Requires: gcc-c++
BuildArch: noarch
%description rpm-macros
RPM macros for building KDE Frameworks 5 packages.


%install
# See macros.kf5 where the directories are specified
mkdir -p %{buildroot}%{_prefix}/{lib,%{_lib}}/qt5/plugins/kf5/
mkdir -p %{buildroot}%{_prefix}/{lib,%{_lib}}/kconf_update_bin
mkdir -p %{buildroot}%{_includedir}/KF5
mkdir -p %{buildroot}%{_datadir}/{config.kcfg,kconf_update,kf5,kservicetypes5}
mkdir -p %{buildroot}%{_datadir}/kpackage/{genericqml,kcms}
mkdir -p %{buildroot}%{_datadir}/kservices5/ServiceMenus
mkdir -p %{buildroot}%{_datadir}/knsrcfiles/
mkdir -p %{buildroot}%{_datadir}/qlogging-categories5/
mkdir -p %{buildroot}%{_datadir}/solid/{actions,devices}
mkdir -p %{buildroot}%{_libexecdir}/kf5
mkdir -p %{buildroot}%{_sysconfdir}/xdg/plasma-workspace/{env,shutdown}

install -Dpm644 %{_sourcedir}/macros.kf5 %{buildroot}%{_rpmconfigdir}/macros.d/macros.kf5
sed -i \
  -e "s|@@KF5_VERSION@@|%{version}|g" \
%if 0%{?rhel} || 0%{?rhel} > 7
  -e 's|rhel:%{__cmake3}|rhel:%{__cmake}|' \
%endif
  %{buildroot}%{_rpmconfigdir}/macros.d/macros.kf5


%files filesystem
%{_sysconfdir}/xdg/plasma-workspace/
%{_prefix}/lib/qt5/plugins/kf5/
%{_prefix}/%{_lib}/qt5/plugins/kf5/
%{_prefix}/lib/kconf_update_bin/
%{_prefix}/%{_lib}/kconf_update_bin/
%{_includedir}/KF5/
%{_libexecdir}/kf5/
%{_datadir}/config.kcfg/
%{_datadir}/kconf_update/
%{_datadir}/kf5/
%{_datadir}/kpackage/
%{_datadir}/kservices5/
%{_datadir}/kservicetypes5/
%{_datadir}/knsrcfiles/
%{_datadir}/qlogging-categories5/
%{_datadir}/solid/

%files rpm-macros
%{_rpmconfigdir}/macros.d/macros.kf5


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

