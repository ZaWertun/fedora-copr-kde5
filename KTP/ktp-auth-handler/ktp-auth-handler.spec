Name:    ktp-auth-handler
Summary: Provide UI/KWallet Integration
Version: 22.12.1
Release: 1%{?dist}

License: LGPLv2+
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

# due to kaccounts-providers
# handled by qt5-srpm-macros, which defines %%qt5_qtwebengine_arches
%{?qt5_qtwebengine_arches:ExclusiveArch: %{qt5_qtwebengine_arches}}

BuildRequires:  gnupg2
BuildRequires:  extra-cmake-modules
BuildRequires:  gettext
BuildRequires:  kf5-ki18n-devel
BuildRequires:  kf5-kio-devel
BuildRequires:  kf5-kwallet-devel
BuildRequires:  kf5-kwidgetsaddons-devel
BuildRequires:  kf5-rpm-macros
BuildRequires:  libaccounts-qt5-devel
BuildRequires:  qca-qt5-devel
BuildRequires:  qt5-qtbase-devel
BuildRequires:  signon-devel
BuildRequires:  telepathy-qt5-devel

%global majmin_ver %(echo %{version} | cut -d. -f1,2)
BuildRequires:  kaccounts-integration-devel
BuildRequires:  ktp-common-internals-devel >= %{majmin_ver}

# runtime dep checked during build, mostly harmless
BuildRequires:  qca-qt5-ossl
Requires:       qca-qt5-ossl

%description
Provide UI/KWallet Integration For Passwords and SSL Errors on Account Connect.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -p1


%build
%cmake_kf5
%cmake_build


%install
%cmake_install

%find_lang %{name} --all-name --with-html


%files -f %{name}.lang
%license COPYING*
%{_libexecdir}/ktp-auth-handler
%{_datadir}/dbus-1/services/org.freedesktop.Telepathy.Client.KTp.ConfAuthObserver.service
%{_datadir}/dbus-1/services/org.freedesktop.Telepathy.Client.KTp.SASLHandler.service
%{_datadir}/dbus-1/services/org.freedesktop.Telepathy.Client.KTp.TLSHandler.service
%{_datadir}/telepathy/clients/KTp.ConfAuthObserver.client
%{_datadir}/telepathy/clients/KTp.SASLHandler.client
%{_datadir}/telepathy/clients/KTp.TLSHandler.client


%changelog
* Thu Jan 05 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.12.1-1
- 22.12.1

* Fri Dec 09 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.12.0-1
- 22.12.0

* Thu Nov 03 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.08.3-1
- 22.08.3

* Thu Oct 13 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.08.2-1
- 22.08.2

* Thu Sep 08 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.08.1-1
- 22.08.1

* Sat Aug 20 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.08.0-1
- 22.08.0

* Thu Jul 07 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.04.3-1
- 22.04.3

* Thu Jun 09 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.04.2-1
- 22.04.2

* Thu May 12 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.04.1-1
- 22.04.1

* Fri Apr 22 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 22.04.0-1
- 22.04.0

* Thu Mar 03 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 21.12.3-1
- 21.12.3

* Thu Feb 03 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 21.12.2-1
- 21.12.2

* Thu Jan 06 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 21.12.1-1
- 21.12.1

* Thu Dec 09 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 21.12.0-1
- 21.12.0

* Thu Nov 04 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 21.08.3-1
- 21.08.3

* Thu Oct 07 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 21.08.2-1
- 21.08.2

* Thu Sep 02 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 21.08.1-1
- 21.08.1

* Thu Aug 12 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 21.08.0-1
- 21.08.0

* Thu Jul 08 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 21.04.3-1
- 21.04.3

* Fri Jun 11 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 21.04.2-1
- 21.04.2

* Fri May 14 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 21.04.1-1
- 21.04.1

* Fri Apr 23 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 21.04.0-1
- 21.04.0

* Tue Mar 09 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.12.3-1
- 20.12.3

