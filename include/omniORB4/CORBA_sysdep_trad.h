// -*- Mode: C++; -*-
//                            Package   : omniORB2
// CORBA_sysdep.h             Created on: 30/1/96
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 1996-1999 AT&T Laboratories Cambridge
//
//    This file is part of the omniORB library
//
//    The omniORB library is free software; you can redistribute it and/or
//    modify it under the terms of the GNU Library General Public
//    License as published by the Free Software Foundation; either
//    version 2 of the License, or (at your option) any later version.
//
//    This library is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//    Library General Public License for more details.
//
//    You should have received a copy of the GNU Library General Public
//    License along with this library; if not, write to the Free
//    Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
//    02111-1307, USA
//
//
// Description:
//	*** PROPRIETARY INTERFACE ***
//
//      Traditional-style hard-coded system dependencies.

/*
  $Log$
  Revision 1.1.4.3  2005/01/13 21:09:57  dgrisby
  New SocketCollection implementation, using poll() where available and
  select() otherwise. Windows specific version to follow.

  Revision 1.1.4.2  2005/01/06 23:08:09  dgrisby
  Big merge from omni4_0_develop.

  Revision 1.1.4.1  2003/03/23 21:04:21  dgrisby
  Start of omniORB 4.1.x development branch.

  Revision 1.1.2.9  2003/03/12 14:07:44  dgrisby
  MacOS port. Thanks Wolfgang Textor.

  Revision 1.1.2.8  2003/02/21 15:56:09  dgrisby
  Silence macro redefinition warnings on Windows.

  Revision 1.1.2.7  2003/02/17 02:03:07  dgrisby
  vxWorks port. (Thanks Michael Sturm / Acterna Eningen GmbH).

  Revision 1.1.2.6  2003/01/16 11:08:26  dgrisby
  Patches to support Digital Mars C++. Thanks Christof Meerwald.

  Revision 1.1.2.5  2002/11/06 11:58:28  dgrisby
  Partial AIX patches.

  Revision 1.1.2.4  2002/10/14 15:09:58  dgrisby
  Cope with platforms where sizeof(bool) != 1.

  Revision 1.1.2.3  2002/03/13 16:05:38  dpg1
  Transport shutdown fixes. Reference count SocketCollections to avoid
  connections using them after they are deleted. Properly close
  connections when in thread pool mode.

  Revision 1.1.2.2  2002/01/31 10:16:33  dpg1
  Missing define in traditional sysdep.

  Revision 1.1.2.1  2002/01/15 16:38:10  dpg1
  On the road to autoconf. Dependencies refactored, configure.ac
  written. No makefiles yet.

*/

#ifndef __CORBA_SYSDEP_TRAD_H__
#define __CORBA_SYSDEP_TRAD_H__


#ifndef _CORBA_WCHAR_DECL
#  define _CORBA_WCHAR_DECL wchar_t
#endif


#define HAS_Cplusplus_const_cast
// Unset this define if the compiler does not support const_cast<T*>

#define HAS_Cplusplus_reinterpret_cast
// Unset this define if the compiler does not support reinterpret_cast<T>

#define HAS_Cplusplus_catch_exception_by_base
// Unset this define if the compiler does not support catching
// exceptions by base class.

#define HAVE_STRDUP 1
// Unset if no strdup()

#define HAVE_GETOPT 1
// Unset if no getopt()

#define HAVE_UNISTD_H 1
// Unset if no unistd.h header

#ifndef HAVE_GETPID
#  define HAVE_GETPID 1
#endif
// Unset if no getpid() function

#define HAVE_GETTIMEOFDAY 1
// Unset if no gettimeofday() function

#define GETTIMEOFDAY_TIMEZONE
// Unset if gettimeofday() doesn't take a timezone argument

#define HAVE_SIGNAL_H 1
// Unset if no signal.h header

#define HAVE_SIGACTION 1
// Unset if no sigaction() function

#define HAVE_SIG_IGN
// Unset if have sigaction() but have no SIG_IGN define

#define HAVE_STRCASECMP 1
// Unset if no strcasecmp() function

#define HAVE_STRNCASECMP 1
// Unset if no strncasecmp() function

#define HAVE_UNAME  1
// Unset if no uname() function

#define HAVE_GETHOSTNAME 1
// Unset if no gethostname() function

#define HAVE_VSNPRINTF 1
// Unset if no vsnprintf() function

#define HAVE_STRTOUL 1
// Unset if no strtoul() function



//
// Compiler dependencies
//

#if defined(__GNUG__)
// GNU G++ compiler

