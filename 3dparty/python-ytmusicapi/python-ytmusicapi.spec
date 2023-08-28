%global srcname ytmusicapi

Name:           python-%{srcname}
Version:        1.2.1
Release:        1%{?dist}
License:        MIT
Summary:        Unofficial API for YouTube Music
Url:            https://github.com/sigma67/%{srcname}
Source:         %{pypi_source}
Patch0:         001-setuptools-version.patch

BuildArch:      noarch
BuildRequires:  python3-devel
BuildRequires:  python3-setuptools

%global _description %{expand:
ytmusicapi is a Python 3 library to send requests to the YouTube Music API. 
It emulates YouTube Music web client requests using the userâ€™s 
cookie data for authentication.}

%description %_description


%package -n     python3-%{srcname}
Summary:        %{summary}
Recommends:     python3-%{srcname}

%description -n python3-%{srcname} %_description

%prep
%autosetup -n %{srcname}-%{version} -p1

%generate_buildrequires
%pyproject_buildrequires -r
 
%build
%pyproject_wheel

%install
%pyproject_install
%pyproject_save_files %{srcname}

%files -n python3-%{srcname} -f %{pyproject_files}
%license LICENSE
%doc README.rst CONTRIBUTING.rst PKG-INFO
%{_bindir}/%{srcname}


%changelog
* Mon Aug 28 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 1.2.1-1
- v1.2.1

* Fri Jan 20 2023 Fedora Release Engineering <releng@fedoraproject.org> - 0.24.1-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_38_Mass_Rebuild

* Sat Dec 10 2022 Onuralp SEZER <thunderbirdtr@fedoraproject.org> - 0.24.1-2
- build fix for pyproject.toml
- Remove old source files

* Sat Dec 10 2022 Justin Zobel <justin@1707.io> - 0.24.1-1
- v0.24.1

* Thu Aug 25 2022 Onuralp SEZER <thunderbirdtr@fedoraproject.org> - 0.22.0-1
- v0.22.0

* Fri Jul 22 2022 Fedora Release Engineering <releng@fedoraproject.org> - 0.20.0-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_37_Mass_Rebuild

* Mon Jun 13 2022 Python Maint <python-maint@redhat.com> - 0.20.0-2
- Rebuilt for Python 3.11

* Sun Jan 16 2022 Onuralp SEZER <thunderbirdtr@fedoraproject.org> - 0.20.0-1
- Initial version of package