* Fri Feb  5 00:48:48 MSK 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.12.2-1
- 20.12.2

* Fri Jan  8 18:36:46 MSK 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.12.1-1
- 20.12.1

* Mon Dec 14 16:03:00 MSK 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.12.0-1
- 20.12.0

* Sat Nov  7 10:44:58 MSK 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.08.3-1
- 20.08.3

* Fri Oct 09 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.08.2-1
- 20.08.2

* Fri Sep 04 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.08.1-1
- 20.08.1

* Sun Aug 16 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.08.0-1
- 20.08.0

* Sun Jul 12 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 20.04.3-1
- 20.04.3

* Wed May 27 2020 Rex Dieter <rdieter@fedoraproject.org> - 20.04.1-1
- 20.04.1

* Sat May 02 2020 Rex Dieter <rdieter@fedoraproject.org> - 20.04.0-1
- 20.04.0

* Sat Mar 07 2020 Rex Dieter <rdieter@fedoraproject.org> - 19.12.3-1
- 19.12.3

* Tue Feb 04 2020 Rex Dieter <rdieter@fedoraproject.org> - 19.12.2-1
- 19.12.2

* Wed Jan 29 2020 Fedora Release Engineering <releng@fedoraproject.org> - 19.12.1-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_32_Mass_Rebuild

* Mon Jan 13 2020 Rex Dieter <rdieter@fedoraproject.org> - 19.12.1-1
- 19.12.1

* Tue Nov 12 2019 Rex Dieter <rdieter@fedoraproject.org> - 19.08.3-1
- 19.08.3

* Thu Oct 17 2019 Rex Dieter <rdieter@fedoraproject.org> - 19.08.2-1
- 19.08.2

* Fri Oct 04 2019 Rex Dieter <rdieter@fedoraproject.org> - 19.08.1-1
- 19.08.1

* Thu Jul 25 2019 Fedora Release Engineering <releng@fedoraproject.org> - 19.04.3-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_31_Mass_Rebuild

* Fri Jul 12 2019 Rex Dieter <rdieter@fedoraproject.org> - 19.04.3-1
- 19.04.3

* Tue Jun 04 2019 Rex Dieter <rdieter@fedoraproject.org> - 19.04.2-1
- 19.04.2

* Wed May 08 2019 Rex Dieter <rdieter@fedoraproject.org> - 19.04.1-1
- 19.04.1

* Fri Mar 08 2019 Rex Dieter <rdieter@fedoraproject.org> - 18.12.3-1
- 18.12.3

* Tue Feb 05 2019 Rex Dieter <rdieter@fedoraproject.org> - 18.12.2-1
- 18.12.2

* Fri Feb 01 2019 Fedora Release Engineering <releng@fedoraproject.org> - 18.12.1-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_30_Mass_Rebuild

* Tue Jan 08 2019 Rex Dieter <rdieter@fedoraproject.org> - 18.12.1-1
- 18.12.1

* Sun Dec 16 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.12.0-1
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

* Thu Apr 19 2018 Rex Dieter <rdieter@fedoraproject.org> - 18.04.0-1
- 18.04.0

* Tue Mar 06 2018 Rex Dieter <rdieter@fedoraproject.org> - 17.12.3-1
- 17.12.3

* Thu Feb 08 2018 Rex Dieter <rdieter@fedoraproject.org> - 17.12.2-1
- 17.12.2

* Wed Feb 07 2018 Fedora Release Engineering <releng@fedoraproject.org> - 17.12.1-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_28_Mass_Rebuild

* Thu Jan 11 2018 Rex Dieter <rdieter@fedoraproject.org> - 17.12.1-1
- 17.12.1

* Thu Dec 28 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.12.0-1
- 17.12.0

* Wed Nov 08 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.08.3-1
- 17.08.3

* Wed Oct 11 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.08.2-1
- 17.08.2

* Thu Sep 28 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.08.1-1
- 17.08.1

