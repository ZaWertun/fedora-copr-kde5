%global framework libkleo
%global tests 1

Name:    kf5-%{framework}
Version: 23.08.0
Release: 1%{?dist}
Summary: KDE PIM cryptographic library

License: GPLv2+
URL:     https://cgit.kde.org/%{framework}.git/

%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0: https://download.kde.org/%{stable}/release-service/%{version}/src/%{framework}-%{version}.tar.xz
Source1: https://download.kde.org/%{stable}/release-service/%{version}/src/%{framework}-%{version}.tar.xz.sig
Source2: gpgkey-D81C0CB38EB725EF6691C385BB463350D6EF31EF.gpg

## upstream patches

BuildRequires:  gnupg2
BuildRequires:  boost-devel

BuildRequires:  cmake(Qt5Widgets)

BuildRequires:  gpgmepp-devel >= 1.7.1
BuildRequires:  qgpgme-devel
# workaround gpgmepp-devel missing Requires: libassuan-devel for now
BuildRequires:  libassuan-devel
# kf5
BuildRequires:  extra-cmake-modules >= 5.19.0
BuildRequires:  kf5-rpm-macros >= 5.19.0
BuildRequires:  cmake(KF5Codecs)
BuildRequires:  cmake(KF5Completion)
BuildRequires:  cmake(KF5Config)
BuildRequires:  cmake(KF5CoreAddons)
BuildRequires:  cmake(KF5CoreAddons)
BuildRequires:  cmake(KF5I18n)
BuildRequires:  cmake(KF5ItemModels)
BuildRequires:  cmake(KF5WidgetsAddons)
BuildRequires:  cmake(KF5WindowSystem)
BuildRequires:  cmake(KF5ConfigWidgets)
# kde-apps
%global majmin_ver %(echo %{version} | cut -d. -f1,2)
BuildRequires:  kf5-kpimtextedit-devel >= %{majmin_ver}

%if 0%{?tests}
BuildRequires: dbus-x11
BuildRequires: mesa-dri-drivers
BuildRequires: xorg-x11-server-Xvfb
%endif

Obsoletes:      kdepim-libs < 7:16.04.0

# gpg support ui
%if 0%{?fedora} < 26 && 0%{?rhel} < 8
Requires:       pinentry-gui
%else
Recommends:     pinentry-gui
%endif

# translations moved here
Conflicts: kde-l10n < 17.03

%description
%{summary}.

%package        devel
Summary:        Development files for %{name}
Requires:       %{name}%{?_isa} = %{version}-%{release}
# INTERFACE_LINK_LIBRARIES "QGpgme;Gpgmepp"
Requires:       cmake(Gpgmepp)
Requires:       cmake(QGpgme)
%description    devel
The %{name}-devel package contains libraries and header files for
developing applications that use %{name}.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -n %{framework}-%{version} -p1


%build
%cmake_kf5 \
  -DBUILD_TESTING:BOOL=%{?tests:ON}%{!?tests:OFF}
%cmake_build


%install
%cmake_install
%find_lang %{name} --all-name --with-html


%check
%if 0%{?tests}
export CTEST_OUTPUT_ON_FAILURE=1
xvfb-run -a \
dbus-launch --exit-with-session \
make test ARGS="--output-on-failure --timeout 30" -C %{_vpath_builddir} ||:
%endif


%ldconfig_scriptlets

%files -f %{name}.lang
%license LICENSES/*.txt
%{_kf5_sysconfdir}/xdg/libkleopatrarc
%{_kf5_libdir}/libKPim5Libkleo.so.*
%{_kf5_datadir}/libkleopatra/
%{_kf5_datadir}/qlogging-categories5/*categories


%files devel
%{_includedir}/KPim5/Libkleo/
%{_kf5_libdir}/libKPim5Libkleo.so
%{_kf5_libdir}/cmake/KF5Libkleo/
%{_kf5_libdir}/cmake/KPim5Libkleo/
%{_kf5_archdatadir}/mkspecs/modules/qt_Libkleo.pri


%changelog
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

