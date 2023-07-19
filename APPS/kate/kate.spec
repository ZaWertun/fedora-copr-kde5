# trim changelog included in binary rpms
%global _changelog_trimtime %(date +%s -d "1 year ago")

# uncomment to enable bootstrap mode
#global bootstrap 1

%if !0%{?bootstrap}
%global tests 1
%endif

Name:    kate
Summary: Advanced Text Editor
Version: 23.04.3
Release: 1%{?dist}

# kwrite LGPLv2+
# kate: app LGPLv2, plugins, LGPLv2 and LGPLv2+ and GPLv2+
# ktexteditor: LGPLv2
License: LGPLv2 and LGPLv2+ and GPLv2+ 
URL:     https://cgit.kde.org/%{name}.git

%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0: https://download.kde.org/%{stable}/release-service/%{version}/src/kate-%{version}.tar.xz
Source1: https://download.kde.org/%{stable}/release-service/%{version}/src/kate-%{version}.tar.xz.sig
Source2: gpgkey-D81C0CB38EB725EF6691C385BB463350D6EF31EF.gpg

## upstream patches

BuildRequires: gnupg2
BuildRequires: desktop-file-utils
BuildRequires: gettext
BuildRequires: pkgconfig(x11)

BuildRequires: extra-cmake-modules
BuildRequires: kf5-rpm-macros
BuildRequires: cmake(KF5Config)
BuildRequires: cmake(KF5Crash)
BuildRequires: cmake(KF5DBusAddons)
BuildRequires: cmake(KF5GuiAddons)
BuildRequires: cmake(KF5I18n)
BuildRequires: cmake(KF5JobWidgets)
BuildRequires: cmake(KF5KIO)
BuildRequires: cmake(KF5Parts)
BuildRequires: cmake(KF5SyntaxHighlighting)
BuildRequires: cmake(KF5TextEditor)
BuildRequires: cmake(KF5WindowSystem)
BuildRequires: cmake(KF5XmlGui)

BuildRequires: cmake(Qt5DBus)
BuildRequires: cmake(Qt5Widgets)
BuildRequires: cmake(Qt5Script)
BuildRequires: cmake(Qt5Sql)
BuildRequires: cmake(Qt5Test)
BuildRequires: cmake(Qt5X11Extras)

%if ! 0%{?bootstrap}
BuildRequires: cmake(KF5Activities)
BuildRequires: cmake(KF5DocTools)
BuildRequires: cmake(KF5Wallet)
BuildRequires: cmake(KF5Plasma)
BuildRequires: cmake(KF5Service)
BuildRequires: cmake(KF5ItemModels)
BuildRequires: cmake(KF5ThreadWeaver)
BuildRequires: cmake(KF5NewStuff)
BuildRequires: cmake(KF5IconThemes)
%endif

%if 0%{?tests}
BuildRequires: libappstream-glib
BuildRequires: xorg-x11-server-Xvfb
%endif

BuildRequires: cmake(KUserFeedback)

# not sure if we want -plugins by default, let's play it safe'ish
# and make it optional
Recommends: %{name}-plugins%{?_isa} = %{version}-%{release}

# translations moved here
Conflicts: kde-l10n < 17.03

%description
%{summary}.

%package libs
Summary:     Private runtime libraries for %{name}
Obsoletes:   libkateprivate < 22.12.2-2
Provides:    libkateprivate = %{version}-%{release}
%description libs
%{summary}.

%package plugins
Summary: Kate plugins
License: LGPLv2
# upgrade path, when -plugins were split
Obsoletes: kate < 14.12.1
Requires: %{name} = %{version}-%{release}
# Kate integrated terminal plugin doesnt work without Konsole
Recommends: konsole5
%description plugins
%{summary}.

%package -n kwrite
Summary: Text Editor
License: LGPLv2+
# translations moved here
Conflicts: kde-l10n < 17.03
%description -n kwrite
%{summary}.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -p1


%build
%cmake_kf5 \
  -Wno-dev \
  -DBUILD_TESTING:BOOL=%{?tests:ON}%{!?tests:OFF}
%cmake_build


%install
%cmake_install
%find_lang all --all-name --with-html --with-man

grep plugin all.lang > plugins.lang
grep kwrite all.lang > kwrite.lang
cat all.lang plugins.lang kwrite.lang | sort | uniq -u > kate.lang


