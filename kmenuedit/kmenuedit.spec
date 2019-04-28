
%global kf5_version 5.26.0

Name:    kmenuedit
Summary: KDE menu editor
Version: 5.15.4
Release: 1%{?dist}

License: GPLv2+
URL:     https://cgit.kde.org/%{name}.git

%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global majmin_ver %(echo %{version} | cut -d. -f1,2).50
%global stable unstable
%else
%global majmin_ver %(echo %{version} | cut -d. -f1,2)
%global stable stable
%endif
Source0: http://download.kde.org/%{stable}/plasma/%{version}/%{name}-%{version}.tar.xz

BuildRequires:  qt5-qtbase-devel
BuildRequires:  qt5-qtscript-devel

BuildRequires:  desktop-file-utils
BuildRequires:  extra-cmake-modules >= %{kf5_version}
BuildRequires:  kf5-rpm-macros >= %{kf5_version}
BuildRequires:  kf5-kdbusaddons-devel >= %{kf5_version}
BuildRequires:  kf5-kdelibs4support-devel >= %{kf5_version}
BuildRequires:  kf5-kdoctools-devel >= %{kf5_version}
BuildRequires:  kf5-ki18n-devel >= %{kf5_version}
BuildRequires:  kf5-kiconthemes-devel >= %{kf5_version}
BuildRequires:  kf5-kinit-devel >= %{kf5_version}
BuildRequires:  kf5-kio-devel >= %{kf5_version}
BuildRequires:  kf5-kxmlgui-devel >= %{kf5_version}
BuildRequires:  kf5-sonnet-devel >= %{kf5_version}
BuildRequires:  khotkeys-devel >= %{majmin_ver}

# libkdeinit5_*
%{?kf5_kinit_requires}

# when split out from kde-workspace-4.11.x
Conflicts:      kde-workspace < 4.11.15-3

%description
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

%find_lang kmenuedit5 --with-html --all-name


%check
desktop-file-validate %{buildroot}%{_datadir}/applications/org.kde.kmenuedit.desktop

%files -f kmenuedit5.lang
%license COPYING*
%{_bindir}/kmenuedit
%{_kf5_libdir}/libkdeinit5_kmenuedit.so
%{_datadir}/kmenuedit/
%{_datadir}/applications/org.kde.kmenuedit.desktop
%{_datadir}/icons/hicolor/*/apps/kmenuedit.*
%{_kf5_datadir}/kxmlgui5/kmenuedit/


%changelog
* Sun Apr 28 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.15.4-1
- 5.15.4

* Tue Feb 19 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.14.5-1
- 5.14.5

* Tue Nov 27 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.14.4-1
- 5.14.4

* Thu Nov 08 2018 Martin Kyral <martin.kyral@gmail.com> - 5.14.3-1
- 5.14.3

* Wed Oct 24 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.14.2-1
- 5.14.2

* Tue Oct 16 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.14.1-1
- 5.14.1

* Fri Oct 05 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.14.0-1
- 5.14.0

* Fri Sep 28 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.13.90-2
- .spec cosmetics, fix/prune changelog/sources

* Fri Sep 14 2018 Martin Kyral <martin.kyral@gmail.com> - 5.13.90-1
- 5.13.90

* Fri Jul 13 2018 Fedora Release Engineering <releng@fedoraproject.org>
- Rebuilt for https://fedoraproject.org/wiki/Fedora_29_Mass_Rebuild

* Wed Jul 11 2018 Martin Kyral <martin.kyral@gmail.com> - 5.13.3-1
- 5.13.3

* Mon Jul 09 2018 Martin Kyral <martin.kyral@gmail.com> - 5.13.2-1
- 5.13.2

* Tue Jun 19 2018 Martin Kyral <martin.kyral@gmail.com> - 5.13.1-1
- 5.13.1

* Sat Jun 09 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.13.0-1
- 5.13.0

* Fri May 18 2018 Martin Kyral <martin.kyral@gmail.com> - 5.12.90-1
- 5.12.90

* Tue May 01 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.12.5-1
- 5.12.5

* Tue Mar 27 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.12.4-1
- 5.12.4

* Tue Mar 06 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.12.3-1
- 5.12.3

* Wed Feb 21 2018 Jan Grulich <jgrulich@redhat.com> - 5.12.2-1
- 5.12.2

* Tue Feb 13 2018 Jan Grulich <jgrulich@redhat.com> - 5.12.1-1
- 5.12.1

* Fri Feb 09 2018 Igor Gnatenko <ignatenkobrain@fedoraproject.org> - 5.12.0-3
- Escape macros in %%changelog

* Wed Feb 07 2018 Fedora Release Engineering <releng@fedoraproject.org> - 5.12.0-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_28_Mass_Rebuild

* Fri Feb 02 2018 Jan Grulich <jgrulich@redhat.com> - 5.12.0-1
- 5.12.0