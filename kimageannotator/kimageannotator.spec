%undefine __cmake_in_source_build
%define appname kImageAnnotator
%global libname lib%{appname}

Name: kimageannotator
Version: 0.4.0
Release: 1%{?dist}

License: LGPLv3+
Summary: Library and a tool for annotating images
URL: https://github.com/ksnip/%{appname}
Source0: %{url}/archive/v%{version}/%{name}-%{version}.tar.gz

BuildRequires: cmake(Qt5LinguistTools)
BuildRequires: cmake(kColorPicker)
BuildRequires: cmake(Qt5X11Extras)
BuildRequires: cmake(Qt5Core)
BuildRequires: cmake(Qt5Gui)
BuildRequires: cmake(Qt5Svg)

BuildRequires: ninja-build
BuildRequires: gcc-c++
BuildRequires: cmake

%description
Library and a tool for annotating images. Part of KSnip project.

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
%find_lang %{appname} --with-qt

%files -f %{appname}.lang
%doc CHANGELOG.md README.md
%license LICENSE
%{_libdir}/%{libname}.so.0*

%files devel
%{_includedir}/%{appname}/
%{_libdir}/cmake/%{appname}/
%{_libdir}/%{libname}.so

%changelog
* Mon Dec 14 2020 Vitaly Zaitsev <vitaly@easycoding.org> - 0.4.0-1
- Updated to version 0.4.0.

* Fri Jul 31 2020 Vitaly Zaitsev <vitaly@easycoding.org> - 0.3.2-1
- Initial SPEC release.
