
## omit main pkg where kf5 konsole exists
%if 0%{?fedora} > 21
%global konsole5 1
%endif

Name:    konsole
Summary: KDE Terminal emulator
Version: 4.14.3
Release: 17%{?dist}

# sources: MIT and LGPLv2 and LGPLv2+ and GPLv2+
License: GPLv2 and GFDL
URL:     http://konsole.kde.org/
%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0: http://download.kde.org/%{stable}/%{version}/src/%{name}-%{version}.tar.xz

## upstreamable patches
# do not store history in /tmp
# http://bugzilla.redhat.com/990197
Patch1: konsole-4.14.1-history_cache_instead_of_tmp.patch
# TERM=xterm-256color default
# https://bugzilla.redhat.com/1172329
Patch2: konsole-4.14.3-xterm-256color.patch
Patch3: konsole-4.14.3-colorterm.patch
# xterm check for background color (echo -e "\033]11;?\a" && sleep 1)
Patch4: konsole-4.14.3-background-check.patch

BuildRequires: desktop-file-utils
BuildRequires: kdelibs4-devel >= 4.14
BuildRequires: pkgconfig(x11)
BuildRequires: pkgconfig(xrender)

Requires: %{name}-part = %{version}-%{release}

%description
%{summary}.

%package part
Summary: Konsole kpart plugin
# when konsole split occurred
Conflicts: kdebase < 6:4.6.95-10
# when -part split occurred
Conflicts: konsole < 4.7.3-2 
# in case we want this (vs. konsole5-part) -- rex
Provides: konsole4-part = %{version}-%{release}
Provides: konsole4-part%{?_isa} = %{version}-%{release}
%{?kdelibs4_requires}
%description part
%{summary}.


%prep
%setup -q

%patch1 -p1 -b .history_cache_instead_of_tmp
%patch2 -p1 -b .xterm-256color
%patch3 -p1 -b .colorterm
%patch4 -p1 -b .bg-color-check

%build
mkdir %{_target_platform}
pushd %{_target_platform}
%{cmake_kde4} ..
popd

make %{?_smp_mflags} -C %{_target_platform}


%install
make install/fast DESTDIR=%{buildroot} -C %{_target_platform}

%if 0%{?konsole5}
rm -fv %{buildroot}%{_kde4_bindir}/konsole*
rm -fv %{buildroot}%{_kde4_libdir}/libkdeinit4_konsole.so
rm -fv %{buildroot}%{_kde4_datadir}/applications/kde4/konsole.desktop
rm -fv %{buildroot}%{_kde4_datadir}/kde4/services/ServiceMenus/konsole*.desktop
rm -frv %{buildroot}%{_kde4_docdir}/HTML/en/konsole/
%else


%check
desktop-file-validate %{buildroot}%{_kde4_datadir}/applications/kde4/konsole.desktop

%files
%doc README
%{_kde4_bindir}/konsole
%{_kde4_bindir}/konsoleprofile
%{_kde4_datadir}/applications/kde4/konsole.desktop
%{_kde4_libdir}/libkdeinit4_konsole.so
%{_kde4_datadir}/kde4/services/ServiceMenus/konsolehere.desktop
%{_kde4_datadir}/kde4/services/ServiceMenus/konsolerun.desktop
%{_kde4_docdir}/HTML/en/konsole/
%endif

%files part
%doc COPYING*
%{_kde4_appsdir}/kconf_update/konsole*
%{_kde4_appsdir}/konsole/
%{_kde4_libdir}/libkonsoleprivate.so
%{_kde4_libdir}/kde4/libkonsolepart.so
%{_kde4_datadir}/kde4/services/konsolepart.desktop
%{_kde4_datadir}/kde4/servicetypes/terminalemulator.desktop


%changelog
* Fri Jul 13 2018 Fedora Release Engineering <releng@fedoraproject.org> - 4.14.3-17
- Rebuilt for https://fedoraproject.org/wiki/Fedora_29_Mass_Rebuild

* Wed Feb 07 2018 Fedora Release Engineering <releng@fedoraproject.org> - 4.14.3-16
- Rebuilt for https://fedoraproject.org/wiki/Fedora_28_Mass_Rebuild

* Thu Aug 03 2017 Fedora Release Engineering <releng@fedoraproject.org> - 4.14.3-15
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Binutils_Mass_Rebuild

* Wed Jul 26 2017 Fedora Release Engineering <releng@fedoraproject.org> - 4.14.3-14
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Mass_Rebuild

* Thu Feb 23 2017 Than Ngo <than@redhat.com> - 4.14.3-13
- fixed bz#1423820 - konsole: FTBFS in rawhide 

* Fri Feb 10 2017 Fedora Release Engineering <releng@fedoraproject.org> - 4.14.3-12
- Rebuilt for https://fedoraproject.org/wiki/Fedora_26_Mass_Rebuild