* Thu Aug 03 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.04.3-1
- 17.04.3

* Thu Aug 03 2017 Fedora Release Engineering <releng@fedoraproject.org> - 17.04.2-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Binutils_Mass_Rebuild

* Wed Jul 26 2017 Fedora Release Engineering <releng@fedoraproject.org> - 17.04.2-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Mass_Rebuild

* Thu Jun 15 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.04.2-1
- 17.04.2

* Sun Jun 04 2017 Rex Dieter <rdieter@fedoraproject.org> - 17.04.1-1
- 17.04.1

* Thu Mar 09 2017 Rex Dieter <rdieter@fedoraproject.org> - 16.12.3-1
- 16.12.3

* Thu Feb 09 2017 Rex Dieter <rdieter@fedoraproject.org> - 16.12.2-1
- 16.12.2

* Tue Jan 10 2017 Rex Dieter <rdieter@fedoraproject.org> - 16.12.1-1
- 16.12.1

* Mon Dec 05 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.08.3-1
- 16.08.3

* Thu Oct 13 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.08.2-1
- 16.08.2

* Wed Sep 07 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.08.1-1
- 16.08.1

* Sat Aug 13 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.08.0-1
- 16.08.0

* Sat Aug 06 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.07.90-1
- 16.07.90

* Sun Jul 31 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.07.80-1
- 16.07.80

* Sat Jul 09 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.04.3-1
- 16.04.3

* Sun Jun 12 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.04.2-1
- 16.04.2

* Sun May 08 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.04.1-1
- 16.04.1

* Tue Apr 19 2016 Rex Dieter <rdieter@fedoraproject.org> - 16.04.0-1
- 16.04.0

* Tue Mar 15 2016 Rex Dieter <rdieter@fedoraproject.org> - 15.12.3-1
- 15.12.3

* Mon Feb 15 2016 Rex Dieter <rdieter@fedoraproject.org> - 15.12.2-1
- 15.12.2

* Thu Feb 04 2016 Fedora Release Engineering <releng@fedoraproject.org> - 15.12.1-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_24_Mass_Rebuild

* Sun Jan 10 2016 Rex Dieter <rdieter@fedoraproject.org> - 15.12.1-1
- 15.12.1

* Tue Dec 22 2015 Rex Dieter <rdieter@fedoraproject.org> - 15.12.0-1
- 15.12.0

* Sat Dec 05 2015 Rex Dieter <rdieter@fedoraproject.org> - 15.08.3-1
- 15.08.3

* Wed Oct 14 2015 Rex Dieter <rdieter@fedoraproject.org> - 15.08.2-1
- 15.08.2

* Mon Sep 28 2015 Rex Dieter <rdieter@fedoraproject.org> 15.08.1-1
- 15.08.1, .spec cosmetics

* Thu Aug 20 2015 Than Ngo <than@redhat.com> - 15.08.0-1
- 15.08.0

* Mon Jun 29 2015 Rex Dieter <rdieter@fedoraproject.org> - 15.04.3-1
- 15.04.3

* Wed Jun 17 2015 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 15.04.2-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_23_Mass_Rebuild

* Wed Jun 10 2015 Rex Dieter <rdieter@fedoraproject.org> - 15.04.2-1
- 15.04.2

* Tue May 26 2015 Rex Dieter <rdieter@fedoraproject.org> - 15.04.1-1
- 15.04.1

* Mon Apr 27 2015 Daniel Vrátil <dvratil@redhat.com> - 15.04.0-1
- Update to 15.04.0

* Mon Mar 09 2015 Rex Dieter <rdieter@fedoraproject.org> 0.9.0-1.1
- Requires: qca-ossl

* Mon Oct 20 2014 Jan Grulich <jgrulich@redhat.com> - 0.9.0-1
- Update to 0.9.0

* Wed Sep 17 2014 Jan Grulich <jgrulich@redhat.com> - 0.8.80-1
- Update to 0.8.80 (beta)

