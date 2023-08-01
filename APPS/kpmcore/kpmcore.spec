%global unstable 0

Name:           kpmcore
Version:        23.04.3
Release:        1%{?dist}
Summary:        Library for managing partitions by KDE programs
License:        GPLv3+
URL:            https://invent.kde.org/system/kpmcore
Source0:        http://download.kde.org/stable/release-service/%{version}/src/%{name}-%{version}.tar.xz
Source1:        http://download.kde.org/stable/release-service/%{version}/src/%{name}-%{version}.tar.xz.sig
Source2:        gpgkey-D81C0CB38EB725EF6691C385BB463350D6EF31EF.gpg

BuildRequires:  gnupg2
BuildRequires:  cmake
BuildRequires:  extra-cmake-modules
BuildRequires:  gettext
BuildRequires:  kf5-kcoreaddons-devel
BuildRequires:  kf5-ki18n-devel
BuildRequires:  kf5-kwidgetsaddons-devel
BuildRequires:  kf5-kauth-devel
BuildRequires:  qt5-qtbase-devel
BuildRequires:  kf5-rpm-macros
BuildRequires:  qca-qt5-devel

BuildRequires:  pkgconfig
BuildRequires:  pkgconfig(blkid)
BuildRequires:  pkgconfig(libatasmart)
BuildRequires:  pkgconfig(libparted)

BuildRequires:  cmake(PolkitQt5-1)

Requires:       parted
Requires:       e2fsprogs
Requires:       kf5-filesystem

Recommends:     f2fs-tools
Recommends:     hfsutils
Recommends:     hfsplus-tools
Recommends:     jfsutils
Recommends:     nilfs-utils
Recommends:     reiserfs-utils
Recommends:     udftools

%description
KPMcore contains common code for managing partitions by KDE Partition Manager 
and other KDE projects


%package        devel
Summary:        Development files for %{name}
Requires:       %{name}%{?_isa} = %{version}-%{release}
Requires:       qt5-qtbase-devel

%description    devel
The %{name}-devel package contains libraries and header files for
developing applications that use %{name}


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -p1

%build
sed -i 's|set(BLKID_MIN_VERSION "2.33.2")|set(BLKID_MIN_VERSION "2.32.1")|' CMakeLists.txt
%cmake_kf5
%cmake_build


%install
%cmake_install
%find_lang %{name} --with-kde --all-name


%files -f %{name}.lang
%license LICENSES/*.txt
%{_kf5_libdir}/libkpmcore.so.*
%{_kf5_qtplugindir}/kpmcore/*.so
%{_kf5_datadir}/polkit-1/actions/*.policy
%{_kf5_datadir}/dbus-1/system-services/*.service
%{_kf5_datadir}/dbus-1/system.d/org.kde.kpmcore.*.conf
%{_libexecdir}/kpmcore_externalcommand


%files devel
%{_includedir}/%{name}/
%{_kf5_libdir}/cmake/KPMcore
%{_kf5_libdir}/libkpmcore.so


%changelog
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

