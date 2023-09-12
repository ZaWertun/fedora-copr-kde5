%global base_name    drkonqi

Name:    plasma-drkonqi
Summary: DrKonqi crash handler for KF5/Plasma5
Version: 5.27.8
Release: 1%{?dist}

License: GPLv2+
URL:     https://cgit.kde.org/%{base_name}.git

%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0:        http://download.kde.org/%{stable}/plasma/%(echo %{version} |cut -d. -f1-3)/%{base_name}-%{version}.tar.xz
Source1:        http://download.kde.org/%{stable}/plasma/%(echo %{version} |cut -d. -f1-3)/%{base_name}-%{version}.tar.xz.sig
Source2:        https://jriddell.org/esk-riddell.gpg

## upstreamable Patches
# dnf debuginfo-install
Patch52:        drkonqi-installdbgsymbols.patch

BuildRequires:  gnupg2
BuildRequires:  extra-cmake-modules
BuildRequires:  kf5-rpm-macros
BuildRequires:  qt5-qtbase-devel

BuildRequires:  cmake(KUserFeedback)

BuildRequires:  cmake(KF5CoreAddons)
BuildRequires:  cmake(KF5I18n)
BuildRequires:  cmake(KF5Notifications)
BuildRequires:  cmake(KF5Service)
BuildRequires:  cmake(KF5ConfigWidgets)
BuildRequires:  cmake(KF5JobWidgets)
BuildRequires:  cmake(KF5KIO)
BuildRequires:  cmake(KF5Crash)
BuildRequires:  cmake(KF5XmlRpcClient)
BuildRequires:  cmake(KF5Wallet)
BuildRequires:  cmake(KF5IdleTime)
BuildRequires:  cmake(KF5WindowSystem)
BuildRequires:  cmake(KF5SyntaxHighlighting)
BuildRequires:  cmake(KF5Declarative)

BuildRequires:  cmake(Qt5Quick)
BuildRequires:  cmake(Qt5X11Extras)

# retired from plasma-workspace
Obsoletes: plasma-workspace-drkonqi < 5.10.95
Provides: plasma-workspace-drkonqi = %{version}-%{release}

Requires: dnf-command(debuginfo-install)
Requires: konsole5
Requires: polkit
# owner of setsebool
Requires(post): policycoreutils

%description
%{summary}


%prep
%{gpgverify} --keyring='%{SOURCE2}' --signature='%{SOURCE1}' --data='%{SOURCE0}'
%setup -q -n %{base_name}-%{version}

%patch52 -p1 -b .installdgbsymbols

%build
%cmake_kf5
%cmake_build

%install
%cmake_install
# installdbgsymbols script
install -p -D -m755 src/doc/examples/installdbgsymbols_fedora.sh \
    %{buildroot}%{_libexecdir}/installdbgsymbols.sh

%find_lang all --with-html --with-qt --all-name
grep drkonqi5.mo all.lang > plasma-drkonqi.lang

%post
# make DrKonqi work by default by taming SELinux enough (suggested by dwalsh)
# if KDE_DEBUG is set, DrKonqi is disabled, so do nothing
# if it is unset (or empty), check if deny_ptrace is already disabled
# if not, disable it
if [ -z "$KDE_DEBUG" ] ; then
if [ "`getsebool deny_ptrace 2>/dev/null`" == 'deny_ptrace --> on' ] ; then
  setsebool -P deny_ptrace off &> /dev/null || :
fi
fi

