
# uncomment to enable bootstrap mode
%global bootstrap 1

%if !0%{?bootstrap}
%global tests 1
%endif

Name:    kmail
Summary: Mail client
Version: 20.08.0
Release: 1%{?dist}

# code (generally) GPLv2, docs GFDL
License: GPLv2 and GFDL
URL:     https://www.kde.org/applications/internet/kmail

%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0: https://download.kde.org/%{stable}/release-service/%{version}/src/%{name}-%{version}.tar.xz

# handled by qt5-srpm-macros, which defines %%qt5_qtwebengine_arches
%{?qt5_qtwebengine_arches:ExclusiveArch: %{qt5_qtwebengine_arches}}

BuildRequires: boost-devel
BuildRequires: cmake(Gpgmepp) cmake(QGpgme)
BuildRequires: desktop-file-utils
BuildRequires: gettext
BuildRequires: libappstream-glib
BuildRequires: perl-generators
BuildRequires: pkgconfig(phonon4qt5)

BuildRequires: cmake(Qt5DBus)
BuildRequires: cmake(Qt5Network)
BuildRequires: cmake(Qt5Test)
BuildRequires: cmake(Qt5Widgets)
BuildRequires: cmake(Qt5WebEngine)
BuildRequires: cmake(Qt5Xml)

# kf5
BuildRequires: extra-cmake-modules
BuildRequires: kf5-rpm-macros
BuildRequires: cmake(KF5Bookmarks)
BuildRequires: cmake(KF5Codecs)
BuildRequires: cmake(KF5Config)
BuildRequires: cmake(KF5ConfigWidgets)
BuildRequires: cmake(KF5Crash)
BuildRequires: cmake(KF5DBusAddons)
BuildRequires: cmake(KF5DocTools)
BuildRequires: cmake(KF5GuiAddons)
BuildRequires: cmake(KF5I18n)
BuildRequires: cmake(KF5ItemViews)
BuildRequires: cmake(KF5JobWidgets)
BuildRequires: cmake(KF5KCMUtils)
BuildRequires: cmake(KF5KIO)
BuildRequires: cmake(KF5NewStuff)
BuildRequires: cmake(KF5Notifications)
BuildRequires: cmake(KF5NotifyConfig)
BuildRequires: cmake(KF5Parts)
BuildRequires: cmake(KF5Service)
BuildRequires: cmake(KF5Sonnet)
BuildRequires: cmake(KF5TextWidgets)
BuildRequires: cmake(KF5WindowSystem)
BuildRequires: cmake(KF5XmlGui)

# kde-apps
%global majmin_ver %(echo %{version} | cut -d. -f1,2)
BuildRequires: kdepim-apps-libs-devel >= %{majmin_ver}
BuildRequires: kf5-akonadi-contacts-devel >= %{majmin_ver}
BuildRequires: kf5-akonadi-mime-devel >= %{majmin_ver}
BuildRequires: kf5-akonadi-search-devel >= %{majmin_ver}
BuildRequires: kf5-akonadi-server-devel >= %{majmin_ver}
BuildRequires: kf5-kcalendarcore-devel >= %{majmin_ver}
BuildRequires: kf5-kcalendarutils-devel >= %{majmin_ver}
BuildRequires: kf5-kcontacts-devel >= %{majmin_ver}
BuildRequires: kf5-kidentitymanagement-devel >= %{majmin_ver}
BuildRequires: kf5-kldap-devel >= %{majmin_ver}
BuildRequires: kf5-kmailtransport-devel >= %{majmin_ver}
BuildRequires: kf5-kmime-devel >= %{majmin_ver}
BuildRequires: kf5-kontactinterface-devel >= %{majmin_ver}
BuildRequires: kf5-kpimtextedit-devel >= %{majmin_ver}
BuildRequires: kf5-ktnef-devel >= %{majmin_ver}
BuildRequires: kf5-libgravatar-devel >= %{majmin_ver}
BuildRequires: kf5-libkdepim-devel >= %{majmin_ver}
BuildRequires: kf5-libkleo-devel >= %{majmin_ver}
BuildRequires: kf5-libksieve-devel >= %{majmin_ver}, cmake(KF5SyntaxHighlighting)
BuildRequires: kf5-mailcommon-devel >= %{majmin_ver}
BuildRequires: kf5-pimcommon-devel >= %{majmin_ver}
BuildRequires: kf5-messagelib-devel >= %{majmin_ver}

%if 0%{?tests}
BuildRequires: dbus-x11
BuildRequires: xorg-x11-server-Xvfb
%endif

BuildRequires: cmake(KUserFeedback)

Obsoletes: pim-storage-service-manager < 17.03

Requires: %{name}-libs%{?_isa} = %{version}-%{release}

## runtime deps
Requires: akonadi-import-wizard >= %{majmin_ver}
Requires: grantlee-editor >= %{majmin_ver}
Requires: kdepim-runtime >= %{majmin_ver}
Requires: kmail-account-wizard >= %{majmin_ver}
Requires: pim-data-exporter >= %{majmin_ver}
Requires: pim-sieve-editor >= %{majmin_ver}

