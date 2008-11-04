# defined empty to enable automatic uid/gid selection.
# set values to select specific user/group ids.
%define omniuid -1
%define omnigid -1

%define _name omniORB

%define lib_name %{?mklibname:%mklibname %{_name} 4.1}%{!?mklibname:lib%{_name}4.1}

%if "%{_vendor}" == "mandriva"
%define py_sitedir %{_prefix}/lib*/python%{py_ver}/site-packages
%endif
%if "%{_vendor}" == "redhat"
%define py_ver     %(python -c 'import sys;print(sys.version[0:3])')
%define py_sitedir %{_prefix}/lib*/python%{py_ver}/site-packages
%endif

Summary: Object Request Broker (ORB)
Name:    %{_name}
Version: 4.1.3
Release: 1
License: GPL / LGPL
Group:   System/Libraries
Source0: %{_name}-%{version}.tar.gz
Prefix:  /usr
URL:     http://omniorb.sourceforge.net/
BuildRequires:  gcc-c++
BuildRequires:  glibc-devel openssl-devel
BuildRequires:  python python-devel
Buildroot:      %{_tmppath}/%{name}-%{version}-root

%description
%{_name} is an Object Request Broker (ORB) which implements
specification 2.6 of the Common Object Request Broker Architecture
(CORBA).
This package contains the libraries needed to run programs dynamically
linked with %{_name}.

%package -n %{lib_name}
Summary: Object Request Broker (ORB)
Group:     System/Libraries
Prereq:    /sbin/ldconfig
#Provides:  corba
Provides:  libomniorb = %{version}-%{release} %{_name} = %{version}-%{release}
Obsoletes: omniORB

%description -n %{lib_name}
%{_name} is an Object Request Broker (ORB) which implements
specification 2.6 of the Common Object Request Broker Architecture
(CORBA).
This package contains the libraries needed to run programs dynamically
linked with %{_name}.

# servers

%package servers
Summary: Utility programs
Group:          Development/C++
Prereq:         /usr/sbin/groupadd /usr/sbin/groupdel
Prereq:         /usr/sbin/useradd /usr/sbin/userdel
Requires:       %{lib_name} = %{version}-%{release}
Provides:       libomniorb-servers = %{version}-%{release}
Obsoletes:      omniORB-servers omniorb

%description servers
%{_name} CORBA services including a Naming Service.

%package bootscripts
Summary: Utility programs
Group: Development/C++
%if "%{_vendor}" == "suse"
Prereq:         /sbin/insserv
%else
Prereq:         /sbin/service /sbin/chkconfig
%endif
Requires: %{name}-servers = %{version}-%{release} %{name}-utils = %{version}-%{release}
Provides: %{_name}-bootscripts = %{version}-%{release}
Obsoletes: omniORB-bootscripts omniorb

%description bootscripts
Automatic starting of the %{_name} CORBA Naming Service.

# utilities

%package utils
Summary: Utility programs
Group:          Development/C++
Requires:       %{lib_name} = %{version}-%{release}
Provides:       libomniorb-utils = %{version}-%{release}
Obsoletes:      omniORB-utils omniorb

%description utils
%{_name} utility programs which may be useful at runtime.

# devel part of the bundle

%package -n %{lib_name}-devel
Summary: Header files and libraries needed for %{_name} development
Group:          Development/C++
Requires:       %{lib_name} = %{version}-%{release}
Provides:       libomniorb-devel = %{version}-%{release}
Conflicts:      libomniorb-devel < %{version}-%{release}
Obsoletes:      omniORB-devel

%description -n %{lib_name}-devel
The header files and libraries needed for developing programs using
%{_name}.

# docs and examples are in a separate package

%package doc
Summary: Documentation and examples for %{_name}
Group:           Development/C++
Obsoletes:       omniORB-doc libomniorb-doc
Provides:        libomniorb-doc = %{version}-%{release}
#Requires:       %{lib_name} = %{version}-%{release}

%description doc
Developer documentation and examples.

%prep
%setup -n %{_name}-%{version}

%if "%{_vendor}" == "suse"
# Replace the init script with something appropriate for SUSE.
# Note that we hardcode a relative path here, since we are replacing
# a file in the source distribution tree.
cp -f etc/init.d/omniNames.SuSE.in etc/init.d/omniNames.in
%endif