#  if __GNUG__ == 2 && __GNUC_MINOR__ == 7
#     undef HAS_Cplusplus_catch_exception_by_base
#  endif

// Minor version number 91 is for egcs version 1.*  Some older
// versions of 1.* may not support namespaces properly - this is
// only tested for egcs 1.1.1
#  if (__GNUG__ == 2 && (__GNUC_MINOR__ >= 91 || __GNUC_MINOR__ == 9)) || \
      (__GNUG__ >= 3)
#     define HAS_Cplusplus_Namespace
#     define HAS_Cplusplus_Bool
#  endif

// Since gcc 3.3 old IOstream's are considered deprecated.
#  if (__GNUG__ > 3 || (__GNUG__ == 3 && __GNUC_MINOR__ >= 3))
#     define HAS_Cplusplus_Namespace
#     define HAS_Std_Namespace
#     ifndef HAVE_STD
#         define HAVE_STD 1
#     endif
#  endif

// GCC claims to support long long on all platforms
#  define HAS_LongLong
#  define HAS_LongDouble
#  define _CORBA_LONGLONG_DECL   long long
#  define _CORBA_ULONGLONG_DECL  unsigned long long
#  define _CORBA_LONGDOUBLE_DECL long double 
#  define _CORBA_LONGLONG_CONST(x) (x##LL)

#elif defined(__DECCXX)
// DEC C++ compiler

#  if __DECCXX_VER >= 60000000
#     define HAS_LongLong
//#     define HAS_LongDouble
#     define _CORBA_LONGLONG_DECL   long long
#     define _CORBA_ULONGLONG_DECL  unsigned long long
#     define _CORBA_LONGDOUBLE_DECL long double
#     define _CORBA_LONGLONG_CONST(x) (x##LL)
#     ifndef NO_Cplusplus_Bool
#       define HAS_Cplusplus_Bool
#     endif
#     define HAS_Cplusplus_Namespace
#     define HAS_Std_Namespace
#     define HAS_pch
#     define OMNI_REQUIRES_FQ_BASE_CTOR
// Uncomment the following lines to enable the use of namespace with cxx v5.6
// Notice that the source code may have to be patched to compile.
//#  elif __DECCXX_VER >= 50600000
//#     define HAS_Cplusplus_Namespace
//#     define NEED_DUMMY_RETURN
#  else
//    Compaq C++ 5.x
#     undef  HAS_Cplusplus_const_cast
#     undef  HAS_Cplusplus_reinterpret_cast
#     define OMNI_REQUIRES_FQ_BASE_CTOR

#  endif

#elif defined(__SUNPRO_CC) 
// SUN C++ compiler
#  if __SUNPRO_CC >= 0x500
#    if __SUNPRO_CC_COMPAT >= 5
#      define HAS_Cplusplus_Namespace
#      define HAS_Std_Namespace
#    endif
#  endif

#  define HAS_LongLong
#  define _CORBA_LONGLONG_DECL   long long
#  define _CORBA_ULONGLONG_DECL  unsigned long long
#  define _CORBA_LONGDOUBLE_DECL long double 
#  define _CORBA_LONGLONG_CONST(x) (x##LL)

#  define HAS_LongDouble


#elif defined(_MSC_VER)
//  Microsoft Visual C++ compiler
#  if _MSC_VER >= 1000
#    ifndef NO_Cplusplus_Bool
#      define HAS_Cplusplus_Bool
#    endif
#    define HAS_Cplusplus_Namespace
#    define HAS_Std_Namespace
#    ifndef HAVE_STD
#        define HAVE_STD 1
#    endif
#  endif

#  define HAS_LongLong
#  define _CORBA_LONGLONG_DECL   __int64
#  define _CORBA_ULONGLONG_DECL  unsigned __int64
#  define _CORBA_LONGLONG_CONST(x) (x)

#elif defined(__DMC__)
//  Digital Mars C++
#  define HAS_Cplusplus_Bool
#  define HAS_Cplusplus_Namespace
#  define HAS_Std_Namespace

#  define HAVE_STRTOULL

#  define HAS_LongDouble
#  define HAS_LongLong
#  define _CORBA_LONGDOUBLE_DECL long double
#  define _CORBA_LONGLONG_DECL   long long
#  define _CORBA_ULONGLONG_DECL  unsigned long long
#  define _CORBA_LONGLONG_CONST(x) (x##LL)

#  define OMNI_REQUIRES_FQ_BASE_CTOR

#elif defined(__BCPLUSPLUS__)
// Borland C++ Builder
#  define HAS_Cplusplus_Namespace
#  define HAS_Std_Namespace

