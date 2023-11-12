Name:    ksystemlog
Summary: System Log Viewer for KDE
Version: 23.08.3
Release: 1%{?dist}

License: GPLv2+
URL:     http://www.kde.org/applications/system/ksystemlog/
%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0: https://download.kde.org/%{stable}/release-service/%{version}/src/%{name}-%{version}.tar.xz
Source1: https://download.kde.org/%{stable}/release-service/%{version}/src/%{name}-%{version}.tar.xz.sig
Source2: gpgkey-D81C0CB38EB725EF6691C385BB463350D6EF31EF.gpg

## upstreamable patches

## downstream patches
# fix ksystemlog to find log files in fedora locations
Patch1: ksystemlog-21.04.0-fedora.patch

BuildRequires: gnupg2
BuildRequires: desktop-file-utils
BuildRequires: extra-cmake-modules
BuildRequires: kf5-rpm-macros
BuildRequires: kf5-kxmlgui-devel
BuildRequires: kf5-kcoreaddons-devel
BuildRequires: kf5-kwidgetsaddons-devel
BuildRequires: kf5-kitemviews-devel
BuildRequires: kf5-kiconthemes-devel
BuildRequires: kf5-kio-devel
BuildRequires: kf5-kconfig-devel
BuildRequires: kf5-karchive-devel
BuildRequires: kf5-kdoctools-devel
BuildRequires: kf5-ki18n-devel
BuildRequires: kf5-kcompletion-devel
BuildRequires: kf5-ktextwidgets-devel
BuildRequires: pkgconfig(libsystemd)
BuildRequires: pkgconfig(Qt5Gui)

Obsoletes:     kdeadmin < 4.10.80
# translations moved here
Conflicts: kde-l10n < 17.03

%description
This program is developed for beginner users, who don't know how to find
information about their Linux system, and don't know where log files are.

It is also of course designed for advanced users, who quickly want to understand
problems of their machine with a more powerful and graphical tool than tail -f
and less commands.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -n %{name}-%{version} -p1


%build
%cmake_kf5
%cmake_build


%install
%cmake_install
%find_lang %{name} --all-name --with-html


%check
desktop-file-validate %{buildroot}%{_kf5_datadir}/applications/org.kde.%{name}.desktop


%files -f %{name}.lang
%license LICENSES/*.txt
%{_kf5_bindir}/ksystemlog
%{_kf5_datadir}/kxmlgui5/ksystemlog/
%{_kf5_metainfodir}/org.kde.ksystemlog.appdata.xml
%{_kf5_datadir}/applications/org.kde.ksystemlog.desktop
%{_kf5_datadir}/qlogging-categories5/%{name}.categories


%changelog
* Fri Nov 10 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.08.3-1
- 23.08.3

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

