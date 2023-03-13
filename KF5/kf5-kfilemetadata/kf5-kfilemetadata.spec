%global framework kfilemetadata

# Define to 1 to enable ffmpeg extractor
#global         ffmpeg 1

%if 0%{?fedora}
%global         ffmpeg  1
%global         catdoc  1
%global         ebook   1
%global         poppler 1
%global         taglib  1
%endif

Name:           kf5-%{framework}
Summary:        A Tier 2 KDE Framework for extracting file metadata
Version:        5.104.0
Release:        2%{?dist}

# # KDE e.V. may determine that future LGPL versions are accepted
License:        LGPLv2 or LGPLv3
URL:            https://cgit.kde.org/%{framework}

%global majmin %(echo %{version} | cut -d. -f1-2)
%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0:        http://download.kde.org/%{stable}/frameworks/%{majmin}/%{framework}-%{version}.tar.xz
Source1:        http://download.kde.org/%{stable}/frameworks/%{majmin}/%{framework}-%{version}.tar.xz.sig
Source2:        gpgkey-53E6B47B45CEA3E0D5B7457758D0EE648A48B3BB.gpg

# filter plugin provides
%global __provides_exclude_from ^(%{_kf5_plugindir}/.*\\.so)$

BuildRequires:  gnupg2
BuildRequires:  extra-cmake-modules >= %{majmin}
BuildRequires:  kf5-karchive-devel >= %{majmin}
BuildRequires:  kf5-kcoreaddons-devel >= %{majmin}
BuildRequires:  kf5-ki18n-devel >= %{majmin}
BuildRequires:  kf5-rpm-macros
# optional
BuildRequires:  kf5-kconfig-devel >= %{majmin}

BuildRequires:  qt5-qtbase-devel

BuildRequires:  libattr-devel
BuildRequires:  pkgconfig(exiv2) >= 0.20

## optional deps
%if 0%{?catdoc}
# not strictly required at build-time, satisfying runtime dep check only
BuildRequires:  catdoc
Recommends:     catdoc
%endif
%if 0%{?ebook}
BuildRequires:  ebook-tools-devel
%endif
%if 0%{?ffmpeg}
BuildRequires:  ffmpeg-devel
%endif
%if 0%{?poppler}
BuildRequires:  pkgconfig(poppler-qt5)
%endif
%if 0%{?taglib}
BuildRequires:  pkgconfig(taglib) >= 1.9
%endif

%description
%{summary}.

%package devel
Summary:        Developer files for %{name}
Requires:       %{name}%{?_isa} = %{version}-%{release}
Requires:       qt5-qtbase-devel
%description devel
%{summary}.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -n %{framework}-%{version} -p1


%build
%cmake_kf5
%cmake_build


%install
%cmake_install
%find_lang %{name} --all-name
mkdir -p %{buildroot}%{_kf5_plugindir}/kfilemetadata/writers/


%ldconfig_scriptlets

