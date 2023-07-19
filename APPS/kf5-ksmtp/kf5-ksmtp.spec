%global framework ksmtp
%global tests 0

Name:    kf5-%{framework}
Version: 23.04.3
Release: 1%{?dist}
Summary: KDE SMTP libraries

License: LGPLv2+
URL:     https://cgit.kde.org/%{framework}.git/

%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0:        https://download.kde.org/%{stable}/release-service/%{version}/src/%{framework}-%{version}.tar.xz
Source1:        https://download.kde.org/%{stable}/release-service/%{version}/src/%{framework}-%{version}.tar.xz.sig
Source2:        gpgkey-D81C0CB38EB725EF6691C385BB463350D6EF31EF.gpg

## upstream patches

## upstreamable patches

%global kf5_ver 5.29
BuildRequires: gnupg2
BuildRequires: extra-cmake-modules >= %{kf5_ver}
BuildRequires: cmake(KF5CoreAddons)
BuildRequires: cmake(KF5I18n)
BuildRequires: cmake(KF5KIO)

BuildRequires: cmake(Qt5Network)

BuildRequires: kf5-kmime-devel >= %{version}
BuildRequires: cmake(KF5Mime)

BuildRequires: pkgconfig(libsasl2)

%if 0%{?tests}
BuildRequires: dbus-x11
BuildRequires: xorg-x11-server-Xvfb
%endif

# runtime sasl plugins
Suggests: cyrus-sasl-gssapi%{?_isa}
Suggests: cyrus-sasl-md5%{?_isa}
Requires: cyrus-sasl-plain%{?_isa}

%description
%{summary}.

%package        devel
Summary:        Development files for %{name}
Requires:       %{name}%{?_isa} = %{version}-%{release}
Requires:       cmake(KF5CoreAddons)
Requires:       cmake(KF5Mime)
Requires:       kf5-kmime-devel >= %{version}
%description    devel
%{summary}.


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
make test/fast ARGS="--output-on-failure --timeout 30" -C %{_vpath_builddir} ||:
%endif


%ldconfig_scriptlets

%files -f %{name}.lang
%license LICENSES/*.txt
%{_kf5_libdir}/libKPim5SMTP.so.*
%{_kf5_datadir}/qlogging-categories5/*.categories


%files devel
%{_includedir}/KPim5/KSMTP/
%{_includedir}/KPim5/ksmtp_version.h
%{_kf5_libdir}/libKPim5SMTP.so
%{_kf5_libdir}/cmake/KPim5SMTP/
%{_kf5_libdir}/cmake/KPimSMTP/
%{_kf5_archdatadir}/mkspecs/modules/qt_KSMTP.pri


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

