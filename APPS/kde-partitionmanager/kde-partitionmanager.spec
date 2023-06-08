Name:           kde-partitionmanager
Version:        23.04.2
Release:        1%{?dist}
Summary:        KDE Partition Manager

License:        GPLv3+
URL:            https://invent.kde.org/system/partitionmanager
Source0:        http://download.kde.org/stable/release-service/%{version}/src/partitionmanager-%{version}.tar.xz
Source1:        http://download.kde.org/stable/release-service/%{version}/src/partitionmanager-%{version}.tar.xz.sig
Source2:        gpgkey-D81C0CB38EB725EF6691C385BB463350D6EF31EF.gpg

BuildRequires:  gnupg2
BuildRequires:  cmake
BuildRequires:  desktop-file-utils
BuildRequires:  extra-cmake-modules
BuildRequires:  gettext
BuildRequires:  kf5-kconfig-devel
BuildRequires:  kf5-kconfigwidgets-devel
BuildRequires:  kf5-kcoreaddons-devel
BuildRequires:  kf5-kcrash-devel
BuildRequires:  kf5-ki18n-devel
BuildRequires:  kf5-kiconthemes-devel
BuildRequires:  kf5-kjobwidgets-devel
BuildRequires:  kf5-kio-devel
BuildRequires:  kf5-rpm-macros
BuildRequires:  kf5-kservice-devel
BuildRequires:  kf5-kxmlgui-devel
BuildRequires:  kf5-kwidgetsaddons-devel
BuildRequires:  kf5-kdbusaddons-devel
BuildRequires:  kpmcore-devel = %{version}
BuildRequires:  qt5-qtbase-devel

BuildRequires:  cmake(PolkitQt5-1)
BuildRequires:  cmake(KF5DocTools)

BuildRequires:  pkgconfig(appstream-glib)
BuildRequires:  pkgconfig(blkid)
BuildRequires:  pkgconfig(libatasmart)

Requires:       kpmcore = %{version}
Requires:       kf5-filesystem

%description
KDE Partition Manager is a utility program to help you manage the disk devices,
partitions and file systems on your computer. It allows you to easily create, 
copy, move, delete, resize without losing data, backup and restore partitions.
 
KDE Partition Manager supports a large number of file systems, 
including ext2/3/4, reiserfs, NTFS, FAT16/32, jfs, xfs and more.
 
Starting from version 1.9.50 KDE Partition Manager has become the GUI part of 
KPMcore (KDE PartitionManager core) which contain the libraries used to 
manipulate filesystems.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -p1 -n partitionmanager-%{version}


%build
%cmake_kf5
%cmake_build


%install
%cmake_install
%find_lang partitionmanager --with-kde --with-html
sed -i 's|SingleMainWindow=True|SingleMainWindow=true|' %{buildroot}%{_kf5_datadir}/applications/org.kde.partitionmanager.desktop


%check
# Validate .desktop file
desktop-file-validate %{buildroot}%{_kf5_datadir}/applications/*partitionmanager.desktop

# Validate appdata file
appstream-util validate-relax --nonet %{buildroot}/%{_datadir}/metainfo/*.appdata.xml


%files -f partitionmanager.lang
%license LICENSES/*.txt
%{_kf5_bindir}/partitionmanager
%{_kf5_datadir}/applications/*partitionmanager.desktop
%{_kf5_datadir}/config.kcfg/partitionmanager.kcfg
%{_kf5_datadir}/kxmlgui5/partitionmanager/
%{_kf5_datadir}/solid/actions/open_in_partitionmanager.desktop
%{_datadir}/icons/hicolor/*/*/*
%{_datadir}/metainfo/*partitionmanager.appdata.xml

%changelog
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

