# defined empty to enable automatic uid/gid selection.
# set values to select specific user/group ids.
%define omniuid -1
%define omnigid -1

%define version_major 4
%define version_minor 1
%define version_micro 5
%define version_full %{version_major}.%{version_minor}.%{version_micro}
%define version_brief %{version_major}.%{version_minor}

%define lib_name %{?mklibname:%mklibname %{name} %{version_brief}}%{!?mklibname:lib%{name}%{version_brief}}

%{!?py_ver: %define py_ver %(python -c "import sys;print(sys.version[0:3])")}
%{!?python_sitearch: %define python_sitearch %(python -c "from distutils.sysconfig import get_python_lib; print get_python_lib(True,0,'%{_prefix}')")}
%{!?python_sitelib: %define python_sitelib %(python -c "from distutils.sysconfig import get_python_lib; print get_python_lib(False,0,'%{_prefix}')")}

Summary: Object Request Broker (ORB)
Name:    omniORB
Version: %{version_full}
Release: 1%{?dist}
License: GPL / LGPL
Group:   System/Libraries
Source0: %{name}-%{version}.tar.gz
Prefix:  /usr
URL:     http://omniorb.sourceforge.net/
BuildRequires:  gcc-c++
BuildRequires:  glibc-devel openssl-devel
BuildRequires:  python python-devel
BuildRequires:	pkgconfig
%if "%{_vendor}" == "suse"
BuildRequires:  klogd rsyslog
%endif
Buildroot:      %{_tmppath}/%{name}-%{version}-root

%description
%{name} is an Object Request Broker (ORB) which implements
specification 2.6 of the Common Object Request Broker Architecture
(CORBA).
This package contains the libraries needed to run programs dynamically
linked with %{name}.

%package -n %{lib_name}
Summary: Object Request Broker (ORB)
Group:   System/Libraries
Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig
#Provides:  corba
Provides:  lib%{name} = %{version}-%{release} %{name} = %{version}-%{release}
Provides:  libomniorb = %{version}-%{release} omniorb = %{version}-%{release}

%description -n %{lib_name}
%{name} is an Object Request Broker (ORB) which implements
specification 2.6 of the Common Object Request Broker Architecture
(CORBA).
This package contains the libraries needed to run programs dynamically
linked with %{name}.

# servers

%package servers
Summary: Utility programs
Group:          Development/C++
Requires(pre):  /usr/sbin/groupadd /usr/sbin/useradd
Requires(postun): /usr/sbin/groupdel /usr/sbin/userdel
Requires:       %{lib_name} = %{version}-%{release}
Provides:       omniorb-servers = %{version}-%{release}
Provides:       libomniorb-servers = %{version}-%{release}

%description servers
%{name} CORBA services including a Naming Service.

%package bootscripts
Summary: Utility programs
Group: Development/C++
%if "%{_vendor}" == "suse"
Requires(post): /sbin/insserv
Requires(preun): /sbin/insserv
%else
Requires(pre):  /sbin/service
Requires(post): /sbin/service /sbin/chkconfig
Requires(preun): /sbin/service /sbin/chkconfig
%endif
Requires: %{name}-servers = %{version}-%{release} %{name}-utils = %{version}-%{release}
Requires(pre): syslog
Provides: omniorb-bootscripts = %{version}-%{release}

%description bootscripts
Automatic starting of the %{name} CORBA Naming Service.

# utilities

%package utils
Summary: Utility programs
Group:          Development/C++
Requires:       %{lib_name} = %{version}-%{release}
Provides:       omniorb-utils = %{version}-%{release}

%description utils
%{name} utility programs which may be useful at runtime.

# devel part of the bundle

%package devel
Summary: Header files and libraries needed for %{name} development
Group:     Development/C++
Requires:  %{lib_name} = %{version}-%{release}
Provides:  libomniorb-devel = %{version}-%{release} omniorb-devel = %{version}-%{release}
Conflicts: %{name}-devel < %{version}-%{release}
Obsoletes: libomniORB-devel

%description devel
The header files and libraries needed for developing programs using
%{name}.

# docs and examples are in a separate package

%package doc
Summary: Documentation and examples for %{name}
Group:           Development/C++
Provides:        libomniorb-doc = %{version}-%{release} omniorb-doc = %{version}-%{release}
Obsoletes:       libomniORB-doc

%description doc
Developer documentation and examples.

%prep
%setup -n %{name} #-%{version}

%if "%{_vendor}" == "suse"
# Replace the init script with something appropriate for SUSE.
# Note that we hardcode a relative path here, since we are replacing
# a file in the source distribution tree.
cp -f etc/init.d/omniNames.SuSE.in etc/init.d/omniNames.in
%endif

%{?configure:%configure}%{!?configure:./configure --prefix=%{_prefix} --libdir=%{_libdir}} --with-openssl=%{_prefix} --with-omniNames-logdir=%{_localstatedir}/lib/omniNames


