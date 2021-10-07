%undefine __cmake_in_source_build
Name:    wrapland
Version: 0.522.0
Release: 1%{?dist}
Summary: Qt/C++ library wrapping libwayland

%global  real_version %(echo %{version} |sed 's/~/-/')
License: LGPLv2+
URL:     https://gitlab.com/kwinft/%{name}
Source0: %{url}/-/archive/%{name}@%{real_version}/%{name}-%{name}@%{real_version}.tar.bz2

BuildRequires: extra-cmake-modules
BuildRequires: kf5-rpm-macros

BuildRequires: qt5-qtbase-devel
BuildRequires: qt5-qtbase-private-devel

BuildRequires: wayland-devel
BuildRequires: wayland-protocols-devel

BuildRequires: pkgconfig(wayland-eglstream)

%description
%{summary}.

%package     devel
Summary:     Development files for %{name}
%description devel
%{summary}.

%package     client
Summary:     Qt/C++ client library wrapping libwayland
Requires:    %{name}%{?_isa} = %{version}-%{release}
%description client
%{summary}.

%package     client-devel
Summary:     Development files for %{name}-client
Requires:    %{name}%{?_isa} = %{version}-%{release}
Requires:    %{name}-devel%{?_isa} = %{version}-%{release}
%description client-devel
%{summary}.

%package     server
Summary:     Qt/C++ server library wrapping libwayland
Requires:    %{name} = %{version}-%{release}
%description server
%{summary}.

%package     server-devel
Summary:     Development files for %{name}-server
Requires:    %{name}%{?_isa} = %{version}-%{release}
Requires:    %{name}-devel%{?_isa} = %{version}-%{release}
%description server-devel
%{summary}.

%package     test-server
Summary:     Test server for %{name}.
%description test-server
%{summary}.


%prep
%autosetup -p1 -n %{name}-%{name}@%{real_version}


%build
%{cmake_kf5} \
  -DBUILD_TESTING:BOOL=ON

%cmake_build


%install
%make_install -C %{__cmake_builddir}


%check
xvfb-run -a \
dbus-launch --exit-with-session \
make test ARGS="--output-on-failure --timeout 10" -C %{__cmake_builddir} ||:


%files
%doc CHANGELOG.md  README.md
%license COPYING.LIB
%{_datadir}/qlogging-categories5/org_kde_wrapland.categories


%files devel
%{_includedir}/wrapland_version.h
%{_libdir}/cmake/Wrapland/Wrapland*.cmake


%files client
%{_libdir}/libWraplandClient.so.*


%files client-devel
%{_libdir}/libWraplandClient.so
%{_includedir}/Wrapland/Client/


%files server
%{_libdir}/libWraplandServer.so.*


%files server-devel
%{_libdir}/libWraplandServer.so
%{_includedir}/Wrapland/Server/


%files test-server
%{_libexecdir}/org-kde-kf5-wrapland-testserver


%changelog
* Thu Jun 10 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 0.522.0-1
- 0.522.0

* Wed Feb 17 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 0.521.0-1
- 0.521.0

* Wed Oct 14 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 0.520.0-1
- 0.520.0

* Tue Jun 16 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 0.519.0-1
- 0.519.0

* Mon May 25 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 0.519.0~beta.0-1
- version 0.519.0-beta.0

* Fri Apr 17 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 0.518.0-2
- wrapland-testserver moved to separate package

* Fri Apr 17 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 0.518.0-1
- first spec


