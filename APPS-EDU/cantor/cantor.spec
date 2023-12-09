# uncomment to enable bootstrap mode
#global bootstrap 1

%if !0%{?bootstrap}
%global qalculate 1
%if 0%{?fedora} < 26
%global libr 1
%endif
%global libspectre 1
%ifarch %{arm} %{ix86} x86_64 aarch64
%global luajit 1
%endif
%global python3 1
%endif

Name:    cantor
Summary: KDE Frontend to Mathematical Software
Version: 23.08.4
Release: 1%{?dist}

License: GPLv2+
URL:     https://edu.kde.org/cantor/

%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0: https://download.kde.org/%{stable}/release-service/%{version}/src/%{name}-%{version}.tar.xz
Source1: https://download.kde.org/%{stable}/release-service/%{version}/src/%{name}-%{version}.tar.xz.sig
Source2: gpgkey-D81C0CB38EB725EF6691C385BB463350D6EF31EF.gpg

## upstream fixes

## upstreamable patches

%global majmin_ver %(echo %{version} | cut -d. -f1,2)
BuildRequires: gnupg2
BuildRequires: analitza-devel >= %{majmin_ver}

BuildRequires: desktop-file-utils
BuildRequires: libappstream-glib

BuildRequires: extra-cmake-modules
BuildRequires: kf5-rpm-macros
BuildRequires: cmake(KF5Archive)
BuildRequires: cmake(KF5Completion)
BuildRequires: cmake(KF5Config)
BuildRequires: cmake(KF5CoreAddons)
BuildRequires: cmake(KF5Crash)
BuildRequires: cmake(KF5DocTools)
BuildRequires: cmake(KF5I18n)
BuildRequires: cmake(KF5IconThemes)
BuildRequires: cmake(KF5KIO)
BuildRequires: cmake(KF5NewStuff)
BuildRequires: cmake(KF5Parts)
BuildRequires: cmake(KF5Pty)
BuildRequires: cmake(KF5SyntaxHighlighting)
BuildRequires: cmake(KF5TextEditor)
BuildRequires: cmake(KF5TextWidgets)
BuildRequires: cmake(KF5XmlGui)

BuildRequires: pkgconfig(Qt5PrintSupport)
BuildRequires: pkgconfig(Qt5Svg)
BuildRequires: pkgconfig(Qt5Widgets)
BuildRequires: pkgconfig(Qt5Xml)
BuildRequires: pkgconfig(Qt5XmlPatterns)
BuildRequires: pkgconfig(Qt5Test)
BuildRequires: pkgconfig(Qt5Help)
BuildRequires: pkgconfig(Qt5WebEngine)
BuildRequires: poppler-qt5-devel

# optional deps/plugins
%if 0%{?qalculate}
BuildRequires: pkgconfig(libqalculate)
%endif
%if 0%{?libspectre}
BuildRequires: pkgconfig(libspectre)
%endif
%if 0%{?luajit}
BuildRequires: pkgconfig(luajit)
%endif
%if 0%{?python3}
BuildRequires: python3-devel
%else
Obsoletes: cantor-python3 < %{version}-%{release}
%endif

Requires: %{name}-libs%{?_isa} = %{version}-%{release}

%description
%{summary}.

%package -n python3-%{name}
Summary: %{name} python3 backend
Requires: %{name}-libs%{?_isa} = %{version}-%{release}
Obsoletes: cantor <  16.08.0-3
%{?python_provide:%python_provide python3-%{name}}
%description -n python3-%{name}
%{name} python3 backend.

%package  libs
Summary:  Runtime files for %{name}
# when split occurred
Conflicts: kdeedu-math-libs < 4.7.0-10
Provides: %{name}-part = %{version}-%{release}
Requires: %{name} = %{version}-%{release}
%description libs
%{summary}.

%if 0%{?libr}
%package R
Summary: R backend for %{name}
BuildRequires: pkgconfig(libR)
Obsoletes: kdeedu-math-cantor-R < 4.7.0-10
Provides:  kdeedu-math-cantor-R = %{version}-%{release}
Requires: %{name}-libs%{?_isa} = %{version}-%{release}
%description R 
%{summary}.
%endif

%package devel
Summary:  Development files for %{name}
# when split occurred
Conflicts: kdeedu-devel < 4.7.0-10
Requires: %{name}-libs%{?_isa} = %{version}-%{release}
%description devel
%{summary}.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -p1


%build
%cmake_kf5
%cmake_build


