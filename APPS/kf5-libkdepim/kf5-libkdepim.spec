%global framework libkdepim

Name:    kf5-%{framework}
Version: 23.08.1
Release: 1%{?dist}
Summary: Library for common kdepim apps

License: GPLv2+
URL:     http://cgit.kde.org/%{framework}.git

%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0:        https://download.kde.org/%{stable}/release-service/%{version}/src/%{framework}-%{version}.tar.xz
Source1:        https://download.kde.org/%{stable}/release-service/%{version}/src/%{framework}-%{version}.tar.xz.sig
Source2:        gpgkey-D81C0CB38EB725EF6691C385BB463350D6EF31EF.gpg

BuildRequires:  gnupg2
BuildRequires:  boost-devel
%global kf5_ver 5.71
BuildRequires:  extra-cmake-modules >= %{kf5_ver}
BuildRequires:  kf5-rpm-macros >= %{kf5_ver} 
BuildRequires:  cmake(KF5I18n)
BuildRequires:  cmake(KF5Completion)
BuildRequires:  cmake(KF5KCMUtils)
BuildRequires:  cmake(KF5Codecs)
BuildRequires:  cmake(KF5Wallet)
BuildRequires:  cmake(KF5IconThemes)
BuildRequires:  cmake(KF5ItemViews)
BuildRequires:  cmake(Qt5Designer)
BuildRequires:  cmake(Qt5UiTools)
BuildRequires:  cmake(Qt5Widgets)
%global majmin_ver %{version}
# kf5-akonadi-contacts available only where qt5-qtwebengine is
%{?qt5_qtwebengine_arches:ExclusiveArch: %{qt5_qtwebengine_arches}}
BuildRequires:  kf5-kcontacts-devel >= %{majmin_ver}
BuildRequires:  kf5-kimap-devel >= %{majmin_ver}
BuildRequires:  kf5-kldap-devel >= %{majmin_ver}
BuildRequires:  kf5-kmime-devel >= %{majmin_ver}

Requires:       kf5-filesystem

Obsoletes:      kdepim-libs < 7:16.04.0
Conflicts:      kdepim-libs < 7:16.04.0
# kdepimwidgets designer plugin moved here
Conflicts:      kdepim-common < 16.04.0
# kcm_ldap moved here
Conflicts:      kaddressbook < 16.04.0

Provides:       %{name}-akonadi = %{?epoch:%{epoch}:}%{version}-%{release}
Obsoletes:      %{name}-akonadi < 20.08.0

%description
%{summary}.

%package        devel
Summary:        Development files for %{name}
Requires:       %{name}%{?_isa} = %{version}-%{release}
%description    devel
The %{name}-devel package contains libraries and header files for
developing applications that use %{name}.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -n %{framework}-%{version}


%build
%cmake_kf5
%cmake_build


%install
%cmake_install
%find_lang %{name} --all-name --with-html


%ldconfig_scriptlets

%files -f %{name}.lang
%license LICENSES/*.txt
%{_kf5_libdir}/libKPim5Libkdepim.so.5*
%{_qt5_plugindir}/designer/kdepim5widgets.so
%{_kf5_datadir}/qlogging-categories5/*categories

%files devel
%{_includedir}/KPim5/Libkdepim/
%{_kf5_libdir}/libKPim5Libkdepim.so
%{_kf5_libdir}/cmake/KF5Libkdepim/
%{_kf5_libdir}/cmake/KPim5Libkdepim/
%{_kf5_archdatadir}/mkspecs/modules/qt_Libkdepim.pri
%{_kf5_libdir}/cmake/MailTransportDBusService/
%{_kf5_libdir}/cmake/KPim5MailTransportDBusService/
%{_kf5_datadir}/dbus-1/interfaces/org.kde.addressbook.service.xml
%{_kf5_datadir}/dbus-1/interfaces/org.kde.mailtransport.service.xml


%changelog
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

