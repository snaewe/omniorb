dnl -*- M4 -*-
AC_INIT(omniORBpy, 4.2.0, bugs@omniorb-support.com)

AC_PREREQ(2.52)
AC_CONFIG_SRCDIR(python/omniORB/CORBA.py)
AC_CONFIG_AUX_DIR(bin/scripts)

AC_CANONICAL_BUILD
AC_CANONICAL_HOST


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

OMNI_OMNIORB_ROOT
OMNI_OPENSSL_ROOT


dnl ** More programs

abssrcdir=`cd $srcdir; pwd`
searchpath=$abssrcdir/bin/scripts:$OMNIORB_ROOT/bin:/usr/local/bin:/usr/bin:$PATH

AC_PATH_PROGS(OMKDEPEND, omkdepend, omkdepend,$searchpath)
AC_PATH_PROGS(MKDIRHIER, omkdirhier mkdirhier, omkdirhier, $searchpath)


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
  *-*-kfreebsd5*) plat_name="kFreeBSD";  plat_def="__FreeBSD_kernel__";  os_v="5";;
  *-*-kfreebsd6*) plat_name="kFreeBSD";  plat_def="__FreeBSD_kernel__";  os_v="6";;
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
  x86_64-*) proc_name="x8664Processor";   proc_def="__x86_64__";;
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

AC_CONFIG_FILES(mk/beforeauto.mk mk/afterauto.mk)

AC_CONFIG_FILES(
                GNUmakefile
                examples/embed/GNUmakefile
                modules/GNUmakefile
                modules/codesets/GNUmakefile
                modules/sslTP/GNUmakefile
                modules/connections/GNUmakefile
                omniidl_be/GNUmakefile
                python/GNUmakefile
                python/COS/GNUmakefile
                python/omniORB/GNUmakefile
		include/GNUmakefile
               )

AC_OUTPUT
