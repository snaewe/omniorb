Summary:  Python Language Mapping for omniORB
Name:     omniORBpy
Version:  3.4
Release:  1
License:  GPL / LGPL
Group:    System/Libraries
Source0:  %{name}-%{version}.tar.gz
URL:      http://omniorb.sourceforge.net/
Prefix:   /usr
Prereq:   /sbin/ldconfig
Requires: omniORB = 4.1.4
BuildRequires: omniORB-devel python
Buildroot: %{_tmppath}/%{name}-%{version}-root

%description
omniORBpy is a Python language mapping for the omniORB CORBA
Object Request Broker (ORB).

# "standard" part of the bundle

%package -n %{name}-standard
Summary:  Files to provide standard top-level CORBA module for %{name}
Group:    Development/Python
Provides: libomniorbpy-standard = %{version}-%{release} %{name}-standard = %{version}-%{release}

%description -n %{name}-standard
The CORBA to Python mapping standard requires top-level CORBA and
PortableServer modules. This provides those standard modules for
%{name}. It will clash with similar packages for other Python ORBs.

# devel part of the bundle

%package -n %{name}-devel
Summary:  Header files and libraries needed for %{name} development
Group:    Development/Python
Requires: %{name} = %{version}-%{release} omniORB-devel
Provides: libomniorbpy-devel = %{version}-%{release} %{name}-devel = %{version}-%{release}

%description -n %{name}-devel
The header files and libraries needed for developing programs using %{name}.

# docs and examples are in a separate package

%package -n %{name}-doc
Summary:  Documentation needed for %{name} development
Group:    Development/Python

%description -n %{name}-doc
Developer documentation and examples.


%define py_ver %(python -c 'import sys;print(sys.version[0:3])')

%prep 

%setup -n %{name}-%{version}
#%patch0 -p1

# Needs to know where omniORB was installed if it is not in /usr.
# If necessary, use the configure option --with-omniorb=%{prefix}
./configure --prefix=%{prefix} --with-openssl=/usr

%build
# We abuse the CPPFLAGS to pass optimisation options through.
make IMPORT_CPPFLAGS+="$RPM_OPT_FLAGS" all

%install
make DESTDIR=$RPM_BUILD_ROOT install

# omit omniidl_be/__init__.py because it is a duplicate of the file
# already provided by omniORB.
rm -rf $RPM_BUILD_ROOT%{prefix}/lib*/python%{py_ver}/site-packages/omniidl_be/__init__.py*


%clean
rm -rf $RPM_BUILD_ROOT

%post
/sbin/ldconfig

%postun
/sbin/ldconfig


# main package includes libraries and servers
%files
%defattr (-,root,root)
%doc COPYING.LIB
#%doc bugfixes*
%prefix/lib*/python%{py_ver}/site-packages/_omni*.so*
%prefix/lib*/python%{py_ver}/site-packages/omniORB

%files -n %{name}-standard
%defattr(-,root,root)
%prefix/lib*/python%{py_ver}/site-packages/*.py*
%prefix/lib*/python%{py_ver}/site-packages/omniORB.pth
%prefix/lib*/python%{py_ver}/site-packages/CosNaming
%prefix/lib*/python%{py_ver}/site-packages/CosNaming__POA

%files -n %{name}-devel
%defattr(-,root,root)
%doc README* ReleaseNotes* update.log
%prefix/include/omniORBpy.h
%prefix/include/omniORB4/pydistdate.hh
%prefix/lib*/python%{py_ver}/site-packages/omniidl_be/python.py*

%files -n %{name}-doc
%defattr(-,root,root)
%doc doc/* 

%changelog
* Fri Dec 16 2005 Thomas Lockhart <lockhart@fourpalms.org> 2.6-3
- Modified postun syntax to eliminate an error from ldconfig

* Thu Apr 21 2005 Sander Steffann <steffann@nederland.net> 2.6-2
- Fixed packaging on RHEL and x86_64

* Wed Apr 20 2005 Sander Steffann <steffann@nederland.net> 2.6-1
- Upgrade to version 2.6

* Mon Jul 26 2004 Duncan Grisby <duncan@grisby.org> 2.4
- Bump version number to 2.4.

* Wed Nov 19 2003 Duncan Grisby <duncan@grisby.org> 2.3
- Merge contributed updates, bump version number.

* Fri Aug 08 2003 Thomas Lockhart <lockhart@fourpalms.org>
- Drop circular definition of RPM macros

* Wed Aug  6 2003 Duncan Grisby <dgrisby@apasphere.com> 2.2
- Remove clashing omniidl __init__.py. Bump version number.

* Tue Jun 10 2003 Duncan Grisby <dgrisby@apasphere.com> 2.2pre1
- Fix some text, bump version number, minor tweaks.

* Wed Apr 23 2003 Sander Steffann <sander@steffann.nl>
- Included SSL support
- Included pydistdate.hh to stop RPM from complaining

* Mon Mar 17 2003 Thomas Lockhart <lockhart@fourpalms.org> 4.0.1
- Update for current release
- Include new omniORB.pth

* Mon Jul 29 2002 Thomas Lockhart <lockhart@fourpalms.org> 4.0.0beta
- Include backend to omniidl to allow generation of Python stubs

* Fri May 24 2002 Thomas Lockhart <lockhart@fourpalms.org> 4.0.0beta
- Use autoconf as available in this new version of omniORBpy
- Assume a separate omniORB RPM has been built and installed
- Based on the single RPM for omniORB and omniORBpy from the 3.x series