#  define HAS_LongLong
#  define _CORBA_LONGLONG_DECL   __int64
#  define _CORBA_ULONGLONG_DECL  unsigned __int64
#  define _CORBA_LONGLONG_CONST(x) (x)

#  define OMNI_REQUIRES_FQ_BASE_CTOR

#elif defined(__KCC)
// Kai C++
#  define HAS_Cplusplus_Namespace
#  define HAS_Std_Namespace
#  define HAS_Cplusplus_Bool

#elif defined(__sgi)

#  if _COMPILER_VERSION >= 721
#    define HAS_Cplusplus_Namespace
#    define HAS_Cplusplus_Bool
#    define HAS_Cplusplus_const_cast
#    define OMNI_REQUIRES_FQ_BASE_CTOR
#    define HAS_LongLong
#    define HAS_LongDouble
#    define _CORBA_LONGLONG_DECL long long
#    define _CORBA_ULONGLONG_DECL unsigned long long
#    define _CORBA_LONGDOUBLE_DECL long double
#    define _CORBA_LONGLONG_CONST(x) (x##LL)
#  endif
#  if  _MIPS_SZINT == 64
#    define SIZEOF_INT 8
#  endif
#  if _MIPS_SZLONG == 64
#    define SIZEOF_LONG 8
#  endif
#  if _MIPS_SZPTR == 64
#    define SIZEOF_PTR 8
#  endif

#elif defined(__xlC__)
#  if (__xlC__ <= 0x0306)
#    undef HAS_Cplusplus_const_cast
#    undef HAS_Cplusplus_reinterpret_cast
#  elif (__xlC__ >= 0x0500) // added in xlC 5.0 (a.k.a. Visual Age 5.0)
#    define HAS_Cplusplus_Bool
#    define HAS_Cplusplus_Namespace
#    define HAS_Std_Namespace
#    define HAS_LongLong
#    define HAS_LongDouble
#    define _CORBA_LONGLONG_DECL long long
#    define _CORBA_ULONGLONG_DECL unsigned long long
#    define _CORBA_LONGDOUBLE_DECL long double
#    define _CORBA_LONGLONG_CONST(x) (x##LL)
#  endif

#elif defined(__hpux__)
// Recent versions of HP aCC (A01.18 and A.03.13) have an identifying macro.
// In the future, we should be able to remove the gcc test.
// In case this is an older compiler aCC, test if this is gcc, if not assume 
// it is aCC.
#  if defined(__HP_aCC) || !defined(__GNUG__)
#    define HAS_Cplusplus_Namespace
#    define HAS_Cplusplus_Bool
#    define HAS_LongLong
#    define _CORBA_LONGLONG_DECL   long long
#    define _CORBA_ULONGLONG_DECL  unsigned long long
#    define _CORBA_LONGLONG_CONST(x) (x##LL)
#  endif

#elif defined(__MWERKS__)
// Metrowerks CodeWarrior Pro 8 or later for Mac OS Classic or Carbon
#  define HAS_Cplusplus_Bool
#  define HAS_Cplusplus_Namespace
#  define HAS_Std_Namespace
#  define HAS_LongLong
#  define _CORBA_LONGLONG_DECL long long
#  define _CORBA_ULONGLONG_DECL unsigned long long
#  define _CORBA_LONGLONG_CONST(x) (x##LL)

#endif


//
// Processor dependencies
//

#if defined(__x86__)
#  define _OMNIORB_HOST_BYTE_ORDER_ 1
#  define SIZEOF_LONG_DOUBLE 12

#elif defined(__sparc__)
#  define _OMNIORB_HOST_BYTE_ORDER_ 0

#elif defined(__alpha__)
#  define _OMNIORB_HOST_BYTE_ORDER_ 1
#  ifndef __VMS
#    define SIZEOF_LONG 8
#    define SIZEOF_INT  4
#    define SIZEOF_PTR  8
#  endif

#elif defined(__hppa__)
#  define _OMNIORB_HOST_BYTE_ORDER_ 0

#elif defined(__powerpc__)
#  define _OMNIORB_HOST_BYTE_ORDER_ 0

#elif defined(__mips__)
#  define _OMNIORB_HOST_BYTE_ORDER_ 0

#elif defined(__s390__)
#  define _OMNIORB_HOST_BYTE_ORDER_ 0

#elif defined(__arm__)
#  define _OMNIORB_HOST_BYTE_ORDER_ 1

#elif defined(__m68k__)
#  define _OMNIORB_HOST_BYTE_ORDER_ 0

