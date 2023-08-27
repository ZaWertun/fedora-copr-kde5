Name:    dolphin-plugins
Summary: Dolphin plugins for revision control systems
Version: 23.08.0
Release: 1%{?dist}

License: GPLv2+
URL:     https://cgit.kde.org/%{name}.git/

%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0: https://download.kde.org/%{stable}/release-service/%{version}/src/%{name}-%{version}.tar.xz
Source1: https://download.kde.org/%{stable}/release-service/%{version}/src/%{name}-%{version}.tar.xz.sig
Source2: gpgkey-D81C0CB38EB725EF6691C385BB463350D6EF31EF.gpg

BuildRequires:  gnupg2
BuildRequires:  desktop-file-utils
%global majmin_ver %(echo %{version} | cut -d. -f1,2)
BuildRequires:  dolphin-devel >= %{majmin_ver}
BuildRequires:  extra-cmake-modules
BuildRequires:  kf5-rpm-macros
BuildRequires:  kf5-kdelibs4support-devel
BuildRequires:  kf5-ki18n-devel
BuildRequires:  kf5-kio-devel
BuildRequires:  kf5-ktexteditor-devel
BuildRequires:  kf5-kxmlgui-devel
BuildRequires:  cmake(KF5SyntaxHighlighting)

BuildRequires:  qt5-qtbase-devel

Requires:       dolphin >= %{majmin_ver}

# translations moved here (omitting since dolphin already has it)
#Conflicts: kde-l10n < 17.03

Conflicts:      kdesdk-common < 4.10.80
Provides:       kdesdk-dolphin-plugins = %{version}-%{release}
Obsoletes:      kdesdk-dolphin-plugins < 4.10.80

%description
Plugins for the Dolphin file manager integrating the following revision control
systems:
* Dropbox
* Git
* Subversion (SVN)
* Bazaar (Bzr)

%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup


%build
%cmake_kf5
%cmake_build


%install
%cmake_install

%find_lang %{name} --all-name


%files -f %{name}.lang
%license LICENSES/*.txt
%{_kf5_plugindir}/kfileitemaction/mountisoaction.so
%{_kf5_qtplugindir}/dolphin/vcs/fileviewbazaarplugin.so
%{_kf5_qtplugindir}/dolphin/vcs/fileviewdropboxplugin.so
%{_kf5_qtplugindir}/dolphin/vcs/fileviewgitplugin.so
%{_kf5_qtplugindir}/dolphin/vcs/fileviewhgplugin.so
%{_kf5_qtplugindir}/dolphin/vcs/fileviewsvnplugin.so
%{_kf5_datadir}/config.kcfg/fileviewgitpluginsettings.kcfg
%{_kf5_datadir}/config.kcfg/fileviewsvnpluginsettings.kcfg
%{_kf5_datadir}/config.kcfg/fileviewhgpluginsettings.kcfg
%{_kf5_metainfodir}/*.metainfo.xml


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