%install
%cmake_install
%find_lang %{name} --all-name --with-html


%check
appstream-util validate-relax --nonet %{buildroot}%{_kf5_metainfodir}/org.kde.%{name}.appdata.xml ||:
desktop-file-validate %{buildroot}%{_kf5_datadir}/applications/org.kde.%{name}.desktop


%files -f %{name}.lang
%doc README*
%license LICENSES/*.txt
%{_kf5_bindir}/cantor*
%{_kf5_metainfodir}/org.kde.%{name}.appdata.xml
%{_kf5_datadir}/applications/org.kde.%{name}.desktop
%{_kf5_datadir}/knsrcfiles/%{name}.knsrc
%{_kf5_datadir}/knsrcfiles/%{name}_sage.knsrc
%{_kf5_datadir}/knsrcfiles/%{name}_scilab.knsrc
%{_kf5_datadir}/knsrcfiles/%{name}_maxima.knsrc
%{_kf5_datadir}/knsrcfiles/%{name}_octave.knsrc
%{_kf5_datadir}/knsrcfiles/%{name}_kalgebra.knsrc
%if 0%{?luajit}
%{_kf5_datadir}/knsrcfiles/%{name}_lua.knsrc
%endif
%if 0%{?qalculate}
%{_kf5_datadir}/knsrcfiles/%{name}_qalculate.knsrc
%endif
%dir %{_kf5_datadir}/kxmlgui5/cantor/
%{_datadir}/icons/hicolor/*/*/*
%{_kf5_datadir}/cantor/
%{_kf5_datadir}/config.kcfg/*.kcfg
%exclude %{_kf5_datadir}/config.kcfg/pythonbackend.kcfg
%{_kf5_datadir}/kxmlgui5/cantor/cantor_scripteditor.rc
%{_kf5_datadir}/kxmlgui5/cantor/cantor_shell.rc
%{_kf5_datadir}/kxmlgui5/cantor/cantor_*assistant.rc
%{_kf5_datadir}/mime/packages/%{name}.xml

%if 0%{?python3}
%files -n python3-%{name}
%{_kf5_datadir}/knsrcfiles/%{name}_python.knsrc
%{_kf5_bindir}/cantor_pythonserver
%{_kf5_libdir}/cantor_pythonbackend.so
%{_kf5_qtplugindir}/cantor/backends/cantor_pythonbackend.so
%{_kf5_datadir}/config.kcfg/pythonbackend.kcfg
%endif

%if 0%{?libr}
%files R
%{_kf5_bindir}/cantor_rserver
%{_kf5_qtplugindir}/cantor/backends/cantor_rbackend.so
%{_kf5_datadir}/config.kcfg/rserver.kcfg
%{_kf5_datadir}/knsrcfiles/%{name}_r.knsrc
%endif

%ldconfig_scriptlets libs

%files libs
%{_libdir}/libcantorlibs.so.*
%{_libdir}/libcantor_config.so
%{_kf5_datadir}/kxmlgui5/cantor/cantor_part.rc
%{_kf5_datadir}/knsrcfiles/cantor-documentation.knsrc
## plugins
%dir %{_kf5_qtplugindir}/cantor/
%{_kf5_qtplugindir}/cantor/assistants/
%{_kf5_qtplugindir}/cantor/panels/
%{_kf5_qtplugindir}/kf5/parts/cantorpart.so
%dir %{_kf5_qtplugindir}/cantor/backends/
%{_kf5_qtplugindir}/cantor/backends/cantor_kalgebrabackend.so
%if 0%{?luajit}
%{_kf5_qtplugindir}/cantor/backends/cantor_luabackend.so
%endif
%{_kf5_qtplugindir}/cantor/backends/cantor_maximabackend.so
%{_kf5_qtplugindir}/cantor/backends/cantor_octavebackend.so
%if 0%{?qalculate}
%{_kf5_qtplugindir}/cantor/backends/cantor_qalculatebackend.so
%endif
%{_kf5_qtplugindir}/cantor/backends/cantor_sagebackend.so
%{_kf5_qtplugindir}/cantor/backends/cantor_scilabbackend.so

%files devel
%{_includedir}/cantor/
%{_libdir}/libcantorlibs.so
%{_libdir}/cmake/Cantor/
%{_kf5_datadir}/config.kcfg/octavebackend.kcfg.in


%changelog
* Sat Dec 09 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.08.4-1
- 23.08.4

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