%description
%{summary}.

%package libs
Summary: Runtime libraries for %{name}
Requires: %{name} = %{version}-%{release}
%description libs
%{summary}.


%prep
%autosetup -n %{name}-%{version} -p1


%build
mkdir %{_target_platform}
pushd %{_target_platform}
%{cmake_kf5} .. \
  -DBUILD_TESTING:BOOL=%{?tests:ON}%{!?tests:OFF}
popd

%make_build -C %{_target_platform}


%install
make install/fast DESTDIR=%{buildroot} -C %{_target_platform}

%find_lang %{name} --all-name --with-html

## unpackaged files
rm -rfv %{buildroot}%{_kf5_datadir}/icons/locolor


%check
for f in %{buildroot}%{_kf5_datadir}/applications/*.desktop ; do
  desktop-file-validate $f
done
for f in %{buildroot}%{_kf5_metainfodir}/*.appdata.xml ; do
appstream-util validate-relax --nonet $f
done
%if 0%{?tests}
export CTEST_OUTPUT_ON_FAILURE=1
xvfb-run -a \
dbus-launch --exit-with-session \
make test ARGS="--output-on-failure --timeout 20" -C %{_target_platform} ||:
%endif


# symlink replaced by dir in 16 -> 17 upgrade
# https://fedoraproject.org/wiki/Packaging:Directory_Replacement
%pretrans -p <lua>
path = "%{_kf5_docdir}/HTML/en/kmail2"
st = posix.stat(path)
if st and st.type == "link" then
  os.remove(path)
end

%files -f %{name}.lang
%license COPYING*
%{_kf5_bindir}/kmail
%{_kf5_metainfodir}/org.kde.kmail2.appdata.xml
%{_kf5_datadir}/applications/org.kde.kmail2.desktop
%{_kf5_datadir}/config.kcfg/kmail.kcfg
%{_kf5_datadir}/kmail2/
%{_kf5_datadir}/kservices5/kmail_*.desktop
%{_datadir}/dbus-1/interfaces/org.kde.kmail.*.xml
%{_kf5_datadir}/icons/hicolor/*/*/*
%{_kf5_datadir}/icons/breeze-dark/*/*/*
%{_kf5_datadir}/kconf_update/kmail*
%{_kf5_datadir}/applications/kmail_view.desktop
%{_kf5_datadir}/knotifications5/kmail2.notifyrc
# Kontact integration
%{_kf5_datadir}/kontact/ksettingsdialog/*.setdlg
%{_kf5_datadir}/kservices5/kcmkontactsummary.desktop
%{_kf5_datadir}/kservices5/kontact/summaryplugin.desktop
%{_kf5_datadir}/kxmlgui5/kontactsummary/
%{_kf5_datadir}/kservices5/kontact/kmailplugin.desktop
%{_kf5_datadir}/kservices5/kcmkmailsummary.desktop
# agents
%{_kf5_datadir}/akonadi/agents/*.desktop
%{_kf5_bindir}/akonadi_*_agent
%{_kf5_datadir}/config.kcfg/archivemailagentsettings.kcfg
%{_kf5_datadir}/knotifications5/akonadi_archivemail_agent.notifyrc
%{_kf5_datadir}/knotifications5/akonadi_followupreminder_agent.notifyrc
%{_kf5_datadir}/knotifications5/akonadi_mailfilter_agent.notifyrc
%{_kf5_datadir}/knotifications5/akonadi_sendlater_agent.notifyrc
# ktnef
%{_kf5_bindir}/ktnef
%{_kf5_datadir}/applications/org.kde.ktnef.desktop
%{_kf5_qtplugindir}/akonadi/config/*.so
# kmail-refresh-settings
%{_kf5_bindir}/kmail-refresh-settings
%{_kf5_datadir}/applications/org.kde.kmail-refresh-settings.desktop
# common
%{_kf5_datadir}/dbus-1/services/*.service
%{_kf5_datadir}/qlogging-categories5/*categories

%ldconfig_scriptlets libs

%files libs
%{_kf5_libdir}/libkmailprivate.so.*
%{_kf5_qtplugindir}/kcm_kmailsummary.so
%{_kf5_qtplugindir}/kcm_kmail.so
%{_kf5_qtplugindir}/kcm_kontactsummary.so
%{_kf5_qtplugindir}/kmailpart.so
%{_kf5_qtplugindir}/kontact5/kontact_kmailplugin.so
%{_kf5_qtplugindir}/kontact5/kontact_summaryplugin.so


%changelog
* Fri Aug 14 2020 Yaroslav Sidlovsky <zawertun@otl.ru> - 20.08.0-1
- 20.08.0

* Thu Jul 09 2020 Yaroslav Sidlovsky <zawertun@otl.ru> - 20.04.3-1
- 20.04.3

* Sun Jul 05 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.04.2-2
- added cmake(KUserFeedback) to BuildRequires

* Fri Jun 12 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.04.2-1
- 20.04.2

* Tue May 19 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.04.1-1
- 20.04.1

* Fri Apr 24 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.04.0-1
- 20.04.0

* Fri Mar 06 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 19.12.3-1
- 19.12.3

* Fri Feb 07 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 19.12.2-1
- 19.12.2

* Fri Jan 10 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 19.12.1-1
- 19.12.1

* Thu Dec 12 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 19.12.0-1
- 19.12.0

* Fri Nov 08 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 19.08.3-1
- 19.08.3

* Thu Oct 10 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 19.08.2-1
- 19.08.2

* Thu Sep 05 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 19.08.1-1
- 19.08.1

* Thu Aug 15 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 19.08.0-1
- 19.08.0

* Thu Jul 11 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 19.04.3-1
- 19.04.3

* Thu Jun 06 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 19.04.2-1
- 19.04.2

* Thu May 09 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 19.04.1-1
- 19.04.1

* Sun Apr 28 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 19.04.0-1
- 19.04.0

* Tue Feb 05 2019 Rex Dieter <rdieter@fedoraproject.org> - 18.12.2-1
- 18.12.2

* Fri Feb 01 2019 Fedora Release Engineering <releng@fedoraproject.org> - 18.12.1-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_30_Mass_Rebuild

* Tue Jan 08 2019 Rex Dieter <rdieter@fedoraproject.org> - 18.12.1-1
- 18.12.1

* Fri Dec 14 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.12.0-1
- 18.12.0

* Tue Nov 06 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.08.3-1
- 18.08.3

* Wed Oct 10 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.08.2-1
- 18.08.2

* Mon Oct 01 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.08.1-1
- 18.08.1

* Fri Jul 13 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.04.3-1
- 18.04.3

* Fri Jul 13 2018 Fedora Release Engineering <releng@fedoraproject.org> - 18.04.2-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_29_Mass_Rebuild

* Wed Jun 06 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.04.2-1
- 18.04.2

* Wed May 09 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.04.1-1
- 18.04.1

* Fri Apr 20 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.04.0-1
- 18.04.0

* Tue Mar 06 2018 Rex Dieter <rdieter@fedoraproject.org> - 17.12.3-1
- 17.12.3

* Wed Feb 07 2018 Fedora Release Engineering <releng@fedoraproject.org> - 17.12.2-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_28_Mass_Rebuild

* Tue Feb 06 2018 Rex Dieter <rdieter@fedoraproject.org> - 17.12.2-1
- 17.12.2

* Thu Jan 11 2018 Rex Dieter <rdieter@fedoraproject.org> - 17.12.1-1
- 17.12.1

* Thu Jan 11 2018 Igor Gnatenko <ignatenkobrain@fedoraproject.org> - 17.12.0-2
- Remove obsolete scriptlets

* Tue Dec 12 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.12.0-1
- 17.12.0

* Wed Dec 06 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.11.90-1
- 17.11.90

* Wed Nov 22 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.11.80-1
- 17.11.80

* Wed Nov 08 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.08.3-1
- 17.08.3

* Mon Sep 25 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.08.1-1
- 17.08.1

* Thu Aug 03 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.04.3-3
- rebuild

* Thu Aug 03 2017 Fedora Release Engineering <releng@fedoraproject.org> - 17.04.3-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Binutils_Mass_Rebuild

* Fri Jul 28 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.04.3-1
- 17.04.3

* Wed Jul 26 2017 Fedora Release Engineering <releng@fedoraproject.org> - 17.04.2-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Mass_Rebuild

* Thu Jun 15 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.04.2-1
- 17.04.2

* Sun May 28 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.04.1-2
- scriptlet to aid replacing symlink with dir: %%{_kf5_docdir}/HTML/en/kmail2

* Mon May 15 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.04.1-1
- 17.04.1

* Thu Mar 09 2017 Rex Dieter <rdieter@fedoraproject.org> - 16.12.3-1
- 16.12.3

* Thu Feb 09 2017 Rex Dieter <rdieter@fedoraproject.org> - 16.12.2-1
- 16.12.2

* Thu Feb 02 2017 Rex Dieter <rdieter@fedoraproject.org> - 16.12.1-5
- add bunch of pim runtime deps

* Mon Jan 23 2017 Rex Dieter <rdieter@fedoraproject.org> - 16.12.1-4
- fix postun scriptlet

* Sat Jan 21 2017 Rex Dieter <rdieter@fedoraproject.org> - 16.12.1-3
- fix -libs dep for real

* Fri Jan 20 2017 Rex Dieter <rdieter@fedoraproject.org> - 16.12.1-2
- update URL, build deps, fix -libs dep

* Mon Jan 16 2017 Rex Dieter <rdieter@fedoraproject.org> - 16.12.1-1
- kmail-16.12.1

