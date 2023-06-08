Name:    baloo-widgets
Summary: Widgets for Baloo
Version: 23.04.2
Release: 1%{?dist}

# # KDE e.V. may determine that future LGPL versions are accepted
License: LGPLv2 or LGPLv3
URL:     https://cgit.kde.org/%{name}.git/

%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0: https://download.kde.org/%{stable}/release-service/%{version}/src/%{name}-%{version}.tar.xz
Source1: https://download.kde.org/%{stable}/release-service/%{version}/src/%{name}-%{version}.tar.xz.sig
Source2: gpgkey-D81C0CB38EB725EF6691C385BB463350D6EF31EF.gpg

# translations moved here
Conflicts: kde-l10n < 17.04.0-2

Provides: kf5-baloo-widgets = %{version}-%{release}
Provides: kf5-baloo-widgets%{?_isa} = %{version}-%{release}

BuildRequires:  gnupg2
BuildRequires:  cmake(Qt5Widgets)
BuildRequires:  cmake(Qt5Test)

BuildRequires:  kf5-rpm-macros
BuildRequires:  extra-cmake-modules >= 5.19
BuildRequires:  cmake(KF5Config)
BuildRequires:  cmake(KF5KIO)
BuildRequires:  cmake(KF5I18n)
BuildRequires:  cmake(KF5FileMetaData)
BuildRequires:  cmake(KF5Baloo)

%description
%{summary}.

%package devel
Summary:  Developer files for %{name}
Provides: kf5-baloo-widgets-devel = %{version}-%{release}
Provides: kf5-baloo-widgets-devel%{?_isa} = %{version}-%{release}
Requires: %{name}%{?_isa} = %{version}-%{release}
Requires: qt5-qtbase-devel
Requires: kf5-kcoreaddons-devel
Requires: kf5-kio-devel
%description devel
%{summary}.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup


%build
%cmake_kf5
%cmake_build


%install
%cmake_install

%find_lang %{name} --all-name


%ldconfig_scriptlets

%files -f %{name}.lang
%doc LICENSES/*.txt
%{_kf5_libdir}/libKF5BalooWidgets.so.*
%{_kf5_bindir}/baloo_filemetadata_temp_extractor
%{_kf5_plugindir}/propertiesdialog/baloofilepropertiesplugin.so
%{_kf5_qtplugindir}/kf5/kfileitemaction/tagsfileitemaction.so
%{_kf5_datadir}/qlogging-categories5/%{name}.categories

%files devel
%{_kf5_libdir}/cmake/KF5BalooWidgets/
%{_kf5_includedir}/BalooWidgets/
%{_kf5_libdir}/libKF5BalooWidgets.so


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