%build
# We abuse the CPPFLAGS to pass optimisation options through.
%{?make:%make}%{!?make:make IMPORT_CPPFLAGS+="$RPM_OPT_FLAGS"} all


%install
[ -z %{buildroot} ] || rm -rf %{buildroot}
mkdir %{buildroot}

%if "%{_vendor}" == "suse"
%{?makeinstall:%makeinstall}%{!?makeinstall:make DESTDIR=%{buildroot} install}
%else
%{?make:%make}%{!?make:make} install DESTDIR=%{buildroot}
%endif

mkdir -p %{buildroot}%{_initrddir}
mkdir -p %{buildroot}%{_sysconfdir}
cp sample.cfg %{buildroot}%{_sysconfdir}/omniORB.cfg
cp etc/init.d/omniNames %{buildroot}%{_initrddir}

mkdir -p %{buildroot}%{_mandir}/man{1,5,8}
cp -r man/* %{buildroot}%{_mandir}

mkdir -p %{buildroot}%{_localstatedir}/lib/omniNames
mkdir -p %{buildroot}%{_localstatedir}/lib/omniMapper

# Rename catior to avoid naming conflict with TAO
mv %{buildroot}%{_bindir}/catior %{buildroot}%{_bindir}/catior.omni
mv %{buildroot}%{_mandir}/man1/catior.1 %{buildroot}%{_mandir}/man1/catior.omni.1

%if "%{_vendor}" == "suse"
  # Most SUSE service scripts have a corresponding link into /usr/sbin
  mkdir -p %{buildroot}%{_sbindir}
  ln -sf %{_initrddir}/omniNames %{buildroot}%{_sbindir}/rcomniNames
%endif

%{?multiarch_includes:%multiarch_includes %{buildroot}%{_includedir}/omniORB4/acconfig.h}
%{?multiarch_includes:%multiarch_includes %{buildroot}%{_includedir}/omniORB4/CORBA_sysdep_trad.h}


%clean
[ -z %{buildroot} ] || rm -rf %{buildroot}


%pre -n %{lib_name}

%post -n %{lib_name}
/sbin/ldconfig

%pre servers
%if "%{omnigid}" == "-1"
OMNIGIDOPT="-r"
%else
OMNIGIDOPT="-g %{omnigid}"
%endif
%if "%{omniuid}" == "-1"
OMNIUIDOPT="-r"
%else
OMNIUIDOPT="-u %{omniuid}"
%endif
/usr/sbin/groupadd ${OMNIGIDOPT} omni >/dev/null 2>&1 || :
/usr/sbin/useradd ${OMNIUIDOPT} -M -g omni -d %{_localstatedir}/lib/omniNames \
  -s /bin/bash -c "omniORB servers" omni >/dev/null 2>&1 || :

%pre bootscripts
# A previous version is already installed?
if [ $1 -ge 2 ]; then
%if "%{_vendor}" == "suse"
  %{_sbindir}/rcomniNames stop >/dev/null 2>&1
%else
  /sbin/service omniNames stop >/dev/null 2>&1
%endif
fi

%post bootscripts
%if "%{_vendor}" == "suse"
/sbin/insserv omniNames
%{_sbindir}/rcomniNames restart >/dev/null 2>&1
%else
/sbin/chkconfig --add omniNames
/sbin/service omniNames restart >/dev/null 2>&1
%endif

%preun bootscripts
# Are we removing the package completely?
if [ $1 -eq 0 ]; then
%if "%{_vendor}" == "suse"
  %{_sbindir}/rcomniNames stop >/dev/null 2>&1
  /sbin/insserv -r omniNames
%else
  /sbin/service omniNames stop >/dev/null 2>&1
  /sbin/chkconfig --del omniNames
%endif
  rm -rf %{_localstatedir}/lib/omniNames/*
  rm -rf %{_localstatedir}/lib/omniMapper/*
fi

%postun -n %{lib_name}
/sbin/ldconfig

%postun servers
# uninstalling all versions?
if [ $1 -eq 0 ] ; then
  /usr/sbin/userdel omni >/dev/null 2>&1 || :
  /usr/sbin/groupdel omni >/dev/null 2>&1 || : 
fi


# main package includes libraries and copyright info
%files -n %{lib_name}
%defattr (-,root,root)
%doc CREDITS COPYING COPYING.LIB
%config(noreplace) %{_sysconfdir}/*.cfg
%{_libdir}/*.so.*


%files servers
%defattr (-,root,root)
%attr(644,root,man) %{_mandir}/man8/omniNames*
%attr(644,root,man) %{_mandir}/man8/omniMapper*
%dir %attr(700,omni,omni) %{_localstatedir}/lib/omniNames
%dir %attr(700,omni,omni) %{_localstatedir}/lib/omniMapper
%attr(755,root,root) %{_bindir}/omniMapper
%attr(755,root,root) %{_bindir}/omniNames


%files bootscripts
%defattr (-,root,root)
%config(noreplace) %attr(775,root,root) %{_initrddir}/*
%if "%{_vendor}" == "suse"
%{_sbindir}/rcomniNames
%endif


%files utils
%defattr (-,root,root)
%attr(644,root,man) %{_mandir}/man1/catior*
%attr(644,root,man) %{_mandir}/man1/convertior*
%attr(644,root,man) %{_mandir}/man1/genior*
%attr(644,root,man) %{_mandir}/man1/nameclt*
%{_bindir}/catior.omni
%{_bindir}/convertior
%{_bindir}/genior
%{_bindir}/nameclt


%files devel
%defattr(-,root,root)
%doc ReleaseNotes* readmes/*
%attr(644,root,man) %{_mandir}/man1/omnicpp*
%attr(644,root,man) %{_mandir}/man1/omniidl*
%{_bindir}/omnicpp
%{_bindir}/omniidl
%{_bindir}/omniidlrun.py*
%{_bindir}/omkdepend
%{_libdir}/*.a
%{_libdir}/*.so
%{_includedir}/*
%{_datadir}/idl
%{_libdir}/pkgconfig/*.pc
%{python_sitelib}/omniidl
%dir %{python_sitelib}/omniidl_be
%{python_sitelib}/omniidl_be/*.py*
%dir %{python_sitelib}/omniidl_be/cxx
%{python_sitelib}/omniidl_be/cxx/*.py*
%{python_sitelib}/omniidl_be/cxx/header
%{python_sitelib}/omniidl_be/cxx/skel
%{python_sitelib}/omniidl_be/cxx/dynskel
%{python_sitelib}/omniidl_be/cxx/impl
%{python_sitearch}/_omniidlmodule.so*


%files doc
%defattr(-,root,root)
%doc doc/* 


%changelog
* Wed Nov 24 2010 Dirk O. Kaar <dok@dok-net.net> 4.2.0-2
- Merge in improvements for current RPM from Thomas Lockhart

* Mon Sep 07 2009 Dirk O. Kaar <dok@dok-net.net> 4.2.0-1
- Use 4.2 sources from SVN

* Thu May 05 2005 Dirk Siebnich <dok@dok-net.net> 4.1.0-1
- rework package names to support x86_64, co-existance of 4.0
  and 4.1 libraries

* Mon Jul 26 2004 Duncan Grisby <duncan@grisby.org> 4.0.4-1
- Bump version number; integrate SUSE changes. Don't automatically
  start omniNames upon RPM install.

* Thu Jul 22 2004 Thomas Lockhart <lockhart@fourpalms.org> 4.0.3-7
- Incorporate additional SUSE features per Dirk O. Siebnich <dok@dok-net.net>
- Use additional standard RPM substitution parameters rather than
  hardcoded paths

* Wed Dec 24 2003 Thomas Lockhart <lockhart@fourpalms.org> 4.0.3
- Fix ownership of boot scripts per Bastiann Bakker
- Clean up pre- and post-install actions to support servers

* Tue Dec 08 2003 Thomas Lockhart <lockhart@fourpalms.org> 4.0.3
- Include additional build dependencies for redhat per Bastiann Bakker
- Put man pages for all distros into %{prefix}/share/man per FHS conventions
- Run omniNames under user "omni" per Jan Holst Jensen

* Mon Dec 01 2003 Thomas Lockhart <lockhart@fourpalms.org> 4.0.3
- Merge SuSE spec contributions from Johan Cronje

* Wed Nov 19 2003 Duncan Grisby <duncan@grisby.org> 4.0.3
- Merge contributed updates, bump version number.

* Fri Aug 08 2003 Thomas Lockhart <lockhart@fourpalms.org> 4.0.2
- Rename catior man page to match catior.omni binary name

* Wed Aug  6 2003 Duncan Grisby <dgrisby@apasphere.com> 4.0.2
- Bump version number.

* Tue Jun 10 2003 Duncan Grisby <dgrisby@apasphere.com> 4.0.2pre1
- Fix some text, bump version number, add init script, minor tweaks.

* Wed Feb 12 2003 Thomas Lockhart <lockhart@fourpalms.org> 4.0.0
- Rename catior to catior.omni to avoid name conflict with TAO

* Tue Oct 01 2002 Thomas Lockhart <lockhart@fourpalms.org> 4.0.0
- Track down changes in documentation for 4.0.0
- Omit patches required to build the previous beta

* Mon Jul 29 2002 Thomas Lockhart <lockhart@fourpalms.org> 4.0.0beta
- Separate out utility programs to manage name conflict for catior with TAO

* Wed Jul 03 2002 Thomas Lockhart <lockhart@fourpalms.org> 4.0.0beta
- Start from 3.04 spec files
- Strip workarounds from the spec file since 4.0 builds more cleanly

