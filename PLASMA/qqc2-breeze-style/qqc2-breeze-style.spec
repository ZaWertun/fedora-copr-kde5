%undefine __cmake_in_source_build

Name:    qqc2-breeze-style
Version: 5.23.1
Release: 1%{?dist}
Summary: Breeze inspired QQC2 Style

License: (LGPLv3 or GPLv3) and LGPLv2+
URL:     https://invent.kde.org/plasma/%{name}

%global majmin %(echo %{version} | cut -d. -f1-2)
%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0: http://download.kde.org/%{stable}/plasma/%(echo %{version} |cut -d. -f1-3)/%{name}-%{version}.tar.xz

## upstream patches

BuildRequires: extra-cmake-modules >= %{majmin}
BuildRequires: kf5-rpm-macros

BuildRequires: cmake(KF5Config)
BuildRequires: cmake(KF5GuiAddons)
BuildRequires: cmake(KF5Kirigami2)
BuildRequires: cmake(KF5IconThemes)
BuildRequires: cmake(KF5ConfigWidgets)

BuildRequires: pkgconfig(x11)
BuildRequires: pkgconfig(Qt5Gui)
BuildRequires: pkgconfig(Qt5Qml)
BuildRequires: pkgconfig(Qt5Core)
BuildRequires: pkgconfig(Qt5DBus)
BuildRequires: pkgconfig(Qt5Quick)
BuildRequires: pkgconfig(Qt5Widgets)
BuildRequires: pkgconfig(Qt5X11Extras)

BuildRequires: qt5-qtquickcontrols2-devel
Requires: qt5-qtquickcontrols2%{?_isa}

Requires: kf5-kirigami2%{?_isa} >= %{majmin}

%description
This is a pure Qt Quick/Kirigami Qt Quick Controls style. Unlike
QQC2-Desktop-Style, it does not depend on Qt Widgets and the system QStyle.


%prep
%autosetup -n %{name}-%{version} -p1


%build
%cmake_kf5
%cmake_build


%install
%cmake_install


%files 
%doc README.md
%license LICENSES/*.txt
%{_kf5_plugindir}/kirigami/org.kde.breeze.so
%{_qt5_qmldir}/QtQuick/Controls.2/org.kde.breeze/
%{_qt5_qmldir}/org/kde/breeze/
%{_qt5_qmldir}/org/kde/kirigami.2/styles/org.kde.breeze/
%{_kf5_libdir}/cmake/KF5QQC2BreezeStyle/KF5QQC2BreezeStyleConfig*.cmake


%changelog
* Tue Oct 19 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.23.1-1
- 5.23.1

* Thu Oct 14 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.23.0-1
- 5.23.0

* Tue Aug 31 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.22.5-1
- 5.22.5

* Tue Jul 27 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.22.4-1
- 5.22.4

* Thu Jul 08 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.22.3-1
- 5.22.3

* Wed Jun 23 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.22.2.1-1
- 5.22.2.1

* Tue Jun 22 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.22.2-1
- 5.22.2

* Tue Jun 15 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.22.1-1
- 5.22.1

* Tue Jun 08 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.22.0-1
- 5.22.0

* Tue May 04 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.21.5-1
- 5.21.5

* Tue Apr 06 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.21.4-1
- 5.21.4

* Tue Mar 16 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.21.3-1
- 5.21.3

* Wed Mar 03 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.21.2-1
- 5.21.2

* Tue Feb 23 13:50:12 MSK 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.21.1-1
- 5.21.1

* Tue Feb 16 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.21.0-1
- 5.21.0

* Tue Feb 09 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.20.90-1
- first spec for version 5.20.90