%files -f plasma-drkonqi.lang
%license LICENSES/*.txt
%{_libexecdir}/drkonqi
%{_libexecdir}/installdbgsymbols.sh
%{_kf5_datadir}/drkonqi/
%{_kf5_datadir}/applications/org.kde.*.desktop
%{_kf5_datadir}/qlogging-categories5/*categories

%changelog
* Tue Sep 12 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.27.8-1
- 5.27.8

* Tue Aug 01 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.27.7-1
- 5.27.7

* Tue Jun 20 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.27.6-1
- 5.27.6

* Tue May 09 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.27.5-1
- 5.27.5

* Tue Apr 04 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.27.4-1
- 5.27.4

* Tue Mar 14 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.27.3-1
- 5.27.3

* Tue Feb 28 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.27.2-1
- 5.27.2

* Tue Feb 21 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.27.1-1
- 5.27.1

* Tue Feb 14 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.27.0-1
- 5.27.0

* Wed Jan 04 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.26.5-1
- 5.26.5

* Tue Nov 29 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.26.4-1
- 5.26.4

* Tue Nov 08 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.26.3-1
- 5.26.3

* Wed Oct 26 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.26.2-1
- 5.26.2

* Tue Oct 18 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.26.1-1
- 5.26.1

* Tue Oct 11 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.26.0-1
- 5.26.0

* Tue Sep 06 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.25.5-1
- 5.25.5

* Tue Aug 02 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.25.4-1
- 5.25.4

* Wed Jul 13 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.25.3-1
- 5.25.3

* Tue Jun 28 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.25.2-1
- 5.25.2

* Tue Jun 21 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.25.1-1
- 5.25.1

* Tue Jun 14 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.25.0-1
- 5.25.0

* Tue May 03 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.24.5-1
- 5.24.5

* Tue Mar 29 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.24.4-1
- 5.24.4

* Tue Mar 08 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.24.3-1
- 5.24.3

* Tue Feb 22 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.24.2-1
- 5.24.2

* Tue Feb 15 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.24.1-1
- 5.24.1

* Tue Feb 08 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.24.0-1
- 5.24.0

* Tue Jan 04 2022 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.23.5-1
- 5.23.5

* Tue Nov 30 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.23.4-1
- 5.23.4

* Tue Nov 09 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.23.3-1
- 5.23.3

* Tue Oct 26 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.23.2-1
- 5.23.2

* Tue Oct 19 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.23.1-1
- 5.23.1

* Thu Oct 14 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.23.0-1
- 5.23.0

* Tue Aug 31 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.22.5-1
- 5.22.5

* Tue Jul 27 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.22.4-1
- 5.22.4

* Thu Jul 08 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.22.3-1
- 5.22.3

* Wed Jun 23 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.22.2.1-1
- 5.22.2.1

* Tue Jun 22 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.22.2-1
- 5.22.2

* Tue Jun 15 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.22.1-1
- 5.22.1

* Tue Jun 08 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.22.0-1
- 5.22.0

* Tue May 04 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.21.5-1
- 5.21.5

* Tue Apr 06 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.21.4-1
- 5.21.4

* Tue Mar 16 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.21.3-1
- 5.21.3

* Wed Mar 03 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.21.2-1
- 5.21.2

* Tue Feb 23 13:50:07 MSK 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.21.1-1
- 5.21.1

* Tue Feb 16 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.21.0-1
- 5.21.0

* Tue Jan  5 22:06:20 MSK 2021 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.20.5-1
- 5.20.5

* Tue Dec  1 22:30:46 MSK 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.20.4-1
- 5.20.4

* Wed Nov 11 11:10:21 MSK 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.20.3-1
- 5.20.3

* Tue Oct 27 16:56:29 MSK 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.20.2-1
- 5.20.2

* Tue Oct 20 17:02:46 MSK 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.20.1-1
- 5.20.1

* Tue Oct 13 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.20.0-1
- 5.20.0

* Tue Sep 01 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.19.5-1
- 5.19.5

* Tue Jul 28 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.19.4-1
- 5.19.4

* Tue Jul 07 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.19.3-1
- 5.19.3

* Tue Jun 23 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.19.2-1
- 5.19.2

* Tue Jun 16 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.19.1-1
- 5.19.1

* Mon Jun 15 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.19.0-1
- 5.19.0

* Wed May 06 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.18.5-1
- 5.18.5

* Wed Apr 01 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.18.4.1-1
- 5.18.4.1

* Wed Mar 11 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.18.3-1
- 5.18.3

* Wed Feb 26 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.18.2-1
- 5.18.2

* Wed Feb 19 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.18.1-1
- 5.18.1

* Tue Feb 11 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.18.0-1
- 5.18.0

* Thu Jan 09 2020 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.17.5-1
- 5.17.5

* Tue Dec 03 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.17.4-1
- 5.17.4

* Tue Nov 12 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.17.3-1
- 5.17.3

* Wed Oct 30 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.17.2-1
- 5.17.2

* Wed Oct 23 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.17.1-1
- 5.17.1

* Tue Oct 15 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.17.0-1
- 5.17.0

* Tue Sep 03 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.16.5-1
- 5.16.5

* Tue Jul 30 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.16.4-1
- 5.16.4

* Tue Jul 09 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.16.3-1
- 5.16.3

* Tue Jun 25 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.16.2-1
- 5.16.2

* Tue Jun 18 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.16.1-1
- 5.16.1

* Tue Jun 11 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.16.0-1
- 5.16.0

* Tue May 07 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.15.5-1
- 5.15.5

* Sun Apr 28 2019 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.15.4-1
- 5.15.4

* Tue Feb 19 2019 Rex Dieter <rdieter@fedoraproject.org> - 5.14.5-1
- 5.14.5

* Tue Nov 27 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.14.4-1
- 5.14.4

* Thu Nov 08 2018 Martin Kyral <martin.kyral@gmail.com> - 5.14.3-1
- 5.14.3

* Wed Oct 24 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.14.2-1
- 5.14.2

* Tue Oct 16 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.14.1-1
- 5.14.1

* Fri Oct 05 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.14.0-1
- 5.14.0

* Fri Sep 14 2018 Martin Kyral <martin.kyral@gmail.com> - 5.13.90-1
- 5.13.90

* Tue Sep 04 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.13.5-1
- 5.13.5

* Thu Aug 02 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.13.4-1
- 5.13.4

* Fri Jul 13 2018 Fedora Release Engineering <releng@fedoraproject.org> - 5.13.3-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_29_Mass_Rebuild

* Wed Jul 11 2018 Martin Kyral <martin.kyral@gmail.com> - 5.13.3-1
- 5.13.3

* Mon Jul 09 2018 Martin Kyral <martin.kyral@gmail.com> - 5.13.2-1
- 5.13.2

* Tue Jun 19 2018 Martin Kyral <martin.kyral@gmail.com> - 5.13.1-1
- 5.13.1

* Sat Jun 09 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.13.0-1
- 5.13.0

* Fri May 18 2018 Martin Kyral <martin.kyral@gmail.com> - 5.12.90-1
- 5.12.90

* Tue May 01 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.12.5-1
- 5.12.5

* Tue Mar 27 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.12.4-1
- 5.12.4

* Tue Mar 06 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.12.3-1
- 5.12.3

* Wed Feb 21 2018 Jan Grulich <jgrulich@redhat.com> - 5.12.2-1
- 5.12.2

* Tue Feb 13 2018 Jan Grulich <jgrulich@redhat.com> - 5.12.1-1
- 5.12.1

* Fri Feb 09 2018 Fedora Release Engineering <releng@fedoraproject.org> - 5.12.0-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_28_Mass_Rebuild

* Fri Feb 02 2018 Jan Grulich <jgrulich@redhat.com> - 5.12.0-1
- 5.12.0

* Mon Jan 15 2018 Jan Grulich <jgrulich@redhat.com> - 5.11.95-1
- 5.11.95

* Tue Jan 02 2018 Rex Dieter <rdieter@fedoraproject.org> - 5.11.5-1
- 5.11.5

* Thu Nov 30 2017 Martin Kyral <martin.kyral@gmail.com> - 5.11.4-1
- 5.11.4

* Wed Nov 22 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.11.3-1
- 5.11.3

* Wed Oct 25 2017 Martin Kyral <martin.kyral@gmail.com> - 5.11.2-1
- 5.11.2

* Tue Oct 17 2017 Rex Dieter <rdieter@fedoraproject.org> - 5.11.1-1
- 5.11.1

* Wed Oct 11 2017 Martin Kyral <martin.kyral@gmail.com> - 5.11.0-1
- 5.11.0

* Wed Oct 11 2017 Martin Kyral <martin.kyral@gmail.com> - 5.11.0-0.1
- 5.11.0

* Thu Sep 14 2017 Martin Kyral <martin.kyral@gmail.com> - 5.10.95-1
- 5.10.95
(split from plasma-workspace)

