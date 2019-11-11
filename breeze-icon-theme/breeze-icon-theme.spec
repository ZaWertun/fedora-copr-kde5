# trim changelog included in binary rpms
%global _changelog_trimtime %(date +%s -d "1 year ago")

## allow building with an older extra-cmake-modules
%global kf5_version 5.33.0

Name:    breeze-icon-theme
Summary: Breeze icon theme
Version: 5.64.0
Release: 1%{?dist}

# http://techbase.kde.org/Policies/Licensing_Policy
License: LGPLv3+
URL:     https://api.kde.org/frameworks-api/frameworks-apidocs/frameworks/breeze-icons/html/

%global versiondir %(echo %{version} | cut -d. -f1-2)
%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0: http://download.kde.org/%{stable}/frameworks/%{versiondir}/breeze-icons-%{version}.tar.xz

## upstream patches (lookaside cache)

## upstreamable patches
Patch100: fix-breeze-dark-inheritance.patch

# must come *after* patches or %%autosetup sometimes doesn't work right -- rex
BuildArch: noarch

BuildRequires:  extra-cmake-modules
BuildRequires:  kf5-rpm-macros
BuildRequires:  qt5-qtbase-devel

# icon optimizations
BuildRequires: hardlink
# for optimizegraphics
#BuildRequires: kde-dev-scripts
BuildRequires: time

# inheritance, though could consider Recommends: if needed -- rex
Requires: hicolor-icon-theme

# upstream name
Provides:       breeze-icons = %{version}-%{release}
Provides:       kf5-breeze-icons = %{version}-%{release}

# upgrade path, since this no longer includes cursors since 5.16.0
Obsoletes:      breeze-icon-theme < 5.17.0

%description
%{summary}.

%package rcc
Summary: breeze Qt resource files
# when split out
#Conflicts: breeze-icon-theme < 5.33.0-2
Requires: %{name} = %{version}-%{release}
%description rcc
%{summary}.



%prep
%autosetup -n breeze-icons-%{version} -p1

%if 0%{?kf5_version:1}
sed -i -e "s|%{version}|%{kf5_version}|g" CMakeLists.txt
%endif


%build
mkdir %{_target_platform}
pushd %{_target_platform}
%{cmake_kf5} ..
popd

%make_build -C %{_target_platform}


%install
make install/fast DESTDIR=%{buildroot} -C %{_target_platform}

## icon optimizations
#du -s  .
#time optimizegraphics ||:
du -s .
hardlink -c -v %{buildroot}%{_datadir}/icons/
du -s .

# %%ghost icon.cache
touch  %{buildroot}%{_kf5_datadir}/icons/{breeze,breeze-dark}/icon-theme.cache


%check
# verify dark inheritence, notorious for getting lost
test "$(grep '^Inherits=' %{buildroot}%{_datadir}/icons/breeze-dark/index.theme)" = "Inherits=breeze"


%if 0%{?fedora} > 25 || 0%{?rhel} > 7
## trigger-based scriptlets
%transfiletriggerin -- %{_datadir}/icons/breeze
gtk-update-icon-cache --force %{_datadir}/icons/breeze &>/dev/null || :

%transfiletriggerin -- %{_datadir}/icons/breeze-dark
gtk-update-icon-cache --force %{_datadir}/icons/breeze-dark &>/dev/null || :

%transfiletriggerpostun -- %{_datadir}/icons/breeze
gtk-update-icon-cache --force %{_datadir}/icons/breeze &>/dev/null || :

%transfiletriggerpostun -- %{_datadir}/icons/breeze-dark
gtk-update-icon-cache --force %{_datadir}/icons/breeze-dark &>/dev/null || :

%else
## classic scriptlets
%post
touch --no-create %{_datadir}/icons/breeze &> /dev/null || :
touch --no-create %{_datadir}/icons/breeze-dark &> /dev/null || :

%posttrans
gtk-update-icon-cache %{_datadir}/icons/breeze &> /dev/null || :
gtk-update-icon-cache %{_datadir}/icons/breeze-dark &> /dev/null || :

%postun
if [ $1 -eq 0 ] ; then
  touch --no-create %{_datadir}/icons/breeze &> /dev/null || :
  gtk-update-icon-cache %{_datadir}/icons/breeze &> /dev/null || :
  touch --no-create %{_datadir}/icons/breeze-dark &> /dev/null || :
  gtk-update-icon-cache %{_datadir}/icons/breze-dark &> /dev/null || :
fi
%endif

%files
%license COPYING-ICONS
%doc README.md
%ghost %{_datadir}/icons/breeze/icon-theme.cache
%ghost %{_datadir}/icons/breeze-dark/icon-theme.cache
%{_datadir}/icons/breeze/
%{_datadir}/icons/breeze-dark/
%exclude %{_datadir}/icons/breeze/breeze-icons.rcc
%exclude %{_datadir}/icons/breeze-dark/breeze-icons-dark.rcc

%files rcc
%{_datadir}/icons/breeze/breeze-icons.rcc
%{_datadir}/icons/breeze-dark/breeze-icons-dark.rcc


%changelog
* Mon Nov 11 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.64.0-1
- 5.64.0

* Sun Oct 13 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.63.0-1
- 5.63.0

* Sun Sep 15 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.62.0-1
- 5.62.0

* Wed Aug 07 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.61.0-1
- 5.61.0

* Wed Jul 24 2019 Fedora Release Engineering <releng@fedoraproject.org> - 5.60.0-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_31_Mass_Rebuild

* Sat Jul 13 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.60.0-1
- 5.60.0

