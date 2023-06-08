Name:    umbrello
Summary: UML modeler and UML diagram tool
Version: 23.04.2
Release: 1%{?dist}

License: GPLv2+ and LGPLv2+
URL:     https://www.kde.org/applications/development/umbrello/

%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0: http://download.kde.org/%{stable}/release-service/%{version}/src/%{name}-%{version}.tar.xz
Source1: https://download.kde.org/%{stable}/release-service/%{version}/src/%{name}-%{version}.tar.xz.sig
Source2: gpgkey-D81C0CB38EB725EF6691C385BB463350D6EF31EF.gpg

## upstream patches

BuildRequires:  gnupg2
BuildRequires: desktop-file-utils
BuildRequires: boost-devel
BuildRequires: pkgconfig(libxslt)
BuildRequires: libappstream-glib

BuildRequires: extra-cmake-modules
BuildRequires: kf5-rpm-macros
BuildRequires: cmake(KF5Archive)
BuildRequires: cmake(KF5Completion)
BuildRequires: cmake(KF5Config)
BuildRequires: cmake(KF5CoreAddons)
BuildRequires: cmake(KF5Crash)
BuildRequires: cmake(KF5DocTools)
BuildRequires: cmake(KF5I18n)
BuildRequires: cmake(KF5IconThemes)
BuildRequires: cmake(KF5KDELibs4Support)
BuildRequires: cmake(KF5KIO)
BuildRequires: cmake(KF5Parts)
BuildRequires: cmake(KF5TextEditor)
BuildRequires: cmake(KF5WidgetsAddons)
BuildRequires: cmake(KF5WindowSystem)
BuildRequires: cmake(KF5XmlGui)

BuildRequires: cmake(Qt5Gui)
BuildRequires: cmake(Qt5PrintSupport)
BuildRequires: cmake(Qt5Svg)
BuildRequires: cmake(Qt5Test)
BuildRequires: cmake(Qt5Widgets)
BuildRequires: cmake(Qt5Xml)
BuildRequires: cmake(Qt5WebKitWidgets)

%ifnarch s390x
BuildRequires: cmake(KDevelop-PG-Qt)
BuildRequires: cmake(KDevPlatform)
%endif
BuildRequires: cmake(KF5ThreadWeaver)
BuildRequires: cmake(KF5KCMUtils)

BuildRequires: cmake(LLVM)
BuildRequires: cmake(Clang)
BuildRequires: llvm-devel

# API doc generation - for later use
#BuildRequires: doxygen
#BuildRequires: cmake(Qt5Help)

Conflicts:      kdesdk-common < 4.10.80
Provides:       kdesdk-umbrello = %{version}-%{release}
Obsoletes:      kdesdk-umbrello < 4.10.80

Conflicts: kde-l10n < 17.08.3-2


%description
GUI for diagramming Unified Modeling Language (UML)


%prep
%autosetup -p1


%build
%cmake_kf5
%cmake_build


%install
%cmake_install
%find_lang %{name} --all-name --with-html


%check
appstream-util validate-relax --nonet %{buildroot}%{_kf5_metainfodir}/org.kde.umbrello.appdata.xml ||:
desktop-file-validate %{buildroot}%{_kf5_datadir}/applications/org.kde.umbrello.desktop


%files -f %{name}.lang
%doc README
%license COPYING
%{_kf5_bindir}/umbrello5
%{_kf5_bindir}/po2xmi5
%{_kf5_bindir}/xmi2pot5
%{_kf5_metainfodir}/org.kde.umbrello.appdata.xml
%{_kf5_datadir}/applications/org.kde.umbrello.desktop
%{_kf5_datadir}/umbrello5/
%{_kf5_datadir}/icons/hicolor/*/*/*


%changelog
* Thu Jun 08 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.04.2-1
- 23.04.2

* Thu May 11 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.04.1-1
- 23.04.1

* Thu Apr 20 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.04.0-1
- 23.04.0

* Thu Mar 02 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.12.3-1
- 22.12.3

* Thu Feb 02 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.12.2-1
- 22.12.2

