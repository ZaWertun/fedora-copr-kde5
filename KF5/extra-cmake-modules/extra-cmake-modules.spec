# uncomment to enable bootstrap mode
#global bootstrap 1

%if !0%{?bootstrap}
%global docs 1
%global tests 1
%endif

Name:    extra-cmake-modules
Summary: Additional modules for CMake build system
Version: 5.99.0
Release: 1%{?dist}

License: BSD
URL:     https://api.kde.org/ecm/
#URL:    https://cgit.kde.org/extra-cmake-modules.git/

%global versiondir %(echo %{version} | cut -d. -f1-2)
%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0:        http://download.kde.org/%{stable}/frameworks/%{versiondir}/%{name}-%{version}.tar.xz
Source1:        http://download.kde.org/%{stable}/frameworks/%{versiondir}/%{name}-%{version}.tar.xz.sig
Source2:        gpgkey-53E6B47B45CEA3E0D5B7457758D0EE648A48B3BB.gpg
BuildArch:      noarch

## bundle clang python bindings here, at least until they are properly packaged elsewhere, see:
## https://bugzilla.redhat.com/show_bug.cgi?id=1490997
#Source1: clang-python-4.0.1.tar.gz
#if 0%{?fedora} && 0%{?fedora} < 27
%if 0
%global clang 1
Provides: bundled(python2-clang) = 4.0.1
%if 0%{?tests}
BuildRequires: python2-PyQt5-devel
%endif
%endif

## upstreamable patches
# do not unconditionally link in base/core libpoppler library
Patch2: extra-cmake-modules-5.39.0-poppler_overlinking.patch
# https://bugzilla.redhat.com/1435525
Patch3: extra-cmake-modules-5.93.0-qt_prefix.patch

BuildRequires: gnupg2
BuildRequires: kf5-rpm-macros
%if 0%{?docs}
# qcollectiongenerator
BuildRequires: qt5-qttools-devel
# sphinx-build
%if 0%{?fedora} || 0%{?rhel} > 7
BuildRequires: python3-sphinx
%global sphinx_build -DSphinx_BUILD_EXECUTABLE:PATH=%{_bindir}/sphinx-build-3
%else
BuildRequires: python2-sphinx
%endif
%endif

Requires: kf5-rpm-macros
%if 0%{?fedora}
# /usr/share/ECM/kde-modules/appstreamtest.cmake references appstreamcli
# hard vs soft dep?  --rex
Requires: appstream
%endif
# /usr/share/ECM/modules/ECMPoQmTools.cmake
%if 0%{?fedora} || 0%{?rhel} > 7
Requires: cmake(Qt5LinguistTools)
%else
# use pkgname instead of cmake since el7 qt5 pkgs currently do not include cmake() provides
Requires: qt5-linguist
%endif

%description
Additional modules for CMake build system needed by KDE Frameworks.


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%autosetup -p1 %{?clang:-a1}


%build

%if 0%{?clang}
PYTHONPATH=`pwd`/python
export PYTHONPATH
%endif

%cmake_kf5 \
  -DBUILD_HTML_DOCS:BOOL=%{?docs:ON}%{!?docs:OFF} \
  -DBUILD_MAN_DOCS:BOOL=%{?docs:ON}%{!?docs:OFF} \
  -DBUILD_TESTING:BOOL=%{?tests:ON}%{!?tests:OFF} \
  %{?sphinx_build}
%cmake_build


%install
%cmake_install

