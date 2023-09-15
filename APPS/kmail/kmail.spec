%global tests 1

Name:    kmail
Summary: Mail client
Version: 23.08.1
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
Source1: https://download.kde.org/%{stable}/release-service/%{version}/src/%{name}-%{version}.tar.xz.sig
Source2: gpgkey-D81C0CB38EB725EF6691C385BB463350D6EF31EF.gpg

# handled by qt5-srpm-macros, which defines %%qt5_qtwebengine_arches
%{?qt5_qtwebengine_arches:ExclusiveArch: %{qt5_qtwebengine_arches}}

BuildRequires: gnupg2
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
BuildRequires: cmake(Qt5Keychain)

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
BuildRequires: cmake(Grantlee5)

BuildRequires: cmake(KF5TextAutoCorrection)

# kde-apps
%global majmin_ver %(echo %{version} | cut -d. -f1,2)
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
BuildRequires: kf5-grantleetheme-devel >= %{majmin_ver}

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
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -n %{name}-%{version} -p1


%build
%cmake_kf5 \
  -DBUILD_TESTING:BOOL=%{?tests:ON}%{!?tests:OFF}
%cmake_build


%install
%cmake_install
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
make test ARGS="--output-on-failure --timeout 30" -C %{_vpath_builddir} ||:
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
%license LICENSES/*.txt
%{_kf5_bindir}/kmail
%{_kf5_metainfodir}/org.kde.kmail2.appdata.xml
%{_kf5_datadir}/applications/org.kde.kmail2.desktop
%{_kf5_datadir}/config.kcfg/kmail.kcfg
%{_kf5_datadir}/kmail2/
%{_datadir}/dbus-1/interfaces/org.kde.kmail.*.xml
%{_kf5_datadir}/icons/hicolor/*/*/*
%{_kf5_datadir}/icons/breeze-dark/*/*/*
%{_kf5_datadir}/applications/kmail_view.desktop
%{_kf5_datadir}/knotifications5/kmail2.notifyrc
# Kontact integration
%{_kf5_datadir}/kxmlgui5/kontactsummary/
# agents
%{_kf5_datadir}/akonadi/agents/*.desktop
%{_kf5_bindir}/akonadi_*_agent
%{_kf5_datadir}/config.kcfg/archivemailagentsettings.kcfg
%{_kf5_datadir}/knotifications5/akonadi_archivemail_agent.notifyrc
%{_kf5_datadir}/knotifications5/akonadi_followupreminder_agent.notifyrc
%{_kf5_datadir}/knotifications5/akonadi_mailfilter_agent.notifyrc
%{_kf5_datadir}/knotifications5/akonadi_sendlater_agent.notifyrc
%{_kf5_datadir}/knotifications5/akonadi_mailmerge_agent.notifyrc
# ktnef
%{_kf5_bindir}/ktnef
%{_kf5_datadir}/applications/org.kde.ktnef.desktop
#{_kf5_qtplugindir}/akonadi/config/*.so
# kmail-refresh-settings
%{_kf5_bindir}/kmail-refresh-settings
%{_kf5_datadir}/applications/org.kde.kmail-refresh-settings.desktop
# common
%{_kf5_datadir}/dbus-1/services/*.service
%{_kf5_datadir}/qlogging-categories5/*categories

%ldconfig_scriptlets libs

%files libs
%{_kf5_libdir}/libkmailprivate.so.*
%{_kf5_qtplugindir}/kmailpart.so
%{_kf5_qtplugindir}/pim5/kcms/kmail/
%{_kf5_qtplugindir}/pim5/kcms/summary/*.so
%{_kf5_qtplugindir}/pim5/kontact/*.so
%{_kf5_qtplugindir}/pim5/akonadi/config/archivemailagentconfig.so
%{_kf5_qtplugindir}/pim5/akonadi/config/followupreminderagentconfig.so


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

