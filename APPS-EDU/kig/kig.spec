%global _python3_include %(%{__python3} -Ic "from distutils.sysconfig import get_python_inc; print(get_python_inc())")
%global _python3_lib %{_libdir}/lib%(basename %{_python3_include}).so

Name:    kig
Summary: Interactive Geometry
Version: 23.08.2
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
Source1: https://download.kde.org/%{stable}/release-service/%{version}/src/%{name}-%{version}.tar.xz.sig
Source2: gpgkey-D81C0CB38EB725EF6691C385BB463350D6EF31EF.gpg

## upstreamable patches
# https://bugzilla.redhat.com/show_bug.cgi?id=1238113
# https://bugs.kde.org/show_bug.cgi?id=335965#c23
# https://git.reviewboard.kde.org/r/126549/
Patch1: 0001-explicitly-use-QLibrary-to-load-libpython-like-pykde.patch

BuildRequires: gnupg2
BuildRequires: boost-devel
BuildRequires: boost-python3-devel
BuildRequires: python3
BuildRequires: python3-rpm-macros
BuildRequires: python3-devel

BuildRequires: desktop-file-utils
BuildRequires: gettext
BuildRequires: libappstream-glib

BuildRequires: extra-cmake-modules
BuildRequires: kf5-rpm-macros

BuildRequires: cmake(KF5DocTools)
BuildRequires: cmake(KF5Parts)
BuildRequires: cmake(KF5I18n)
BuildRequires: cmake(KF5TextEditor)
BuildRequires: cmake(KF5ConfigWidgets)
BuildRequires: cmake(KF5IconThemes)
BuildRequires: cmake(KF5Archive)
BuildRequires: cmake(KF5XmlGui)
BuildRequires: cmake(KF5Crash)
BuildRequires: cmake(KF5CoreAddons)
BuildRequires: cmake(KF5SyntaxHighlighting)

BuildRequires: pkgconfig(Qt5PrintSupport)
BuildRequires: pkgconfig(Qt5Svg)
BuildRequires: pkgconfig(Qt5XmlPatterns)

# when split occurred
Conflicts: kdeedu-math < 4.7.0-10

%description
%{summary}.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -p1

sed -ie "s|^#!/usr/bin/env python3|#!%{__python3}|" pykig/pykig.py


%build
%cmake_kf5 \
  -DPYTHON_EXECUTABLE:PATH=%{__python3} \
  -DPYTHON_INCLUDE_DIR=%{_python3_include} \
  -DPYTHON_LIBRARY=%{_python3_lib} \
  -DBoostPython_INCLUDE_DIRS="%{_python3_include};%{_includedir}/boost" \
  -DBoostPython_LIBRARIES="%{_python3_lib};%{_libdir}/libboost_python%{python3_version_nodots}.so"
%cmake_build


%install
%cmake_install
%find_lang %{name} --all-name --with-html --with-man


%check
appstream-util validate-relax --nonet %{buildroot}%{_kf5_metainfodir}/org.kde.%{name}.appdata.xml
desktop-file-validate %{buildroot}%{_kf5_datadir}/applications/org.kde.%{name}.desktop


%files -f %{name}.lang
%license LICENSES/*.txt
%{_kf5_bindir}/%{name}*
%{_kf5_bindir}/pykig.*
%{_kf5_qtplugindir}/kf5/parts/kigpart.so
%{_kf5_datadir}/applications/org.kde.%{name}.desktop
%{_kf5_metainfodir}/org.kde.%{name}.appdata.xml
%{_kf5_datadir}/icons/hicolor/*/apps/%{name}.*
%{_kf5_datadir}/icons/hicolor/*/mimetypes/application-x-%{name}.*
%{_kf5_datadir}/%{name}/
%{_kf5_datadir}/kxmlgui5/%{name}/
%{_kf5_datadir}/katepart5/syntax/python-kig.xml
%{_mandir}/man1/kig.1*


%changelog
* Fri Oct 13 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.08.2-1
- 23.08.2

* Thu Sep 14 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.08.1-1
- 23.08.1

* Mon Aug 28 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.08.0-1
- 23.08.0

* Thu Jul 06 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.04.3-1
- 23.04.3

* Thu Jun 08 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.04.2-1
- 23.04.2

* Thu May 11 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.04.1-1
- 23.04.1

* Fri Apr 21 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.04.0-1
- 23.04.0

* Thu Mar 02 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.12.3-1
- 22.12.3

* Thu Jan 05 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.12.1-1
- 22.12.1

