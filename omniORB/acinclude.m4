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
             omni_cv_openssl_root=no)
])

dnl Hairy logic to work out OpenSSL path and options.
dnl
dnl  If the option is "no", do nothing.
dnl
dnl  If the option is "yes", try pkg-config, failing that use prefix.
dnl
dnl  If the option is a path, and <path>/lib/pkgconfig exists, try to
dnl   use pkg-config based on that directory, failing that just use
dnl   the directory/

if test "$omni_cv_openssl_root" = "no"; then
  :
else
  if test "$PKG_CONFIG" != "no" ; then
    set_pkg_config_path=no
    if test "$omni_cv_openssl_root" = "yes"; then
      do_pkg_config=yes
    else
      if test -d $omni_cv_openssl_root/lib/pkgconfig; then
        saved_pkg_config_libdir="$PKG_CONFIG_LIBDIR"
        saved_pkg_config_path="$PKG_CONFIG_PATH"
        PKG_CONFIG_LIBDIR=$omni_cv_openssl_root/lib/pkgconfig
        PKG_CONFIG_PATH=
        export PKG_CONFIG_LIBDIR
        export PKG_CONFIG_PATH
        do_pkg_config=yes
        set_pkg_config_path=yes
      else
        do_pkg_config=no
      fi
    fi

    if test "$do_pkg_config" = "yes"; then
      PKG_CHECK_MODULES(OPENSSL, openssl,
          [open_ssl_root=`$PKG_CONFIG --variable=prefix openssl`
           open_ssl_cppflags="$OPENSSL_CFLAGS"
           open_ssl_lib="$OPENSSL_LIBS"
           open_ssl_pkgconfig="yes"
           omni_cv_openssl_root="$open_ssl_root"
          ],
          [open_ssl_pkgconfig="no"])
    fi

    if test "$set_pkg_config_path" = "yes"; then
      if test "x$saved_pkg_config_libdir" = "x"; then
        unset PKG_CONFIG_LIBDIR
      else
        PKG_CONFIG_LIBDIR=$saved_pkg_config_libdir
        export PKG_CONFIG_LIBDIR
      fi
      if test "x$saved_pkg_config_path" = "x"; then
        unset PKG_CONFIG_PATH
      else
        PKG_CONFIG_PATH=$saved_pkg_config_path
        export PKG_CONFIG_PATH
      fi
    fi
  fi
  if test "$omni_cv_openssl_root" = "yes"; then
    if test "x$prefix" != "xNONE"; then
      omni_cv_openssl_root=$prefix
    else
      omni_cv_openssl_root=$ac_default_prefix
    fi
    if test -d $omni_cv_openssl_root/openssl/lib; then
      omni_cv_openssl_root=$omni_cv_openssl_root/openssl
    elif test -d $omni_cv_openssl_root/lib; then
      :
    else
      AC_MSG_ERROR(Can't find OpenSSL in '$omni_cv_openssl_root'. Please give me the full path or leave out --with-openssl.)
      omni_cv_openssl_root=no
    fi
  fi
fi
open_ssl_root=$omni_cv_openssl_root
if test "$open_ssl_root" = "no"; then
  open_ssl_root=""
  open_ssl_cppflags=""
  open_ssl_lib=""
else 
  if test "x$open_ssl_pkgconfig" = "xyes"; then
    :
  else
    open_ssl_cppflags="-I$open_ssl_root/include"
    open_ssl_lib="-L$open_ssl_root/lib -lssl -lcrypto"
  fi
fi
AC_SUBST(OPEN_SSL_ROOT, $open_ssl_root)
AC_SUBST(OPEN_SSL_CPPFLAGS, $open_ssl_cppflags)
AC_SUBST(OPEN_SSL_LIB, $open_ssl_lib)
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
/* Test sub-classes */
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
/* Test namespaces */
namespace P { class R {};               };
namespace Q { class R : public P::R {}; };
class S : public Q::R {
public:
  S(): R() {}
};
],
[C c; S s;],
 omni_cv_cxx_need_fq_base_ctor=no, omni_cv_cxx_need_fq_base_ctor=yes)
 AC_LANG_POP(C++)
])
if test "$omni_cv_cxx_need_fq_base_ctor" = yes; then
  AC_DEFINE(OMNI_REQUIRES_FQ_BASE_CTOR,,
            [define if base constructors have to be fully qualified])
fi
])

