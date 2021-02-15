%undefine __cmake_in_source_build
%define appname kColorPicker
%global libname lib%{appname}

Name: kcolorpicker
Version: 0.1.5
Release: 1%{?dist}

License: LGPLv3+
Summary: QToolButton control with color popup menu
URL: https://github.com/ksnip/%{appname}
Source0: %{url}/archive/v%{version}/%{name}-%{version}.tar.gz

BuildRequires: cmake(Qt5Core)
BuildRequires: cmake(Qt5Gui)

BuildRequires: ninja-build
BuildRequires: gcc-c++
BuildRequires: cmake

%description
QToolButton with color popup menu which lets you select a color.

The popup features a color dialog button which can be used to add
custom colors to the popup menu.

%package devel
Summary: Development files for %{name}
Requires: %{name}%{?_isa} = %{?epoch:%{epoch}:}%{version}-%{release}

%description devel
%{summary}.

%prep
%autosetup -n %{appname}-%{version} -p1

%build
%cmake -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_TESTS:BOOL=OFF \
    -DBUILD_EXAMPLE:BOOL=OFF
%cmake_build

%install
%cmake_install

%files
%doc README.md
%license LICENSE
%{_libdir}/%{libname}.so.0*

%files devel
%{_includedir}/%{appname}/
%{_libdir}/cmake/%{appname}/
%{_libdir}/%{libname}.so

%changelog
* Mon Feb 15 2021 Vitaly Zaitsev <vitaly@easycoding.org> - 0.1.5-1
- Updated to version 0.1.5.

* Tue Jan 26 2021 Fedora Release Engineering <releng@fedoraproject.org> - 0.1.4-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_34_Mass_Rebuild

* Fri Jul 31 2020 Vitaly Zaitsev <vitaly@easycoding.org> - 0.1.4-1
- Initial SPEC release.
