# aclocal.m4 generated automatically by aclocal 1.6.1 -*- Autoconf -*-

# Copyright 1996, 1997, 1998, 1999, 2000, 2001, 2002
# Free Software Foundation, Inc.
# This file is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY, to the extent permitted by law; without
# even the implied warranty of MERCHANTABILITY or FITNESS FOR A
# PARTICULAR PURPOSE.

dnl
dnl omniORB specific tests
dnl

AC_DEFUN([OMNI_OPENSSL_ROOT],
[AC_CACHE_CHECK(for OpenSSL root,
omni_cv_openssl_root,
[AC_ARG_WITH(openssl,
             AC_HELP_STRING([--with-openssl],
               [OpenSSL root directory (default none)]),
             omni_cv_openssl_root=$withval,
             if test "x$prefix" != "xNONE"; then
               omni_cv_openssl_root=$prefix/openssl
             else
               omni_cv_openssl_root=$ac_default_prefix/openssl
             fi
	     if test -d $omni_cv_openssl_root/lib; then
               :
             else
               omni_cv_openssl_root=no
	     fi)
])
if test "$omni_cv_openssl_root" = "yes"; then
  if test "x$prefix" != "xNONE"; then
    omni_cv_openssl_root=$prefix/openssl
  else
    omni_cv_openssl_root=$ac_default_prefix/openssl
  fi
  if test -d $omni_cv_openssl_root; then
    :
  else
    AC_MSG_ERROR(Can't find OpenSSL in '$omni_cv_openssl_root'. Please give me the full path or leave out --with-openssl.)
    omni_cv_openssl_root=no
  fi
fi
open_ssl_root=$omni_cv_openssl_root
if test "$open_ssl_root" = "no"; then
  open_ssl_root=""
fi
AC_SUBST(OPEN_SSL_ROOT, $open_ssl_root)
])


AC_DEFUN([OMNI_CXX_CATCH_BY_BASE],
[AC_CACHE_CHECK(whether exceptions can be caught by base class,
omni_cv_cxx_catch_by_base,
[AC_REQUIRE([AC_CXX_EXCEPTIONS])
 AC_LANG_PUSH(C++)
 AC_TRY_RUN([
class A {
public:
  A() {}
};
class B : public virtual A {
public:
  B() {}
};
int main() {
  try {
    throw B();
  }
  catch (A&) {
    return 0;
  }
  catch (...) {
    return 1;
  }
  return 2;
}
],
 omni_cv_cxx_catch_by_base=yes, omni_cv_cxx_catch_by_base=no,
 omni_cv_cxx_catch_by_base=no)
 AC_LANG_POP(C++)
])
if test "$omni_cv_cxx_catch_by_base" = yes; then
  AC_DEFINE(HAVE_CATCH_BY_BASE,,
            [define if the compiler can catch exceptions by base class])
fi
])

AC_DEFUN([OMNI_CXX_NEED_FQ_BASE_CTOR],
[AC_CACHE_CHECK(whether base constructors have to be fully-qualified,
omni_cv_cxx_need_fq_base_ctor,
[AC_LANG_PUSH(C++)
 AC_TRY_COMPILE([
class A {
public:
  class B {
  public:
    B(int i) {}
  };
};
class C : public A::B {
public:
  C() : B(5) {}
};
int main() {
  return 0;
}
],
 omni_cv_cxx_need_fq_base_ctor=no, omni_cv_cxx_need_fq_base_ctor=yes,
 omni_cv_cxx_need_fq_base_ctor=no)
 AC_LANG_POP(C++)
])
if test "$omni_cv_cxx_need_fq_base_ctor=" = yes; then
  AC_DEFINE(OMNI_REQUIRES_FQ_BASE_CTOR,,
            [define if base constructors have to be fully qualified])
fi
])

AC_DEFUN([OMNI_CXX_LONG_IS_INT],
[AC_CACHE_CHECK(whether long is the same type as int,
omni_cv_cxx_long_is_int,
[AC_LANG_PUSH(C++)
 AC_TRY_COMPILE([
int f(int  x){return 1;}
int f(long x){return 1;}
],[long l = 5; return f(b);],
 omni_cv_cxx_long_is_int=yes, omni_cv_cxx_long_is_int=no)
 AC_LANG_POP(C++)
])
if test "$omni_cv_cxx_long_is_int" = yes; then
  AC_DEFINE(OMNI_LONG_IS_INT,,[define if long is the same type as int])
fi
])


AC_DEFUN([OMNI_HAVE_SIG_IGN],
[AC_CACHE_CHECK(whether SIG_IGN is available,
omni_cv_sig_ign_available,
[AC_LANG_PUSH(C++)
 AC_TRY_COMPILE([
#ifdef HAVE_SIGNAL_H
#include <signal.h>
#else
die here
#endif
#ifndef HAVE_SIGACTION
extern "C" int sigaction(int, const struct sigaction *, struct sigaction *);
#endif
],[
    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_handler = SIG_IGN;
],
 omni_cv_sig_ign_available=yes, omni_cv_sig_ign_available=no)
 AC_LANG_POP(C++)
])
if test "$omni_cv_sig_ign_available" = yes; then
  AC_DEFINE(HAVE_SIG_IGN,,[define if SIG_IGN is available])
fi
])

AC_DEFUN([OMNI_GETTIMEOFDAY_TIMEZONE],
[AC_CACHE_CHECK(whether gettimeofday() takes a timezone argument,
omni_cv_gettimeofday_timezone,
[AC_LANG_PUSH(C++)
 AC_TRY_COMPILE([
#ifdef HAVE_GETTIMEOFDAY
#include <sys/time.h>
#else
die here
#endif
],[
  struct timeval v;
  gettimeofday(&v, 0);
],
 omni_cv_gettimeofday_timezone=yes, omni_cv_gettimeofday_timezone=no)
 AC_LANG_POP(C++)
])
if test "$omni_cv_gettimeofday_timezone" = yes; then
  AC_DEFINE(GETTIMEOFDAY_TIMEZONE,,
            [define if gettimeofday() takes a timezone argument])
fi
])


AC_DEFUN([OMNI_HAVE_ISNANORINF],
[AC_CACHE_CHECK(for IsNANorINF,
omni_cv_have_isnanorinf,
[AC_LANG_PUSH(C++)
 AC_TRY_COMPILE([
#include <math.h>
#include <nan.h>
],[
  double d = 1.23;
  int i = IsNANorINF(d);
],
 omni_cv_have_isnanorinf=yes, omni_cv_have_isnanorinf=no)
 AC_LANG_POP(C++)
])
if test "$omni_cv_have_isnanorinf" = yes; then
  AC_DEFINE(HAVE_ISNANORINF,,
            [Define if you have the `IsNANorINF' function])
fi
])

AC_DEFUN([OMNI_SOCKNAME_ARG],
[AC_MSG_CHECKING([third argument of getsockname])
 omni_cv_sockname_size_t=no
 AC_LANG_PUSH(C++)
 AC_TRY_COMPILE([
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
],[
  socklen_t l;
  getsockname(0, 0, &l);
],
 omni_cv_sockname_size_t=socklen_t)
 if test "$omni_cv_sockname_size_t" = no; then
 AC_TRY_COMPILE([
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
],[
  size_t l;
  getsockname(0, 0, &l);
],
 omni_cv_sockname_size_t=size_t, omni_cv_sockname_size_t=int)
 fi
 AC_DEFINE_UNQUOTED(OMNI_SOCKNAME_SIZE_T, $omni_cv_sockname_size_t,
                    [Define to the type of getsockname's third argument])
 AC_MSG_RESULT([$omni_cv_sockname_size_t])
])

AC_DEFUN([OMNI_CONFIG_FILE],
[AC_CACHE_CHECK(omniORB config file location,
omni_cv_omniorb_config,
[AC_ARG_WITH(omniORB-config,
             AC_HELP_STRING([--with-omniORB-config],
               [location of omniORB config file (default /etc/omniORB.cfg)]),
             omni_cv_omniorb_config=$withval,
             omni_cv_omniorb_config="/etc/omniORB.cfg")
])
if test "$omni_cv_omniorb_config" = "no" || test "$omni_cv_omniorb_config" = "yes"; then
  echo "*** invalid omniORB config file '$omni_cv_omniorb_config'; using '/etc/omniORB.cfg'"
  omni_cv_omniorb_config="/etc/omniORB.cfg"
fi
AC_SUBST(OMNIORB_CONFIG, $omni_cv_omniorb_config)
])

AC_DEFUN([OMNI_OMNINAMES_LOGDIR],
[AC_CACHE_CHECK(omniNames log directory,
omni_cv_omninames_logdir,
[AC_ARG_WITH(omniNames-logdir,
             AC_HELP_STRING([--with-omniNames-logdir],
               [location of omniNames log directory (default /var/omninames)]),
             omni_cv_omninames_logdir=$withval,
             omni_cv_omninames_logdir="/var/omninames")
])
if test "$omni_cv_omninames_logdir" = "no" || test "$omni_cv_omninames_logdir" = "yes"; then
  echo "*** invalid omniNames log directory '$omni_cv_omninames_logdir'; using '/var/omninames'"
  omni_cv_omninames_logdir="/var/omninames"
fi
AC_SUBST(OMNINAMES_LOGDIR, $omni_cv_omninames_logdir)
])

AC_DEFUN([OMNI_DISABLE_STATIC],
[AC_CACHE_CHECK(whether to build static libraries,
omni_cv_enable_static,
[AC_ARG_ENABLE(static,
               AC_HELP_STRING([--disable-static],
                  [disable build of static libraries (default enable-static)]),
               omni_cv_enable_static=$enableval,
               omni_cv_enable_static=yes)
])
AC_SUBST(ENABLE_STATIC, $omni_cv_enable_static)
])


dnl This defaults to enabled, and is appropriate for development
dnl For the release, the obvious chunk below should be replaced with:
dnl               AC_HELP_STRING([--enable-thread-tracing],
dnl                  [enable thread and mutex tracing (default disable-thread-tracing)]),
dnl               omni_cv_enable_thread_tracing=$enableval)
dnl               omni_cv_enable_thread_tracing=no)
AC_DEFUN([OMNI_DISABLE_THREAD_TRACING],
[AC_CACHE_CHECK(whether to trace threads and locking,
omni_cv_enable_thread_tracing,
[AC_ARG_ENABLE(thread-tracing,
               AC_HELP_STRING([--disable-thread-tracing],
                  [disable thread and mutex tracing (default enable-thread-tracing)]),
               omni_cv_enable_thread_tracing=$enableval,
               omni_cv_enable_thread_tracing=yes)
])
if test "$omni_cv_enable_thread_tracing" = "yes"; then
  AC_DEFINE(OMNIORB_ENABLE_LOCK_TRACES,,[define if you want mutexes to be traced])
fi
])



dnl
dnl Tests from http://www.gnu.org/software/ac-archive/
dnl

AC_DEFUN([AC_CXX_EXCEPTIONS],
[AC_CACHE_CHECK(whether the compiler supports exceptions,
ac_cv_cxx_exceptions,
[AC_LANG_SAVE
 AC_LANG_CPLUSPLUS
 AC_TRY_COMPILE(,[try { throw  1; } catch (int i) { return i; }],
 ac_cv_cxx_exceptions=yes, ac_cv_cxx_exceptions=no)
 AC_LANG_RESTORE
])
if test "$ac_cv_cxx_exceptions" = yes; then
  AC_DEFINE(HAVE_EXCEPTIONS,,[define if the compiler supports exceptions])
fi
])

AC_DEFUN([AC_CXX_BOOL],
[AC_CACHE_CHECK(whether the compiler recognizes bool as a built-in type,
ac_cv_cxx_bool,
[AC_LANG_SAVE
 AC_LANG_CPLUSPLUS
 AC_TRY_COMPILE([
int f(int  x){return 1;}
int f(char x){return 1;}
int f(bool x){return 1;}
],[bool b = true; return f(b);],
 ac_cv_cxx_bool=yes, ac_cv_cxx_bool=no)
 AC_LANG_RESTORE
])
if test "$ac_cv_cxx_bool" = yes; then
  AC_DEFINE(HAVE_BOOL,,[define if bool is a built-in type])
fi
])

AC_DEFUN([AC_CXX_CONST_CAST],
[AC_CACHE_CHECK(whether the compiler supports const_cast<>,
ac_cv_cxx_const_cast,
[AC_LANG_SAVE
 AC_LANG_CPLUSPLUS
 AC_TRY_COMPILE(,[int x = 0;const int& y = x;int& z = const_cast<int&>(y);return z;],
 ac_cv_cxx_const_cast=yes, ac_cv_cxx_const_cast=no)
 AC_LANG_RESTORE
])
if test "$ac_cv_cxx_const_cast" = yes; then
  AC_DEFINE(HAVE_CONST_CAST,,[define if the compiler supports const_cast<>])
fi
])

AC_DEFUN([AC_CXX_DYNAMIC_CAST],
[AC_CACHE_CHECK(whether the compiler supports dynamic_cast<>,
ac_cv_cxx_dynamic_cast,
[AC_LANG_SAVE
 AC_LANG_CPLUSPLUS
 AC_TRY_COMPILE([#include <typeinfo>
class Base { public : Base () {} virtual void f () = 0;};
class Derived : public Base { public : Derived () {} virtual void f () {} };],[
Derived d; Base& b=d; return dynamic_cast<Derived*>(&b) ? 0 : 1;],
 ac_cv_cxx_dynamic_cast=yes, ac_cv_cxx_dynamic_cast=no)
 AC_LANG_RESTORE
])
if test "$ac_cv_cxx_dynamic_cast" = yes; then
  AC_DEFINE(HAVE_DYNAMIC_CAST,,[define if the compiler supports dynamic_cast<>])
fi
])

AC_DEFUN([AC_CXX_NAMESPACES],
[AC_CACHE_CHECK(whether the compiler implements namespaces,
ac_cv_cxx_namespaces,
[AC_LANG_SAVE
 AC_LANG_CPLUSPLUS
 AC_TRY_COMPILE([namespace Outer { namespace Inner { int i = 0; }}],
                [using namespace Outer::Inner; return i;],
 ac_cv_cxx_namespaces=yes, ac_cv_cxx_namespaces=no)
 AC_LANG_RESTORE
])
if test "$ac_cv_cxx_namespaces" = yes; then
  AC_DEFINE(HAVE_NAMESPACES,,[define if the compiler implements namespaces])
fi
])

AC_DEFUN([AC_CXX_HAVE_STD],
[AC_CACHE_CHECK(whether the compiler supports ISO C++ standard library,
ac_cv_cxx_have_std,
[AC_REQUIRE([AC_CXX_NAMESPACES])
 AC_LANG_SAVE
 AC_LANG_CPLUSPLUS
 AC_TRY_COMPILE([#include <iostream>
#include <map>
#include <iomanip>
#include <cmath>
#ifdef HAVE_NAMESPACES
using namespace std;
#endif],[return 0;],
 ac_cv_cxx_have_std=yes, ac_cv_cxx_have_std=no)
 AC_LANG_RESTORE
])
if test "$ac_cv_cxx_have_std" = yes; then
  AC_DEFINE(HAVE_STD,,[define if the compiler supports ISO C++ standard library])
fi
])

AC_DEFUN([AC_CXX_MEMBER_CONSTANTS],
[AC_CACHE_CHECK(whether the compiler supports member constants,
ac_cv_cxx_member_constants,
[AC_LANG_SAVE
 AC_LANG_CPLUSPLUS
 AC_TRY_COMPILE([class C {public: static const int i = 0;}; const int C::i;],
[return C::i;],
 ac_cv_cxx_member_constants=yes, ac_cv_cxx_member_constants=no)
 AC_LANG_RESTORE
])
if test "$ac_cv_cxx_member_constants" = yes; then
  AC_DEFINE(HAVE_MEMBER_CONSTANTS,,[define if the compiler supports member constants])
fi
])



# Copyright 1999, 2000, 2001, 2002  Free Software Foundation, Inc.

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
# 02111-1307, USA.

# AM_PATH_PYTHON([MINIMUM-VERSION])

# Adds support for distributing Python modules and packages.  To
# install modules, copy them to $(pythondir), using the python_PYTHON
# automake variable.  To install a package with the same name as the
# automake package, install to $(pkgpythondir), or use the
# pkgpython_PYTHON automake variable.

# The variables $(pyexecdir) and $(pkgpyexecdir) are provided as
# locations to install python extension modules (shared libraries).
# Another macro is required to find the appropriate flags to compile
# extension modules.

# If your package is configured with a different prefix to python,
# users will have to add the install directory to the PYTHONPATH
# environment variable, or create a .pth file (see the python
# documentation for details).

# If the MINIUMUM-VERSION argument is passed, AM_PATH_PYTHON will
# cause an error if the version of python installed on the system
# doesn't meet the requirement.  MINIMUM-VERSION should consist of
# numbers and dots only.

AC_DEFUN([AM_PATH_PYTHON],
 [
  dnl Find a Python interpreter.  Python versions prior to 1.5 are not
  dnl supported because the default installation locations changed from
  dnl $prefix/lib/site-python in 1.4 to $prefix/lib/python1.5/site-packages
  dnl in 1.5.
  m4_define([_AM_PYTHON_INTERPRETER_LIST],
	    [python python2 python2.2 python2.1 python2.0 python1.6 python1.5])

  m4_if([$1],[],[
    dnl No version check is needed.
    # Find any Python interpreter.
    AC_PATH_PROG([PYTHON], _AM_PYTHON_INTERPRETER_LIST)],[
    dnl A version check is needed.
    if test -n "$PYTHON"; then
      # If the user set $PYTHON, use it and don't search something else.
      AC_MSG_CHECKING([whether $PYTHON version >= $1])
      AM_PYTHON_CHECK_VERSION([$PYTHON], [$1],
			      [AC_MSG_RESULT(yes)],
			      [AC_MSG_ERROR(too old)])
    else
      # Otherwise, try each interpreter until we find one that satisfies
      # VERSION.
      AC_CACHE_CHECK([for a Python interpreter with version >= $1],
	[am_cv_pathless_PYTHON],[
	for am_cv_pathless_PYTHON in _AM_PYTHON_INTERPRETER_LIST : ; do
          if test "$am_cv_pathless_PYTHON" = : ; then
            AC_MSG_ERROR([no suitable Python interpreter found])
	  fi
          AM_PYTHON_CHECK_VERSION([$am_cv_pathless_PYTHON], [$1], [break])
        done])
      # Set $PYTHON to the absolute path of $am_cv_pathless_PYTHON.
      AC_PATH_PROG([PYTHON], [$am_cv_pathless_PYTHON])
    fi
  ])

  dnl Query Python for its version number.  Getting [:3] seems to be
  dnl the best way to do this; it's what "site.py" does in the standard
  dnl library.

  AC_CACHE_CHECK([for $am_cv_pathless_PYTHON version], [am_cv_python_version],
    [am_cv_python_version=`$PYTHON -c "import sys; print sys.version[[:3]]"`])
  AC_SUBST([PYTHON_VERSION], [$am_cv_python_version])

  dnl Use the values of $prefix and $exec_prefix for the corresponding
  dnl values of PYTHON_PREFIX and PYTHON_EXEC_PREFIX.  These are made
  dnl distinct variables so they can be overridden if need be.  However,
  dnl general consensus is that you shouldn't need this ability.

  AC_SUBST([PYTHON_PREFIX], ['${prefix}'])
  AC_SUBST([PYTHON_EXEC_PREFIX], ['${exec_prefix}'])

  dnl At times (like when building shared libraries) you may want
  dnl to know which OS platform Python thinks this is.

  AC_CACHE_CHECK([for $am_cv_pathless_PYTHON platform],
	         [am_cv_python_platform],
    [am_cv_python_platform=`$PYTHON -c "import sys; print sys.platform"`])
  AC_SUBST([PYTHON_PLATFORM], [$am_cv_python_platform])


  dnl Set up 4 directories:

  dnl pythondir -- where to install python scripts.  This is the
  dnl   site-packages directory, not the python standard library
  dnl   directory like in previous automake betas.  This behaviour
  dnl   is more consistent with lispdir.m4 for example.
  dnl
  dnl Also, if the package prefix isn't the same as python's prefix,
  dnl then the old $(pythondir) was pretty useless.

  AC_SUBST([pythondir],
	   [$PYTHON_PREFIX"/lib/python"$PYTHON_VERSION/site-packages])

  dnl pkgpythondir -- $PACKAGE directory under pythondir.  Was
  dnl   PYTHON_SITE_PACKAGE in previous betas, but this naming is
  dnl   more consistent with the rest of automake.
  dnl   Maybe this should be put in python.am?

  AC_SUBST([pkgpythondir], [\${pythondir}/$PACKAGE])

  dnl pyexecdir -- directory for installing python extension modules
  dnl   (shared libraries)  Was PYTHON_SITE_EXEC in previous betas.

  AC_SUBST([pyexecdir],
	   [${PYTHON_EXEC_PREFIX}/lib/python${PYTHON_VERSION}/site-packages])

  dnl pkgpyexecdir -- $(pyexecdir)/$(PACKAGE)
  dnl   Maybe this should be put in python.am?

  AC_SUBST([pkgpyexecdir], [\${pyexecdir}/$PACKAGE])
])


# AM_PYTHON_CHECK_VERSION(PROG, VERSION, [ACTION-IF-TRUE], [ACTION-IF-FALSE])
# ---------------------------------------------------------------------------
# Run ACTION-IF-TRUE if the Python interpreter PROG has version >= VERSION.
# Run ACTION-IF-FALSE otherwise.
# This test uses sys.hexversion instead of the string equivalant (first
# word of sys.version), in order to cope with versions such as 2.2c1.
# hexversion has been introduced in Python 1.5.2; it's probably not
# worth to support older versions (1.5.1 was released on October 31, 1998).
AC_DEFUN([AM_PYTHON_CHECK_VERSION],
 [prog="import sys, string
# split strings by '.' and convert to numeric.  Append some zeros
# because we need at least 4 digits for the hex conversion.
minver = map(int, string.split('$2', '.')) + [[0, 0, 0]]
minverhex = 0
for i in xrange(0, 4): minverhex = (minverhex << 8) + minver[[i]]
sys.exit(sys.hexversion < minverhex)"
  AS_IF([AM_RUN_LOG([$1 -c "$prog"])], [$3], [$4])])

# Copyright 2001 Free Software Foundation, Inc.             -*- Autoconf -*-

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
# 02111-1307, USA.

# AM_RUN_LOG(COMMAND)
# -------------------
# Run COMMAND, save the exit status in ac_status, and log it.
# (This has been adapted from Autoconf's _AC_RUN_LOG macro.)
AC_DEFUN([AM_RUN_LOG],
[{ echo "$as_me:$LINENO: $1" >&AS_MESSAGE_LOG_FD
   ($1) >&AS_MESSAGE_LOG_FD 2>&AS_MESSAGE_LOG_FD
   ac_status=$?
   echo "$as_me:$LINENO: \$? = $ac_status" >&AS_MESSAGE_LOG_FD
   (exit $ac_status); }])

