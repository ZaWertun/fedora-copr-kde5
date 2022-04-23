Name:    kuserfeedback
Summary: Framework for collecting user feedback for applications via telemetry and surveys
Version: 1.0.0
Release: 2%{?dist}

License: MIT
URL:     https://cgit.kde.org/%{name}.git

Source0: https://download.kde.org/stable/%{name}/%{name}-%{version}.tar.xz

BuildRequires: extra-cmake-modules
BuildRequires: kf5-rpm-macros
BuildRequires: desktop-file-utils

BuildRequires: cmake(Qt5Core)
BuildRequires: cmake(Qt5Network)
BuildRequires: cmake(Qt5Widgets)
BuildRequires: cmake(Qt5Charts)
BuildRequires: cmake(Qt5Test)
BuildRequires: cmake(Qt5Qml)
BuildRequires: cmake(Qt5Svg)
BuildRequires: cmake(Qt5PrintSupport)

%description
%{summary}.

%package        devel
Summary:        Development files for %{name}
License:        MIT
Requires:       %{name}%{?_isa} = %{version}-%{release}

%description    devel
The %{name}-devel package contains libraries and header files for
developing applications that use %{name}.

%package        console
Summary:        Analytics and administration tool for UserFeedback servers
License:        MIT
Requires:       %{name}%{?_isa} = %{version}-%{release}

%description    console
Analytics and administration tool for UserFeedback servers.


%prep
%autosetup -p1


%build
%cmake_kf5 -DENABLE_DOCS:BOOL=OFF
%cmake_build


%install
%cmake_install
%{find_lang} userfeedbackconsole5 --with-qt
%{find_lang} userfeedbackprovider5 --with-qt


%check
desktop-file-validate %{buildroot}%{_datadir}/applications/UserFeedbackConsole.desktop


%ldconfig_scriptlets


%files -f userfeedbackprovider5.lang
%doc README.md
%license COPYING.LIB
%{_sysconfdir}/xdg/org_kde_UserFeedback.categories
%{_bindir}/userfeedbackctl
%{_libdir}/libKUserFeedbackCore.so.1*
%{_libdir}/libKUserFeedbackWidgets.so.1*
%{_kf5_qmldir}/org/kde/userfeedback/qmldir
%{_kf5_qmldir}/org/kde/userfeedback/libKUserFeedbackQml.so

%files devel
%{_includedir}/KUserFeedback/
%{_libdir}/libKUserFeedbackCore.so
%{_libdir}/libKUserFeedbackWidgets.so
%{_kf5_libdir}/cmake/KUserFeedback/KUserFeedback*.cmake
%{_kf5_archdatadir}/mkspecs/modules/qt_KUserFeedback*.pri

%files console -f userfeedbackconsole5.lang
%{_bindir}/UserFeedbackConsole
%{_datadir}/applications/UserFeedbackConsole.desktop


%changelog
* Wed Feb 12 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 1.0.0-2
- UserFeedbackConsole moved to separate package

* Wed Feb 12 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 1.0.0-1
- first spec version

