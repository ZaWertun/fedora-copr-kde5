%global framework kguiaddons

Name:           kf5-%{framework}
Version:        5.111.0
Release:        1%{?dist}
Summary:        KDE Frameworks 5 Tier 1 addon with various classes on top of QtGui

License:        GPLv2+ and LGPLv2+
URL:            https://cgit.kde.org/%{framework}.git

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
BuildRequires:  desktop-file-utils
BuildRequires:  extra-cmake-modules >= %{majmin}
BuildRequires:  kf5-rpm-macros >= %{majmin}
BuildRequires:  libX11-devel
BuildRequires:  libxcb-devel
BuildRequires:  qt5-qtbase-devel
BuildRequires:  qt5-qtx11extras-devel
BuildRequires:  qt5-qtbase-private-devel

BuildRequires:  wayland-devel
BuildRequires:  qt5-qtwayland-devel
BuildRequires:  cmake(PlasmaWaylandProtocols) >= 1.7.0

Requires:       kf5-filesystem >= %{majmin}

%description
KDBusAddons provides convenience classes on top of QtGui.

%package        devel
Summary:        Development files for %{name}
Requires:       %{name}%{?_isa} = %{version}-%{release}
Requires:       qt5-qtbase-devel
%description    devel
The %{name}-devel package contains libraries and header files for
developing applications that use %{name}.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -n %{framework}-%{version}


%build
%cmake_kf5
%cmake_build


%install
%cmake_install


%check
desktop-file-validate %{buildroot}%{_kf5_datadir}/applications/google-maps-geo-handler.desktop ||:
desktop-file-validate %{buildroot}%{_kf5_datadir}/applications/openstreetmap-geo-handler.desktop ||:
desktop-file-validate %{buildroot}%{_kf5_datadir}/applications/qwant-maps-geo-handler.desktop ||:
desktop-file-validate %{buildroot}%{_kf5_datadir}/applications/wheelmap-geo-handler.desktop ||:


%ldconfig_scriptlets

%files
%doc README.md
%license LICENSES/*.txt
%{_kf5_bindir}/kde-geo-uri-handler
%{_kf5_libdir}/libKF5GuiAddons.so.*
%{_kf5_datadir}/applications/google-maps-geo-handler.desktop
%{_kf5_datadir}/applications/openstreetmap-geo-handler.desktop
%{_kf5_datadir}/applications/qwant-maps-geo-handler.desktop
%{_kf5_datadir}/applications/wheelmap-geo-handler.desktop
%{_kf5_datadir}/qlogging-categories5/kguiaddons.categories

%files devel
%{_kf5_includedir}/KGuiAddons/
%{_kf5_libdir}/libKF5GuiAddons.so
%{_kf5_libdir}/cmake/KF5GuiAddons/
%{_kf5_archdatadir}/mkspecs/modules/qt_KGuiAddons.pri


%changelog
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