%{?configure:%configure}%{!?configure:./configure --prefix=%{_prefix} --libdir=%{_libdir}} --with-openssl=%{_prefix}


%build
# We abuse the CPPFLAGS to pass optimisation options through.
%{?make:%make}%{!?make:make IMPORT_CPPFLAGS+="$RPM_OPT_FLAGS"} all


%install
[ -z %{buildroot} ] || rm -rf %{buildroot}

%{?makeinstall_std:%makeinstall_std}%{!?makeinstall_std:make DESTDIR=%{buildroot} install}

mkdir -p %{buildroot}%{_initrddir}
mkdir -p %{buildroot}%{_sysconfdir}
cp sample.cfg %{buildroot}%{_sysconfdir}/omniORB.cfg
cp etc/init.d/omniNames %{buildroot}%{_initrddir}

mkdir -p %{buildroot}%{_mandir}/man{1,8}
cp -r man/* %{buildroot}%{_mandir}

mkdir -p %{buildroot}%{_var}/omniNames
mkdir -p %{buildroot}%{_localstatedir}/omniMapper

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
/usr/sbin/useradd ${OMNIUIDOPT} -M -g omni -d /var/omniNames \
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
#%{_sbindir}/rcomniNames restart >/dev/null 2>&1
%else
/sbin/chkconfig --add omniNames
#/sbin/service omniNames restart >/dev/null 2>&1
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
  rm -rf /var/omniNames/*
  rm -rf /var/lib/omniMapper/*
fi

%postun -n %{lib_name}
/sbin/ldconfig

%postun servers
# uninstalling all versions?
if [ $1 -eq 0 ] ; then
  /usr/sbin/userdel omni >/dev/null 2>&1 || :
  /usr/sbin/groupdel omni >/dev/null 2>&1 || : 
fi


%files -n %{lib_name}
%defattr (-,root,root)
%doc CREDITS COPYING COPYING.LIB
%config(noreplace) %{_sysconfdir}/*.cfg
%{_libdir}/*.so.*


%files servers
%defattr (-,root,root)
%dir %attr(700,omni,omni) %{_var}/omniNames
%dir %attr(700,omni,omni) %{_localstatedir}/omniMapper
%attr(644,root,man) %{_mandir}/man1/omniNames*
#%attr(644,root,man) %{_mandir}/man1/omniMapper*
%attr(755,root,root) %{_bindir}/omniMapper
%attr(755,root,root) %{_bindir}/omniNames
# Thin substitute for standard Linux init script


%files bootscripts
%defattr (-,root,root)
%config(noreplace) %attr(775,root,root) %{_initrddir}/*
%if "%{_vendor}" == "suse"
%{_sbindir}/rcomniNames
%endif


%files utils
%defattr (-,root,root)
%attr(644,root,man) %{_mandir}/man1/catior*
%attr(644,root,man) %{_mandir}/man1/genior*
%attr(644,root,man) %{_mandir}/man1/nameclt*
%{_bindir}/catior.omni
%{_bindir}/convertior
%{_bindir}/genior
%{_bindir}/nameclt


%files -n %{lib_name}-devel
%defattr(-,root,root)
%doc readmes/*
%attr(644,root,man) %{_mandir}/man1/omniidl*
%{_bindir}/omnicpp
%{_bindir}/omniidl
%{_bindir}/omniidlrun.py
%{_bindir}/omkdepend
%{_libdir}/*.a
%{_libdir}/*.so
%{_includedir}/*
%{_datadir}/idl/*
%{py_sitedir}/omniidl/*
%{py_sitedir}/omniidl_be/*.py*
%{py_sitedir}/omniidl_be/cxx/*.py*
%{py_sitedir}/omniidl_be/cxx/header/*
%{py_sitedir}/omniidl_be/cxx/skel/*
%{py_sitedir}/omniidl_be/cxx/dynskel/*
%{py_sitedir}/omniidl_be/cxx/impl/*
%{py_sitedir}/_omniidlmodule.so*
%{_libdir}/pkgconfig/*.pc


%files doc
%defattr(-,root,root)
%doc doc/* 


%changelog
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

