%undefine __cmake_in_source_build
Name:    pulseaudio-qt
Summary: Pulseaudio bindings for Qt
Version: 1.1.0
Release: 1%{?dist}

License: LGPLv2+
URL:     https://cgit.kde.org/%{name}.git

Source0: http://download.kde.org/stable/%{name}/%{name}-%{version}.tar.xz

BuildRequires:  extra-cmake-modules
BuildRequires:  kf5-rpm-macros

BuildRequires:  pulseaudio-libs-devel

BuildRequires:  cmake(Qt5Core) >= 5.10
BuildRequires:  cmake(Qt5Gui)  >= 5.10
BuildRequires:  cmake(Qt5DBus) >= 5.10

BuildRequires:  cmake(Qt5Test)
BuildRequires:  cmake(Qt5Qml)
BuildRequires:  cmake(Qt5Quick)

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
%{cmake_kf5} \
    -DBUILD_TESTING:BOOL=ON

%cmake_build


%install
%make_install -C %{_target_platform}


%check
export CTEST_OUTPUT_ON_FAILURE=1
make test ARGS="--output-on-failure --timeout 20" -C %{_target_platform} ||:


%ldconfig_scriptlets

%files
%license COPYING.LIB
%{_libdir}/libKF5PulseAudioQt.so.*


%files devel
%{_libdir}/libKF5PulseAudioQt.so
%{_libdir}/cmake/KF5PulseAudioQt/*.cmake
%{_includedir}/KF5/pulseaudioqt_version.h
%{_includedir}/KF5/KF5PulseAudioQt/PulseAudioQt/*


%changelog
* Mon Dec 09 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 1.1.0-1
- version 1.1.0

