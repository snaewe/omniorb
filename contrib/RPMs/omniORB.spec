Summary: Object Request Broker (ORB)
Name:    omniORB
Version: 4.0.3
Release: 1
License: GPL / LGPL
Group:   System/Libraries
Source0: %{name}-%{version}.tar.gz
# omniORB.cfg is a (possibly modified) version of sample.cfg in the omniORB distro.
Prefix: /usr
Prereq: /sbin/chkconfig /sbin/ldconfig
URL:            http://omniorb.sourceforge.net/
#Provides:       corba
BuildRequires:  python glibc-devel openssl
Buildroot:      %{_tmppath}/%{name}-%{version}-root
#BuildArch:      i586

%description
%{name} is an Object Request Broker (ORB) which implements
specification 2.6 of the Common Object Request Broker Architecture
(CORBA). Contains the libraries needed to run programs dynamically
linked with %{name}.

# servers

%package -n %{name}-servers
Summary: Utility programs
Group:          Development/C++
Requires:       %{name} = %{version}-%{release}
Provides:       libomniorb-servers = %{version}-%{release} %{name}-servers = %{version}-%{release}

%description -n %{name}-servers
%{name} CORBA services including a Naming Service.

%package -n %{name}-bootscripts
Summary: Utility programs
Group: Development/C++
Requires: %{name}-servers = %{version}-%{release} %{name}-utils = %{version}-%{release}
Provides: %{name}-bootscripts = %{version}-%{release}

%description -n %{name}-bootscripts
Automatic starting of the %{name} CORBA Naming Service.

# utilities

%package -n %{name}-utils
Summary: Utility programs
Group:          Development/C++
Requires:       %{name} = %{version}-%{release}
Provides:       libomniorb-utils = %{version}-%{release} %{name}-utils = %{version}-%{release}

%description -n %{name}-utils
%{name} utility programs which may be useful at runtime.

# devel part of the bundle

%package -n %{name}-devel
Summary: Header files and libraries needed for %{name} development
Group:          Development/C++
Requires:       %{name} = %{version}-%{release}
Provides:       libomniorb-devel = %{version}-%{release} %{name}-devel = %{version}-%{release}

%description -n %{name}-devel
The header files and libraries needed for developing programs using
%{name}.

# docs and examples are in a separate package

%package -n %{name}-doc
Summary: Documentation and examples for %{name}
Group:          Development/C++
#Requires:       %{name} = %{version}

%description -n %{name}-doc
Developer documentation and examples.


%define py_ver        %(python -c 'import sys;print(sys.version[0:3])')

%prep 

%setup -n %{name}-%{version}
#%patch0 -p1

./configure --prefix=%{prefix} --with-openssl=/usr


%build
# We abuse the CPPFLAGS to pass optimisation options through.
make IMPORT_CPPFLAGS+="$RPM_OPT_FLAGS" all


%install
make DESTDIR=$RPM_BUILD_ROOT install

mkdir -p $RPM_BUILD_ROOT/etc/rc.d/init.d
cp sample.cfg $RPM_BUILD_ROOT/etc/omniORB.cfg
cp etc/init.d/omniNames $RPM_BUILD_ROOT/etc/rc.d/init.d/

mkdir -p $RPM_BUILD_ROOT/%{prefix}/man/man{1,5}
cp -r man/* $RPM_BUILD_ROOT/%{prefix}/man

mkdir -p $RPM_BUILD_ROOT/var/log/omniNames
mkdir -p $RPM_BUILD_ROOT/var/lib/omniMapper

# Rename catior to avoid naming conflict with TAO
mv $RPM_BUILD_ROOT/%{prefix}/bin/catior $RPM_BUILD_ROOT/%{prefix}/bin/catior.omni
mv $RPM_BUILD_ROOT/%prefix/man/man1/catior.1 $RPM_BUILD_ROOT/%prefix/man/man1/catior.omni.1


%clean
[ -z $RPM_BUILD_ROOT ] || rm -rf $RPM_BUILD_ROOT

%pre
%post -n %{name} -p /sbin/ldconfig
%postun -n %{name} -p /sbin/ldconfig

%post bootscripts
/sbin/chkconfig --add omniNames

%preun bootscripts
/sbin/chkconfig --del omniNames
rm -rf /var/log/omniNames/*
rm -rf /var/lib/omniMapper/*


# main package includes libraries and copyright info
%files
%defattr (-,root,root)
%doc CREDITS COPYING COPYING.LIB
%config(noreplace) %_sysconfdir/*.cfg
%prefix/lib/*.so.*
%prefix/share/idl


%files -n %{name}-servers
%defattr (-,root,root)
%attr(644,root,man) %prefix/man/man1/omniNames*
#%attr(644,root,man) %prefix/man/man1/omniMapper*
%prefix/bin/omniMapper
%prefix/bin/omniNames
# Thin substitute for standard Linux init script

%files -n %{name}-bootscripts
%defattr (-,root,root)
%config(noreplace) %_sysconfdir/rc.d/init.d/*
%dir %attr(754,root,root) /var/log/omniNames
%dir %attr(754,root,root) /var/lib/omniMapper


%files -n %{name}-utils
%defattr (-,root,root)
%attr(644,root,man) %prefix/man/man1/catior*
%attr(644,root,man) %prefix/man/man1/genior*
%attr(644,root,man) %prefix/man/man1/nameclt*
%prefix/bin/catior.omni
%prefix/bin/convertior
%prefix/bin/genior
%prefix/bin/nameclt


%files -n %{name}-devel
%defattr(-,root,root)
%doc ReleaseNotes* readmes/*
%attr(644,root,man) %prefix/man/man1/omniidl*
%prefix/bin/omnicpp
%prefix/bin/omniidl
%prefix/bin/omniidlrun.py
%prefix/bin/omkdepend
%prefix/lib/*.a
%prefix/lib/*.so
%prefix/include/*
%prefix/lib/python%{py_ver}/site-packages/omniidl/*
%prefix/lib/python%{py_ver}/site-packages/omniidl_be/*.py*
%prefix/lib/python%{py_ver}/site-packages/omniidl_be/cxx/*.py*
%prefix/lib/python%{py_ver}/site-packages/omniidl_be/cxx/header/*
%prefix/lib/python%{py_ver}/site-packages/omniidl_be/cxx/skel/*
%prefix/lib/python%{py_ver}/site-packages/omniidl_be/cxx/dynskel/*
%prefix/lib/python%{py_ver}/site-packages/omniidl_be/cxx/impl/*
%prefix/lib/python%{py_ver}/site-packages/_omniidlmodule.so*
%prefix/lib/pkgconfig/*.pc

%files -n %{name}-doc
%defattr(-,root,root)
%doc doc/* 


%changelog
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
