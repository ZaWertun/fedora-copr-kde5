%global  base_name print-manager

Name:    kde-print-manager
Summary: Printer management for KDE
Version: 23.04.3
Release: 1%{?dist}

License: GPLv2+ and LGPLv2+
URL:     https://cgit.kde.org/%{base_name}.git

%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0: https://download.kde.org/%{stable}/release-service/%{version}/src/%{base_name}-%{version}.tar.xz
Source1: https://download.kde.org/%{stable}/release-service/%{version}/src/%{base_name}-%{version}.tar.xz.sig
Source2: gpgkey-D81C0CB38EB725EF6691C385BB463350D6EF31EF.gpg

BuildRequires: gnupg2
BuildRequires: desktop-file-utils
BuildRequires: gettext
BuildRequires: extra-cmake-modules
BuildRequires: kf5-rpm-macros
BuildRequires: kf5-kconfig-devel
BuildRequires: kf5-kconfigwidgets-devel
BuildRequires: kf5-kcoreaddons-devel
BuildRequires: kf5-kdbusaddons-devel
BuildRequires: kf5-kiconthemes-devel
BuildRequires: kf5-kcmutils-devel
BuildRequires: kf5-kio-devel
BuildRequires: kf5-knotifications-devel
BuildRequires: kf5-plasma-devel
BuildRequires: kf5-kwidgetsaddons-devel
BuildRequires: kf5-kwindowsystem-devel
BuildRequires: kf5-kitemviews-devel
BuildRequires: kf5-ki18n-devel

BuildRequires: qt5-qtbase-devel
BuildRequires: qt5-qtdeclarative-devel
# required because of Qt5Designer - should be removed in final release
BuildRequires: qt5-qttools-devel

BuildRequires: cups-devel >= 1.5.0

# translations moved here
Conflicts: kde-l10n < 17.03

Requires: plasma-workspace

Requires: %{name}-libs%{?_isa} = %{version}-%{release}
# currently requires local cups for majority of proper function
Requires: cups
# required for the com.redhat.NewPrinterNotification D-Bus service
Requires: system-config-printer-libs

%description
Printer management for KDE.

%package  libs
Summary:  Runtime files for %{name}
Requires: %{name} = %{version}-%{release}
%description libs
%{summary}.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -p1 -n %{base_name}-%{version}


%build
%cmake_kf5
%cmake_build


%install
%cmake_install
%find_lang %{name} --all-name --with-html


%check
desktop-file-validate %{buildroot}%{_kf5_datadir}/applications/org.kde.kde-add-printer.desktop
desktop-file-validate %{buildroot}%{_kf5_datadir}/applications/org.kde.ConfigurePrinter.desktop
desktop-file-validate %{buildroot}%{_kf5_datadir}/applications/org.kde.PrintQueue.desktop
desktop-file-validate %{buildroot}%{_kf5_datadir}/applications/kcm_printer_manager.desktop


%files -f %{name}.lang
%license LICENSES/*.txt
%{_bindir}/kde-add-printer
%{_bindir}/kde-print-queue
%{_bindir}/configure-printer
%{_kf5_qmldir}/org/kde/plasma/printmanager/
%{_kf5_datadir}/plasma/plasmoids/org.kde.plasma.printmanager/
%{_kf5_datadir}/knotifications5/printmanager.notifyrc
%{_kf5_datadir}/applications/org.kde.kde-add-printer.desktop
%{_kf5_datadir}/applications/org.kde.ConfigurePrinter.desktop
%{_kf5_datadir}/applications/org.kde.PrintQueue.desktop
%{_kf5_datadir}/applications/kcm_printer_manager.desktop
%{_kf5_metainfodir}/org.kde.plasma.printmanager.appdata.xml
%{_kf5_metainfodir}/*.metainfo.xml

%ldconfig_scriptlets libs

%files libs
# private unversioned library
%{_libdir}/libkcupslib.so
%{_kf5_qtplugindir}/kf5/kded/printmanager.so
%{_kf5_qtplugindir}/plasma/kcms/systemsettings_qwidgets/kcm_printer_manager.so


%changelog
* Tue Jul 18 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.04.3-1
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

