%global framework messagelib

Name:    kf5-%{framework}
Version: 23.08.2
Release: 1%{?dist}
Summary: KDE Message libraries

License: GPLv2
URL:     https://cgit.kde.org/%{framework}.git/

%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0:        https://download.kde.org/%{stable}/release-service/%{version}/src/%{framework}-%{version}.tar.xz
Source1:        https://download.kde.org/%{stable}/release-service/%{version}/src/%{framework}-%{version}.tar.xz.sig
Source2:        gpgkey-D81C0CB38EB725EF6691C385BB463350D6EF31EF.gpg

# handled by qt5-srpm-macros, which defines %%qt5_qtwebengine_arches
%{?qt5_qtwebengine_arches:ExclusiveArch: %{qt5_qtwebengine_arches}}

## upstream patches

BuildRequires:  gnupg2
BuildRequires:  cmake(Grantlee5)

BuildRequires:  cmake(Qt5Network)
BuildRequires:  cmake(Qt5Test)
BuildRequires:  cmake(Qt5UiTools)
BuildRequires:  cmake(Qt5Positioning)
BuildRequires:  cmake(Qt5Qml) cmake(Qt5Quick)
BuildRequires:  cmake(Qt5WebChannel)
BuildRequires:  cmake(Qt5WebEngine)
BuildRequires:  cmake(Qt5Widgets)
BuildRequires:  cmake(QGpgme)
BuildRequires:  cmake(Qca-qt5)

%global kf5_ver 5.28
BuildRequires:  extra-cmake-modules >= %{kf5_ver}
BuildRequires:  kf5-rpm-macros >= %{kf5_ver}
BuildRequires:  kf5-karchive-devel >= %{kf5_ver}
BuildRequires:  kf5-kcodecs-devel >= %{kf5_ver}
BuildRequires:  kf5-kcompletion-devel >= %{kf5_ver}
BuildRequires:  kf5-kconfig-devel >= %{kf5_ver}
BuildRequires:  kf5-ki18n-devel >= %{kf5_ver}
BuildRequires:  kf5-kiconthemes-devel >= %{kf5_ver}
BuildRequires:  kf5-kitemviews-devel >= %{kf5_ver}
BuildRequires:  kf5-kwidgetsaddons-devel >= %{kf5_ver}
BuildRequires:  kf5-ktextwidgets-devel >= %{kf5_ver}
BuildRequires:  kf5-kxmlgui-devel >= %{kf5_ver}
BuildRequires:  cmake(KF5SyntaxHighlighting)
BuildRequires:  cmake(KF5NewStuff)

BuildRequires:  cmake(KF5TextAutoCorrection)

%global majmin_ver %{version}
BuildRequires:  kf5-akonadi-mime-devel >= %{majmin_ver}
BuildRequires:  kf5-akonadi-notes-devel >= %{majmin_ver}
BuildRequires:  kf5-akonadi-search-devel >= %{majmin_ver}
BuildRequires:  kf5-grantleetheme-devel >= %{majmin_ver}
BuildRequires:  kf5-kcontacts-devel >= %{majmin_ver}
BuildRequires:  kf5-kidentitymanagement-devel >= %{majmin_ver}
BuildRequires:  kf5-kmbox-devel >= %{majmin_ver}
BuildRequires:  kf5-kmime-devel >= %{majmin_ver}
BuildRequires:  kf5-kldap-devel >= %{majmin_ver}
BuildRequires:  kf5-kmailtransport-devel >= %{majmin_ver}
BuildRequires:  kf5-kpimtextedit-devel >= %{majmin_ver}
BuildRequires:  kf5-libgravatar-devel >= %{majmin_ver}
BuildRequires:  kf5-libkdepim-devel >= %{majmin_ver}
BuildRequires:  kf5-libkleo-devel >= %{majmin_ver}, cmake(QGpgme)
BuildRequires:  kf5-pimcommon-devel >= %{majmin_ver}

Obsoletes:      kdepim-libs < 7:16.04.0

# messageviewer_defaultgrantleeheaderstyleplugin.so moved here
Conflicts:      kdepim-addons < 16.12

Requires:       kf5-filesystem

%description
%{summary}.