%if 0%{?clang}
# hack clang-python install
mkdir -p %{buildroot}%{_datadir}/ECM/python/clang
install -m644 -p python/clang/* %{buildroot}%{_datadir}/ECM/python/clang/
%endif


%check
%if 0%{?tests}
%if 0%{?clang}
PYTHONPATH=`pwd`/python
export PYTHONPATH
%endif
export CTEST_OUTPUT_ON_FAILURE=1
make test ARGS="--output-on-failure --timeout 300" -C %{_target_platform} ||:
%endif


%files
%doc README.rst
%license COPYING-CMAKE-SCRIPTS
%{_datadir}/ECM/
%if 0%{?docs}
%{_kf5_docdir}/ECM/html/
%{_kf5_mandir}/man7/ecm*.7*
%endif


%changelog
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

* Sat Jan  9 16:30:11 MSK 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.78.0-1
- 5.78.0

* Mon Dec 14 16:50:05 MSK 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.77.0-1
- 5.77.0

* Sun Nov 15 22:13:33 MSK 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.76.0-1
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

* Tue Jan 28 2020 Fedora Release Engineering <releng@fedoraproject.org> - 5.66.0-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_32_Mass_Rebuild

* Tue Jan 07 2020 Rex Dieter <rdieter@fedoraproject.org> - 5.66.0-1
- 5.66.0

* Tue Dec 17 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.65.0-1
- 5.65.0

* Wed Dec 11 2019 Troy Dawson <tdawson@redhat.com> - 5.64.0-2
- Remove appstream dependency for RHEL8; appstream not in RHEL8

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

* Mon Apr 08 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.57.0-1
- 5.57.0

* Thu Mar 28 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.56.0-3
- re-enable doc generation on rawhide (#1687572)

* Mon Mar 11 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.56.0-2
- use %%make_build
- use python3-sphinx (on f30, python-sphinx 2.0b1 busted, #1687572)

* Tue Mar 05 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.56.0-1
- 5.56.0

* Mon Feb 04 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.55.0-1
- 5.55.0

* Thu Jan 31 2019 Fedora Release Engineering <releng@fedoraproject.org> - 5.54.0-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_30_Mass_Rebuild

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

* Sun Jul 08 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.48.0-1
- 5.48.0

* Sat Jun 02 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.47.0-1
- 5.47.0
- Issue with KDE_INSTALL_QMLDIR (#1435525)

* Thu May 31 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.46.0-2
- Issue with KDE_INSTALL_QMLDIR (#1435525)

* Sat May 05 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.46.0-1
- 5.46.0

* Sun Apr 08 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.45.0-1
- 5.45.0

* Sat Mar 03 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.44.0-1
- 5.44.0

* Wed Feb 07 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.43.0-1
- 5.43.0

* Wed Feb 07 2018 Fedora Release Engineering <releng@fedoraproject.org> - 5.42.0-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_28_Mass_Rebuild

* Mon Jan 08 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.42.0-1
- 5.42.0
- bundle python2-clang only on < f28

* Mon Dec 04 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.41.0-1
- 5.41.0

* Fri Nov 10 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.40.0-1
- 5.40.0

* Thu Nov 09 2017 Troy Dawson <tdawson@redhat.com> - 5.39.0-2
- Cleanup conditionals

* Mon Nov 06 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.39.0-2
- FindPoppler.cmake: avoid overlinking base/core libpoppler

* Sun Oct 08 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.39.0-1
- 5.39.0

* Wed Sep 13 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.38.0-2
- Provides: bundled(python2-clang)
- FindPythonModuleGeneration.cmake for fedora (kde#372311)

* Mon Sep 11 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.38.0-1
- 5.38.0

* Fri Aug 25 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.37.0-1
- 5.37.0

* Wed Jul 26 2017 Fedora Release Engineering <releng@fedoraproject.org> - 5.36.0-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Mass_Rebuild

* Mon Jul 03 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.36.0-1
- 5.36.0

* Sun Jun 04 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.35.0-1
- 5.35.0

* Mon May 15 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.34.0-1
- 5.34.0

* Mon Apr 03 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.33.0-1
- 5.33.0

* Sat Mar 04 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.32.0-1
- 5.32.0

* Mon Feb 06 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.31.0-3
- fix qt5-linguist dep, fix bootstrap (docs)

* Mon Feb 06 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.31.0-2
- Requires: Qt5LinguistTools

* Mon Feb 06 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.31.0-1
- 5.31.0

* Tue Jan 24 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.29.0-2
- Requires: appstream
- update URL
- .spec cosmetics

* Fri Dec 16 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.29.0-1
- 5.29.0

* Mon Oct 03 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.27.0-1
- 5.27.0

* Wed Sep 07 2016 Daniel Vrátil <dvratil@fedoraproject.org> - 5.26.0-1
- KDE Frameworks 5.26.0

* Sun Aug 07 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.25.0-1
- 5.25.0

* Wed Jul 06 2016 Daniel Vrátil <dvratil@fedoraproject.org> - 5.24.0-1
- KDE Frameworks 5.24.0

* Tue Jun 07 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.23.0-2
- support bootstrap, add docs/tests

* Tue Jun 07 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.23.0-1
- 5.23.0, relax kf5-rpm-macros dep

* Mon May 16 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.22.0-1
- KDE Frameworks 5.22.0

* Mon Apr 04 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.21.0-2
- Update URL

* Mon Apr 04 2016 Rex Dieter <rdieter@fedoraproject.org> - 5.21.0-1
- KDE Frameworks 5.21.0

* Mon Mar 14 2016 Daniel Vrátil <dvratil@fedoraproject.org> - 5.20.0-1
- KDE Frameworks 5.20.0

* Thu Feb 11 2016 Daniel Vrátil <dvratil@fedoraproject.org> - 5.19.0-1
- KDE Frameworks 5.19.0

* Wed Feb 03 2016 Fedora Release Engineering <releng@fedoraproject.org> - 5.18.0-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_24_Mass_Rebuild

* Thu Jan 14 2016 Rex Dieter <rdieter@fedoraproject.org> 5.18.0-2
- use kf5-rpm-macros, update URL, use %%license

* Sun Jan 03 2016 Daniel Vrátil <dvratil@fedoraproject.org> - 5.18.0-1
- KDE Frameworks 5.18.0

* Sun Dec 13 2015 Helio Chissini de Castro <helio@kde.org> - 5.17.0-2
- Adapt epel cmake3 changes

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
- KDE Frameworks 5.13

* Thu Jul 09 2015 Rex Dieter <rdieter@fedoraproject.org> - 5.12.0-1
- 5.12.0, update URL (to reference projects.kde.org), .spec cosmetics

* Wed Jun 17 2015 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 5.11.0-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_23_Mass_Rebuild

* Wed Jun 10 2015 Daniel Vrátil <dvratil@redhat.com> - 5.11.0-1
- KDE Frameworks 5.11.0

* Mon May 11 2015 Daniel Vrátil <dvratil@redhat.com> - 5.10.0-1
- KDE Frameworks 5.10.0

* Tue Apr 07 2015 Daniel Vrátil <dvratil@redhat.com> - 5.9.0-1
- KDE Frameworks 5.9.0

* Mon Mar 16 2015 Daniel Vrátil <dvratil@redhat.com> - 1.8.0-1
- extra-cmake-modules 1.8.0 (KDE Frameworks 5.8.0)

* Fri Feb 13 2015 Daniel Vrátil <dvratil@redhat.com> - 1.7.0-1
- extra-cmake-modules 1.7.0 (KDE Frameworks 5.7.0)

* Mon Jan 12 2015 Daniel Vrátil <dvratil@redhat.com> - 1.6.1-1
- Update to 1.6.1 which includes upstream fix for kde#341717

* Sun Jan 11 2015 Kevin Kofler <Kevin@tigcc.ticalc.org> - 1.6.0-3
- Use upstream version of the kde#342717 patch by Alex Merry

* Sun Jan 11 2015 Kevin Kofler <Kevin@tigcc.ticalc.org> - 1.6.0-2
- Do not unset old-style variables in KDEInstallDirs.cmake, it breaks projects
  using GNUInstallDirs for some parts and KDEInstallDirs for others (kde#342717)

* Thu Jan 08 2015 Daniel Vrátil <dvratil@redhat.com> - 1.6.0-1
- extra-cmake-modules 1.6.0 (KDE Frameworks 5.6.0)

* Thu Dec 11 2014 Daniel Vrátil <dvratil@redhat.com> - 1.5.0-1
- extra-cmake-modules 1.5.0 (KDE Frameworks 5.5.0)

* Mon Nov 03 2014 Daniel Vrátil <dvratil@redhat.com> - 1.4.0-1
- extra-cmake-modules 1.4.0 (KDE Frameworks 5.4.0)

* Tue Oct 07 2014 Daniel Vrátil <dvratil@redhat.com> - 1.3.0-1
- extra-cmake-modules 1.3.0 (KDE Frameworks 5.3.0)

* Tue Sep 16 2014 Daniel Vrátil <dvratil@redhat.com> - 1.2.1-1
- extra-cmake-modules 1.2.1 (KDE Frameworks 5.2.0)

* Mon Sep 15 2014 Daniel Vrátil <dvratil@redhat.com> - 1.2.0-1
- extra-cmake-modules 1.2.0 (KDE Frameworks 5.2.0)

* Wed Aug 06 2014 Daniel Vrátil <dvratil@redhat.com> - 1.1.0-1
- extra-cmake-modules 1.1.0 (KDE Frameworks 5.1.0)

* Thu Jul 10 2014 Daniel Vrátil <dvratil@redhat.com> - 1.0.0-1
- extra-cmake-modules 1.0.0 (KDE Frameworks 5.0.0)

* Sat Jun 07 2014 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 0.0.14-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_21_Mass_Rebuild

* Tue Jun 03 2014 Daniel Vrátil <dvratil@redhat.com> 0.0.14-2
- Strip architecture check from a CMake-generated file to fix noarch build

* Tue Jun 03 2014 Daniel Vrátil <dvratil@redhat.com> 0.0.14-1
- extra-cmake-modules 0.0.14 (KDE Frameworks 4.100.0)

* Mon May 05 2014 Daniel Vrátil <dvratil@redhat.com> 0.0.13-1
- extra-cmake-modules 0.0.13 (KDE Frameworks 4.99.0)

* Fri Apr 11 2014 Daniel Vrátil <dvratil@redhat.com> 0.0.12-3
- Remove debug_package, add %%{?dist} to Release

* Fri Apr 11 2014 Daniel Vrátil <dvratil@redhat.com> 0.0.12-2
- Don't depend on kf5-filesystem

* Mon Mar 31 2014 Jan Grulich <jgrulich@redhat.com> 0.0.12-1
- Update to KDE Frameworks 5 Beta 1 (4.98.0)

* Wed Mar 05 2014 Jan Grulich <jgrulich@redhat.com> 0.0.11-1
- Update to KDE Frameworks 5 Alpha 2 (4.97.0)

* Wed Feb 12 2014 Daniel Vrátil <dvratil@redhat.com> 0.0.10-1
- Update to KDE Frameworks 5 Alpha 1 (4.96.0)

* Wed Feb 05 2014 Daniel Vrátil <dvratil@redhat.com> 0.0.10-0.1.20140205git
- Update to pre-relase snapshot of 0.0.10

* Tue Feb 04 2014 Lubomir Rintel <lkundrak@v3.sk> - 0.0.9-1
- Update to Jan 7 release

* Mon Sep 16 2013 Lubomir Rintel <lkundrak@v3.sk> - 0.0.9-0.1.20130013git5367954
- Initial packaging
