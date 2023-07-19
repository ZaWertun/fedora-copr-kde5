Name:    kgpg
Summary: Manage GPG encryption keys 
Version: 23.04.3
Release: 1%{?dist}

License: GPLv2+
URL:     https://www.kde.org/applications/utilities/kgpg/

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

BuildRequires: cmake(Qt5DBus)
BuildRequires: cmake(Qt5PrintSupport)
BuildRequires: cmake(Qt5Widgets)

BuildRequires: extra-cmake-modules
BuildRequires: kf5-rpm-macros
BuildRequires: cmake(KF5Archive)
BuildRequires: cmake(KF5DocTools)
BuildRequires: cmake(KF5Codecs)
BuildRequires: cmake(KF5CoreAddons)
BuildRequires: cmake(KF5Crash)
BuildRequires: cmake(KF5DBusAddons)
BuildRequires: cmake(KF5I18n)
BuildRequires: cmake(KF5IconThemes)
BuildRequires: cmake(KF5JobWidgets)
BuildRequires: cmake(KF5KIO)
BuildRequires: cmake(KF5Notifications)
BuildRequires: cmake(KF5Service)
BuildRequires: cmake(KF5TextWidgets)
BuildRequires: cmake(KF5XmlGui)
BuildRequires: cmake(KF5WidgetsAddons)
BuildRequires: cmake(KF5WindowSystem)

# pim-related deps below are available only where qtwebengine is
%{?qt5_qtwebengine_arches:ExclusiveArch: %{qt5_qtwebengine_arches}}
BuildRequires: cmake(KF5AkonadiContact)
BuildRequires: cmake(KF5GrantleeTheme)
BuildRequires: cmake(KF5Contacts)
BuildRequires: cmake(Grantlee5)

# support kde4 servicemenus too
BuildRequires: kde-filesystem
Requires:      kde-filesystem

BuildRequires: gpgme-devel

# when split occured
Conflicts: kdeutils-common < 6:4.7.80

# translations moved here
Conflicts: kde-l10n < 17.03

Obsoletes: kdeutils-kgpg < 6:4.7.80
Provides:  kdeutils-kgpg = 6:%{version}-%{release}

# kgpg (can be either gnupg or gnupg2, we'll default to the latter)
Requires: gnupg2

%description
KGpg is a simple interface for GnuPG, a powerful encryption utility.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup


%build
%cmake_kf5
%cmake_build


%install
%cmake_install
%find_lang %{name} --all-name --with-html

# only plasma supports X-KDE-autostart-condition, else it starts unconditionally
# everywhere else, see also https://bugzilla.redhat.com/1427707
desktop-file-edit \
  --add-only-show-in=KDE \
  %{buildroot}%{_kf5_sysconfdir}/xdg/autostart/org.kde.kgpg.desktop


%check
appstream-util validate-relax --nonet %{buildroot}%{_kf5_metainfodir}/org.kde.%{name}.appdata.xml
desktop-file-validate %{buildroot}%{_kf5_datadir}/applications/org.kde.%{name}.desktop


%files -f %{name}.lang
%doc AUTHORS
%license LICENSES/*.txt
%{_kf5_bindir}/kgpg
%{_kf5_sysconfdir}/xdg/autostart/org.kde.kgpg.desktop
%{_kf5_metainfodir}/org.kde.kgpg.appdata.xml
%{_kf5_datadir}/applications/org.kde.kgpg.desktop
%{_kf5_datadir}/config.kcfg/kgpg.kcfg
%{_kf5_datadir}/dbus-1/interfaces/org.kde.kgpg.Key.xml
%{_kf5_datadir}/kgpg/
%{_kf5_datadir}/kio/servicemenus/%{name}_*.desktop
%{_kf5_datadir}/kxmlgui5/kgpg/
%{_kf5_datadir}/icons/hicolor/*/*/*
%{_kf5_datadir}/qlogging-categories5/*categories


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

