Name:    ktp-desktop-applets
Summary: KDE Telepathy desktop applets
Version: 23.04.3
Release: 1%{?dist}

License: GPLv2+
URL:     https://cgit.kde.org/%{name}.git

%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0: http://download.kde.org/stable/release-service/%{version}/src/%{name}-%{version}.tar.xz
Source1: http://download.kde.org/stable/release-service/%{version}/src/%{name}-%{version}.tar.xz.sig
Source2: gpgkey-D81C0CB38EB725EF6691C385BB463350D6EF31EF.gpg

BuildRequires:  gnupg2
BuildRequires:  extra-cmake-modules
BuildRequires:  kf5-rpm-macros
BuildRequires:  qt5-qtbase-devel
BuildRequires:  qt5-qtdeclarative-devel

BuildRequires:  kf5-plasma-devel
BuildRequires:  kf5-kwindowsystem-devel
BuildRequires:  kf5-kcoreaddons-devel
BuildRequires:  kf5-ki18n-devel
BuildRequires:  kf5-kpackage-devel
BuildRequires:  kf5-kservice-devel

Obsoletes:      telepathy-kde-presence-applet < 0.3.0
Provides:       telepathy-kde-presence-applet = %{version}-%{release}

# not sure where best to put this other than here -- rex
Obsoletes:      telepathy-kde-presence-dataengine < 0.3.0
Provides:       telepathy-kde-presence-dataengine = %{version}-%{release}

Obsoletes:      ktp-contact-applet < 0.5.80
Obsoletes:      ktp-presence-applet < 0.5.80
Provides:       ktp-contact-applet = %{version}-%{release}
Provides:       ktp-presence-applet = %{version}-%{release}

# translations moved here
Conflicts: kde-l10n < 17.03

%description
KDE Telepathy desktop applets, including:
* contacts
* presence


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -p1


%build
%cmake_kf5
%cmake_build


%install
%cmake_install

%find_lang %{name} --all-name --with-html


%files -f %{name}.lang
%license COPYING*
%{_kf5_datadir}/plasma/plasmoids/org.kde.person/
%{_kf5_datadir}/plasma/plasmoids/org.kde.ktp-chat/
%{_kf5_datadir}/plasma/plasmoids/org.kde.ktp-contactlist/
%{_kf5_datadir}/kservices5/*.desktop
%{_kf5_qmldir}/org/kde/ktpchat/
%{_kf5_qmldir}/org/kde/ktpcontactlist/


%changelog
* Tue Jul 18 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.04.3-1
- 23.04.3

* Thu Jun 08 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.04.2-1
- 23.04.2

* Thu May 11 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.04.1-1
- 23.04.1

* Fri Apr 21 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.04.0-1
- 23.04.0

* Thu Mar 02 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.12.3-1
- 22.12.3

* Thu Jan 05 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.12.1-1
- 22.12.1

