Name:    kcron
Summary: Cron KDE configuration module
Version: 23.08.3
Release: 1%{?dist}

License: GPLv2+
URL:     https://cgit.kde.org/%{name}.git

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
BuildRequires: extra-cmake-modules
BuildRequires: kf5-rpm-macros
BuildRequires: kf5-kconfigwidgets-devel
BuildRequires: kf5-kdoctools-devel
BuildRequires: kf5-ki18n-devel
BuildRequires: kf5-kiconthemes-devel
BuildRequires: kf5-kio-devel
BuildRequires: cmake(KF5KCMUtils)
BuildRequires: cmake(Qt5PrintSupport)
BuildRequires: cmake(Qt5Widgets)

Obsoletes:     kdeadmin < 4.10.80

# translations moved here
Conflicts: kde-l10n < 17.03

%description
Systemsettings module for the cron task scheduler.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup


%build
%cmake_kf5
%cmake_build


%install
%cmake_install

%find_lang %{name} --all-name --with-html


%files -f %{name}.lang
%license LICENSES/*.txt
%doc README
%{_kf5_qtplugindir}/plasma/kcms/systemsettings_qwidgets/kcm_cron.so
%{_kf5_libexecdir}/kauth/kcron_helper
%{_kf5_datadir}/applications/kcm_cron.desktop
%{_kf5_datadir}/qlogging-categories5/*.categories
%{_kf5_datadir}/polkit-1/actions/local.kcron.crontab.policy
%{_kf5_datadir}/dbus-1/system.d/local.kcron.crontab.conf
%{_kf5_datadir}/dbus-1/system-services/local.kcron.crontab.service
%{_kf5_metainfodir}/*.metainfo.xml


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

