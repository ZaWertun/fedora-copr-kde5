Name:    step
Summary: Interactive Physics Simulator 
Version: 23.04.3
Release: 1%{?dist}

License: GPLv2+
URL:     https://cgit.kde.org/%{name}.git

%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0: https://download.kde.org/%{stable}/release-service/%{version}/src/%{name}-%{version}.tar.xz
Source1: https://download.kde.org/%{stable}/release-service/%{version}/src/%{name}-%{version}.tar.xz.sig
Source2: gpgkey-D81C0CB38EB725EF6691C385BB463350D6EF31EF.gpg

BuildRequires: gnupg2
BuildRequires: desktop-file-utils
BuildRequires: extra-cmake-modules
BuildRequires: gettext
BuildRequires: kf5-attica-devel
BuildRequires: kf5-kcmutils-devel
BuildRequires: kf5-kcompletion-devel
BuildRequires: kf5-kconfig-devel
BuildRequires: kf5-kconfigwidgets-devel
BuildRequires: kf5-kcoreaddons-devel
BuildRequires: kf5-kcrash-devel
BuildRequires: kf5-kdbusaddons-devel
BuildRequires: kf5-kdeclarative-devel
BuildRequires: kf5-kdelibs4support-devel
BuildRequires: kf5-kdoctools-devel
BuildRequires: kf5-kguiaddons-devel
BuildRequires: kf5-khtml-devel
BuildRequires: kf5-ki18n-devel
BuildRequires: kf5-kiconthemes-devel
BuildRequires: kf5-kio-devel
BuildRequires: kf5-kitemviews-devel
BuildRequires: kf5-kjobwidgets-devel
BuildRequires: kf5-knewstuff-devel
BuildRequires: kf5-knotifyconfig-devel
BuildRequires: kf5-kplotting-devel
BuildRequires: kf5-kross-devel
BuildRequires: kf5-kservice-devel
BuildRequires: kf5-kwidgetsaddons-devel
BuildRequires: kf5-kwindowsystem-devel
BuildRequires: kf5-kxmlgui-devel
BuildRequires: kf5-rpm-macros
BuildRequires: kf5-sonnet-devel
BuildRequires: libappstream-glib
BuildRequires: pkgconfig(eigen3)
BuildRequires: pkgconfig(gsl)
BuildRequires: pkgconfig(libqalculate)
BuildRequires: pkgconfig(Qt5Qml) pkgconfig(Qt5Quick) pkgconfig(Qt5Xml) pkgconfig(Qt5Svg) pkgconfig(Qt5OpenGL) pkgconfig(Qt5PrintSupport) pkgconfig(Qt5Test)

%description
%{summary}.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -p1


%build
%cmake_kf5
%cmake_build


%install
%cmake_install

%find_lang %{name} --all-name --with-html --with-qt

## unpackaged files
# omit bundled copies of python-(mwclient,simplejson)
rm -frv %{buildroot}%{_kf5_datadir}/parley/plugins/mwclient/
rm -fv %{buildroot}%{_kf5_datadir}/locale/*/LC_SCRIPTS/step/*.js


%check
appstream-util validate-relax --nonet %{buildroot}%{_kf5_metainfodir}/org.kde.%{name}.appdata.xml ||:
desktop-file-validate %{buildroot}%{_kf5_datadir}/applications/org.kde.%{name}.desktop ||:

%files -f %{name}.lang
%license LICENSES/*.txt
%doc README
%{_kf5_bindir}/%{name}
%{_kf5_datadir}/applications/org.kde.%{name}.desktop
%{_kf5_metainfodir}/org.kde.%{name}.appdata.xml
%{_kf5_datadir}/icons/hicolor/*/actions/*
%{_kf5_datadir}/icons/hicolor/*/apps/%{name}*
%{_kf5_datadir}/%{name}/
%{_kf5_datadir}/config.kcfg/%{name}.kcfg
%{_datadir}/mime/packages/org.kde.%{name}.xml
%{_kf5_datadir}/knsrcfiles/step.knsrc


%changelog
* Thu Jul 06 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.04.3-1
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