* Thu Feb 04 2016 Fedora Release Engineering <releng@fedoraproject.org> - 4.14.3-11
- Rebuilt for https://fedoraproject.org/wiki/Fedora_24_Mass_Rebuild

* Sun Sep 20 2015 Pavel Raiskup <praiskup@redhat.com> - 4.14.3-10
- KDE bug 352945, rhbz#1264682

* Wed Jun 24 2015 Rex Dieter <rdieter@fedoraproject.org> 4.14.3-9
- really omit main pkg on f22+

* Wed Jun 17 2015 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 4.14.3-8
- Rebuilt for https://fedoraproject.org/wiki/Fedora_23_Mass_Rebuild

* Fri May 15 2015 Rex Dieter <rdieter@fedoraproject.org> - 4.14.3-7
- drop main pkg (f22+), move kconf_update bits to -part
- -part: Provides: konsole4-part%%{?_isa}

* Sat May 02 2015 Kalev Lember <kalevlember@gmail.com> - 4.14.3-6
- Rebuilt for GCC 5 C++11 ABI change

* Thu Feb 26 2015 Than Ngo <than@redhat.com> 4.14.3-5
- add COLORTERM

* Sat Jan 31 2015 Rex Dieter <rdieter@fedoraproject.org> 4.14.3-4
- apply xterm-256color patch for real (#1172329)

* Sat Jan 17 2015 Rex Dieter <rdieter@fedoraproject.org> - 4.14.3-3
- kde-applications fixes
- -part: Provides: konsole4-part

* Tue Dec 23 2014 Rex Dieter <rdieter@fedoraproject.org> 4.14.3-2
- TERM=xterm-256color default (#1172329)

* Sun Nov 09 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.14.3-1
- 4.14.3

* Sun Oct 12 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.14.2-1
- 4.14.2

* Tue Sep 23 2014 Rex Dieter <rdieter@fedoraproject.org> 4.14.1-2
- do not store history in /tmp (#990197)

* Tue Sep 16 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.14.1-1
- 4.14.1

* Sun Aug 17 2014 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 4.14.0-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_21_22_Mass_Rebuild

* Fri Aug 15 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.14.0-1
- 4.14.0

* Tue Aug 05 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.13.97-1
- 4.13.97

* Tue Jul 15 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.13.3-1
- 4.13.3

* Mon Jun 09 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.13.2-1
- 4.13.2

* Sun Jun 08 2014 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 4.13.1-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_21_Mass_Rebuild

* Sun May 11 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.13.1-1
- 4.13.1

* Sat Apr 12 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.13.0-1
- 4.13.0

* Fri Apr 04 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.12.97-1
- 4.12.97

* Sun Mar 23 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.12.95-1
- 4.12.95

* Wed Mar 19 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.12.90-1
- 4.12.90

* Sun Mar 02 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.12.3-1
- 4.12.3

* Fri Jan 31 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.12.2-1
- 4.12.2

* Fri Jan 10 2014 Rex Dieter <rdieter@fedoraproject.org> - 4.12.1-1
- 4.12.1

* Thu Dec 19 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.12.0-1
- 4.12.0

* Sun Dec 01 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.11.97-1
- 4.11.97

* Thu Nov 21 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.11.95-1
- 4.11.95

* Sat Nov 16 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.11.90-1
- 4.11.90

* Sat Nov 02 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.11.3-1
- 4.11.3

* Sat Sep 28 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.11.2-1
- 4.11.2

* Wed Sep 04 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.11.1-1
- 4.11.1

* Thu Aug 08 2013 Than Ngo <than@redhat.com> - 4.11.0-1
- 4.11.0

* Thu Jul 25 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.10.97-1
- 4.10.97

* Tue Jul 23 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.10.95-1
- 4.10.95

* Thu Jun 27 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.10.90-1
- 4.10.90

* Sat Jun 01 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.10.4-1
- 4.10.4

* Mon May 06 2013 Than Ngo <than@redhat.com> - 4.10.3-1
- 4.10.3

* Sun Mar 31 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.10.2-1
- 4.10.2

* Sat Mar 02 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.10.1-1
- 4.10.1

* Fri Feb 01 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.10.0-1
- 4.10.0

* Sun Jan 20 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.9.98-1
- 4.9.98

* Fri Jan 04 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.9.97-1
- 4.9.97

* Thu Dec 20 2012 Rex Dieter <rdieter@fedoraproject.org> - 4.9.95-1
- 4.9.95

* Tue Dec 04 2012 Rex Dieter <rdieter@fedoraproject.org> - 4.9.90-1
- 4.9.90

* Mon Dec 03 2012 Than Ngo <than@redhat.com> - 4.9.4-1
- 4.9.4

* Fri Nov 23 2012 Than Ngo <than@redhat.com> - 4.9.3-2
- fix license issue

* Sat Nov 03 2012 Rex Dieter <rdieter@fedoraproject.org> - 4.9.3-1
- 4.9.3

* Thu Oct 04 2012 Rex Dieter <rdieter@fedoraproject.org> 4.9.2-2
- Failed to launch Konsole ... Failed to change $HOME and there is no such directory (#861504)

* Sat Sep 29 2012 Rex Dieter <rdieter@fedoraproject.org> - 4.9.2-1
- 4.9.2

* Mon Sep 24 2012 Rex Dieter <rdieter@fedoraproject.org> 4.9.1-2
- Konsole (or Terminal) start in /Documents folder (#841471)

* Mon Sep 03 2012 Than Ngo <than@redhat.com> - 4.9.1-1
- 4.9.1

* Thu Jul 26 2012 Lukas Tinkl <ltinkl@redhat.com> - 4.9.0-1
- 4.9.0

* Thu Jul 19 2012 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 4.8.97-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_18_Mass_Rebuild

* Wed Jul 11 2012 Rex Dieter <rdieter@fedoraproject.org> - 4.8.97-1
- 4.8.97

* Wed Jun 27 2012 Jaroslav Reznik <jreznik@redhat.com> - 4.8.95-1
- 4.8.95

* Sat Jun 09 2012 Rex Dieter <rdieter@fedoraproject.org> - 4.8.90-1
- 4.8.90

* Fri Jun 01 2012 Jaroslav Reznik <jreznik@redhat.com> 4.8.80-2
- respin

* Mon May 28 2012 Rex Dieter <rdieter@fedoraproject.org> 4.8.80-1
- 4.8.80

* Mon Apr 30 2012 Jaroslav Reznik <jreznik@redhat.com> - 4.8.3-1
- 4.8.3

* Mon Apr 02 2012 Than Ngo <than@redhat.com> - 4.8.2-4
- respin

* Mon Apr 02 2012 Than Ngo <than@redhat.com> - 4.8.2-3
- add better workaround for kde#280896

* Sun Apr 01 2012 Rex Dieter <rdieter@fedoraproject.org> 4.8.2-2
- konsole crashes running 'ssh -t' (kde#297156)

* Fri Mar 30 2012 Jaroslav Reznik <jreznik@redhat.com> - 4.8.2-1
- 4.8.2

* Mon Mar 05 2012 Radek Novacek <rnovacek@redhat.com> - 4.8.1-1
- 4.8.1

* Tue Feb 28 2012 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 4.8.0-3
- Rebuilt for c++ ABI breakage

* Tue Jan 31 2012 Rex Dieter <rdieter@fedoraproject.org> 4.8.0-2
- improved 'fonts get chopped' patch, fixes ascenders too (#742583,kde#280896)

* Fri Jan 20 2012 Jaroslav Reznik <jreznik@redhat.com> - 4.8.0-1
- 4.8.0

* Wed Jan 04 2012 Radek Novacek <rnovacek@redhat.com> - 4.7.97-1
- 4.7.97

* Tue Dec 27 2011 Rex Dieter <rdieter@fedoraproject.org> 4.7.95-2
- Some fonts get chopped (#742583,kde#280896)

* Wed Dec 21 2011 Radek Novacek <rnovacek@redhat.com> - 4.7.95-1
- 4.7.95

* Sun Dec 04 2011 Rex Dieter <rdieter@fedoraproject.org> - 4.7.90-1
- 4.7.90

* Fri Nov 25 2011 Radek Novacek <rnovacek@redhat.com> 4.7.80-1
- 4.7.80 (beta 1)
- add BR: kdebase-devel

* Tue Nov 15 2011 Rex Dieter <rdieter@fedoraproject.org> 4.7.3-2
- konsole-part subpkg

* Sat Oct 29 2011 Rex Dieter <rdieter@fedoraproject.org> 4.7.3-1
- 4.7.3

* Tue Oct 04 2011 Rex Dieter <rdieter@fedoraproject.org> 4.7.2-1
- 4.7.2

* Wed Sep 07 2011 Than Ngo <than@redhat.com> - 4.7.1-1
- 4.7.1

* Tue Jul 26 2011 Jaroslav Reznik <jreznik@redhat.com> 4.7.0-1
- 4.7.0

* Thu Jul 21 2011 Rex Dieter <rdieter@fedoraproject.org> 4.6.95-4
- fix Conflicts: kdebase (wrong epoch)

* Thu Jul 21 2011 Rex Dieter <rdieter@fedoraproject.org> 4.6.95-3
- License: GPLv2 (per FAQ)

* Thu Jul 21 2011 Rex Dieter <rdieter@fedoraproject.org> 4.6.95-2
- fix URL
- drop Requires: kdebase-runtime (overkill)
- License: +LGPLv2 

* Mon Jul 18 2011 Rex Dieter <rdieter@fedoraproject.org> 4.6.95-1
- first try
