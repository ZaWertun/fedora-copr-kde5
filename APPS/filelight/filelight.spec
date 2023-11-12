Name:    filelight
Summary: Graphical disk usage statistics 
Epoch:   1
Version: 23.08.3
Release: 1%{?dist}

# KDE e.V. may determine that future GPL versions are accepted
License: GPLv2 or GPLv3
URL:     http://utils.kde.org/projects/filelight

%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0: https://download.kde.org/%{stable}/release-service/%{version}/src/%{name}-%{version}.tar.xz
Source1: https://download.kde.org/%{stable}/release-service/%{version}/src/%{name}-%{version}.tar.xz.sig
Source2: gpgkey-D81C0CB38EB725EF6691C385BB463350D6EF31EF.gpg

BuildRequires: gnupg2
BuildRequires: desktop-file-utils
BuildRequires: libappstream-glib

BuildRequires: extra-cmake-modules
BuildRequires: kf5-rpm-macros
BuildRequires: cmake(KF5KIO)
BuildRequires: cmake(KF5I18n)
BuildRequires: cmake(KF5XmlGui)
BuildRequires: cmake(KF5DocTools)
BuildRequires: cmake(KF5Declarative)

BuildRequires: cmake(Qt5Gui)
BuildRequires: cmake(Qt5Svg)
BuildRequires: cmake(Qt5Quick)
BuildRequires: cmake(Qt5Widgets)
BuildRequires: cmake(Qt5QuickControls2)

# when split occured
Conflicts: kdeutils-common < 6:4.7.80

# translations moved here
Conflicts: kde-l10n < 17.03

Obsoletes: kdeutils-filelight < 6:4.7.80
Provides:  kdeutils-filelight = 6:%{version}-%{release}


%description
Filelight allows you to quickly understand exactly where your diskspace
is being used by graphically representing your file system.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup


%build
%cmake_kf5
%cmake_build


%install
%cmake_install

%find_lang %{name} --all-name --with-html


%check
appstream-util validate-relax --nonet %{buildroot}%{_kf5_metainfodir}/org.kde.%{name}.appdata.xml ||:
desktop-file-validate %{buildroot}%{_kf5_datadir}/applications/org.kde.%{name}.desktop

%files -f %{name}.lang
%license LICENSES/*.txt
%doc AUTHORS README*
%{_kf5_bindir}/filelight
%{_kf5_datadir}/applications/org.kde.filelight.desktop
%{_kf5_metainfodir}/org.kde.filelight.appdata.xml
%{_kf5_datadir}/icons/hicolor/*/*/*filelight.*
%{_sysconfdir}/xdg/filelightrc
%{_kf5_datadir}/kxmlgui5/filelight/
%{_kf5_datadir}/qlogging-categories5/*.categories


%changelog
* Fri Nov 10 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 1:23.08.3-1
- 23.08.3

* Fri Oct 13 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 1:23.08.2-1
- 23.08.2

* Thu Sep 14 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 1:23.08.1-1
- 23.08.1

* Sun Aug 27 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 1:23.08.0-1
- 23.08.0

* Thu Jul 06 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 1:23.04.3-1
- 23.04.3

* Thu Jun 08 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 1:23.04.2-1
- 23.04.2

* Thu May 11 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 1:23.04.1-1
- 23.04.1

* Thu Apr 20 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 1:23.04.0-1
- 23.04.0

* Thu Mar 02 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 1:22.12.3-1
- 22.12.3

* Thu Feb 02 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 1:22.12.2-1
- 22.12.2