* Sun Aug 17 2014 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 0.8.1-4
- Rebuilt for https://fedoraproject.org/wiki/Fedora_21_22_Mass_Rebuild

* Thu Jun 19 2014 Rex Dieter <rdieter@fedoraproject.org> 0.8.1-3
- BR: kdelibs4-webkit-devel

* Sun Jun 08 2014 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 0.8.1-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_21_Mass_Rebuild

* Fri Apr 25 2014 Jan Grulich <jgrulich@redhat.com> 0.8.1-1
- 0.8.1

* Wed Mar 12 2014 Jan  Grulich <jgrulich@redhat.com> 0.8.0-1
- 0.8.0

* Wed Feb 26 2014 Jan Grulich <jgrulich@redhat.com> - 0.7.80-1
- 0.7.80

* Wed Jan 15 2014 Jan Grulich <jgrulich@redhat.com> 0.7.1-1
- 0.7.1

* Tue Oct 29 2013 Jan Grulich <jgrulich@redhat.com> - 0.7.0-1
- 0.7.0

* Wed Sep 25 2013 Rex Dieter <rdieter@fedoraproject.org> - 0.6.80-1
- 0.6.80

* Tue Aug 06 2013 Rex Dieter <rdieter@fedoraproject.org> - 0.6.3-1
- 0.6.3

* Sat Aug 03 2013 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 0.6.2-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_20_Mass_Rebuild

* Mon May 20 2013 Jan Grulich <jgrulich@redhat.com> 0.6.2-1
- 0.6.2

* Wed Apr 17 2013 Jan Grulich <jgrulich@redhat.com> - 0.6.1-1
- 0.6.1

* Tue Apr 02 2013 Jan Grulich <jgrulich@redhat.com> - 0.6.0-1
- 0.6.0

* Thu Mar 07 2013 Rex Dieter <rdieter@fedoraproject.org> 0.5.80-1
- 0.5.80

* Sun Feb 17 2013 Jan Grulich <jgrulich@redhat.com> 0.5.3-1
- 0.5.3

* Thu Feb 14 2013 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 0.5.2-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_19_Mass_Rebuild

* Mon Dec 17 2012 Jan Grulich <jgrulich@redhat.com> 0.5.2-1
- 0.5.2

* Tue Nov 27 2012 Jan Grulich <jgrulich@redhat.com> 0.5.1-3
- Rebuild (qjson)

* Fri Nov 23 2012 Jan Grulich <jgrulich@redhat.com> 0.5.1-2
- Rebuild for qjson 0.8.0

* Fri Oct 05 2012 Rex Dieter <rdieter@fedoraproject.org> - 0.5.1-1
- 0.5.1

* Mon Aug 27 2012 Rex Dieter <rdieter@fedoraproject.org> 0.5.0-1
- 0.5.0

* Thu Jul 26 2012 Jan Grulich <jgrulich@redhat.com> - 0.4.1-1
- 0.4.1

* Thu Jul 19 2012 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 0.4.0-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_18_Mass_Rebuild

* Mon Jun 11 2012 Rex Dieter <rdieter@fedoraproject.org> 0.4.0-1
- 0.4.0

* Mon Apr 02 2012 Rex Dieter <rdieter@fedoraproject.org> 0.3.1-1
- 0.3.1

* Fri Feb 10 2012 Rex Dieter <rdieter@fedoraproject.org> 0.3.0-2
- drop explicit BR: telepathy-qt4-devel

* Wed Jan 25 2012 Rex Dieter <rdieter@fedoraproject.org> 0.3.0-1
- ktp-auth-handler-0.3.0

* Wed Dec 21 2011 Rex Dieter <rdieter@fedoraproject.org> 0.2.0-2
- BR: gettext
- shorten Summary

* Fri Nov 25 2011 Rex Dieter <rdieter@fedoraproject.org> 0.2.0-1
- 0.2.0

