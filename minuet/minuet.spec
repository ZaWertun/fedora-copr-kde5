Name:    minuet
Summary: Minuet is an application for music education
Version: 20.04.0
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

BuildRequires: desktop-file-utils
BuildRequires: extra-cmake-modules
BuildRequires: gettext
BuildRequires: fluidsynth-devel >= 1.1.6
BuildRequires: cmake(Qt5Core)
BuildRequires: cmake(Qt5Gui)
BuildRequires: cmake(Qt5Qml)
BuildRequires: cmake(Qt5Quick)
BuildRequires: cmake(Qt5QuickControls2)
BuildRequires: cmake(Qt5Svg)
BuildRequires: cmake(KF5CoreAddons)
BuildRequires: cmake(KF5I18n)
BuildRequires: cmake(KF5Crash)
BuildRequires: cmake(KF5DocTools)

%description
Minuet is an application for music education. It features a set of ear training
exercises regarding intervals, chords, scales and more.

%package        devel
Summary:        Development files for %{name}
Requires:       %{name}%{?_isa} = %{version}-%{release}
%description    devel
%{summary}.


%prep
%autosetup -p1


%build
mkdir %{_target_platform}
pushd %{_target_platform}
%{cmake_kf5} ..
popd

make %{?_smp_mflags} -C %{_target_platform}


%install
make install/fast DESTDIR=%{buildroot} -C %{_target_platform}

%find_lang %{name} --all-name --with-html --with-man --with-qt


%check
appstream-util validate-relax --nonet %{buildroot}%{_kf5_metainfodir}/org.kde.%{name}.appdata.xml ||:
desktop-file-validate %{buildroot}%{_kf5_datadir}/applications/org.kde.%{name}.desktop ||:


%files -f %{name}.lang
%license COPYING*
%{_kf5_bindir}/%{name}
%{_kf5_libdir}/libminuetinterfaces.so.*
%{_qt5_plugindir}/minuet/minuetfluidsynthsoundcontroller.so
%{_kf5_datadir}/applications/org.kde.%{name}.desktop
%{_kf5_datadir}/icons/hicolor/*/apps/%{name}.*
%{_kf5_datadir}/icons/hicolor/*/actions/%{name}-*.*
%{_kf5_datadir}/minuet/definitions/*.json
%{_kf5_datadir}/minuet/exercises/*.json
%{_kf5_datadir}/minuet/soundfonts/GeneralUser-v1.47.sf2
%{_kf5_metainfodir}/org.kde.%{name}.appdata.xml


%files devel
%{_kf5_libdir}/libminuetinterfaces.so
%{_includedir}/minuet/interfaces/*.h


%changelog
* Fri Apr 24 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.04.0-1
- 20.04.0


