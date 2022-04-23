Name:    kpeoplevcard
Summary: Makes it possible to expose vcards to KPeople
Version: 0.1
Release: 1%{?dist}

License: LGPLv2+
URL:     https://invent.kde.org/pim/kpeoplevcard

Source0: http://download.kde.org/stable/%{name}/%{version}/%{name}-%{version}.tar.xz

BuildRequires:  extra-cmake-modules
BuildRequires:  kf5-rpm-macros

BuildRequires:  cmake(Qt5Widgets) >= 5.2

BuildRequires:  cmake(KF5CoreAddons) >= 5.8
BuildRequires:  cmake(KF5I18n)       >= 5.8
BuildRequires:  cmake(KF5People)     >= 5.62
BuildRequires:  cmake(KF5Config)
BuildRequires:  cmake(KF5Codecs)
BuildRequires:  cmake(KF5Contacts)

%description
%{summary}.


%package devel
Summary:        Development files for %{name}
Requires:       %{name}%{?_isa} = %{version}-%{release}

%description    devel
The %{name}-devel package contains libraries and header files for
developing applications that use %{name}.


%prep
%autosetup -p1


%build
%cmake_kf5
%cmake_build


%install
%cmake_install

%ldconfig_scriptlets

%files
%license COPYING
%{_qt5_plugindir}/kpeople/datasource/KPeopleVCard.so


%files devel
%{_libdir}/cmake/KF5PeopleVCard/KF5PeopleVCardConfig.cmake


%changelog
* Mon Dec 09 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 0.1-1
- version 0.1

