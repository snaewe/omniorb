dnl -*- M4 -*-
AC_INIT(omniORB, 4.1.0, bugs@omniorb-support.com)

AC_PREREQ(2.52)
AC_CONFIG_SRCDIR(bin/scripts/omkdirhier)
AC_CONFIG_AUX_DIR(bin/scripts)

AC_CANONICAL_BUILD
AC_CANONICAL_HOST

AC_CONFIG_HEADERS(include/omniORB4/acconfig.h)


dnl ** Programs

AC_PROG_CC
AC_PROG_CXX
AC_PROG_CPP
AC_PROG_RANLIB
AC_PROG_INSTALL
AC_PROG_MAKE_SET

AM_PATH_PYTHON

AC_PATH_PROG(PKG_CONFIG, pkg-config, no)

AC_LANG(C++)

dnl OMNI_CHECK_NO_UNIT_AT_A_TIME
dnl if test "x$omni_cv_no_unit_at_a_time" = "xyes"
dnl then
dnl   EXTRA_GCC_CXXFLAGS="-fno-unit-at-a-time"
dnl fi
AC_SUBST(EXTRA_GCC_CXXFLAGS)

dnl ** Libraries

OMNI_OPENSSL_ROOT


dnl ** Headers

AC_CHECK_HEADERS(errno.h fcntl.h netdb.h signal.h stdlib.h string.h strings.h)
AC_CHECK_HEADERS(unistd.h nan.h sys/if.h sys/ioctl.h sys/param.h sys/time.h)
AC_CHECK_HEADERS(sys/poll.h ifaddrs.h)
AC_HEADER_TIME


dnl ** Integer representation

AC_C_BIGENDIAN
AC_CHECK_SIZEOF(char, 1)
AC_CHECK_SIZEOF(unsigned char, 1)
AC_CHECK_SIZEOF(bool, 1)
AC_CHECK_SIZEOF(short, 2)
AC_CHECK_SIZEOF(int, 4)
AC_CHECK_SIZEOF(long, 4)
AC_CHECK_SIZEOF(long long, 8)
AC_CHECK_SIZEOF(float, 4)
AC_CHECK_SIZEOF(double, 8)
AC_CHECK_SIZEOF(long double, 16)
AC_CHECK_SIZEOF(wchar_t, 4)
AC_CHECK_SIZEOF(void*, 4)


dnl ** Structs and members

AC_CHECK_TYPES([struct sockaddr_in6, struct sockaddr_storage, struct lifconf],,,
[#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>])

AC_CHECK_MEMBERS([struct sockaddr_in.sin_len, struct sockaddr_in.sin_zero],,,
[#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>])


dnl ** C++ features

AC_CXX_BOOL
AC_CXX_CONST_CAST
AC_CXX_DYNAMIC_CAST
AC_CXX_REINTERPRET_CAST
AC_CXX_NAMESPACES
AC_CXX_HAVE_STD
AC_CXX_MEMBER_CONSTANTS

OMNI_CXX_CATCH_BY_BASE
OMNI_CXX_NEED_FQ_BASE_CTOR
OMNI_CXX_COVARIANT_RETURNS
OMNI_CXX_LONG_IS_INT


dnl ** Functions

AC_LANG(C)
AC_CHECK_FUNCS(getaddrinfo gethostname getopt getpid gettimeofday)
AC_CHECK_FUNCS(getnameinfo inet_ntop)
AC_CHECK_FUNCS(isinf insinff isinfl localtime nanosleep poll sigaction)
AC_CHECK_FUNCS(sigvec snprintf strcasecmp strdup strerror strftime stricmp)
AC_CHECK_FUNCS(strncasecmp strtoul strtoull strtouq uname vprintf vsnprintf)
AC_FUNC_ALLOCA
AC_LANG(C++)

OMNI_HAVE_ISNANORINF
OMNI_HAVE_SIG_IGN
OMNI_GETTIMEOFDAY_TIMEZONE
OMNI_SOCKNAME_ARG


dnl ** omniORB config

OMNI_CONFIG_FILE
OMNI_OMNINAMES_LOGDIR
OMNI_DISABLE_STATIC
OMNI_DISABLE_THREAD_TRACING
OMNI_DISABLE_IPV6_CHECK
OMNI_DISABLE_ALLOCA


dnl ** Compiler name

compiler_name="Compiler_Unknown"

if test "$ac_cv_cxx_compiler_gnu" = "yes"; then
  compiler_name="Compiler_GCC"
elif test `basename $CXX` = "KCC"; then
  compiler_name="Compiler_KCC"
else
  case "$host" in
  *-*-solaris*)
    if $CXX -V 2>&1 | grep '5\.[[0-9]]' >/dev/null 2>&1; then
      compiler_name="Compiler_Sun5"
    elif $CXX -V 2>&1 | grep '4\.[[0-9]]' >/dev/null 2>&1; then
      compiler_name="Compiler_Sun4"
    fi
  ;;
  *-*-osf*)
    # *UNK* We need to know how to ask DEC CXX what version it is
    compiler_name="Compiler_DEC61"
  ;;
  *-*-hpux*)
    compiler_name="Compiler_aCC"
  ;;
  *-*-irix*)
    compiler_name="Compiler_SGI"
  ;;
  *-*-aix*)
    compiler_name="Compiler_xlC"
  ;;
  esac
