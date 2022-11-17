%global framework qqc2-desktop-style

Name:    %{framework}
Version: 5.100.0
Release: 5%{?dist}
Summary: QtQuickControls2 style for consistency between QWidget and QML apps 

# kirigami-plasmadesktop-integration: LGPLv2+
# plugin,org.kde.desktop: LGPLv3 or GPLv3
License: (LGPLv3 or GPLv3) and LGPLv2+
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

BuildRequires: gnupg2
BuildRequires: extra-cmake-modules >= %{majmin}
BuildRequires: kf5-rpm-macros
BuildRequires: kf5-kirigami2-devel >= %{majmin}
Requires: kf5-kirigami2%{?_isa} >= %{majmin}
BuildRequires: kf5-kconfigwidgets-devel >= %{majmin}
BuildRequires: kf5-kiconthemes-devel >= %{majmin}
# cmake() style
BuildRequires: cmake(KF5Kirigami2)
BuildRequires: cmake(KF5ConfigWidgets)
BuildRequires: cmake(KF5IconThemes)

BuildRequires: pkgconfig(Qt5Gui)
BuildRequires: pkgconfig(Qt5Quick)
BuildRequires: pkgconfig(Qt5Widgets)
# not sure if this is *really* needed or not -- rex
# CMakeLists.txt:# When building as a static plugin, dependencies may add a -lQt5X11Extras
BuildRequires: pkgconfig(Qt5X11Extras)

BuildRequires: qt5-qtquickcontrols2-devel
Requires: qt5-qtquickcontrols2%{?_isa}

%description
This is a style for QtQuickControls 2 that uses QWidget's QStyle for
painting, making possible to achieve an higher degree of consistency
between QWidget-based and QML-based apps.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -n %{framework}-%{version} -p1


%build
%cmake_kf5
%cmake_build


%install
%cmake_install


%files 
%doc README.md
%license LICENSE* 
%dir %{_kf5_plugindir}/kirigami/
%{_kf5_plugindir}/kirigami/org.kde.desktop.so
%{_qt5_qmldir}/QtQuick/Controls.2/org.kde.desktop/
%{_qt5_qmldir}/org/kde/qqc2desktopstyle/
# yes, here
%{_kf5_libdir}/cmake/KF5QQC2DeskopStyle/
%{_kf5_libdir}/cmake/KF5QQC2DesktopStyle/


%changelog
* Thu Nov 17 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.100.0-5
- rebuild

* Mon Nov 14 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.100.0-1
- 5.100.0

* Mon Oct 10 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.99.0-1
- 5.99.0

* Fri Sep 23 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.98.0-2
- rebuild

* Mon Sep 12 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.98.0-1
- 5.98.0

* Sun Aug 14 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.97.0-1
- 5.97.0

* Sun Jul 10 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.96.0-1
- 5.96.0

* Mon Jun 13 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.95.0-1
- 5.95.0

* Sat May 14 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.94.0-1
- 5.94.0

* Sun Apr 10 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.93.0-1
- 5.93.0

* Sun Mar 13 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.92.0-1
- 5.92.0

* Mon Feb 14 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.91.0-1
- 5.91.0

* Sat Jan 08 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.90.0-1
- 5.90.0

* Mon Dec 13 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.89.0-1
- 5.89.0

* Sat Nov 13 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.88.0-1
- 5.88.0

* Sat Oct 09 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.87.0-1
- 5.87.0

* Sat Sep 11 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.86.0-1
- 5.86.0

* Sat Aug 14 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.85.0-1
- 5.85.0

* Sat Jul 10 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.84.0-1
- 5.84.0

* Sun Jun 13 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.83.0-1
- 5.83.0

* Sat May 08 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.82.0-1
- 5.82.0

* Sun Apr 11 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.81.0-1
- 5.81.0

* Sat Mar 13 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.80.0-1
- 5.80.0

* Sat Feb 13 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.79.0-1
- 5.79.0

