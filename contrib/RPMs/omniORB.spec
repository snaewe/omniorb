%define version       4.0.2pre1
%define release       1
%define name          omniORB
%define lib_major     4
%define lib_name      lib%{name}%{lib_major}
%define py_ver        %(python -c 'import sys;print(sys.version[0:3])')
%define prefix        /usr

Summary: Object Request Broker (ORB)
Name:           %{name}
Version:        %{version}
Release:        %{release}
License:        GPL / LGPL
Group:          System/Libraries
Source0:        omniORB-%{version}.tar.gz
# omniORB.cfg is a (possibly modified) version of sample.cfg in the omniORB distro.
#Source1:        omniORB.cfg
#Patch0:         omniORB.patches
URL:            http://omniorb.sourceforge.net/
#Provides:       corba
BuildRequires:  python >= 2.0 glibc-devel openssl
Buildroot:      %{_tmppath}/%{name}-%{version}-root
#BuildArch:      i586

%description
omniORB is an Object Request Broker (ORB) which implements
specification 2.6 of the Common Object Request Broker Architecture
(CORBA).  This package contains the libraries needed to run programs
dynamically linked with %{name}.

# servers

%package -n %{name}-servers
Summary: Utility programs
Group:          Development/C++
Requires:       %{name} = %{version}-%{release}
Provides:       libomniorb-servers = %{version}-%{release} %{name}-servers = %{version}-%{release}
#BuildArch:      i586

%description -n %{name}-servers
omniORB CORBA services including a Naming Service.

# utilities

%package -n %{name}-utils
Summary: Utility programs
Group:          Development/C++
Requires:       %{name} = %{version}-%{release}
Provides:       libomniorb-utils = %{version}-%{release} %{name}-utils = %{version}-%{release}
#BuildArch:      i586

%description -n %{name}-utils
This package includes utility programs which may be useful at runtime.

# devel part of the bundle

%package -n %{name}-devel
Summary: Header files and libraries needed for %{name} development
Group:          Development/C++
Requires:       %{name} = %{version}-%{release}
Provides:       libomniorb-devel = %{version}-%{release} %{name}-devel = %{version}-%{release}
#BuildArch:      i586

%description -n %{name}-devel
This package includes the header files and libraries needed for
developing programs using %{name}.

# docs and examples are in a separate package

%package -n %{name}-doc
Summary: Documentation and examples for %{name}
Group:          Development/C++
#Requires:       %{name} = %{version}
#BuildArch:      noarch

%description -n %{name}-doc
This package includes developer documentation including examples.


%prep 

%setup -n omniORB-%{version}
#%patch0 -p1

./configure --prefix=%{prefix} --with-openssl=/usr


%build
make CCFLAGS+="$RPM_OPT_FLAGS" all


%install
make DESTDIR=$RPM_BUILD_ROOT install

mv $RPM_BUILD_ROOT/%{prefix}/bin/catior $RPM_BUILD_ROOT/%{prefix}/bin/catior.omni

mkdir -p $RPM_BUILD_ROOT/etc/rc.d/init.d
cp etc/init.d/omninames $RPM_BUILD_ROOT/etc/rc.d/init.d/omninames
chmod +x $RPM_BUILD_ROOT/etc/rc.d/init.d/omninames
#cp %{SOURCE1} $RPM_BUILD_ROOT/etc/

mkdir -p $RPM_BUILD_ROOT/var/omninames

mkdir -p $RPM_BUILD_ROOT/%{prefix}/man/man1
cp -r man/* $RPM_BUILD_ROOT/%{prefix}/man

cp sample.cfg $RPM_BUILD_ROOT/etc/omniORB.cfg

%clean
[ -z $RPM_BUILD_ROOT ] || rm -rf $RPM_BUILD_ROOT

%post -n %{name} -p /sbin/ldconfig

%postun -n %{name} -p /sbin/ldconfig

# main package includes libraries and servers
%files
%defattr (-,root,root)
%doc CREDITS COPYING COPYING.LIB
%config(noreplace) %_sysconfdir/*.cfg
%prefix/lib/*.so.*
%prefix/share/idl


%files -n %{name}-servers
%defattr (-,root,root)
%dir %attr(754,root,root) /var/omninames
%config(noreplace) %_sysconfdir/rc.d/init.d/*
%attr(644,root,man) %prefix/man/man1/omniNames*
#%attr(644,root,man) %prefix/man/man1/omniMapper*
%prefix/bin/omniMapper
%prefix/bin/omniNames
%prefix/bin/omniNames-daemon


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


%files -n %{name}-doc
%defattr(-,root,root)
%doc doc/* 
%doc src/examples


%changelog
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