fi

AC_SUBST(COMPILER_NAME, $compiler_name)


dnl ** Platform name

plat_name="Platform_Unknown"
plat_def="__unknown_platform__"
os_v="0"

case "$host" in
  *-*-linux-*)   plat_name="Linux";    plat_def="__linux__";    os_v="2";;
  *-*-cygwin*)   plat_name="Cygwin";   plat_def="__cygwin__";   os_v="1";;
  *-*-solaris*)  plat_name="SunOS";    plat_def="__sunos__";    os_v="5";;
  *-*-osf3*)     plat_name="OSF1";     plat_def="__osf1__";     os_v="3";;
  *-*-osf4*)     plat_name="OSF1";     plat_def="__osf1__";     os_v="4";;
  *-*-osf5*)     plat_name="OSF1";     plat_def="__osf1__";     os_v="5";;
  *-*-hpux10*)   plat_name="HPUX";     plat_def="__hpux__";     os_v="10";;
  *-*-hpux11*)   plat_name="HPUX";     plat_def="__hpux__";     os_v="11";;
  *-*-nextstep*) plat_name="NextStep"; plat_def="__nextstep__"; os_v="3";;
  *-*-openstep*) plat_name="NextStep"; plat_def="__nextstep__"; os_v="3";;
  *-*-irix*)     plat_name="IRIX";     plat_def="__irix__";     os_v="6";;
  *-*-aix*)      plat_name="AIX";      plat_def="__aix__";      os_v="4";;
  *-*-darwin*)   plat_name="Darwin";   plat_def="__darwin__";   os_v="1";;
  *-*-freebsd3*) plat_name="FreeBSD";  plat_def="__freebsd__";  os_v="3";;
  *-*-freebsd4*) plat_name="FreeBSD";  plat_def="__freebsd__";  os_v="4";;
  *-*-freebsd5*) plat_name="FreeBSD";  plat_def="__freebsd__";  os_v="5";;
  *-*-freebsd6*) plat_name="FreeBSD";  plat_def="__freebsd__";  os_v="6";;
  *-*-freebsd7*) plat_name="FreeBSD";  plat_def="__freebsd__";  os_v="7";;
  *-*-netbsd*)   plat_name="NetBSD";   plat_def="__netbsd__";   os_v="1";;
  *-*-openbsd*)  plat_name="OpenBSD";  plat_def="__openbsd__";  os_v="3";;
  *-*-sco*)      plat_name="OSR5";     plat_def="__osr5__";     os_v="5";;
esac

AC_SUBST(PLATFORM_NAME, $plat_name)
AC_SUBST(PLATFORM_DEFINE, $plat_def)
AC_SUBST(OSVERSION, $os_v)


dnl ** Processor

proc_name="UnknownProcessor"
proc_def="__processor_unknown__"

case "$host" in
  i?86-*)   proc_name="x86Processor";     proc_def="__x86__";;
  sparc-*)  proc_name="SparcProcessor";   proc_def="__sparc__";;
  alpha*)   proc_name="AlphaProcessor";   proc_def="__alpha__";;
  m68k-*)   proc_name="m68kProcessor";    proc_def="__m68k__";;
  mips*)    proc_name="IndigoProcessor";  proc_def="__mips__";;
  arm-*)    proc_name="ArmProcessor";     proc_def="__arm__";;
  s390-*)   proc_name="s390Processor";    proc_def="__s390__";;
  ia64-*)   proc_name="ia64Processor";    proc_def="__ia64__";;
  hppa*)    proc_name="HppaProcessor";    proc_def="__hppa__";;
  powerpc*) proc_name="PowerPCProcessor"; proc_def="__powerpc__";;
