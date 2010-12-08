%define version_major 3
%define version_minor 5
%define version_full %{version_major}.%{version_minor}
%define version_brief %{version_major}.%{version_minor}

%define lib_name %{?mklibname:%mklibname %{name} %{version_brief}}%{!?mklibname:lib%{name}%{version_brief}}

%{!?py_ver: %define py_ver %(python -c "import sys;print(sys.version[0:3])")}
%{!?python_sitearch: %define python_sitearch %(python -c "from distutils.sysconfig import get_python_lib; print get_python_lib(True,0,'%{_prefix}')")}
%{!?python_sitelib: %define python_sitelib %(python -c "from distutils.sysconfig import get_python_lib; print get_python_lib(False,0,'%{_prefix}')")}

Summary:   Python Language Mapping for omniORB
Name:      omniORBpy
Version:   %{version_full}
Release:   1%{?dist}
License:   GPL / LGPL
Group:     System/Libraries
Source0:   %{name}-%{version}.tar.gz
Prefix:    /usr
URL:       http://omniorb.sourceforge.net/
BuildRequires: gcc-c++
BuildRequires: glibc-devel openssl-devel
BuildRequires: omniORB-devel = %{version}
BuildRequires: python python-devel
Buildroot: %{_tmppath}/%{name}-%{version}-root

%description
%{name} is a Python language mapping for the omniORB CORBA
Object Request Broker (ORB).

%package -n %{lib_name}
Summary:   Python Language Mapping for omniORB
Group:     System/Libraries
Requires:  libomniORB = %{version}
Provides:  lib%{name} = %{version}-%{release} %{name} = %{version}-%{release}
Provides:  libomniorbpy = %{version}-%{release}
Conflicts: libomniORBpy < %{version}-%{release}
Obsoletes: libomniorbpy2

%description -n %{lib_name}
%{name} is a Python language mapping for the omniORB CORBA
Object Request Broker (ORB).

# "standard" part of the bundle

%package standard
Summary:   Files to provide standard top-level CORBA module for %{name}
Group:     Development/Python
Provides:  omniorbpy-standard = %{version}-%{release}
Conflicts: %{name}-standard < %{version}-%{release}

%description standard
The CORBA to Python mapping standard requires top-level CORBA and
PortableServer modules. This provides those standard modules for
%{name}. It will clash with similar packages for other Python ORBs.

# devel part of the bundle

%package devel
Summary:   Header files and libraries needed for %{name} development
Group:     Development/Python
Requires:  %{lib_name} = %{version}-%{release} omniORB-devel = %{version}
Provides:  libomniorbpy-devel = %{version}-%{release} omniorbpy-devel = %{version}-%{release}
Conflicts: %{name}-devel < %{version}-%{release}
Obsoletes: libomniORBpy-devel

%description devel
The header files and libraries needed for developing programs using %{name}.

# docs and examples are in a separate package

%package doc
Summary:   Documentation needed for %{name} development
Group:     Development/Python
Obsoletes: omniorbpy-doc libomniorbpy-doc

%description doc
Developer documentation and examples.

%prep 

%setup -n %{name} #-%{version}

# Needs to know where omniORB was installed if it is not in /usr.
# If necessary, use the configure option --with-omniorb=%{_prefix}
%{?configure:%configure}%{!?configure:./configure --prefix=%{_prefix} --libdir=%{_libdir}} --with-openssl=%{_prefix}


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

# omit omniidl_be/__init__.py because it is a duplicate of the file
# already provided by omniORB.
rm -rf %{buildroot}%{python_sitelib}/omniidl_be/__init__.py*


%clean
[ -z %{buildroot} ] || rm -rf %{buildroot}

%post -n %{lib_name}
/sbin/ldconfig

%postun -n %{lib_name}
/sbin/ldconfig


# main package includes libraries and servers
%files -n %{lib_name}
%defattr (-,root,root)
%doc COPYING.LIB
#%doc bugfixes*
%{python_sitearch}/_omni*.so*
%{python_sitelib}/omniORB

%files standard
%defattr(-,root,root)
%{python_sitelib}/*.py*
%{python_sitelib}/omniORB.pth
%{python_sitelib}/CosNaming
%{python_sitelib}/CosNaming__POA

%files devel
%defattr(-,root,root)
%doc README* ReleaseNotes* update.log
%{_includedir}/omniORBpy.h
%{_includedir}/omniORB4/pydistdate.hh
%{python_sitelib}/omniidl_be/python.py*

%files doc
%defattr(-,root,root)
%doc doc/* 

%changelog
* Wed Nov 24 2010 Dirk O. Kaar <dok@dok-net.net> 4.2.0-2
- Merge in improvements for current RPM from Thomas Lockhart

* Mon Sep 07 2009 Dirk O. Kaar <dok@dok-net.net> 4.2.0-1
- Use 4.2 sources from SVN

* Thu May 05 2005 Dirk O. Siebnich <dok@dok-net.net> 3.0-1
- rework package names to support x86_64

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
