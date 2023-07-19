# workaround for bz#1546230
# ocaml doesn't support relocation
%undefine _hardened_build

%if ! 0%{?bootstrap}
# The Kalzium solver needs OCaml with native compilation support (ocamlopt) and
# ocaml-facile.
%ifarch %{?ocaml_native_compiler}
%global with_facile 1
%endif

# pending fix for https://bugzilla.redhat.com/1544510
%global avogadro 1
%endif

Name:    kalzium
Summary: Periodic Table of Elements
Version: 23.04.3
Release: 1%{?dist}

License: GPLv2+
URL:     https://edu.kde.org/kalzium/

%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0: https://download.kde.org/%{stable}/release-service/%{version}/src/%{name}-%{version}.tar.xz
Source1: https://download.kde.org/%{stable}/release-service/%{version}/src/%{name}-%{version}.tar.xz.sig
Source2: gpgkey-D81C0CB38EB725EF6691C385BB463350D6EF31EF.gpg

## upstream patches

%if ! 0%{?bootstrap}
BuildRequires: gnupg2
BuildRequires: libappstream-glib
BuildRequires: chemical-mime-data
BuildRequires: desktop-file-utils
%endif

# kf5
BuildRequires: extra-cmake-modules
BuildRequires: kf5-rpm-macros
BuildRequires: cmake(KF5Archive)
BuildRequires: cmake(KF5Config)
BuildRequires: cmake(KF5CoreAddons)
BuildRequires: cmake(KF5DocTools)
BuildRequires: cmake(KF5I18n)
BuildRequires: cmake(KF5KDELibs4Support)
BuildRequires: cmake(KF5KHtml)
BuildRequires: cmake(KF5NewStuff)
BuildRequires: cmake(KF5Parts)
BuildRequires: cmake(KF5Plotting)
BuildRequires: cmake(KF5Solid)
BuildRequires: cmake(KF5UnitConversion)
BuildRequires: cmake(KF5WidgetsAddons)

BuildRequires: cmake(Qt5Widgets)
BuildRequires: cmake(Qt5Script)
BuildRequires: cmake(Qt5Quick)
BuildRequires: cmake(Qt5Gui)
BuildRequires: cmake(Qt5OpenGL)
BuildRequires: cmake(Qt5Svg)

%if 0%{?fedora} >= 37
BuildRequires: pkgconfig(openbabel-3)
%else
BuildRequires: pkgconfig(openbabel-2.0)
%endif

%if 0%{?avogadro}
# Eigen is only used for the Avogadro-based compound viewer.
BuildRequires: pkgconfig(eigen3)
BuildRequires: cmake(AvogadroLibs)
# workaround missing dep in avogadro2-libs-devel for now
BuildRequires: glew-devel
BuildRequires: spglib-devel
%endif
%if 0%{?with_facile}
# OCaml is only used with the Facile library, in the equation balancer.
BuildRequires: ocaml(compiler)
BuildRequires: ocaml-facile-devel
%endif

Requires: %{name}-libs%{?_isa} = %{version}-%{release}

# build.log wants this one, but it is really only used at runtime:
Requires: chemical-mime-data

%description
%{summary}.

%package  libs
Summary:  Runtime files for %{name}
Requires: %{name} = %{version}-%{release}
%description libs
%{summary}.

%package devel
Summary:  Development files for %{name}
Requires: %{name}-libs%{?_isa} = %{version}-%{release}
%description devel
%{summary}.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -p1


%build
%cmake_kf5
%cmake_build


%install
%cmake_install
%find_lang %{name} --all-name --with-html --with-man


%check
%if !0%{?bootstrap}
appstream-util validate-relax --nonet %{buildroot}%{_kf5_metainfodir}/org.kde.kalzium.appdata.xml
desktop-file-validate %{buildroot}%{_kf5_datadir}/applications/org.kde.kalzium.desktop
desktop-file-validate %{buildroot}%{_kf5_datadir}/applications/org.kde.kalzium_cml.desktop
%endif


%files -f %{name}.lang
%license LICENSES/*.txt
%{_kf5_bindir}/kalzium
%{_kf5_datadir}/kalzium/
%{_kf5_metainfodir}/org.kde.kalzium.appdata.xml
%{_kf5_datadir}/applications/org.kde.kalzium.desktop
%{_kf5_datadir}/applications/org.kde.kalzium_cml.desktop
%{_kf5_datadir}/config.kcfg/kalzium.kcfg
%{_kf5_datadir}/icons/hicolor/*/*/kalzium.*
%{_mandir}/man1/kalzium.*
%dir %{_kf5_datadir}/libkdeedu/
%{_kf5_datadir}/libkdeedu/data/
%{_kf5_datadir}/knsrcfiles/kalzium.knsrc
%{_kf5_datadir}/qlogging-categories5/%{name}.categories

%{?ldconfig_scriptlets:%ldconfig_scriptlets libs}

%files libs
%if 0%{?avogadro}
%{_kf5_libdir}/libcompoundviewer.so.5*
%endif
%{_kf5_libdir}/libscience.so.5*

%files devel
%dir %{_includedir}/libkdeedu/
%{_includedir}/libkdeedu/*.h
%if 0%{?avogadro}
%{_kf5_libdir}/libcompoundviewer.so
%endif
%{_kf5_libdir}/libscience.so


%changelog
* Tue Jul 18 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.04.3-1
- 23.04.3

* Thu Jun 08 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.04.2-1
- 23.04.2

* Thu May 11 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.04.1-1
- 23.04.1

* Fri Apr 21 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.04.0-1
- 23.04.0

* Thu Mar 02 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.12.3-1
- 22.12.3

* Thu Jan 05 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.12.1-1
- 22.12.1

