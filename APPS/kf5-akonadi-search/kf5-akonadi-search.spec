%global framework akonadi-search
%global tests 1

Name:    kf5-%{framework}
Version: 23.04.3
Release: 1%{?dist}
Summary: The Akonadi Search library and indexing agent

License: LGPLv2+
URL:     https://cgit.kde.org/%{framework}.git

%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0: https://download.kde.org/%{stable}/release-service/%{version}/src/%{framework}-%{version}.tar.xz
Source1: https://download.kde.org/%{stable}/release-service/%{version}/src/%{framework}-%{version}.tar.xz.sig
Source2: gpgkey-D81C0CB38EB725EF6691C385BB463350D6EF31EF.gpg

BuildRequires:  gnupg2
BuildRequires:  extra-cmake-modules
BuildRequires:  kf5-rpm-macros
BuildRequires:  kf5-ki18n-devel >= 5.15
BuildRequires:  kf5-kconfig-devel >= 5.15
BuildRequires:  kf5-kcrash-devel >= 5.15
BuildRequires:  kf5-krunner-devel >= 5.15
BuildRequires:  kf5-kcmutils-devel >= 5.15
%global majmin_ver %(echo %{version} | cut -d. -f1,2)
BuildRequires:  kf5-akonadi-mime-devel >= %{majmin_ver} 
BuildRequires:  kf5-akonadi-server-devel >= %{majmin_ver}
BuildRequires:  kf5-kcontacts-devel >= %{majmin_ver}
BuildRequires:  kf5-kcalendarcore-devel >= %{majmin_ver}
BuildRequires:  kf5-kmime-devel >= %{majmin_ver}
BuildRequires:  qt5-qtbase-devel
BuildRequires:  xapian-core-devel
%if 0%{?tests}
BuildRequires: dbus-x11
BuildRequires: time
BuildRequires: xorg-x11-server-Xvfb
%endif

%description
%{summary}.

%package        devel
Summary:        Development files for %{name}
Requires:       %{name}%{?_isa} = %{version}-%{release}
Requires:       qt5-qtbase-devel
Requires:       kf5-kcoreaddons-devel
Requires:       kf5-akonadi-server-devel
Requires:       kf5-akonadi-mime-devel
Requires:       kf5-kcontacts-devel
Requires:       kf5-kmime-devel
Requires:       kf5-kcalendarcore-devel
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
time \
make test ARGS="--output-on-failure --timeout 30" -C %{_vpath_builddir} ||:
%endif


%ldconfig_scriptlets

%files -f %{name}.lang
%doc README.md
%license LICENSES/*.txt
%{_kf5_libdir}/libKPim5AkonadiSearchPIM.so.*
%{_kf5_libdir}/libKPim5AkonadiSearchCore.so.*
%{_kf5_libdir}/libKPim5AkonadiSearchXapian.so.*
%{_kf5_libdir}/libKPim5AkonadiSearchDebug.so.*
%{_kf5_bindir}/akonadi_indexing_agent
%{_kf5_datadir}/akonadi/agents/akonadiindexingagent.desktop
%{_kf5_plugindir}/krunner/krunner_pimcontacts.so
%{_kf5_plugindir}/krunner/kcms/kcm_krunner_pimcontacts.so
%{_kf5_qtplugindir}/pim5/akonadi/*.so
%{_kf5_datadir}/qlogging-categories5/*categories

%files devel
%{_includedir}/KPim5/AkonadiSearch/
%{_kf5_libdir}/libKPim5AkonadiSearchPIM.so
%{_kf5_libdir}/libKPim5AkonadiSearchCore.so
%{_kf5_libdir}/libKPim5AkonadiSearchXapian.so
%{_kf5_libdir}/libKPim5AkonadiSearchDebug.so
%{_kf5_libdir}/cmake/KF5AkonadiSearch/
%{_kf5_libdir}/cmake/KPim5AkonadiSearch/


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

