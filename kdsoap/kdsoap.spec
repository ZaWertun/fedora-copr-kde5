%undefine __cmake_in_source_build
Name:           kdsoap
Version:        1.9.0
Release:        1%{?dist}
Summary:        A Qt-based client-side and server-side SOAP component

# The entire source code is the GPLv3 expect libkdsoap-server which is AGPLv3
License:        GPLv3 and AGPLv3
URL:            https://github.com/KDAB/KDSoap
Source0:        https://github.com/KDAB/KDSoap/releases/download/%{name}-%{version}/%{name}-%{version}.tar.gz

BuildRequires:  gcc-c++
BuildRequires:  cmake
BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  dos2unix

%description
KD Soap is a Qt-based client-side and server-side SOAP component.

It can be used to create client applications for web services and also provides
the means to create web services without the need for any further component
such as a dedicated web server.

KD Soap targets C++ programmers who use Qt in their applications.

For more information, see http://www.kdab.com/kdab-products/kd-soap

%package        devel
Summary:        Development files for %{name}
Requires:       %{name}%{?_isa} = %{version}-%{release}

%description    devel
The %{name}-devel package contains libraries and header files for
developing applications that use %{name}.

%package        doc
Summary:        Documentation for %{name}
BuildArch:      noarch

%description    doc
Documentation for %{name}

%prep
%autosetup


%build
%cmake -DCMAKE_BUILD_TYPE=Release .
%cmake_build
dos2unix doc/config/doxygen.css


%install
%make_install
rm -rf $RPM_BUILD_ROOT/%{_datarootdir}/doc/KDSoap
find $RPM_BUILD_ROOT -name '*.la' -exec rm -f {} ';'


%check
ctest -V %{?_smp_mflags}


%files
%{_libdir}/libkdsoap-server.so.1*
%{_libdir}/libkdsoap.so.1*
%doc README.txt
%license LICENSE.GPL.txt LICENSE.AGPL3-modified.txt

%files devel
%doc kdsoap.pri kdwsdl2cpp.pri
%dir %{_datadir}/mkspecs
%dir %{_datadir}/mkspecs/features
%{_datadir}/mkspecs/features/kdsoap.prf
%{_includedir}/KDSoapClient/
%{_includedir}/KDSoapServer/
%{_libdir}/libkdsoap-server.so
%{_libdir}/libkdsoap.so
%{_bindir}/kdwsdl2cpp
%{_libdir}/cmake/KDSoap/

%files doc
%doc doc


%changelog
* Fri Apr 24 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 1.9.0-1
- version 1.9.0

* Wed Jan 29 2020 Fedora Release Engineering <releng@fedoraproject.org> - 1.8.0-5
- Rebuilt for https://fedoraproject.org/wiki/Fedora_32_Mass_Rebuild

* Thu Jul 25 2019 Fedora Release Engineering <releng@fedoraproject.org> - 1.8.0-4
- Rebuilt for https://fedoraproject.org/wiki/Fedora_31_Mass_Rebuild

* Sat May 18 2019 Casper Meijn <casper@meijn.net> - 1.8.0-3
- Update to KDSoap 1.8.0

* Fri Feb 01 2019 Fedora Release Engineering <releng@fedoraproject.org> - 1.7.0-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_30_Mass_Rebuild

* Fri Oct 12 2018 Casper Meijn <casper@meijn.net> - 1.7.0-1
- First kdsoap package
