%undefine __cmake_in_source_build
%global framework messagelib

Name:    kf5-%{framework}
Version: 21.04.1
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

# handled by qt5-srpm-macros, which defines %%qt5_qtwebengine_arches
%{?qt5_qtwebengine_arches:ExclusiveArch: %{qt5_qtwebengine_arches}}

## upstream patches

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

#global majmin_ver %(echo %{version} | cut -d. -f1,2)
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
Requires:       cmake(KF5Akonadi)
Requires:       cmake(KF5AkonadiMime)
Requires:       cmake(KF5Contacts)
Requires:       cmake(KF5IdentityManagement)
Requires:       cmake(KF5Libkleo)
Requires:       cmake(KF5MessageCore)
Requires:       cmake(KF5Mime)
Requires:       cmake(KF5PimCommon)
Requires:       cmake(Qt5WebEngine)
%description    devel
%{summary}.


%prep
%autosetup -n %{framework}-%{version} -p1


%build
sed -i 's|Qca-qt5 2.3.0|Qca-qt5 2.2.1|' messageviewer/src/CMakeLists.txt

%{cmake_kf5}

%cmake_build


%install
%cmake_install

%find_lang %{name} --all-name --with-html


%ldconfig_scriptlets

%files -f %{name}.lang
%license LICENSES/*.txt
%{_kf5_libdir}/libKF5MessageComposer.so.*
%{_kf5_libdir}/libKF5MessageCore.so.*
%{_kf5_libdir}/libKF5MessageList.so.*
%{_kf5_datadir}/messagelist/
%{_kf5_libdir}/libKF5MessageViewer.so.*
%dir %{_kf5_qtplugindir}/messageviewer/
%{_kf5_qtplugindir}/messageviewer/headerstyle/messageviewer_defaultgrantleeheaderstyleplugin.so
%{_kf5_qtplugindir}/messageviewer/grantlee/5.0/messageviewer_grantlee_extension.so
%{_kf5_datadir}/libmessageviewer/
%{_kf5_datadir}/messageviewer/
%{_kf5_datadir}/kconf_update/messageviewer.upd
%{_kf5_datadir}/knotifications5/messageviewer.notifyrc
%{_kf5_libdir}/libKF5MimeTreeParser.so.*
%{_kf5_libdir}/libKF5TemplateParser.so.*
%{_kf5_datadir}/config.kcfg/customtemplates_kfg.kcfg
%{_kf5_datadir}/config.kcfg/templatesconfiguration_kfg.kcfg
%{_kf5_libdir}/libKF5WebEngineViewer.so.*
## check this -- rex
%dir %{_kf5_datadir}/org.kde.syntax-highlighting/
%{_kf5_datadir}/org.kde.syntax-highlighting/syntax/kmail-template.xml
%{_kf5_datadir}/knsrcfiles/*.knsrc
%{_kf5_datadir}/qlogging-categories5/*categories

%files devel
%{_kf5_libdir}/libKF5MessageComposer.so
%{_kf5_libdir}/cmake/KF5MessageComposer/
%{_kf5_includedir}/MessageComposer/
%{_kf5_includedir}/messagecomposer/
%{_kf5_includedir}/messagecomposer_version.h
%{_kf5_archdatadir}/mkspecs/modules/qt_MessageComposer.pri

%{_kf5_libdir}/libKF5MessageCore.so
%{_kf5_libdir}/cmake/KF5MessageCore/
%{_kf5_includedir}/MessageCore/
%{_kf5_includedir}/messagecore/
%{_kf5_includedir}/messagecore_version.h
%{_kf5_archdatadir}/mkspecs/modules/qt_MessageCore.pri

%{_kf5_libdir}/libKF5MessageList.so
%{_kf5_libdir}/cmake/KF5MessageList/
%{_kf5_includedir}/MessageList/
%{_kf5_includedir}/messagelist/
%{_kf5_includedir}/messagelist_version.h
%{_kf5_archdatadir}/mkspecs/modules/qt_MessageList.pri

%{_kf5_libdir}/libKF5MessageViewer.so
%{_kf5_libdir}/cmake/KF5MessageViewer/
%{_kf5_includedir}/MessageViewer/
%{_kf5_includedir}/messageviewer/
%{_kf5_includedir}/messageviewer_version.h
%{_kf5_archdatadir}/mkspecs/modules/qt_MessageViewer.pri

%{_kf5_libdir}/libKF5MimeTreeParser.so
%{_kf5_libdir}/cmake/KF5MimeTreeParser/
%{_kf5_includedir}/MimeTreeParser/
%{_kf5_includedir}/mimetreeparser/
%{_kf5_includedir}/mimetreeparser_version.h

%{_kf5_libdir}/libKF5TemplateParser.so
%{_kf5_libdir}/cmake/KF5TemplateParser/
%{_kf5_includedir}/TemplateParser/
%{_kf5_includedir}/templateparser/
%{_kf5_includedir}/templateparser_version.h
%{_kf5_archdatadir}/mkspecs/modules/qt_TemplateParser.pri

%{_kf5_libdir}/libKF5WebEngineViewer.so
%{_kf5_libdir}/cmake/KF5WebEngineViewer/
%{_kf5_includedir}/WebEngineViewer/
%{_kf5_includedir}/webengineviewer/
%{_kf5_includedir}/webengineviewer_version.h
%{_kf5_archdatadir}/mkspecs/modules/qt_WebEngineViewer.pri


%changelog
* Thu May 13 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 21.04.1-1
- 21.04.1

* Thu Apr 22 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 21.04.0-1
- 21.04.0

* Sat Mar 06 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.12.3-1
- 20.12.3

* Thu Feb 04 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.12.2-1
- 20.12.2

* Thu Jan  7 22:09:31 MSK 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.12.1-1
- 20.12.1

* Thu Dec 10 21:56:27 MSK 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.12.0-1
- 20.12.0

* Fri Nov  6 13:25:58 MSK 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.08.3-1
- 20.08.3

* Fri Oct 09 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.08.2-1
- 20.08.2

* Thu Sep 03 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.08.1-1
- 20.08.1

* Fri Aug 14 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.08.0-1
- 20.08.0

* Thu Jul 09 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.04.3-1
- 20.04.3

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

* Wed Nov 28 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.08.3-2
- CVE-2018-19516: messagelib: HTML email can open browser window automatically

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

* Thu Aug 03 2017 Fedora Release Engineering <releng@fedoraproject.org> - 17.04.3-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Binutils_Mass_Rebuild

* Fri Jul 28 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.04.3-1
- 17.04.3

* Wed Jul 26 2017 Fedora Release Engineering <releng@fedoraproject.org> - 17.04.2-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Mass_Rebuild

* Thu Jun 15 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.04.2-1
- 17.04.2

* Mon May 15 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.04.1-1
- 17.04.1

* Thu Mar 09 2017 Rex Dieter <rdieter@fedoraproject.org> - 16.12.3-1
- 16.12.3

* Thu Feb 09 2017 Rex Dieter <rdieter@fedoraproject.org> - 16.12.2-1
- 16.12.2

* Mon Jan 16 2017 Rex Dieter <rdieter@fedoraproject.org> - 16.12.1-1
- 16.12.1

* Sat Dec 10 2016 Igor Gnatenko <i.gnatenko.brain@gmail.com> - 16.08.3-2
- Rebuild for gpgme 1.18

* Mon Dec 05 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.08.3-1
- 16.08.3

* Fri Oct 28 2016 Than Ngo <than@redhat.com> - 16.08.2-2
- don't build on ppc64/s390x as qtwebengine is not supported yet

* Thu Oct 13 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.08.2-1
- 16.08.2

* Mon Oct 10 2016 Than Ngo <than@redhat.com> - 16.08.1-3
- CVE-2016-7967, JavaScript access to local and remote URLs (bz#1383610, bz#1382288)
- CVE-2016-7968, JavaScript execution in HTML Mails (bz#1382293, bz#1383959)

* Thu Sep 15 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.08.1-2
- pull in upstream fixes, including crasher fix for kde#364994

* Thu Sep 08 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.08.1-1
- 16.08.1

* Sun Sep 04 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.08.0-1
- 16.08.0, Conflicts: kdepim-addons < 16.07

* Sun Jul 10 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.04.3-1
- 16.04.3

* Sun Jun 12 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.04.2-1
- 16.04.2

* Wed May 25 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.04.1-1
- First try
