%global framework grantleetheme
%global tests 1

Name:    kf5-%{framework}
Version: 23.04.2
Release: 1%{?dist}
Summary: KDE PIM library for Grantlee template system

License: LGPLv2+
URL:     http://cgit.kde.org/%{framework}.git

%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0:        https://download.kde.org/%{stable}/release-service/%{version}/src/%{framework}-%{version}.tar.xz
Source1:        https://download.kde.org/%{stable}/release-service/%{version}/src/%{framework}-%{version}.tar.xz.sig
Source2:        gpgkey-D81C0CB38EB725EF6691C385BB463350D6EF31EF.gpg

BuildRequires:  gnupg2
BuildRequires:  extra-cmake-modules >= 5.32.0
BuildRequires:  kf5-rpm-macros

BuildRequires:  cmake(Qt5Network)
BuildRequires:  cmake(Qt5Test)
BuildRequires:  cmake(Qt5Widgets)

# when macros.grantlee5 was introduced
BuildRequires:  grantlee-qt5-devel >= 5.1.0-2
%{?grantlee5_requires}

BuildRequires:  cmake(KF5I18n)
BuildRequires:  cmake(KF5IconThemes)
BuildRequires:  cmake(KF5NewStuff)
BuildRequires:  cmake(KF5GuiAddons)

%if 0%{?tests}
BuildRequires:  dbus-x11
BuildRequires:  xorg-x11-server-Xvfb
%endif

Requires:       kf5-filesystem

Conflicts:      kdepim-libs < 7:16.04.0
Obsoletes:      kdepim-libs < 7:16.04.0

# translations moved here
Conflicts: kde-l10n < 17.03

%description
%{summary}.

%package        devel
Summary:        Development files for %{name}
Requires:       %{name}%{?_isa} = %{version}-%{release}
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
%{_kf5_libdir}/libKPim5GrantleeTheme.so.*
%{grantlee5_plugindir}/kde_grantlee_plugin.so
%{_kf5_datadir}/qlogging-categories5/*categories


%files devel
%{_includedir}/KPim5/GrantleeTheme/
%{_kf5_libdir}/libKPim5GrantleeTheme.so
%{_kf5_libdir}/cmake/KF5GrantleeTheme/
%{_kf5_libdir}/cmake/KPim5GrantleeTheme/
%{_kf5_archdatadir}/mkspecs/modules/qt_GrantleeTheme.pri


%changelog
* Thu Jun 08 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.04.2-1
- 23.04.2

* Thu May 11 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.04.1-1
- 23.04.1

* Thu Apr 20 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.04.0-1
- 23.04.0

* Sat Mar 25 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.12.3-6
- rebuild

* Sat Mar 25 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.12.3-5
- rebuild

