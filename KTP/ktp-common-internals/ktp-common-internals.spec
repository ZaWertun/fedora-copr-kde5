Name:    ktp-common-internals
Summary: Common internals for KDE Telepathy
Version: 23.08.2
Release: 1%{?dist}

License: LGPLv2+
URL:     https://cgit.kde.org/%{name}.git

%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0: http://download.kde.org/stable/release-service/%{version}/src/%{name}-%{version}.tar.xz
Source1: http://download.kde.org/stable/release-service/%{version}/src/%{name}-%{version}.tar.xz.sig
Source2: gpgkey-D81C0CB38EB725EF6691C385BB463350D6EF31EF.gpg

# due to kaccounts-providers
# handled by qt5-srpm-macros, which defines %%qt5_qtwebengine_arches
%{?qt5_qtwebengine_arches:ExclusiveArch: %{qt5_qtwebengine_arches}}

BuildRequires:  gnupg2
BuildRequires:  extra-cmake-modules
BuildRequires:  kf5-rpm-macros
BuildRequires:  qt5-qtbase-devel
BuildRequires:  qt5-qtdeclarative-devel

BuildRequires:  kf5-kcoreaddons-devel
BuildRequires:  kf5-knotifications-devel
BuildRequires:  kf5-kiconthemes-devel
BuildRequires:  kf5-kio-devel
BuildRequires:  kf5-kwidgetsaddons-devel
BuildRequires:  kf5-kcmutils-devel
BuildRequires:  kf5-knotifyconfig-devel
BuildRequires:  kf5-ktexteditor-devel
BuildRequires:  kf5-kwallet-devel
BuildRequires:  kf5-kconfig-devel
BuildRequires:  kf5-kpeople-devel
BuildRequires:  kf5-kwindowsystem-devel
BuildRequires:  cmake(KF5SyntaxHighlighting)

BuildRequires:  pkgconfig(mission-control-plugins)
BuildRequires:  pkgconfig(TelepathyQt5)
BuildRequires:  pkgconfig(accounts-qt5)
BuildRequires:  pkgconfig(libaccounts-glib)
BuildRequires:  telepathy-logger-qt-devel >= 15.04

%global majmin_ver %(echo %{version} | cut -d. -f1,2)
BuildRequires:  kaccounts-integration-devel >= %{majmin_ver}

BuildRequires:  libotr-devel
BuildRequires:  libgcrypt-devel

Requires:       kaccounts-providers
Requires:       telepathy-accounts-signon%{?_isa}

# These did not get 15.04.0 release and have been retired as their
# ktp-common-internals-0.9 dependency cannot be satisfied
Obsoletes:      ktp-call-ui < 15.04.0
Obsoletes:      kte-collaborative < 15.04.0

# translations moved here
Conflicts: kde-l10n < 17.03

%description
%{summary}.

%package        devel
Summary:        Development files for %{name}
Requires:       %{name}%{?_isa} = %{version}-%{release}
Requires:       telepathy-qt5-devel
Requires:       kf5-kwallet-devel
Requires:       telepathy-logger-qt-devel
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


%ldconfig_scriptlets

%files -f %{name}.lang
%license COPYING*
%{_bindir}/ktp-debugger
%{_libexecdir}/ktp-proxy
%{_kf5_qtplugindir}/*.so
%{_kf5_datadir}/kservices5/*.desktop
%{_kf5_datadir}/kservicetypes5/ktp_logger_plugin.desktop
%{_kf5_qtplugindir}/kaccounts/daemonplugins/*.so
%dir %{_kf5_qtplugindir}/kpeople/
%dir %{_kf5_qtplugindir}/kpeople/actions
%{_kf5_qtplugindir}/kpeople/actions/ktp_kpeople_plugin.so
%dir %{_kf5_qtplugindir}/kpeople/datasource/
%{_kf5_qtplugindir}/kpeople/datasource/im_persons_data_source_plugin.so
%dir %{_kf5_qtplugindir}/kpeople/widgets
%{_kf5_qtplugindir}/kpeople/widgets/imdetailswidgetplugin.so
%{_kf5_qtplugindir}/kpeople/widgets/kpeople_chat_plugin.so
%{_libdir}/libKTpCommonInternals.so.*
%{_libdir}/libKTpModels.so.*
%{_libdir}/libKTpWidgets.so.*
%{_libdir}/libKTpLogger.so.*
%{_libdir}/libKTpOTR.so.*
%{_kf5_qmldir}/org/kde/telepathy
%{_kf5_datadir}/katepart5/syntax/ktpdebugoutput.xml
%{_kf5_datadir}/knotifications5/*.notifyrc
%{_datadir}/icons/hicolor/*/*/*
%{_datadir}/config.kcfg/ktp-proxy-config.kcfg
%{_datadir}/dbus-1/services/*.service
%{_datadir}/telepathy/clients/KTp.Proxy.client

%files devel
%{_libdir}/cmake/KTp/
%{_libdir}/libKTpCommonInternals.so
%{_libdir}/libKTpModels.so
%{_libdir}/libKTpWidgets.so
%{_libdir}/libKTpLogger.so
%{_libdir}/libKTpOTR.so
%{_includedir}/KTp/


%changelog
* Fri Oct 13 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.08.2-1
- 23.08.2

* Thu Sep 14 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.08.1-1
- 23.08.1

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

