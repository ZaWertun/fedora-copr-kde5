Name:           kdevelop
Summary:        Integrated Development Environment for C++/C
Epoch:          9
Version:        23.08.2
Release:        1%{?dist}
License:        GPLv2
URL:            http://www.kdevelop.org/
Source0:        https://download.kde.org/stable/release-service/%{version}/src/%{name}-%{version}.tar.xz
Source1:        https://download.kde.org/stable/release-service/%{version}/src/%{name}-%{version}.tar.xz.sig
Source2:        gpgkey-D81C0CB38EB725EF6691C385BB463350D6EF31EF.gpg

## kdevelop-pg-qt FTBFS s390x
ExcludeArch: s390x

%global rpm_macros_dir %(d=%{_rpmconfigdir}/macros.d; [ -d $d ] || d=%{_sysconfdir}/rpm; echo $d)
Source10:       macros.kdevelop
Patch0:         kdevelop-5.2.3-qmake.patch

# upstreamable patches

# upstream patches

BuildRequires:  gnupg2
BuildRequires:  gcc-c++ gcc
BuildRequires:  boost-devel
BuildRequires:  desktop-file-utils
BuildRequires:  libappstream-glib
BuildRequires:  gettext
BuildRequires:  shared-mime-info
BuildRequires:  llvm-devel
BuildRequires:  clang-devel
BuildRequires:  okteta-devel
BuildRequires:  pcre-devel
BuildRequires:  subversion-devel
BuildRequires:  kdevelop-pg-qt-devel >= 2.0
BuildRequires:  libksysguard-devel
BuildRequires:  grantlee-qt5-devel
BuildRequires:  bash-completion

%{?grantlee5_requires}
BuildRequires:  libkomparediff2-devel
BuildRequires:  kf5-rpm-macros
BuildRequires:  extra-cmake-modules
BuildRequires:  kf5-karchive-devel
BuildRequires:  kf5-kconfig-devel
BuildRequires:  kf5-kguiaddons-devel
BuildRequires:  kf5-kiconthemes-devel
BuildRequires:  kf5-ki18n-devel
BuildRequires:  kf5-kitemmodels-devel
BuildRequires:  kf5-kitemviews-devel
BuildRequires:  kf5-kjobwidgets-devel
BuildRequires:  kf5-kcmutils-devel
BuildRequires:  kf5-kio-devel
BuildRequires:  kf5-kdoctools-devel
BuildRequires:  kf5-knewstuff-devel
BuildRequires:  kf5-knotifications-devel
BuildRequires:  kf5-knotifyconfig-devel
BuildRequires:  kf5-kparts-devel
BuildRequires:  kf5-kservice-devel
BuildRequires:  kf5-ktexteditor-devel
BuildRequires:  cmake(KF5SyntaxHighlighting)
BuildRequires:  kf5-threadweaver-devel
BuildRequires:  kf5-kwindowsystem-devel
BuildRequires:  kf5-kdeclarative-devel
BuildRequires:  kf5-kxmlgui-devel
BuildRequires:  kf5-plasma-devel
BuildRequires:  kf5-krunner-devel
BuildRequires:  kf5-kcrash-devel

BuildRequires:  qt5-qtbase-devel
BuildRequires:  qt5-qtdeclarative-devel
BuildRequires:  qt5-qtwebkit-devel
BuildRequires:  qt5-qtscript-devel
BuildRequires:  qt5-qttools-devel

# For AutoReq cmake-filesystem
BuildRequires:  cmake

# some arches don't have valgrind so we need to disable its support on them
%ifarch %{ix86} x86_64 ppc ppc64 s390x
BuildRequires: valgrind-devel
%endif

# kdevelop-custom-buildsystem has been merged to kdevelop 4.5.0
Provides:  kdevelop-custom-buildsystem = 9:%{version}-%{release}
Obsoletes: kdevelop-custom-buildsystem < 1.2.1-5

Provides:  kdevplatform = %{version}-%{release}
Obsoletes: kdevplatform < 5.1.80-1

# kdevappwizard/templates/qmake_qt4guiapp moved here
Conflicts: kapptemplate < 16.03.80

Requires: %{name}-libs%{?_isa} = %{epoch}:%{version}-%{release}
Requires: astyle
Requires: cmake
Requires: clang-tools-extra
Suggests: clazy
Requires: cppcheck
Requires: git
Requires: meson

%description
The KDevelop Integrated Development Environment provides many features
that developers need as well as providing a unified interface to programs
like gdb, the C/C++ compiler, and make. KDevelop manages or provides:

All development tools needed for C++ programming like Compiler,
Linker, automake and autoconf; KAppWizard, which generates complete,
ready-to-go sample applications; Classgenerator, for creating new
classes and integrating them into the current project; File management
for sources, headers, documentation etc. to be included in the
project; The creation of User-Handbooks written with SGML and the
automatic generation of HTML-output with the KDE look and feel;
Automatic HTML-based API-documentation for your project's classes with
cross-references to the used libraries; Internationalization support
for your application, allowing translators to easily add their target
language to a project;

KDevelop also includes WYSIWYG (What you see is what you get)-creation
of user interfaces with a built-in dialog editor; Debugging your
application by integrating KDbg; Editing of project-specific pixmaps
with KIconEdit; The inclusion of any other program you need for
development by adding it to the "Tools"-menu according to your
individual needs.


