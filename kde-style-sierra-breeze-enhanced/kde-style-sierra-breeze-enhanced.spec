Name:    kde-style-sierra-breeze-enhanced
Version: 1.3.2
Release: 1%{?dist}
Summary: A fork of BreezeEnhanced to make it (arguably) more minimalistic and informative

License: GPLv3
URL:     https://github.com/kupiqu/SierraBreezeEnhanced

%global real_version %(echo %{version}| tr -d '~' |tr [a-z] [A-Z])
Source0: https://github.com/kupiqu/SierraBreezeEnhanced/archive/V%{real_version}/SierraBreezeEnhanced-%{real_version}.tar.gz

BuildRequires: cmake
BuildRequires: gcc-c++
BuildRequires: fftw-devel
BuildRequires: desktop-file-utils
BuildRequires: extra-cmake-modules
BuildRequires: cmake(Qt5Core)
BuildRequires: cmake(Qt5Gui)
BuildRequires: cmake(Qt5DBus)
BuildRequires: cmake(Qt5X11Extras)
BuildRequires: cmake(KF5GuiAddons)
BuildRequires: cmake(KF5WindowSystem)
BuildRequires: cmake(KF5I18n)
BuildRequires: cmake(KDecoration2)
BuildRequires: cmake(KF5CoreAddons)
BuildRequires: cmake(KF5ConfigWidgets)
BuildRequires: cmake(KF5IconThemes)


%description
%{summary}.


%prep
%setup -qn SierraBreezeEnhanced-%{real_version}


%build
sed -i "s|0.4.4|0.4.5|" CMakeLists.txt
%cmake_kf5
%cmake_build


%install
%cmake_install
#desktop-file-validate %{buildroot}/%{_datadir}/kservices5/sierrabreezeenhancedconfig.desktop

%post -p /sbin/ldconfig
%postun -p /sbin/ldconfig

%files
%doc LICENSE README.md
%{_libdir}/libsierrabreezeenhancedcommon5.so.0
%{_libdir}/libsierrabreezeenhancedcommon5.so.%{version}
%{_libdir}/qt5/plugins/org.kde.kdecoration2/sierrabreezeenhanced.so
%{_datadir}/kservices5/sierrabreezeenhancedconfig.desktop


%changelog
* Mon Mar 20 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 1.3.2-1
- version 1.3.2

* Sat Feb 04 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 1.3.1-1
- version 1.3.1

* Wed Aug 31 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 1.2.0-1
- version 1.2.0

* Sun Apr 11 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 1.0.0-1
- version 1.0.0

* Wed Mar 25 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 0.9~rc-1
- version 0.9rc

* Tue Feb 04 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 0.8.10-1
- version 0.8.10

* Tue Dec 10 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 0.8.4-1
- version 0.8.4

* Tue Dec 03 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 0.7.7-1
- version 0.7.7

* Mon Dec 02 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 0.7.6-1
- version 0.7.6

* Sun Dec 01 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 0.7.4-1
- version 0.7.4

* Sun Dec 01 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 0.7.3-1
- version 0.7.3

* Sun Dec 01 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 0.7.2-1
- version 0.7.2

* Thu Nov 28 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 0.7.0-1
- version 0.7.0

* Mon Nov 25 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 0.6.3-1
- version 0.6.3

* Sun Nov 24 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 0.6.2-1
- version 0.6.2

* Sat Nov 23 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 0.6.0-1
- version 0.6.0

* Thu Nov 21 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 0.5.3-1
- version 0.5.3

* Wed Nov 20 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 0.5.1-1
- version 0.5.1

* Mon Jul 15 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 0.4.6-6
- version 0.4.6

* Sun Jan 06 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 0.4.5-5
- version 0.4.5

* Mon Dec 17 2018 Yaroslav Sidlovsky <zawertun@gmail.com> - 0.4.4-4
- version 0.4.4

* Wed Dec 12 2018 Yaroslav Sidlovsky <zawertun@gmail.com> - 0.4.3-3
- version 0.4.3

* Thu Dec 06 2018 Yaroslav Sidlovsky <zawertun@gmail.com> - 0.4-2
- version 0.4

* Fri Nov 30 2018 Yaroslav Sidlovsky <zawertun@gmail.com> - 0.3-1
- first spec for version 0.3

