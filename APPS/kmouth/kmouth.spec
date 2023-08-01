Name:    kmouth
Version: 23.04.3
Release: 1%{?dist}
Summary: A program that speaks for you 

License: GPLv2+
URL:     http://accessibility.kde.org/
%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0: http://download.kde.org/%{stable}/release-service/%{version}/src/%{name}-%{version}.tar.xz
Source1: http://download.kde.org/%{stable}/release-service/%{version}/src/%{name}-%{version}.tar.xz.sig
Source2: gpgkey-D81C0CB38EB725EF6691C385BB463350D6EF31EF.gpg

BuildRequires: gnupg2
BuildRequires: desktop-file-utils
BuildRequires: libappstream-glib

BuildRequires: extra-cmake-modules
BuildRequires: cmake(KF5Crash)
BuildRequires: cmake(KF5Completion)
BuildRequires: cmake(KF5Config)
BuildRequires: cmake(KF5ConfigWidgets)
BuildRequires: cmake(KF5CoreAddons)
BuildRequires: cmake(KF5DocTools)
BuildRequires: cmake(KF5I18n)
BuildRequires: cmake(KF5KIO)
BuildRequires: cmake(KF5WidgetsAddons)
BuildRequires: cmake(KF5XmlGui)

BuildRequires: cmake(Qt5PrintSupport)
BuildRequires: cmake(Qt5TextToSpeech)
BuildRequires: cmake(Qt5Xml)
BuildRequires: cmake(Qt5Widgets)

# when split occured
Conflicts: kdeaccessibility < 1:4.7.80

Conflicts: kde-l10n < 17.08.3-2

%description
Program that allows people who have lost their voice to let their
computer speak for them.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -p1


%build
%cmake_kf5
%cmake_build


%install
%cmake_install

%find_lang %{name} --all-name --with-html --with-man


%check
appstream-util validate-relax --nonet %{buildroot}%{_kf5_metainfodir}/org.kde.%{name}.appdata.xml
desktop-file-validate %{buildroot}%{_kf5_datadir}/applications/org.kde.kmouth.desktop


%files -f %{name}.lang
%license COPYING*
%{_kf5_sysconfdir}/xdg/kmouthrc
%{_kf5_bindir}/kmouth
%{_kf5_datadir}/kmouth/
%{_kf5_datadir}/applications/org.kde.kmouth.desktop
%{_kf5_metainfodir}/org.kde.kmouth.appdata.xml
%{_kf5_datadir}/icons/hicolor/*/*/*
%{_kf5_datadir}/kxmlgui5/kmouth/
%{_mandir}/man1/*.1*


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

