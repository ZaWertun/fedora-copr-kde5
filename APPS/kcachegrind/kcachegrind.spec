Name:    kcachegrind
Summary: GUI to profilers such as Valgrind
Version: 23.08.1
Release: 1%{?dist}

License: GPLv2 and GFDL
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

## upstream patches

BuildRequires: gnupg2
BuildRequires: desktop-file-utils
BuildRequires: perl-generators
%if 0%{?fedora} >= 31
BuildRequires: python3-rpm-macros
%else
BuildRequires: python2-rpm-macros
%endif

BuildRequires: extra-cmake-modules
BuildRequires: kf5-rpm-macros
BuildRequires: cmake(KF5Archive)
BuildRequires: cmake(KF5CoreAddons)
BuildRequires: cmake(KF5DBusAddons)
BuildRequires: cmake(KF5DocTools)
BuildRequires: cmake(KF5WidgetsAddons)
BuildRequires: cmake(KF5XmlGui)
BuildRequires: cmake(KF5I18n)
BuildRequires: cmake(KF5Config)
BuildRequires: cmake(KF5KIO)

BuildRequires: cmake(Qt5DBus)
BuildRequires: cmake(Qt5Gui)
BuildRequires: cmake(Qt5Widgets)

# translations moved here
Conflicts: kde-l10n < 17.03

Conflicts:      kdesdk-common < 4.10.80
Provides:       kdesdk-kcachegrind = %{version}-%{release}
Obsoletes:      kdesdk-kcachegrind < 4.10.80

%description
Browser for data produced by profiling tools (e.g. cachegrind)

%package converters
Summary: Converters for kcachegrind
# when split out
Obsoletes: kcachegrind < 17.12.3-2
Requires: %{name} = %{version}-%{release}
%description converters
%{summary}.

%package -n qcachegrind
Summary: QT GUI to profilers such as Valgrind

%description -n qcachegrind
QT-based browser for data produced by profiling tools (e.g. cachegrind).


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -p1

# Avoid use of #!/usr/bin/env as interpeter
%if 0%{?fedora} >= 31
sed -i.env -e "s|^#!/usr/bin/env python$|#!%{__python3}|g" converters/hotshot2calltree.in
%else
sed -i.env -e "s|^#!/usr/bin/env python$|#!%{__python2}|g" converters/hotshot2calltree.in
%endif
sed -i.env -e "s|^#!/usr/bin/env php$|#!%{_bindir}/php|g"  converters/pprof2calltree


%build
%cmake_kf5
%cmake_build


%install
%cmake_install

# qcachegrind needs manual installation
install -p -m 755 %{__cmake_builddir}/bin/qcachegrind %{buildroot}%{_bindir}/
install -p -m 755 %{__cmake_builddir}/bin/cgview %{buildroot}%{_bindir}/
install -p -m 644 qcachegrind/qcachegrind.desktop %{buildroot}%{_datadir}/applications/


%check
desktop-file-validate %{buildroot}%{_kf5_datadir}/applications/org.kde.%{name}.desktop
desktop-file-validate %{buildroot}%{_datadir}/applications/qcachegrind.desktop

%find_lang %{name} --all-name --with-html
%find_lang_kf5 kcachegrind_qt
cat kcachegrind_qt.lang >> kcachegrind.lang


%files -f %{name}.lang
%doc README
%license LICENSES/*.txt
%{_kf5_bindir}/kcachegrind
%{_kf5_datadir}/kcachegrind/
%{_kf5_datadir}/applications/org.kde.kcachegrind.desktop
%{_kf5_metainfodir}/org.kde.kcachegrind.appdata.xml
%{_kf5_datadir}/icons/hicolor/*/apps/kcachegrind.*

%files converters
%doc converters/README
# perl
%{_kf5_bindir}/dprof2calltree
%{_kf5_bindir}/memprof2calltree
%{_kf5_bindir}/op2calltree
# python
%{_kf5_bindir}/hotshot2calltree
# php
%{_kf5_bindir}/pprof2calltree

%files -n qcachegrind
%doc README
%license LICENSES/*.txt
%{_bindir}/qcachegrind
%{_bindir}/cgview
%{_datadir}/applications/qcachegrind.desktop
# icons are shared with kcachegrind
%{_datadir}/icons/hicolor/*/apps/kcachegrind.*


%changelog
* Thu Sep 14 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.08.1-1
- 23.08.1

* Sun Aug 27 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.08.0-1
- 23.08.0

* Thu Jul 06 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.04.3-1
- 23.04.3

* Thu Jun 08 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.04.2-1
- 23.04.2

* Thu May 11 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.04.1-1
- 23.04.1

* Thu Apr 20 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.04.0-1
- 23.04.0

* Thu Mar 02 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.12.3-1
- 22.12.3

* Thu Feb 02 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.12.2-1
- 22.12.2