%check
appstream-util validate-relax --nonet %{buildroot}%{_kf5_metainfodir}/org.kde.kate.appdata.xml ||:
desktop-file-validate %{buildroot}%{_kf5_datadir}/applications/org.kde.kate.desktop
desktop-file-validate %{buildroot}%{_kf5_datadir}/applications/org.kde.kwrite.desktop
%if 0%{?tests}
export CTEST_OUTPUT_ON_FAILURE=1
xvfb-run -a \
make test ARGS="--output-on-failure --timeout 20" -C %{__cmake_builddir} ||:
%endif


%files -f kate.lang
%license LICENSES/*.txt
%doc README.md
%{_kf5_bindir}/kate
%{_kf5_datadir}/applications/org.kde.kate.desktop
%{_kf5_datadir}/kconf_update/katesession_migration.upd
%{_kf5_datadir}/kconf_update/migrate_kate_sessions_applet_to_kdeplasma-addons.sh
%{_kf5_metainfodir}/org.kde.kate.appdata.xml
%{_kf5_datadir}/icons/hicolor/*/apps/kate.*
%{_mandir}/man1/kate.1*

%files plugins -f plugins.lang
%{_kf5_qtplugindir}/ktexteditor/cmaketoolsplugin.so
%{_kf5_qtplugindir}/ktexteditor/compilerexplorer.so
%{_kf5_qtplugindir}/ktexteditor/eslintplugin.so
%{_kf5_qtplugindir}/ktexteditor/externaltoolsplugin.so
%{_kf5_qtplugindir}/ktexteditor/formatplugin.so
%{_kf5_qtplugindir}/ktexteditor/katebacktracebrowserplugin.so
%{_kf5_qtplugindir}/ktexteditor/katebuildplugin.so
%{_kf5_qtplugindir}/ktexteditor/katecloseexceptplugin.so
%{_kf5_qtplugindir}/ktexteditor/katecolorpickerplugin.so
%{_kf5_qtplugindir}/ktexteditor/katectagsplugin.so
%{_kf5_qtplugindir}/ktexteditor/katefilebrowserplugin.so
%{_kf5_qtplugindir}/ktexteditor/katefiletreeplugin.so
%{_kf5_qtplugindir}/ktexteditor/kategdbplugin.so
%{_kf5_qtplugindir}/ktexteditor/kategitblameplugin.so
%{_kf5_qtplugindir}/ktexteditor/katekonsoleplugin.so
%{_kf5_qtplugindir}/ktexteditor/kateprojectplugin.so
%{_kf5_qtplugindir}/ktexteditor/katereplicodeplugin.so
%{_kf5_qtplugindir}/ktexteditor/katesearchplugin.so
%{_kf5_qtplugindir}/ktexteditor/katesnippetsplugin.so
%{_kf5_qtplugindir}/ktexteditor/katesqlplugin.so
%{_kf5_qtplugindir}/ktexteditor/katesymbolviewerplugin.so
%{_kf5_qtplugindir}/ktexteditor/katexmlcheckplugin.so
%{_kf5_qtplugindir}/ktexteditor/katexmltoolsplugin.so
%{_kf5_qtplugindir}/ktexteditor/keyboardmacrosplugin.so
%{_kf5_qtplugindir}/ktexteditor/ktexteditorpreviewplugin.so
%{_kf5_qtplugindir}/ktexteditor/latexcompletionplugin.so
%{_kf5_qtplugindir}/ktexteditor/lspclientplugin.so
%{_kf5_qtplugindir}/ktexteditor/rainbowparens.so
%{_kf5_qtplugindir}/ktexteditor/tabswitcherplugin.so
%{_kf5_qtplugindir}/ktexteditor/textfilterplugin.so
%{_kf5_datadir}/kateproject/
%{_kf5_datadir}/katexmltools/

%files libs
%{_kf5_libdir}/libkateprivate.so.%{version}

%files -n kwrite -f kwrite.lang
%{_kf5_bindir}/kwrite
%{_kf5_datadir}/applications/org.kde.kwrite.desktop
%{_kf5_metainfodir}/org.kde.kwrite.appdata.xml
%{_kf5_datadir}/icons/hicolor/*/apps/kwrite.*


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

* Sun Feb 05 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.12.2-2
- libkateprivate -> kate-libs