%files -f %{name}.lang
%license LICENSES/*.txt
%{_kf5_datadir}/qlogging-categories5/%{framework}*
%{_kf5_libdir}/libKF5FileMetaData.so.*

# consider putting these into some subpkg ?
%dir %{_kf5_plugindir}/kfilemetadata/
%{_kf5_plugindir}/kfilemetadata/kfilemetadata_*.so
%dir %{_kf5_plugindir}/kfilemetadata/writers/
%if 0%{?taglib}
%{_kf5_plugindir}/kfilemetadata/writers/kfilemetadata_taglibwriter.so
%endif

%files devel
%{_kf5_libdir}/libKF5FileMetaData.so
%{_kf5_libdir}/cmake/KF5FileMetaData
%{_kf5_includedir}/KFileMetaData/
%{_kf5_archdatadir}/mkspecs/modules/qt_KFileMetaData.pri


%changelog
* Mon Mar 13 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.104.0-2
- ffmpeg support enabled

* Sat Mar 11 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.104.0-1
- 5.104.0

* Sun Feb 12 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.103.0-1
- 5.103.0

* Sat Jan 14 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.102.0-1
- 5.102.0

* Sat Dec 10 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.101.0-1
- 5.101.0

* Mon Nov 14 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.100.0-1
- 5.100.0

* Mon Oct 10 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.99.0-1
- 5.99.0

* Mon Sep 12 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.98.0-1
- 5.98.0

* Sun Aug 14 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.97.0-1
- 5.97.0

* Sun Jul 10 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.96.0-1
- 5.96.0

* Mon Jun 13 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.95.0-1
- 5.95.0

* Sat May 14 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.94.0-1
- 5.94.0

* Sun Apr 10 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.93.0-1
- 5.93.0

* Sun Mar 13 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.92.0-1
- 5.92.0

* Mon Feb 14 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.91.0-1
- 5.91.0

* Sat Jan 08 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.90.0-1
- 5.90.0

* Mon Dec 13 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.89.0-1
- 5.89.0

* Sat Nov 13 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.88.0-1
- 5.88.0

* Sat Oct 09 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.87.0-1
- 5.87.0

* Sat Sep 11 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.86.0-1
- 5.86.0

* Sat Aug 14 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.85.0-1
- 5.85.0

* Sat Jul 10 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.84.0-1
- 5.84.0

* Sun Jun 13 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.83.0-1
- 5.83.0

* Sat May 08 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.82.0-1
- 5.82.0

* Sun Apr 11 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.81.0-1
- 5.81.0

* Sat Mar 13 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.80.0-1
- 5.80.0

* Sat Feb 13 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.79.0-1
- 5.79.0

* Sat Jan  9 16:30:21 MSK 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.78.0-1
- 5.78.0

* Mon Dec 14 16:50:14 MSK 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.77.0-1
- 5.77.0

* Sun Nov 15 22:13:43 MSK 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.76.0-1
- 5.76.0

* Sat Oct 10 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.75.0-1
- 5.75.0

* Thu Sep 17 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.74.0-1
- 5.74.0

* Mon Aug 10 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.73.0-1
- 5.73.0

* Mon Jul 13 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.72.0-1
- 5.72.0

* Sun Jun 14 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.71.0-1
- 5.71.0

* Sun May 10 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.70.0-1
- 5.70.0

* Sat Apr 11 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.69.0-1
- 5.69.0

* Mon Mar 16 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.68.0-1
- 5.68.0

* Thu Feb 27 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.67.0-2
- rebuild

* Sun Feb 02 2020 Rex Dieter <rdieter@fedoraproject.org> - 5.67.0-1
- 5.67.0

* Wed Jan 29 2020 Fedora Release Engineering <releng@fedoraproject.org> - 5.66.0-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_32_Mass_Rebuild

* Fri Jan 17 2020 Marek Kasik <mkasik@redhat.com> - 5.66.0-2
- Rebuild for poppler-0.84.0

* Tue Jan 07 2020 Rex Dieter <rdieter@fedoraproject.org> - 5.66.0-1
- 5.66.0

* Tue Dec 17 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.65.0-1
- 5.65.0

* Fri Nov 08 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.64.0-1
- 5.64.0

* Tue Oct 22 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.63.0-1
- 5.63.0

* Mon Sep 16 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.62.0-1
- 5.62.0

* Wed Aug 07 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.61.0-1
- 5.61.0

* Thu Jul 25 2019 Fedora Release Engineering <releng@fedoraproject.org> - 5.60.0-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_31_Mass_Rebuild

* Sat Jul 13 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.60.0-1
- 5.60.0

* Thu Jun 06 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.59.0-1
- 5.59.0

* Tue May 07 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.58.0-1
- 5.58.0

* Tue Apr 09 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.57.0-1
- 5.57.0

* Tue Mar 05 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.56.0-1
- 5.56.0

* Mon Feb 04 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.55.0-1
- 5.55.0

* Fri Feb 01 2019 Fedora Release Engineering <releng@fedoraproject.org> - 5.54.0-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_30_Mass_Rebuild

* Wed Jan 30 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.54.0-2
- rebuild (exiv2)

* Tue Jan 08 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.54.0-1
- 5.54.0

* Sun Dec 09 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.53.0-1
- 5.53.0

* Sun Nov 04 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.52.0-1
- 5.52.0

* Wed Oct 10 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.51.0-1
- 5.51.0

* Tue Sep 04 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.50.0-1
- 5.50.0

* Tue Aug 07 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.49.0-1
- 5.49.0

* Fri Jul 13 2018 Fedora Release Engineering <releng@fedoraproject.org> - 5.48.0-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_29_Mass_Rebuild

* Mon Jul 09 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.48.0-1
- 5.48.0

* Thu Jun 07 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.47.0-2
- use %%autosetup

* Sat Jun 02 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.47.0-1
- 5.47.0

* Sat May 05 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.46.0-1
- 5.46.0

* Sun Apr 08 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.45.0-1
- 5.45.0

* Fri Mar 23 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.44.0-3
- use %%ldconfig_scriptlets %%make_build

* Fri Mar 23 2018 Marek Kasik <mkasik@redhat.com> - 5.44.0-2
- Rebuild for poppler-0.63.0

* Sat Mar 03 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.44.0-1
- 5.44.0

* Wed Feb 07 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.43.0-1
- 5.43.0

* Mon Jan 08 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.42.0-1
- 5.42.0

* Mon Dec 04 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.41.0-1
- 5.41.0

* Fri Nov 10 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.40.0-1
- 5.40.0

* Wed Nov 08 2017 David Tardon <dtardon@redhat.com> - 5.39.0-3
- rebuild for poppler 0.61.0

* Mon Nov 06 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.39.0-2
- rebuild (ecm)

* Sun Oct 08 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.39.0-1
- 5.39.0

* Fri Oct 06 2017 David Tardon <dtardon@redhat.com> - 5.38.0-2
- rebuild for poppler 0.60.1

* Mon Sep 11 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.38.0-1
- 5.38.0

* Fri Sep 08 2017 David Tardon <dtardon@redhat.com> - 5.37.0-2
- rebuild for poppler 0.59.0

* Fri Aug 25 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.37.0-1
- 5.37.0

* Thu Aug 03 2017 David Tardon <dtardon@redhat.com> - 5.36.0-4
- rebuild for poppler 0.57.0

* Thu Aug 03 2017 Fedora Release Engineering <releng@fedoraproject.org> - 5.36.0-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Binutils_Mass_Rebuild

* Wed Jul 26 2017 Fedora Release Engineering <releng@fedoraproject.org> - 5.36.0-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Mass_Rebuild

* Mon Jul 03 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.36.0-1
- 5.36.0

* Wed Jun 07 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.35.0-2
- rebuild

* Sun Jun 04 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.35.0-1
- 5.35.0

* Mon May 15 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.34.0-1
- 5.34.0

* Tue May 02 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.33.0-2
- rebuild (exiv2), fix URL

* Mon Apr 03 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.33.0-1
- 5.33.0

* Sat Mar 04 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.32.0-1
- 5.32.0

* Mon Feb 06 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.31.0-1
- 5.31.0

* Mon Jan 02 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.29.0-2
- filter plugin provides

* Fri Dec 16 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.29.0-1
- 5.29.0

* Tue Oct 04 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.27.0-1
- 5.27.0

* Wed Sep 07 2016 Daniel Vrátil <dvratil@fedoraproject.org> - 5.26.0-1
- KDE Frameworks 5.26.0

* Mon Aug 08 2016 Daniel Vrátil <dvratil@fedoraproject.org> - 5.25.0-1
- KDE Frameworks 5.25.0

* Wed Jul 06 2016 Daniel Vrátil <dvratil@fedoraproject.org> - 5.24.0-1
- KDE Frameworks 5.24.0

* Tue Jun 07 2016 Daniel Vrátil <dvratil@fedoraproject.org> - 5.23.0-1
- KDE Frameworks 5.23.0

* Mon May 16 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.22.0-1
- KDE Frameworks 5.22.0

* Tue Apr 05 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.21.0-2
- update URL, make some deps fedora-only

* Mon Apr 04 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.21.0-1
- KDE Frameworks 5.21.0

* Mon Mar 14 2016 Daniel Vrátil <dvratil@fedoraproject.org> - 5.20.0-1
- KDE Frameworks 5.20.0

* Thu Feb 11 2016 Daniel Vrátil <dvratil@fedoraproject.org> - 5.19.0-1
- KDE Frameworks 5.19.0

* Thu Feb 04 2016 Fedora Release Engineering <releng@fedoraproject.org> - 5.18.0-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_24_Mass_Rebuild

* Sun Jan 03 2016 Daniel Vrátil <dvratil@fedoraproject.org> - 5.18.0-1
- KDE Frameworks 5.18.0

* Fri Dec 18 2015 Rex Dieter <rdieter@fedoraproject.org> 5.17.0-2
- .spec cosmetics, update URL, use %%license

* Tue Dec 08 2015 Daniel Vrátil <dvratil@fedoraproject.org> - 5.17.0-1
- KDE Frameworks 5.17.0

* Sun Nov 08 2015 Daniel Vrátil <dvratil@fedoraproject.org> - 5.16.0-1
- KDE Frameworks 5.16.0

* Thu Oct 08 2015 Daniel Vrátil <dvratil@redhat.com> - 5.15.0-1
- KDE Frameworks 5.15.0

* Wed Sep 16 2015 Daniel Vrátil <dvratil@redhat.com> - 5.14.0-1
- KDE Frameworks 5.14.0

* Wed Aug 19 2015 Daniel Vrátil <dvratil@redhat.com> - 5.13.0-1
- KDE Frameworks 5.13.0

* Wed Aug 19 2015 Daniel Vrátil <dvratil@redhat.com> - 5.13.0-1
- KDE Frameworks 5.13.0

* Tue Aug 11 2015 Daniel Vrátil <dvratil@redhat.com> - 5.13.0-0.1
- KDE Frameworks 5.13 (moved from Plasma 5 to KF5)

* Thu Jun 25 2015 Daniel Vrátil <dvratil@redhat.com> - 5.9.2-1
- Plasma 5.3.2

* Wed Jun 24 2015 Rex Dieter <rdieter@fedoraproject.org> - 5.9.1-3
- rebuild (exiv2)

* Wed Jun 17 2015 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 5.9.1-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_23_Mass_Rebuild

* Tue May 26 2015 Daniel Vrátil <dvratil@redhat.com> - 5.9.1-1
- Plasma 5.3.1

* Mon Apr 27 2015 Daniel Vrátil <dvratil@redhat.com> - 5.9.0-1
- Plasma 5.3.0

* Wed Apr 22 2015 Daniel Vrátil <dvratil@redhat.com> - 5.6.95-1
- Plasma 5.2.95

* Fri Mar 20 2015 Daniel Vrátil <dvratil@redhat.com> - 5.6.2-1
- Plasma 5.2.2

* Fri Feb 27 2015 Daniel Vrátil <dvratil@redhat.com> - 5.6.1-2
- Rebuild (GCC 5)

* Tue Feb 24 2015 Daniel Vrátil <dvratil@redhat.com> - 5.6.1-1
- Plasma 5.2.1

* Mon Jan 26 2015 Daniel Vrátil <dvratil@redhat.com> - 5.6.0-1
- Plasma 5.2.0

* Mon Jan 12 2015 Daniel Vrátil <dvratil@redhat.com> - 5.5.95-1
- Plasma 5.1.95 Beta (KFileMetaData 5.5.95)

* Wed Dec 17 2014 Daniel Vrátil <dvratil@redhat.com> - 5.1.2-1
- Plasma 5.1.2

* Fri Nov 07 2014 Daniel Vrátil <dvratil@redhat.com> - 5.1.1-1
- Plasma 5.1.1

* Tue Oct 14 2014 Daniel Vrátil <dvratil@redhat.com> - 5.1.0.1-1
- Plasma 5.1.0.1

* Thu Oct 09 2014 Daniel Vrátil <dvratil@redhat.com> - 5.1.0-1
- Plasma 5.1.0

* Tue Sep 16 2014 Daniel Vrátil <dvratil@redhat.com> - 5.0.2-1
- Plasma 5.0.2

* Sun Aug 10 2014 Daniel Vrátil <dvratil@redhat.com> - 5.0.1-1
- Plasma 5.0.1

* Wed Jul 16 2014 Daniel Vrátil <dvratil@redhat.com> - 5.0.0-1
- Plasma 5.0.0

* Wed Jun 11 2014 Daniel Vrátil <dvratil@redhat.com> - 4.97.0-3.20140611git034abaa
- Update to latest git snapshot

* Wed May 14 2014 Daniel Vrátil <dvratil@redhat.com> - 4.97.0-2.20140611gitdcc52ae
- Updated to latest git snapshot

* Fri Apr 18 2014 Daniel Vrátil <dvratil@redhat.com> - 4.97.0-1
- Fork kfilemetadata into kf5-kfilemetadata
