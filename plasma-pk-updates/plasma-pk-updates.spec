%undefine __cmake_in_source_build

Name:           plasma-pk-updates
Version:        0.3.2
Release:        8%{?dist}
Summary:        Plasma applet for system updates using PackageKit

License:        GPLv2+
URL:            https://invent.kde.org/system/plasma-pk-updates
Source0:        https://download.kde.org/stable/plasma-pk-updates/%{version}/plasma-pk-updates-%{version}.tar.xz

# Upstream patches
Patch5: 0005-Several-fixes-related-to-the-network-state-and-apple.patch
Patch6: 0006-Don-t-force-a-check-for-updates-when-the-applet-runs.patch
Patch8: 0008-Replace-KIconLoader-pixmaps-with-standard-icon-names.patch
Patch9: 0009-Fix-usage-of-0-for-null-pointer-constants.patch
Patch10: 0010-Use-own-eventIds-and-ComponentName-instead-of-generi.patch
Patch11: 0011-Make-the-notifications-less-obtrusive.patch
Patch12: 0012-Fix-minor-typos.patch
Patch13: 0013-Fix-warning-remove-unsigned-int-0-check.patch
Patch14: 0014-Remove-explicit-initialization-of-default-constructe.patch
## Requires new SIP Power API from solid, not enabled by default
Patch15: 0015-Port-away-from-KDELibs4Support-use-Solid-Power-inter.patch
Patch30: 0030-Add-support-for-license-prompts.patch
Patch35: 0035-Make-action-buttons-translatable.patch
Patch42: 0042-Don-t-show-an-error-for-a-failed-automatic-refresh.patch

# Downstream patches
Patch100: plasma-pk-updates-0.3.2-notif.patch

BuildRequires:  extra-cmake-modules
BuildRequires:  kf5-kcoreaddons-devel
BuildRequires:  kf5-ki18n-devel
BuildRequires:  kf5-kiconthemes-devel
BuildRequires:  kf5-knotifications-devel
BuildRequires:  kf5-plasma-devel
# 5.75.0-2 when WIP api's used here were enabled -- rdieter
BuildRequires:  kf5-solid-devel >= 5.75.0-2
BuildRequires:  kf5-rpm-macros
BuildRequires:  PackageKit-Qt5-devel
BuildRequires:  qt5-qtbase-devel
BuildRequires:  qt5-qtdeclarative-devel

BuildRequires:  libappstream-glib

Requires:       PackageKit
Requires:       kf5-solid%{?_isa} >= 5.75.0-2

%description
%{summary}.


%prep
%autosetup -p1


%build
%cmake_kf5

%cmake_build


%install
%cmake_install

%find_lang %{name} --all-name


%check
appstream-util validate-relax --nonet %{buildroot}%{_kf5_metainfodir}/org.kde.plasma.pkupdates.appdata.xml ||:


%files -f %{name}.lang
%{_kf5_datadir}/kservices5/plasma-applet-org.kde.plasma.pkupdates.desktop
%{_kf5_qmldir}/org/kde/plasma/PackageKit/
%{_kf5_datadir}/plasma/plasmoids/org.kde.plasma.pkupdates/
%{_kf5_metainfodir}/org.kde.plasma.pkupdates.appdata.xml
%{_kf5_datadir}/knotifications5/plasma_pk_updates.notifyrc


%changelog
* Thu Nov 05 2020 Rex Dieter <rdieter@fedoraproject.org> - 0.3.2-8
- pull in upstream fixes
- update URL
- .spec cleanup

