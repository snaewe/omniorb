dnl -*- M4 -*-
AC_INIT(omniORB, 4.0.0, omniorb@omniorb.org)

AC_PREREQ(2.52)
AC_CONFIG_SRCDIR(bin/scripts/omkdirhier)
AC_CONFIG_AUX_DIR(bin/scripts)

AC_CANONICAL_BUILD
AC_CANONICAL_HOST

AC_CONFIG_HEADERS(include/omniconfig.h)


dnl ** Programs

AC_PROG_CC
AC_PROG_CXX
AC_PROG_CPP
AC_PROG_RANLIB
AC_PROG_INSTALL
AC_PROG_MAKE_SET

AM_PATH_PYTHON

AC_LANG(C++)


dnl ** Libraries

OMNI_OPENSSL_ROOT


dnl ** Headers

AC_CHECK_HEADERS(errno.h fcntl.h netdb.h signal.h stdlib.h string.h strings.h)
AC_CHECK_HEADERS(unistd.h nan.h sys/if.h sys/ioctl.h sys/param.h sys/time.h)
AC_HEADER_TIME


dnl ** Integer representation

AC_C_BIGENDIAN
AC_CHECK_SIZEOF(char, 1)
AC_CHECK_SIZEOF(unsigned char, 1)
AC_CHECK_SIZEOF(short, 2)
AC_CHECK_SIZEOF(int, 4)
AC_CHECK_SIZEOF(long, 4)
AC_CHECK_SIZEOF(long long, 8)
AC_CHECK_SIZEOF(float, 4)
AC_CHECK_SIZEOF(double, 8)
AC_CHECK_SIZEOF(long double, 16)
AC_CHECK_SIZEOF(wchar_t, 4)
AC_CHECK_SIZEOF(void*, 4)


dnl ** C++ features

AC_CXX_BOOL
AC_CXX_CONST_CAST
AC_CXX_DYNAMIC_CAST
AC_CXX_NAMESPACES
AC_CXX_HAVE_STD
AC_CXX_MEMBER_CONSTANTS

OMNI_CXX_CATCH_BY_BASE
OMNI_CXX_NEED_FQ_BASE_CTOR
OMNI_CXX_LONG_IS_INT


dnl ** Functions

AC_CHECK_FUNCS(gethostname getopt getpid gettimeofday isinf insinff isinfl)
AC_CHECK_FUNCS(nanosleep sigaction sigvec snprintf strcasecmp strdup)
AC_CHECK_FUNCS(strerror stricmp strncasecmp strtoul strtoull strtouq)
AC_CHECK_FUNCS(uname vsnprintf)

OMNI_HAVE_ISNANORINF
OMNI_HAVE_SIG_IGN
OMNI_GETTIMEOFDAY_TIMEZONE
OMNI_SOCKNAME_ARG


dnl ** omniORB config directories

OMNI_CONFIG_FILE
OMNI_OMNINAMES_LOGDIR
OMNI_DISABLE_STATIC


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
  *-*-irix*)
    compiler_name="Compiler_xlC"
  ;;
  esac
fi

AC_SUBST(COMPILER_NAME, $compiler_name)


dnl ** Platform name

platform_name="Platform_Unknown"
os_version="0"

case "$host" in
  *-*-linux-*)   platform_name="Linux";    os_version="2";;
  *-*-solaris*)  platform_name="SunOS";    os_version="5";;
  *-*-osf3*)     platform_name="OSF1";     os_version="3";;
  *-*-osf4*)     platform_name="OSF1";     os_version="4";;
  *-*-osf5*)     platform_name="OSF1";     os_version="5";;
  *-*-hpux10*)   platform_name="HPUX";     os_version="10";;
  *-*-hpux11*)   platform_name="HPUX";     os_version="11";;
  *-*-nextstep*) platform_name="NextStep"; os_version="3";;
  *-*-openstep*) platform_name="NextStep"; os_version="3";;
  *-*-irix*)     platform_name="IRIX";     os_version="6";;
  *-*-aix*)      platform_name="AIX";      os_version="4";;
  *-*-darwin*)   platform_name="Darwin";   os_version="1";;
  *-*-freebsd3*) platform_name="FreeBSD";  os_version="3";;
  *-*-freebsd4*) platform_name="FreeBSD";  os_version="4";;
  *-*-sco*)      platform_name="OSR5";     os_version="5";;
esac

AC_SUBST(PLATFORM_NAME, $platform_name)
AC_SUBST(OSVERSION, $os_version)


dnl ** Processor

processor_name="UnknownProcessor"

case "$host" in
  i?86-*)   processor_name="x86Processor";;
  sparc-*)  processor_name="SparcProcessor";;
  alpha*)   processor_name="AlphaProcessor";;
  m68k-*)   processor_name="m68kProcessor";;
  mips*)    processor_name="IndigoProcessor";;
  arm-*)    processor_name="ArmProcessor";;
  s390-*)   processor_name="s390Processor";;
  ia64-*)   processor_name="ia86Processor";;
  hppa*)    processor_name="HppaProcessor";;
  powerpc*) processor_name="PowerPCProcessor";;
esac

AC_SUBST(PROCESSOR_NAME, $processor_name)


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
                src/examples/boa/GNUmakefile
                src/examples/call_back/GNUmakefile
                src/examples/dii/GNUmakefile
                src/examples/GNUmakefile
                src/examples/dsi/GNUmakefile
                src/examples/echo/GNUmakefile
                src/examples/partcl/GNUmakefile
                src/examples/poa/GNUmakefile
                src/examples/poa/implicit_activation/GNUmakefile
                src/examples/poa/persistent_objref/GNUmakefile
                src/examples/poa/servant_manager/GNUmakefile
                src/examples/poa/threading/GNUmakefile
                src/examples/ssl_echo/GNUmakefile
                src/examples/thread/GNUmakefile
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
                include/omniORB4/GNUmakefile
                include/omniORB4/internal/GNUmakefile)

AC_OUTPUT