%package        devel
Summary:        Development files for %{name}
Requires:       %{name}%{?_isa} = %{version}-%{release}
Requires:       cmake(KPim5Akonadi)
Requires:       cmake(KPim5AkonadiMime)
Requires:       cmake(KPim5AkonadiContact)
Requires:       cmake(KF5IdentityManagement)
Requires:       cmake(KPim5Libkleo)
Requires:       cmake(KPim5Mime)
Requires:       cmake(KPim5PimCommon)
Requires:       cmake(Qt5WebEngine)
%description    devel
%{summary}.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -n %{framework}-%{version} -p1


%build
%cmake_kf5
%cmake_build


%install
%cmake_install
%find_lang %{name} --all-name --with-html


%ldconfig_scriptlets

%files -f %{name}.lang
%license LICENSES/*.txt
%{_kf5_libdir}/libKPim5MessageComposer.so.*
%{_kf5_libdir}/libKPim5MessageCore.so.*
%{_kf5_libdir}/libKPim5MessageList.so.*
%{_kf5_datadir}/messagelist/
%{_kf5_libdir}/libKPim5MessageViewer.so.*
%{_kf5_qtplugindir}/pim5/messageviewer/headerstyle/messageviewer_defaultgrantleeheaderstyleplugin.so
%{_kf5_qtplugindir}/pim5/messageviewer/grantlee/5.0/messageviewer_grantlee_extension.so
%{_kf5_datadir}/libmessageviewer/
%{_kf5_datadir}/messageviewer/
%{_kf5_datadir}/knotifications5/messageviewer.notifyrc
%{_kf5_libdir}/libKPim5MimeTreeParser.so.*
%{_kf5_libdir}/libKPim5TemplateParser.so.*
%{_kf5_datadir}/config.kcfg/customtemplates_kfg.kcfg
%{_kf5_datadir}/config.kcfg/templatesconfiguration_kfg.kcfg
%{_kf5_libdir}/libKPim5WebEngineViewer.so.*
## check this -- rex
%dir %{_kf5_datadir}/org.kde.syntax-highlighting/
%{_kf5_datadir}/org.kde.syntax-highlighting/syntax/kmail-template.xml
%{_kf5_datadir}/knsrcfiles/*.knsrc
%{_kf5_datadir}/qlogging-categories5/*categories

%files devel
%{_kf5_libdir}/libKPim5MessageComposer.so
%{_kf5_libdir}/cmake/KPim5MessageComposer/
%{_includedir}/KPim5/MessageComposer/
%{_kf5_archdatadir}/mkspecs/modules/qt_MessageComposer.pri

%{_kf5_libdir}/libKPim5MessageCore.so
%{_kf5_libdir}/cmake/KPim5MessageCore/
%{_includedir}/KPim5/MessageCore/
%{_kf5_archdatadir}/mkspecs/modules/qt_MessageCore.pri

%{_kf5_libdir}/libKPim5MessageList.so
%{_kf5_libdir}/cmake/KPim5MessageList/
%{_includedir}/KPim5/MessageList/
%{_kf5_archdatadir}/mkspecs/modules/qt_MessageList.pri

%{_kf5_libdir}/libKPim5MessageViewer.so
%{_kf5_libdir}/cmake/KPim5MessageViewer/
%{_includedir}/KPim5/MessageViewer/
%{_kf5_archdatadir}/mkspecs/modules/qt_MessageViewer.pri

%{_kf5_libdir}/libKPim5MimeTreeParser.so
%{_kf5_libdir}/cmake/KPim5MimeTreeParser/
%{_includedir}/KPim5/MimeTreeParser/

%{_kf5_libdir}/libKPim5TemplateParser.so
%{_kf5_libdir}/cmake/KPim5TemplateParser/
%{_includedir}/KPim5/TemplateParser/
%{_kf5_archdatadir}/mkspecs/modules/qt_TemplateParser.pri

%{_kf5_libdir}/libKPim5WebEngineViewer.so
%{_kf5_libdir}/cmake/KPim5WebEngineViewer/
%{_includedir}/KPim5/WebEngineViewer/
%{_kf5_archdatadir}/mkspecs/modules/qt_WebEngineViewer.pri


%changelog
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

* Sat Mar 25 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.12.3-6
- rebuild

* Thu Mar 02 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.12.3-1
- 22.12.3