* Tue Aug 18 2020 Rex Dieter <rdieter@fedoraproject.org> - 0.3.2-7
- drop persistent notifications (#1316705,#1358146)

* Tue Jul 28 2020 Fedora Release Engineering <releng@fedoraproject.org> - 0.3.2-6
- Rebuilt for https://fedoraproject.org/wiki/Fedora_33_Mass_Rebuild

* Thu Jan 30 2020 Fedora Release Engineering <releng@fedoraproject.org> - 0.3.2-5
- Rebuilt for https://fedoraproject.org/wiki/Fedora_32_Mass_Rebuild

* Fri Jul 26 2019 Fedora Release Engineering <releng@fedoraproject.org> - 0.3.2-4
- Rebuilt for https://fedoraproject.org/wiki/Fedora_31_Mass_Rebuild

* Sat Feb 02 2019 Fedora Release Engineering <releng@fedoraproject.org> - 0.3.2-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_30_Mass_Rebuild

* Fri Jul 13 2018 Fedora Release Engineering <releng@fedoraproject.org> - 0.3.2-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_29_Mass_Rebuild

* Tue May 22 2018 Jan Grulich <jgrulich@redhat.com> - 0.3.2-1
- 0.3.2

* Fri Feb 09 2018 Fedora Release Engineering <releng@fedoraproject.org> - 0.3.1-9
- Rebuilt for https://fedoraproject.org/wiki/Fedora_28_Mass_Rebuild

* Sun Jan 28 2018 Rex Dieter <rdieter@fedoraproject.org> - 0.3.1-8
- rebuild

* Sun Jan 21 2018 Kevin Kofler <Kevin@tigcc.ticalc.org> - 0.3.1-7
- PackageKit-Qt 1.0.x build fix upstreamed, use patch from upstream git

* Sun Jan 21 2018 Kevin Kofler <Kevin@tigcc.ticalc.org> - 0.3.1-6
- fix FTBFS with PackageKit-Qt 1.0.x: remove unused obsolete PkStrings::message

* Tue Jan 16 2018 Rex Dieter <rdieter@fedoraproject.org> - 0.3.1-5
- pull in upstream fixes

* Thu Aug 03 2017 Fedora Release Engineering <releng@fedoraproject.org> - 0.3.1-4
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Binutils_Mass_Rebuild

* Thu Jul 27 2017 Fedora Release Engineering <releng@fedoraproject.org> - 0.3.1-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Mass_Rebuild

* Thu Jun 29 2017 Rex Dieter <rdieter@fedoraproject.org> - 0.3.1-2
- .spec cosmetics: fix URL for real, use %%autosetup

* Wed May 31 2017 Jan Grulich <jgrulich@redhat.com> - 0.3.1-1
- Update to 0.3.1

* Thu Apr 13 2017 Rex Dieter <rdieter@fedoraproject.org> - 0.2-12.20170102git73b70b3
- update URL, fix %%snap

* Sat Feb 11 2017 Fedora Release Engineering <releng@fedoraproject.org> - 0.2-11.20160307git73b70b3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_26_Mass_Rebuild

* Mon Jan 02 2017 Jan Grulich <jgrulich@redhat.com> - 0.2-10-20170102git73b70b3
- Fresh snapshot
  Resolves: kdebz#374429

* Mon Mar 21 2016 Rex Dieter <rdieter@fedoraproject.org> - 0.2-9.20160307git7b484b0
- update URL, fresh snapshot

* Mon Mar 21 2016 Rex Dieter <rdieter@fedoraproject.org> - 0.2-8.20160216git
- omit plasma update script (no longer needed)

* Tue Feb 16 2016 Jan Grulich <jgrulich@redhat.com> - 0.2-7.20160216git
- Update to latest git snapshot

* Thu Feb 04 2016 Fedora Release Engineering <releng@fedoraproject.org> - 0.2-6
- Rebuilt for https://fedoraproject.org/wiki/Fedora_24_Mass_Rebuild

* Thu Oct 29 2015 Rex Dieter <rdieter@fedoraproject.org> 0.2-5
- rebuild (PackageKit-Qt)

* Thu Oct 29 2015 Rex Dieter <rdieter@fedoraproject.org> 0.2-4
- .spec cosmetics, (explicit) Requires: PackageKit

* Thu Jun 18 2015 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 0.2-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_23_Mass_Rebuild

* Sat May 02 2015 Kalev Lember <kalevlember@gmail.com> - 0.2-2
- Rebuilt for GCC 5 C++11 ABI change

* Tue Apr 07 2015 Jan Grulich <jgrulich@redhat.com> 0.2-1
- update to 0.2

* Mon Mar 30 2015 Rex Dieter <rdieter@fedoraproject.org> 0.1-4
- enable org.kde.plasma.pkupdates by default, except not liveimage (#1206760)

* Mon Mar 23 2015 Jan Grulich <jgrulich@redhat.com> - 0.1-3
- backport minor fixes from upstream

* Mon Mar 23 2015 Jan Grulich <jgrulich@redhat.com> - 0.1-2
- fix URL

* Wed Mar 18 2015 Jan Grulich <jgrulich@redhat.com> - 0.1-1
- Initial relase
