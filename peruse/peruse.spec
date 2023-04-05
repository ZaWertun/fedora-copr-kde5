Name:    peruse
Version: 1.80
Release: 1%{?dist}
Summary: A comic book viewer and creator

License: LGPLv2.1
URL:     https://invent.kde.org/graphics/peruse
Source0: %{url}/-/archive/v%{version}/%{name}-v%{version}.tar.bz2

BuildRequires: gettext
BuildRequires: desktop-file-utils
BuildRequires: libappstream-glib

BuildRequires: extra-cmake-modules
BuildRequires: kf5-rpm-macros

BuildRequires: qt5-qtbase-devel
BuildRequires: qt5-qtbase-private-devel
BuildRequires: qt5-qtdeclarative-devel

BuildRequires: cmake(Qt5Qml)
BuildRequires: cmake(Qt5Quick)
BuildRequires: cmake(Qt5Gui)
BuildRequires: cmake(Qt5Widgets)
BuildRequires: cmake(Qt5OpenGL)
BuildRequires: cmake(Qt5Sql)

BuildRequires: cmake(KF5Archive)
BuildRequires: cmake(KF5Config)
BuildRequires: cmake(KF5Crash)
BuildRequires: cmake(KF5Declarative)
BuildRequires: cmake(KF5GuiAddons)
BuildRequires: cmake(KF5FileMetaData)
BuildRequires: cmake(KF5I18n)
BuildRequires: cmake(KF5IconThemes)
BuildRequires: cmake(KF5KIO)
BuildRequires: cmake(KF5NewStuffQuick)
BuildRequires: cmake(KF5Attica)
BuildRequires: cmake(KF5Codecs)
BuildRequires: cmake(KF5Completion)
BuildRequires: cmake(KF5ConfigWidgets)
BuildRequires: cmake(KF5CoreAddons)
BuildRequires: cmake(KF5ItemViews)
BuildRequires: cmake(KF5JobWidgets)
BuildRequires: cmake(KF5NewStuffCore)
BuildRequires: cmake(KF5Package)
BuildRequires: cmake(KF5Service)
BuildRequires: cmake(KF5Solid)
BuildRequires: cmake(KF5WidgetsAddons)
BuildRequires: cmake(KF5WindowSystem)
BuildRequires: cmake(KF5XmlGui)

BuildRequires: cmake(KF5Kirigami2)
BuildRequires: pkgconfig(zlib)

Requires:      kf5-kirigami2%{?_isa}
Requires:      okular-kirigami%{?_isa}
Requires:      hicolor-icon-theme

%description
A comic book viewer and creator based on Frameworks 5,
for use on multiple form factors.

%package        devel
Summary:        Development files for %{name}
Requires:       %{name}%{?_isa} = %{version}-%{release}
Requires:       qt5-qtbase-devel
%description    devel
Development files for %{name}.

%prep
%autosetup -n %{name}-v%{version}


%build
%cmake_kf5
%cmake_build


%install
%cmake_install


%check
appstream-util validate-relax --nonet %{buildroot}%{_kf5_metainfodir}/org.kde.%{name}.appdata.xml
appstream-util validate-relax --nonet %{buildroot}%{_kf5_metainfodir}/org.kde.%{name}creator.appdata.xml
desktop-file-validate %{buildroot}%{_kf5_datadir}/applications/org.kde.%{name}.desktop
desktop-file-validate %{buildroot}%{_kf5_datadir}/applications/org.kde.%{name}creator.desktop


%files
%doc README.md
%license COPYING.LIB
%{_bindir}/%{name}
%{_bindir}/%{name}creator
%{_kf5_libdir}/libacbf.so.*
%{_kf5_qmldir}/org/kde/%{name}/
%{_kf5_qmldir}/org/kde/contentlist/
%{_kf5_datadir}/icons/hicolor/*/apps/%{name}*.{png,svg}
%{_kf5_datadir}/knsrcfiles/%{name}.knsrc
%{_kf5_datadir}/applications/org.kde.%{name}.desktop
%{_kf5_datadir}/applications/org.kde.%{name}creator.desktop
%{_kf5_datadir}/qlogging-categories5/AdvancedComicBookFormat.categories
%{_kf5_metainfodir}/org.kde.%{name}.appdata.xml
%{_kf5_metainfodir}/org.kde.%{name}creator.appdata.xml


%files devel
%{_kf5_libdir}/libacbf.so
%{_kf5_libdir}/cmake/AdvancedComicBookFormat/
%{_kf5_includedir}/AdvancedComicBookFormat/


%changelog
* Wed Apr 05 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 1.80-1
- first spec for version 1.80 (2.0 Beta 1)