%package devel
Summary: Development files for %{name}
Requires: %{name}-libs%{?_isa} = %{epoch}:%{version}-%{release}
Provides: kdevplatform-devel = %{version}-%{release}
Obsoletes: kdevplatform-devel < 5.1.80-1
%description devel
%{summary}.

%package libs
Summary: %{name} runtime libraries
# helps multilib upgrades
Obsoletes: kdevelop < 9:3.9.95
Requires: %{name} = %{epoch}:%{version}-%{release}
Provides: kdevplatform-libs = %{version}-%{release}
Obsoletes: kdevplatform-libs < 5.1.80-1
%description libs
%{summary}.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%setup -q -n kdevelop-%{version}
%patch0 -p1 -b .qmake

%build
%cmake_kf5
%cmake_build


%install
%cmake_install
%find_lang %{name} --all-name

# rpm macros
install -p -m644 -D %{SOURCE10} \
  %{buildroot}%{rpm_macros_dir}/macros.kdevelop
sed -i \
  -e "s|@@NAME@@|%{name}|g" \
  -e "s|@@EPOCH@@|%{?epoch}%{!?epoch:0}|g" \
  -e "s|@@VERSION@@|%{version}|g" \
  -e "s|@@EVR@@|%{?epoch:%{epoch}:}%{version}-%{release}|g" \
  %{buildroot}%{rpm_macros_dir}/macros.kdevelop

# drop zsh, using bash as default
rm -f %{buildroot}%{_datadir}/kdevplatform/shellutils/.zshrc


%check
appstream-util validate-relax --nonet %{buildroot}%{_kf5_metainfodir}/org.kde.kdevelop.appdata.xml
desktop-file-validate %{buildroot}%{_datadir}/applications/org.kde.kdevelop.desktop


%files -f %{name}.lang
%doc AUTHORS README.md
%license COPYING.DOC LICENSES/*.txt
%{_bindir}/kdevelop
%{_bindir}/kdevelop!
%{_bindir}/kdev_includepathsconverter
%{_bindir}/kdev_dbus_socket_transformer
%{_bindir}/kdevplatform_shell_environment.sh
%{_bindir}/kdev_format_source
%{_datadir}/kdev*/
%{_datadir}/applications/org.kde.kdevelop.desktop
%{_datadir}/applications/org.kde.kdevelop_ps.desktop
%{_datadir}/applications/org.kde.kdevelop_bzr.desktop
%{_datadir}/applications/org.kde.kdevelop_git.desktop
%{_datadir}/applications/org.kde.kdevelop_kdev4.desktop
%{_datadir}/applications/org.kde.kdevelop_svn.desktop
%{_datadir}/kservices5/*.desktop
%{_datadir}/mime/packages/kdevelop.xml
%{_datadir}/mime/packages/kdevclang.xml
%{_datadir}/mime/packages/kdevgit.xml
%{_datadir}/plasma/plasmoids/kdevelopsessions/*
%{_datadir}/knotifications5/kdevelop.notifyrc
%{_datadir}/icons/hicolor/*/*/*
%{_kf5_metainfodir}/org.kde.kdevelop.appdata.xml
%{_datadir}/qlogging-categories5/kdevelop.categories
%{_datadir}/qlogging-categories5/kdevplatform.categories
%{_datadir}/bash-completion/completions/kdevelop
%{_datadir}/knsrcfiles/kdev*.knsrc
%{_docdir}/HTML/*/kdevelop/
%{_kf5_datadir}/kservicetypes5/kdevelopplugin.desktop
%{_qt5_qmldir}/org/kde/plasma/private/kdevelopsessions/libkdevelopsessionsplugin.so
%{_qt5_qmldir}/org/kde/plasma/private/kdevelopsessions/qmldir

%ldconfig_scriptlets libs

%files libs
%{_libdir}/lib*.so.*
%{_libdir}/*.so
%{_libdir}/libKDevClangPrivate.so*
%{_kf5_qtplugindir}/kf5/krunner/krunner_kdevelopsessions.so
%{_kf5_qtplugindir}/kdevplatform/
%{_kf5_qtplugindir}/grantlee/%{grantlee5_plugins}/kdev_filters.so
# FIXME/TODO: does not use standard %%{?grantlee5_plugindir}, is that a problem?  -- rex
%dir %{_kf5_qtplugindir}/grantlee/
%dir %{_kf5_qtplugindir}/grantlee/%{grantlee5_plugins}/
%{_qt5_qmldir}/org/kde/kdevplatform/

%files devel
%{_libdir}/cmake/KDevelop/
%{_libdir}/cmake/KDevPlatform
%{_includedir}/kdevelop/
%{_includedir}/kdevplatform/
%{_libdir}/lib*.so
%{rpm_macros_dir}/macros.kdevelop

%changelog
* Fri Oct 13 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 9:23.08.2-1
- 23.08.2

* Thu Sep 14 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 9:23.08.1-1
- 23.08.1

* Sun Aug 27 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 9:23.08.0-1
- 23.08.0

* Thu Jul 06 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 9:23.04.3-1
- 23.04.3

* Thu Jun 08 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 9:23.04.2-1
- 23.04.2

* Thu May 11 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 9:23.04.1-1
- 23.04.1

* Thu Apr 20 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 9:23.04.0-1
- 23.04.0

* Sat Mar 25 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 9:22.12.3-6
- rebuild

* Thu Mar 02 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 9:22.12.3-1
- 22.12.3