* Thu Feb 11 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.78.0-2
- Added patch that fixes font rendering, corresponding merge request:
  https://invent.kde.org/frameworks/qqc2-desktop-style/-/merge_requests/52

* Sat Jan  9 16:30:36 MSK 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.78.0-1
- 5.78.0

* Mon Dec 14 16:50:28 MSK 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.77.0-1
- 5.77.0

* Sat Dec  5 13:32:37 MSK 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.76.0-2
- rebuild

* Sun Nov 15 22:13:58 MSK 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.76.0-1
- 5.76.0

* Sat Oct 10 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.75.0-1
- 5.75.0

* Sun Sep 27 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.74.0-2
- rebuild

* Thu Sep 17 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.74.0-1
- 5.74.0

* Mon Aug 10 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.73.0-1
- 5.73.0

* Mon Jul 13 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.72.0-1
- 5.72.0

* Thu Jun 18 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.71.0-2
- rebuild

* Sun Jun 14 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.71.0-1
- 5.71.0

* Sun May 10 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.70.0-1
- 5.70.0

* Sat Apr 11 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.69.0-1
- 5.69.0

* Mon Mar 16 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.68.0-1
- 5.68.0

* Thu Feb 27 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.67.1-2
- rebuild

* Tue Feb 11 2020 Rex Dieter <rdieter@fedoraproject.org> - 5.67.1-1
- 5.67.1

* Mon Feb 03 2020 Rex Dieter <rdieter@fedoraproject.org> - 5.67.0-1
- 5.67.0

* Thu Jan 30 2020 Fedora Release Engineering <releng@fedoraproject.org> - 5.66.0-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_32_Mass_Rebuild

* Tue Jan 07 2020 Rex Dieter <rdieter@fedoraproject.org> - 5.66.0-1
- 5.66.0

* Tue Dec 17 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.65.0-1
- 5.65.0

* Fri Nov 08 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.64.0-1
- 5.64.0

* Tue Oct 22 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.63.0-1
- 5.63.0

* Mon Sep 16 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.62.0-1
- 5.62.0

* Wed Aug 07 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.61.0-1
- 5.61.0

* Fri Jul 26 2019 Fedora Release Engineering <releng@fedoraproject.org> - 5.60.0-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_31_Mass_Rebuild

* Sat Jul 13 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.60.0-1
- 5.60.0

* Thu Jun 06 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.59.0-1
- 5.59.0

* Tue May 07 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.58.0-1
- 5.58.0

* Tue Apr 09 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.57.0-1
- 5.57.0

* Tue Mar 05 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.56.0-1
- 5.56.0

* Mon Feb 04 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.55.0-1
- 5.55.0

* Sat Feb 02 2019 Fedora Release Engineering <releng@fedoraproject.org> - 5.54.0-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_30_Mass_Rebuild

* Fri Jan 18 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.54.0-2
- add versioned runtime dep for kf5-kirigami2

* Wed Jan 09 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.54.0-1
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

* Sat Jul 14 2018 Fedora Release Engineering <releng@fedoraproject.org> - 5.48.0-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_29_Mass_Rebuild

* Mon Jul 09 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.48.0-1
- 5.48.0

* Sat Jun 02 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.47.0-1
- 5.47.0

* Thu May 17 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.46.2-1
- 5.46.2

* Wed May 16 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.46.1-1
- 5.46.1

* Wed May 16 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.46.0-2
- pull in upstream fixes, use %%make_build

* Sat May 05 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.46.0-1
- 5.46.0

* Sun Apr 08 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.45.0-1
- 5.45.0

* Sat Mar 03 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.44.0-1
- 5.44.0

* Wed Feb 07 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.43.0-1
- 5.43.0

* Mon Jan 08 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.42.0-1
- 5.42.0

* Mon Dec 04 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.41.0-1
- 5.41.0

* Wed Nov 15 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.40.0-1
- 5.40.0

* Wed Oct 25 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.39.0-1
- qqc2-desktop-style