#elif defined(__vax__)
#  define _OMNIORB_HOST_BYTE_ORDER_ 1

#endif


//
// OS dependencies
//

#if defined(__linux__)
#  define OMNI_SOCKNAME_SIZE_T socklen_t
#  define HAVE_STRTOULL 1
#  ifndef HAVE_STD
#    define HAVE_STD 1
#  endif

#elif defined(__sunos__)
#  ifndef HAVE_STD
#    define HAVE_STD 1
#  endif
#  define HAVE_STRTOULL 1
#  define HAVE_ISNANORINF
#  define HAVE_NAN_H
#  if __OSVERSION__ == 4
#    define OMNI_SOCKNAME_SIZE_T int
#  elif __OSVERSION__ == 5 || __OSVERSION__ == 6
#    define OMNI_SOCKNAME_SIZE_T size_t
#  else
#    define OMNI_SOCKNAME_SIZE_T socklen_t
#  endif
#  if __OSVERSION__ == 5 && (!defined(__GNUG__) || __GNUG__ < 3)
#    define NEED_GETHOSTNAME_PROTOTYPE
#  endif

#elif defined(__hpux__)

#elif defined(__irix__)
#  undef HAVE_GETHOSTNAME

#elif defined(__freebsd__)
#  define HAVE_STRTOUQ 1
#  if __OSVERSION__ >= 4
#    define OMNI_SOCKNAME_SIZE_T socklen_t
#  endif

#elif defined(__aix__)
#  define OMNI_SOCKNAME_SIZE_T size_t

#elif defined(__SINIX__)
#  undef GETTIMEOFDAY_TIMEZONE
#  define OMNI_SOCKNAME_SIZE_T size_t

#elif defined(__uw7__)
#  define OMNI_SOCKNAME_SIZE_T size_t

#elif defined(__darwin__)
#  define HAVE_STRTOUQ 1

#elif defined(__nextstep__)
#  undef HAVE_STRDUP
#  undef HAVE_UNAME
#  undef HAVE_SIGACTION
#  define HAVE_SIGVEC

#elif defined(__atmos__)
#  define NO_FLOAT

#elif defined(__VMS)
#  define OMNI_SOCKNAME_SIZE_T size_t
#  undef HAVE_STRDUP
#  if __VMS_VER < 70000000
#    undef HAVE_GETOPT
#    undef HAVE_STRCASECMP
#    undef HAVE_STRNCASECMP
#    undef HAVE_GETTIMEOFDAY
#  endif

#elif defined(__WIN32__)
#  define SIZEOF_WCHAR 2
#  undef HAVE_UNISTD_H
#  undef HAVE_GETOPT
#  undef HAVE_GETTIMEOFDAY
#  undef HAVE_GETPID
#  undef HAVE_SIGNAL_H
#  undef HAVE_SIGACTION
#  undef HAVE_STRCASECMP
#  undef HAVE_STRNCASECMP
#  undef HAVE_UNAME
#  undef HAVE_GETHOSTNAME
#  undef HAVE_POLL

#ifdef __MINGW32__
#  define HAVE_STRCASECMP
#  define HAVE_STRNCASECMP
#  define HAVE_VPRINTF
#endif

#elif defined(__vxWorks__)
#  undef HAVE_GETTIMEOFDAY
#  undef HAVE_STRCASECMP
#  undef HAVE_STRNCASECMP

#elif defined(__macos__)
#  define SIZEOF_WCHAR 2
#  define OMNI_SOCKNAME_SIZE_T socklen_t
#  define HAVE_STRTOULL 1

#endif



//
// Default flag values if not already overridden above
//

#ifndef SIZEOF_BOOL
#define SIZEOF_BOOL 1
#endif

#ifndef SIZEOF_LONG
#define SIZEOF_LONG 4
#endif

#ifndef SIZEOF_INT
#define SIZEOF_INT 4
#endif

#ifndef SIZEOF_PTR
#define SIZEOF_PTR  4
#endif

#ifndef SIZEOF_LONG_DOUBLE
#define SIZEOF_LONG_DOUBLE 16
#endif

#ifndef SIZEOF_WCHAR
#  define SIZEOF_WCHAR 4
#endif

#ifndef OMNI_SOCKNAME_SIZE_T
#  define OMNI_SOCKNAME_SIZE_T int
#endif

#ifndef _OMNIORB_HOST_BYTE_ORDER_
# error "The byte order of this platform is unknown"
#endif



#endif // __CORBA_SYSDEP_TRAD_H__
