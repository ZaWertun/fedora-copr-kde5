Name:    ktp-text-ui
Summary: Telepathy text chat handler
Version: 23.04.3
Release: 1%{?dist}

# GPLv2+: most code
# (BSD or AFL): data/styles/renkoo.AdiumMessageStyle
# MIT:  data/styles/simkete/, fadomatic javascript code used in Renkoo
License: GPLv2+ and (BSD or AFL) and MIT
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

# handled by qt5-srpm-macros, which defines %%qt5_qtwebengine_arches
%{?qt5_qtwebengine_arches:ExclusiveArch: %{qt5_qtwebengine_arches}}

## upstream patches

## upstreamable patches
Patch51: ktp-text-ui-15.04-lib-soversion.patch

BuildRequires:  gnupg2
BuildRequires:  desktop-file-utils
BuildRequires:  dos2unix
BuildRequires:  extra-cmake-modules
BuildRequires:  kf5-rpm-macros
BuildRequires:  qt5-qtbase-devel
BuildRequires:  qt5-qtwebengine-devel

BuildRequires:  kf5-karchive-devel
BuildRequires:  kf5-sonnet-devel
BuildRequires:  kf5-kwidgetsaddons-devel
BuildRequires:  kf5-kservice-devel
BuildRequires:  kf5-kemoticons-devel
BuildRequires:  kf5-kio-devel
BuildRequires:  kf5-kcmutils-devel
BuildRequires:  kf5-knotifyconfig-devel
BuildRequires:  kf5-knotifications-devel
BuildRequires:  kf5-ki18n-devel
BuildRequires:  kf5-kdewebkit-devel
BuildRequires:  kf5-kwindowsystem-devel
BuildRequires:  kf5-kxmlgui-devel
BuildRequires:  kf5-kitemviews-devel
BuildRequires:  kf5-kdbusaddons-devel
BuildRequires:  kf5-kiconthemes-devel
BuildRequires:  kf5-ktextwidgets-devel
BuildRequires:  kf5-kpeople-devel

%global majmin_ver %(echo %{version} | cut -d. -f1,2)
BuildRequires:  ktp-common-internals-devel >= %{majmin_ver}
BuildRequires:  telepathy-logger-qt-devel

Requires:       ktp-accounts-kcm

Obsoletes:      telepathy-kde-text-ui < 0.3.0
Provides:       telepathy-kde-text-ui = %{version}-%{release}

Obsoletes:      ktp-text-ui-devel < 0.6.80

%description
%{summary}.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -p1

# looks like someone cat'd several files with different encoding (and line endings)
# into one.  we'll do our best to make it usable
mac2unix data/styles/renkoo.AdiumMessageStyle/Contents/Resources/Renkoo*LICENSE.txt


%build
%cmake_kf5
%cmake_build


%install
%cmake_install

%find_lang %{name} --all-name --with-html

# remove files we don't install
rm -v %{buildroot}/%{_libdir}/libktpimagesharer.so
rm -v %{buildroot}/%{_libdir}/libktpchat.so

%check
desktop-file-validate %{buildroot}%{_datadir}/applications/org.kde.ktplogviewer.desktop


%ldconfig_scriptlets

%files -f %{name}.lang
%license COPYING*
%doc data/styles/renkoo.AdiumMessageStyle/Contents/Resources/Renkoo*LICENSE.txt
%doc data/styles/SimKete.AdiumMessageStyle/Contents/README
%{_bindir}/ktp-log-viewer
%{_datadir}/applications/org.kde.ktplogviewer.desktop
%{_datadir}/ktp-log-viewer/
%{_libdir}/libktpimagesharer.so.*
%{_libdir}/libktpchat.so.*
%{_kf5_qtplugindir}/kcm_ktp_chat_appearance.so
%{_kf5_qtplugindir}/kcm_ktp_chat_behavior.so
%{_kf5_qtplugindir}/kcm_ktp_chat_messages.so
%{_kf5_qtplugindir}/kcm_ktp_chat_otr.so
%{_kf5_qtplugindir}/kcm_ktp_logviewer_behavior.so
%{_kf5_qtplugindir}/kcm_ktptextui_message_filter_emoticons.so
%{_kf5_qtplugindir}/kcm_ktptextui_message_filter_latex.so
%{_kf5_qtplugindir}/ktptextui_message_filter_*.so
%{_libexecdir}/ktp-adiumxtra-protocol-handler
%{_libexecdir}/ktp-text-ui
%{_kf5_datadir}/kservices5/kcm_ktp_chat_appearance.desktop
%{_kf5_datadir}/kservices5/kcm_ktp_chat_behavior.desktop
%{_kf5_datadir}/kservices5/kcm_ktp_chat_messages.desktop
%{_kf5_datadir}/kservices5/kcm_ktp_chat_otr.desktop
%{_kf5_datadir}/kservices5/kcm_ktp_logviewer_behavior.desktop
%{_kf5_datadir}/kservices5/kcm_ktptextui_message_filter_latex.desktop
%{_kf5_datadir}/kservices5/kcm_ktptextui_message_filter_emoticons.desktop
%{_kf5_datadir}/kservices5/ktptextui_message_filter_*.desktop
%{_kf5_datadir}/kservices5/adiumxtra.protocol
%{_kf5_datadir}/kservicetypes5/ktptxtui_message_filter.desktop
%{_datadir}/ktelepathy/
%{_datadir}/dbus-1/services/org.freedesktop.Telepathy.Client.KTp.TextUi.service
%{_datadir}/telepathy/clients/KTp.TextUi.client
%{_kf5_datadir}/kxmlgui5/ktp-text-ui/


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

