Name:    ktp-approver
Summary: KDE Channel Approver for Telepathy
Version: 23.04.3
Release: 1%{?dist}

License: LGPLv2+
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
BuildRequires:  gettext
BuildRequires:  kf5-rpm-macros
BuildRequires:  kf5-kdbusaddons-devel
BuildRequires:  kf5-kconfig-devel
BuildRequires:  kf5-ki18n-devel
BuildRequires:  kf5-knotifications-devel
BuildRequires:  kf5-kservice-devel

BuildRequires:  qt5-qtbase-devel
BuildRequires:  telepathy-qt5-devel

Obsoletes:      telepathy-kde-approver < 0.3.0
Provides:       telepathy-kde-approver = %{version}-%{release}

BuildRequires: /usr/bin/dbus-send
Requires: /usr/bin/dbus-send

# translations moved here
Conflicts: kde-l10n < 17.03

%description
%{summary}.


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
%{_kf5_datadir}/kservicetypes5/ktp-approver.desktop
%{_kf5_datadir}/kservices5/kded/ktp_approver.desktop
%{_kf5_qtplugindir}/kded_ktp_approver.so
%{_sysconfdir}/xdg/ktp_approverrc
%{_datadir}/dbus-1/services/org.freedesktop.Telepathy.Client.KTp.Approver.service


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

