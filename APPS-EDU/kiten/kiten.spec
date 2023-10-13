Name:    kiten
Summary: Japanese Reference/Study Tool
Version: 23.08.2
Release: 1%{?dist}

License: GPLv2+
URL:     https://www.kde.org/applications/education/kiten/
#URL:    https://edu.kde.org/kiten/

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
BuildRequires: gettext
# kf5 deps
BuildRequires: extra-cmake-modules
BuildRequires: kf5-rpm-macros
BuildRequires: kf5-karchive-devel
BuildRequires: kf5-kcompletion-devel
BuildRequires: kf5-kconfig-devel
BuildRequires: kf5-kconfigwidgets-devel
BuildRequires: kf5-kcoreaddons-devel
BuildRequires: kf5-kdoctools-devel
BuildRequires: kf5-ki18n-devel
BuildRequires: kf5-khtml-devel
BuildRequires: kf5-kcrash-devel
BuildRequires: kf5-kxmlgui-devel
BuildRequires: kf5-knotifications-devel
# qt deps
BuildRequires: pkgconfig(Qt5Widgets)

Requires: %{name}-libs%{?_isa} = %{version}-%{release}
Requires: kanjistrokeorders-fonts

%description
%{summary}.

%package  libs
Summary:  Runtime files for %{name}
Requires: %{name} = %{version}-%{release}
# when split occurred
Conflicts: kdeedu-libs < 4.7.0-10
License: LGPLv2+
%description libs
%{summary}.

%package devel
Summary:  Development files for %{name}
# when split occurred
Conflicts: kdeedu-devel < 4.7.0-10
License: LGPLv2+
Requires: %{name}-libs%{?_isa} = %{version}-%{release}
%description devel
%{summary}.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%setup -q


%build
# remove the skip code from kanjidic
sed -i 's/ P[0-9]*-[0-9]*-[0-9]*//g' data/kanjidic

%cmake_kf5
%cmake_build


%install
%cmake_install

%find_lang %{name} --all-name --with-html

## unpackaged files
rm -fv %{buildroot}%{_datadir}/fonts/kanjistrokeorders/KanjiStrokeOrders.ttf


%check
for f in %{buildroot}%{_kf5_datadir}/applications/org.kde.kiten*.desktop ; do
desktop-file-validate $f
done


%files -f %{name}.lang
%license LICENSES/*.txt
%doc AUTHORS README TODO
%{_kf5_bindir}/kiten
%{_kf5_bindir}/kitengen
%{_kf5_bindir}/kitenkanjibrowser
%{_kf5_bindir}/kitenradselect
%{_kf5_datadir}/kiten/
%{_kf5_metainfodir}/org.kde.kiten.appdata.xml
%{_kf5_datadir}/applications/org.kde.kiten*.desktop
%{_kf5_datadir}/config.kcfg/kiten.kcfg
%{_kf5_datadir}/kxmlgui5/kiten/
%{_kf5_datadir}/kxmlgui5/kitenkanjibrowser/
%{_kf5_datadir}/kxmlgui5/kitenradselect/
%{_kf5_datadir}/icons/hicolor/*/*/kiten.*

%ldconfig_scriptlets libs

%files libs
%{_kf5_libdir}/libkiten.so.5*

%files devel
%{_kf5_libdir}/libkiten.so
%{_includedir}/libkiten/


%changelog
* Fri Oct 13 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.08.2-1
- 23.08.2

* Thu Sep 14 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.08.1-1
- 23.08.1

* Mon Aug 28 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.08.0-1
- 23.08.0

* Thu Jul 06 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 23.04.3-1
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