* Thu Jun 06 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.59.0-1
- 5.59.0

* Tue May 07 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.58.0-1
- 5.58.0

* Sat Apr 13 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.57.0-2
- simplify scriptlets (bug #1699280)

* Mon Apr 08 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.57.0-1
- 5.57.0

* Tue Mar 05 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.56.0-1
- 5.56.0

* Mon Feb 04 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.55.0-1
- 5.55.0

* Thu Jan 31 2019 Fedora Release Engineering <releng@fedoraproject.org> - 5.54.0-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_30_Mass_Rebuild

* Tue Jan 08 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.54.0-1
- 5.54.0

* Sun Dec 09 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.53.0-1
- 5.53.0

* Sun Nov 04 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.52.0-1
- 5.52.0

* Wed Oct 10 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.51.0-1
- 5.51.0

* Tue Sep 04 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.50.0-1
- 5.50.0

* Tue Aug 07 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.49.0-1
- 5.49.0

* Thu Jul 12 2018 Fedora Release Engineering <releng@fedoraproject.org> - 5.48.0-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_29_Mass_Rebuild

* Sun Jul 08 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.48.0-1
- 5.48.0

* Sat Jun 02 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.47.0-1
- 5.47.0

* Sat May 05 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.46.0-1
- 5.46.0

* Sun Apr 08 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.45.0-1
- 5.45.0

* Sat Mar 03 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.44.0-1
- 5.44.0

* Wed Feb 07 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.43.0-1
- 5.43.0

* Wed Feb 07 2018 Fedora Release Engineering <releng@fedoraproject.org> - 5.42.0-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_28_Mass_Rebuild

* Fri Jan 19 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.42.0-2
- Requires: hicolor-icon-theme

* Mon Jan 08 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.42.0-1
- 5.42.0

* Mon Dec 04 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.41.0-1
- 5.41.0

* Fri Nov 10 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.40.0-1
- 5.40.0

* Wed Oct 25 2017 Troy Dawson <tdawson@redhat.com> - 5.39.0-2
- Cleanup spec file conditionals

* Sun Oct 08 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.39.0-1
- 5.39.0

* Wed Sep 27 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.38.0-2
- use scriptlet triggers (f26+)

* Sun Sep 03 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.38.0-1
- 5.38.0

* Fri Aug 25 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.37.0-1
- 5.37.0

* Wed Jul 26 2017 Fedora Release Engineering <releng@fedoraproject.org> - 5.36.0-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Mass_Rebuild

* Mon Jul 03 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.36.0-1
- 5.36.0

* Sun Jun 04 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.35.0-1
- 5.35.0

* Sat May 20 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.34.0-2
- summary typo

* Mon May 15 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.34.0-1
- 5.34.0

* Tue Apr 25 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.33.0-2
- -rcc subpkg
- drop creating/owning extra dirs (layout has changed since)
- .spec cosmetics

* Mon Apr 03 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.33.0-1
- 5.33.0

* Sat Mar 04 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.32.0-1
- 5.32.0

* Mon Feb 06 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.31.0-1
- 5.31.0, update URL

* Fri Dec 16 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.29.0-1
- 5.29.0

* Fri Nov 18 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.27.0-2
- Breeze Dark is listed as Breeze in System Settings (kde#370213)

* Mon Oct 03 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.27.0-1
- 5.27.0

* Wed Sep 07 2016 Daniel Vrátil <dvratil@fedoraproject.org> - 5.26.0-1
- KDE Frameworks 5.26.0

* Sun Aug 07 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.25.0-1
- 5.25.0

* Wed Jul 06 2016 Daniel Vrátil <dvratil@fedoraproject.org> - 5.24.0-1
- KDE Frameworks 5.24.0

* Tue Jun 07 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.23.0-1
- 5.23.0

* Mon May 16 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.22.0-2
- backport fix for gtk warnings (kde#362041)

* Mon May 16 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.22.0-1
- KDE Frameworks 5.22.0

* Thu Apr 14 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.21.0-3
- Obsoletes: breeze-icon-theme < 5.17.0

* Thu Apr 14 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.21.0-2
- make Requires: breeze-cursor-theme fedora only
- update URL, -BR: cmake

* Mon Apr 04 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.21.0-1
- KDE Frameworks 5.21.0

* Mon Mar 14 2016 Daniel Vrátil <dvratil@fedoraproject.org> - 5.20.0-1
- KDE Frameworks 5.20.0

* Thu Feb 11 2016 Daniel Vrátil <dvratil@fedoraproject.org> - 5.19.0-1
- KDE Frameworks 5.19.0

* Wed Feb 03 2016 Fedora Release Engineering <releng@fedoraproject.org> - 5.18.0-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_24_Mass_Rebuild

* Sun Jan 03 2016 Daniel Vrátil <dvratil@fedoraproject.org> - 5.18.0-1
- KDE Frameworks 5.18.0

* Tue Dec 08 2015 Daniel Vrátil <dvratil@fedoraproject.org> - 5.17.0-1
- KDE Frameworks 5.17.0

* Sun Nov 15 2015 Rex Dieter <rdieter@fedoraproject.org> - 5.16.0-2
- Requires: breeze-cursor-theme (#1282203)
- use hardlink optimizer
- fix breeze dark inheritance (again)

* Sun Nov 08 2015 Daniel Vrátil <dvratil@fedoraproject.org> - 5.16.0-1
- KDE Frameworks 5.16.0

* Sun Nov 08 2015 Daniel Vrátil <dvratil@fedoraproject.org> - 5.16.0-1
- Moved to KDE Frameworks 5
