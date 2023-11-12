%global app_id  org.kde.markdownpart

Name:           markdownpart
Summary:        Markdown KPart
Version:        23.08.3
Release:        1%{?dist}
License:        LGPL-2.1-or-later
URL:            https://apps.kde.org/categories/utilities/

%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0:        https://download.kde.org/%{stable}/release-service/%{version}/src/%{name}-%{version}.tar.xz
Source1:        https://download.kde.org/%{stable}/release-service/%{version}/src/%{name}-%{version}.tar.xz.sig
Source2:        gpgkey-D81C0CB38EB725EF6691C385BB463350D6EF31EF.gpg

BuildRequires:  gnupg2
BuildRequires:  extra-cmake-modules
BuildRequires:  kf5-rpm-macros
BuildRequires:  cmake(Qt5Widgets)
BuildRequires:  cmake(KF5I18n)
BuildRequires:  cmake(KF5Parts)
BuildRequires:  gcc-c++
BuildRequires:  gettext
BuildRequires:  libappstream-glib

%description
A Markdown viewer KParts plugin, which allows KParts-using applications to
display files in Markdown format in the target format.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -p1


%build
%cmake_kf5
%cmake_build


%install
%cmake_install
%find_lang %{name}


%check
appstream-util validate-relax --nonet %{buildroot}%{_kf5_metainfodir}/%{app_id}.metainfo.xml


%files -f %{name}.lang
%doc README.md
%license LICENSES/*.txt
%{_kf5_plugindir}/parts/markdownpart.so
%{_kf5_metainfodir}/%{app_id}.metainfo.xml


%changelog
* Fri Nov 10 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.08.3-1
- 23.08.3

* Fri Oct 13 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.08.2-1
- 23.08.2

* Thu Sep 14 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.08.1-1
- 23.08.1

* Sun Aug 27 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.08.0-1
- 23.08.0

