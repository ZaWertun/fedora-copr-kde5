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
Version:        5.114.0
Release:        1%{?dist}

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
BuildRequires:  pkgconfig(libavcodec)
BuildRequires:  pkgconfig(libavformat)
BuildRequires:  pkgconfig(libavutil)
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
* Fri Jan 19 2024 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.114.0-1
- 5.114.0

* Fri Dec 15 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.113.0-1
- 5.113.0

* Sun Nov 12 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.112.0-1
- 5.112.0

* Thu Oct 19 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.111.0-1
- 5.111.0

* Sun Sep 10 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.110.0-1
- 5.110.0

* Fri Aug 18 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.109.0-1
- 5.109.0

* Sun Jul 09 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.108.0-1
- 5.108.0

* Sat Jun 10 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.107.0-1
- 5.107.0

* Sat May 13 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.106.0-1
- 5.106.0

* Sat Apr 08 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.105.0-1
- 5.105.0

* Mon Mar 13 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.104.0-2
- ffmpeg support enabled

* Sat Mar 11 2023 Yaroslav Sidlovsky <zawertun@gmail.com> - 5.104.0-1
- 5.104.0