esac

AC_SUBST(PROCESSOR_NAME, $proc_name)
AC_SUBST(PROCESSOR_DEFINE, $proc_def)

dnl ** Output

AC_CONFIG_FILES(mk/beforeauto.mk mk/afterauto.mk GNUmakefile)
AC_CONFIG_FILES(src/tool/omniidl/python/scripts/omniidl)
AC_CONFIG_FILES(src/appl/GNUmakefile
                src/appl/omniMapper/GNUmakefile
                src/appl/omniNames/GNUmakefile
                src/appl/utils/catior/GNUmakefile
                src/appl/utils/convertior/GNUmakefile
                src/appl/utils/GNUmakefile
                src/appl/utils/genior/GNUmakefile
                src/appl/utils/nameclt/GNUmakefile
                src/GNUmakefile
                src/examples/anyExample/GNUmakefile
                src/examples/bidir/GNUmakefile
                src/examples/boa/GNUmakefile
                src/examples/call_back/GNUmakefile
                src/examples/dii/GNUmakefile
                src/examples/GNUmakefile
                src/examples/dsi/GNUmakefile
                src/examples/echo/GNUmakefile
                src/examples/poa/GNUmakefile
                src/examples/poa/implicit_activation/GNUmakefile
                src/examples/poa/persistent_objref/GNUmakefile
                src/examples/poa/servant_manager/GNUmakefile
                src/examples/poa/threading/GNUmakefile
                src/examples/ssl_echo/GNUmakefile
                src/examples/thread/GNUmakefile
                src/examples/valuetype/GNUmakefile
                src/examples/valuetype/simple/GNUmakefile
                src/lib/GNUmakefile
                src/lib/omniORB/codesets/GNUmakefile
                src/lib/omniORB/GNUmakefile
                src/lib/omniORB/dynamic/GNUmakefile
                src/lib/omniORB/omniidl_be/cxx/GNUmakefile
                src/lib/omniORB/omniidl_be/cxx/dynskel/GNUmakefile
                src/lib/omniORB/omniidl_be/cxx/header/GNUmakefile
                src/lib/omniORB/omniidl_be/cxx/impl/GNUmakefile
                src/lib/omniORB/omniidl_be/cxx/skel/GNUmakefile
                src/lib/omniORB/omniidl_be/GNUmakefile
                src/lib/omniORB/orbcore/GNUmakefile
                src/lib/omniORB/orbcore/ssl/GNUmakefile
                src/lib/omniORB/connections/GNUmakefile
                src/lib/omnithread/GNUmakefile
                src/services/GNUmakefile
                src/services/mklib/GNUmakefile
                src/services/mklib/mkBOAlib/GNUmakefile
                src/tool/GNUmakefile
                src/tool/omkdepend/GNUmakefile
                src/tool/omniidl/cxx/cccp/GNUmakefile
                src/tool/omniidl/cxx/GNUmakefile
                src/tool/omniidl/GNUmakefile
                src/tool/omniidl/python/GNUmakefile
                src/tool/omniidl/python/omniidl_be/GNUmakefile
                src/tool/omniidl/python/omniidl/GNUmakefile
                src/tool/omniidl/python/scripts/GNUmakefile
               )

AC_CONFIG_FILES(include/GNUmakefile
		include/omniconfig.h
                include/omnithread/GNUmakefile
                include/omniORB4/GNUmakefile
                include/omniORB4/internal/GNUmakefile)

AC_CONFIG_FILES(idl/GNUmakefile
                idl/COS/GNUmakefile)

AC_CONFIG_FILES(etc/init.d/omniNames)

AC_CONFIG_FILES(contrib/GNUmakefile
                contrib/pkgconfig/GNUmakefile)

AC_OUTPUT([
contrib/pkgconfig/omnithread3.pc
contrib/pkgconfig/omniORB4.pc
contrib/pkgconfig/omniDynamic4.pc
contrib/pkgconfig/omniCOS4.pc
contrib/pkgconfig/omniCOSDynamic4.pc
contrib/pkgconfig/omniConnectionMgmt4.pc
])
