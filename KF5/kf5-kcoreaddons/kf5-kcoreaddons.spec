%global framework kcoreaddons

# uncomment to enable bootstrap mode
#global bootstrap 1

%if !0%{?bootstrap}
#global tests 1
%endif

Name:    kf5-%{framework}
Version: 5.107.0
Release: 1%{?dist}
Summary: KDE Frameworks 5 Tier 1 addon with various classes on top of QtCore

License: LGPLv2+
URL:     https://cgit.kde.org/%{framework}.git

%global majmin %(echo %{version} | cut -d. -f1-2)
%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0: http://download.kde.org/%{stable}/frameworks/%{majmin}/%{framework}-%{version}.tar.xz
Source1: http://download.kde.org/%{stable}/frameworks/%{majmin}/%{framework}-%{version}.tar.xz.sig
Source2: gpgkey-53E6B47B45CEA3E0D5B7457758D0EE648A48B3BB.gpg

## upstream patches

BuildRequires:  gnupg2
BuildRequires:  extra-cmake-modules >= %{majmin}
BuildRequires:  kf5-rpm-macros >= %{majmin}
BuildRequires:  qt5-qtbase-devel
BuildRequires:  qt5-qttools-devel
%if ! 0%{?bootstrap}
## Drop/omit FAM/gamin support: it is no longer supported upstream,
## e.g. https://bugzilla.gnome.org/show_bug.cgi?id=777997
#BuildRequires:  gamin-devel
%endif
%if 0%{?tests}
BuildRequires: dbus-x11
BuildRequires: time
BuildRequires: xorg-x11-server-Xvfb
%endif

Requires:       kf5-filesystem >= %{majmin}

%description
KCoreAddons provides classes built on top of QtCore to perform various tasks
such as manipulating mime types, autosaving files, creating backup files,
generating random sequences, performing text manipulations such as macro
replacement, accessing user information and many more.

%package        devel
Summary:        Development files for %{name}
Requires:       %{name}%{?_isa} = %{version}-%{release}
Requires:       qt5-qtbase-devel
%description    devel
The %{name}-devel package contains libraries and header files for
developing applications that use %{name}.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -n %{framework}-%{version} -p1


%build
%cmake_kf5 \
  %{?tests:-DBUILD_TESTING:BOOL=ON}
%cmake_build


%install
%cmake_install

%find_lang_kf5 kcoreaddons5_qt


%check
%if 0%{?tests}
export CTEST_OUTPUT_ON_FAILURE=1
xvfb-run -a \
dbus-launch --exit-with-session \
time \
%make_build test ARGS="--output-on-failure --timeout 300" -C %{_target_platform} ||:
%endif


%if 0%{?rhel} && 0%{?rhel} < 8
%ldconfig_post

%postun
%{?ldconfig}
if [ $1 -eq 0 ] ; then
update-mime-database %{_datadir}/mime &> /dev/null || :
fi

%posttrans
update-mime-database %{_datadir}/mime &> /dev/null || :

%else
%ldconfig_scriptlets
%endif

%files -f kcoreaddons5_qt.lang
%doc README.md
%license LICENSES/*.txt
%{_kf5_bindir}/desktoptojson
%{_kf5_libdir}/libKF5CoreAddons.so.*
%{_kf5_datadir}/mime/packages/kde5.xml
%{_kf5_datadir}/kf5/licenses/
%{_kf5_datadir}/qlogging-categories5/kcoreaddons.categories
%{_kf5_datadir}/qlogging-categories5/kcoreaddons.renamecategories

%files devel
%{_kf5_includedir}/KCoreAddons/
%{_kf5_libdir}/libKF5CoreAddons.so
%{_kf5_libdir}/cmake/KF5CoreAddons/
%{_kf5_archdatadir}/mkspecs/modules/qt_KCoreAddons.pri


%changelog
* Sat Jun 10 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.107.0-1
- 5.107.0

* Sat May 13 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.106.0-1
- 5.106.0

* Sat Apr 08 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.105.0-1
- 5.105.0

* Sat Mar 11 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.104.0-1
- 5.104.0

* Sun Feb 12 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.103.0-1
- 5.103.0

