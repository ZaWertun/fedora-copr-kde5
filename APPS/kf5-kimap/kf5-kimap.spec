%global framework kimap
%global tests 1

Name:    kf5-%{framework}
Version: 23.08.2
Release: 1%{?dist}
Summary: The KIMAP Library

License: GPLv2+ and LGPLv2+
URL:     https://cgit.kde.org/%{framework}.git

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
BuildRequires:  boost-devel
BuildRequires:  cyrus-sasl-devel
BuildRequires:  extra-cmake-modules
BuildRequires:  kf5-rpm-macros
BuildRequires:  kf5-kcoreaddons-devel >= 5.23
BuildRequires:  kf5-kdelibs4support-devel >= 5.23
#global majmin_ver %(echo %{version} | cut -d. -f1,2)
%global majmin_ver %{version}
BuildRequires:  kf5-kmime-devel >= %{majmin_ver}
BuildRequires:  qt5-qtbase-devel
%if 0%{?tests}
BuildRequires: dbus-x11
BuildRequires: xorg-x11-server-Xvfb
%endif

%if ! 0%{?bootstrap}
# runtime sasl plugins
Suggests: cyrus-sasl-gssapi%{?_isa}
Suggests: cyrus-sasl-md5%{?_isa}
Requires: cyrus-sasl-plain%{?_isa}
%endif

# translations moved here
Conflicts: kde-l10n < 17.03

%description
%{summary}.

%package        devel
Summary:        Development files for %{name}
Requires:       %{name}%{?_isa} = %{version}-%{release}
Requires:       kf5-kcoreaddons-devel
Requires:       kf5-kmime-devel
Requires:       boost-devel
Requires:       cyrus-sasl-devel
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
%{_kf5_libdir}/libKPim5IMAP.so.*
%{_kf5_datadir}/qlogging-categories5/*categories


%files devel
%{_includedir}/KPim5/KIMAP/
%{_includedir}/KPim5/KIMAPTest/
%{_kf5_libdir}/libKPim5IMAP.so
%{_kf5_libdir}/cmake/KF5IMAP/
%{_kf5_libdir}/cmake/KPim5IMAP/
%{_kf5_libdir}/libkimaptest.a
%{_kf5_archdatadir}/mkspecs/modules/qt_KIMAP.pri


%changelog
* Fri Oct 13 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.08.2-1
- 23.08.2

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