AC_DEFUN([OMNI_CXX_COVARIANT_RETURNS],
[AC_CACHE_CHECK(whether the compiler supports covariant return types,
omni_cv_cxx_covariant_returns,
[AC_LANG_PUSH(C++)
 AC_TRY_COMPILE([
class A {};
class B : public virtual A {};
class C {
public:
  virtual A* test();
};
class D : public virtual C {
public:
  virtual B* test();
};
],
[D d;],
 omni_cv_cxx_covariant_returns=yes, omni_cv_cxx_covariant_returns=no)
 AC_LANG_POP(C++)
])
if test "$omni_cv_cxx_covariant_returns" = yes; then
  AC_DEFINE(OMNI_HAVE_COVARIANT_RETURNS,,
            [define if the compiler supports covariant return types])
fi
])


AC_DEFUN([OMNI_CXX_LONG_IS_INT],
[AC_CACHE_CHECK(whether long is the same type as int,
omni_cv_cxx_long_is_int,
[AC_LANG_PUSH(C++)
 AC_TRY_COMPILE([
int f(int  x){return 1;}
int f(long x){return 1;}
],[long l = 5; return f(l);],
 omni_cv_cxx_long_is_int=no, omni_cv_cxx_long_is_int=yes)
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
               AC_HELP_STRING([--enable-thread-tracing],
                  [enable thread and mutex tracing (default disable-thread-tracing)]),
               omni_cv_enable_thread_tracing=$enableval,
               omni_cv_enable_thread_tracing=no)
])
if test "$omni_cv_enable_thread_tracing" = "yes"; then
  AC_DEFINE(OMNIORB_ENABLE_LOCK_TRACES,,[define if you want mutexes to be traced])
fi
])

dnl Allow IPv6 to be disabled on platforms that look like they support
dnl it, but where it doesn't work.
AC_DEFUN([OMNI_DISABLE_IPV6_CHECK],
[AC_CACHE_CHECK(whether to support IPv6,
omni_cv_enable_ipv6,
[AC_ARG_ENABLE(ipv6,
               AC_HELP_STRING([--disable-ipv6],
                  [disable IPv6 support (default enable-ipv6)]),
               omni_cv_enable_ipv6=$enableval,
               omni_cv_enable_ipv6=yes)
])
if test "$omni_cv_enable_ipv6" = "no"; then
  AC_DEFINE(OMNI_DISABLE_IPV6,,[define if you want to disable IPv6 support])
fi
])

dnl Support for platforms where we just can't get alloca to work
AC_DEFUN([OMNI_DISABLE_ALLOCA],
[AC_CACHE_CHECK(whether alloca should be used in omnicpp,
omni_cv_enable_alloca,
[AC_ARG_ENABLE(alloca,
               AC_HELP_STRING([--disable-alloca],
                  [disable use of alloca in omnicpp (default enable-alloca)]),
               omni_cv_enable_alloca=$enableval,
               omni_cv_enable_alloca=yes)
])
if test "$omni_cv_enable_alloca" = "no"; then
  AC_DEFINE(OMNIORB_DISABLE_ALLOCA,,[define if you want to avoid use of alloca])
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

AC_DEFUN([AC_CXX_REINTERPRET_CAST],
[AC_CACHE_CHECK(whether the compiler supports reinterpret_cast<>,
ac_cv_cxx_reinterpret_cast,
[AC_LANG_SAVE
 AC_LANG_CPLUSPLUS
 AC_TRY_COMPILE([#include <typeinfo>
class Base { public : Base () {} virtual void f () = 0;};
class Derived : public Base { public : Derived () {} virtual void f () {} };
class Unrelated { public : Unrelated () {} };
int g (Unrelated&) { return 0; }],[
Derived d;Base& b=d;Unrelated& e=reinterpret_cast<Unrelated&>(b);return g(e);],
 ac_cv_cxx_reinterpret_cast=yes, ac_cv_cxx_reinterpret_cast=no)
 AC_LANG_RESTORE
])
if test "$ac_cv_cxx_reinterpret_cast" = yes; then
  AC_DEFINE(HAVE_REINTERPRET_CAST,,
            [define if the compiler supports reinterpret_cast<>])
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

AC_DEFUN([OMNI_CHECK_NO_UNIT_AT_A_TIME],
[AC_CACHE_CHECK(whether $CC accepts -fno-unit-at-a-time,
omni_cv_no_unit_at_a_time,
[
    AC_LANG_PUSH(C)
    save_CFLAGS=$CFLAGS
    CFLAGS="$CFLAGS -fno-unit-at-a-time"
    AC_COMPILE_IFELSE(AC_LANG_PROGRAM(,), omni_cv_no_unit_at_a_time="yes", omni_cv_no_unit_at_a_time="no")
    CFLAGS=$save_CFLAGS
    AC_LANG_POP
])])
