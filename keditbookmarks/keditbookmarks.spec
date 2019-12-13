
Name:    keditbookmarks
Summary: Bookmark organizer and editor
Version: 19.12.0
Release: 1%{?dist}

# Documentation is GFDL, rest GPLv2 and GPLv3 (note: NOT any later version)
License: GPLv2 and GPLv3 and GFDL
URL:     https://www.kde.org/

%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0: https://download.kde.org/stable/release-service/%{version}/src/%{name}-%{version}.tar.xz

BuildRequires: desktop-file-utils

BuildRequires: extra-cmake-modules
BuildRequires: kf5-rpm-macros
BuildRequires: cmake(KF5Bookmarks)
BuildRequires: cmake(KF5CoreAddons)
BuildRequires: cmake(KF5DocTools)
BuildRequires: cmake(KF5I18n)
BuildRequires: cmake(KF5IconThemes)
BuildRequires: cmake(KF5KIO)
BuildRequires: cmake(KF5Parts)
BuildRequires: cmake(KF5WindowSystem)

BuildRequires: cmake(Qt5Core)

# translations moved here
Conflicts: kde-l10n < 17.03

Requires:      %{name}-libs%{?_isa} = %{version}-%{release}

%description
keditbookmarks is a bookmark organizer and editor.


%package libs
Summary:       Runtime libraries for %{name}
Requires:      %{name} = %{version}-%{release}
%description libs
%{summary}.


%prep
%autosetup -p1


%build
mkdir %{_target_platform}
pushd %{_target_platform}
%{cmake_kf5} ..
popd

%make_build -C %{_target_platform}


%install
make install/fast DESTDIR=%{buildroot} -C %{_target_platform}

%find_lang %{name} --all-name --with-html --with-man

## unpackaged files
rm -fv %{buildroot}%{_kf5_libdir}/libkbookmarkmodel_private.so


%check
desktop-file-validate %{buildroot}%{_kf5_datadir}/applications/org.kde.%{name}.desktop


%files -f %{name}.lang
%license COPYING*
%{_kf5_bindir}/keditbookmarks
%{_kf5_bindir}/kbookmarkmerger
%{_kf5_datadir}/applications/org.kde.%{name}.desktop
%{_kf5_datadir}/config.kcfg/keditbookmarks.kcfg
%{_kf5_datadir}/kxmlgui5/keditbookmarks/
%{_mandir}/man1/kbookmarkmerger.1*

%ldconfig_scriptlets libs

%files libs
%{_kf5_libdir}/libkbookmarkmodel_private.so.*


%changelog
* Thu Dec 12 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 19.12.0-1
- 19.12.0

* Fri Nov 08 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 19.08.3-1
- 19.08.3

* Thu Oct 10 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 19.08.2-1
- 19.08.2

* Thu Sep 05 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 19.08.1-1
- 19.08.1

* Thu Aug 15 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 19.08.0-1
- 19.08.0

* Thu Jul 11 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 19.04.3-1
- 19.04.3

* Thu Jun 06 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 19.04.2-1
- 19.04.2

* Thu May 09 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 19.04.1-1
- 19.04.1

* Sun Apr 28 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 19.04.0-1
- 19.04.0

* Tue Feb 05 2019 Rex Dieter <rdieter@fedoraproject.org> - 18.12.2-1
- 18.12.2

* Fri Feb 01 2019 Fedora Release Engineering <releng@fedoraproject.org> - 18.12.1-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_30_Mass_Rebuild

* Tue Jan 08 2019 Rex Dieter <rdieter@fedoraproject.org> - 18.12.1-1
- 18.12.1

* Sat Dec 08 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.12.0-1
- 18.12.0

* Tue Nov 06 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.08.3-1
- 18.08.3

* Wed Oct 10 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.08.2-1
- 18.08.2

* Fri Sep 07 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.08.1-1
- 18.08.1

* Wed Aug 15 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.08.0-1
- 18.08.0

* Thu Jul 12 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.04.3-1
- 18.04.3

* Tue Jun 05 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.04.2-1
- 18.04.2

* Tue May 08 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.04.1-1
- 18.04.1

* Sat Apr 14 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.04.0-1
- 18.04.0

* Tue Mar 06 2018 Rex Dieter <rdieter@fedoraproject.org> - 17.12.3-1
- 17.12.3

* Tue Feb 06 2018 Rex Dieter <rdieter@fedoraproject.org> - 17.12.2-1
- 17.12.2

* Thu Jan 11 2018 Rex Dieter <rdieter@fedoraproject.org> - 17.12.1-1
- 17.12.1

* Tue Dec 12 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.12.0-1
- 17.12.0

* Wed Nov 08 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.08.3-1
- 17.08.3

* Wed Oct 11 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.08.2-1
- 17.08.2

* Tue Sep 05 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.08.1-1
- 17.08.1

* Sun Aug 27 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.08.0-2
- cosmetics

* Sat Aug 26 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.08.0-1
- 17.08.0

* Fri Jul 28 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.04.3-1
- 17.04.3

* Wed Jul 26 2017 Fedora Release Engineering <releng@fedoraproject.org> - 17.04.2-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Mass_Rebuild

* Thu Jun 15 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.04.2-1
- 17.04.2

* Wed May 10 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.04.1-1
- 17.04.1

* Sat Apr 22 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.04.0-2
- use %%find_lang for handbooks

* Tue Apr 18 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.04.0-1
- 17.04.0

* Wed Mar 08 2017 Rex Dieter <rdieter@fedoraproject.org> - 16.12.3-1
- 16.12.3

* Wed Feb 08 2017 Rex Dieter <rdieter@fedoraproject.org> - 16.12.2-1
- 16.12.2

* Sat Jan 28 2017 Rex Dieter <rdieter@fedoraproject.org> - 16.12.1-1
- import

* Fri Jan 20 2017 Christian Dersch <lupinix@mailbox.org> - 16.12.1-0.1
- initial packaging effort
