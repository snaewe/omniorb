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
//	*** PROPRIETORY INTERFACE ***
//
// 	Define a set of flags in C++ macros. These flags provide information
//      about the system and the compiler used.
// 

/*
 $Log$
 Revision 1.2.2.17  2002/01/02 18:13:43  dpg1
 Platform fixes/additions.

 Revision 1.2.2.16  2001/11/14 17:13:41  dpg1
 Long double support.

 Revision 1.2.2.15  2001/11/06 15:41:35  dpg1
 Reimplement Context. Remove CORBA::Status. Tidying up.

 Revision 1.2.2.14  2001/10/17 16:26:50  dpg1
 Support for Sun CC > 5 in 4.x compatibility mode.

 Revision 1.2.2.13  2001/08/15 10:14:41  dpg1
 Remove obsolete USE_omniORB_logStream.

 Revision 1.2.2.12  2001/06/21 11:35:17  sll
 Define size of wchar_t for freebsd and darwin.

 Revision 1.2.2.11  2001/06/20 18:39:03  sll
 On solaris with gcc 3.0, there is no need to provide a prototype for
 gethostname.

 Revision 1.2.2.10  2001/06/15 10:23:40  sll
 Added wchar size for HPUX.

 Revision 1.2.2.9  2001/06/08 17:12:08  dpg1
 Merge all the bug fixes from omni3_develop.

 Revision 1.2.2.8  2001/06/07 16:26:10  dpg1
 __WIN32__ misspelt as __win32__

 Revision 1.2.2.7  2001/04/18 17:50:45  sll
 Big checkin with the brand new internal APIs.
 Scoped where appropriate with the omni namespace.

 Revision 1.2.2.6  2000/11/15 19:16:30  sll
 Added provision to override default mapping of CORBA::WChar to C++ wchar_t.

 Revision 1.2.2.5  2000/11/09 12:27:49  dpg1
 Huge merge from omni3_develop, plus full long long from omni3_1_develop.

 Revision 1.2.2.4  2000/11/03 19:00:26  sll
 Removed Suppress_Spurious_gcc_Warnings cpp macro.

 Revision 1.2.2.3  2000/10/27 15:42:03  dpg1
 Initial code set conversion support. Not yet enabled or fully tested.

 Revision 1.2.2.2  2000/10/03 17:34:47  sll
 Temporary debugging #define for gcc. To be removed before final release.

 Revision 1.2.2.1  2000/07/17 10:35:33  sll
 Merged from omni3_develop the diff between omni3_0_0_pre3 and omni3_0_0.

 Revision 1.3  2000/07/13 15:26:06  dpg1
 Merge from omni3_develop for 3.0 release.

 Revision 1.1.2.14  2000/07/07 10:31:33  sll
 DEC C++ 6.{0,1} on Digital Unix needs OMNI_REQUIRES_FQ_BASE_CTOR.

 Revision 1.1.2.13  2000/07/07 10:18:33  sll
 Fixed unbalanced #if #endif.

 Revision 1.1.2.12  2000/07/05 11:13:56  dpg1
 Untested support for AIX with xlC 5.

 Revision 1.1.2.11  2000/06/30 14:12:07  dpg1
 Minor fixes for FreeBSD.

 Revision 1.1.2.10  2000/06/27 16:23:24  sll
 Merged OpenVMS port.

 Revision 1.1.2.9  2000/03/23 16:27:47  djr
 Added NEED_DUMMY_RETURN to egcs/gcc.

 Revision 1.1.2.8  2000/03/10 16:45:52  dpg1
 MSVC++ can't catch exceptions by base class.

 Revision 1.1.2.7  2000/02/23 12:57:15  dpg1
 Some support for NextStep x86.

 Revision 1.1.2.6  2000/02/04 14:48:09  djr
 Added a couple of feature test macros.

 Revision 1.1.2.5  2000/01/27 11:11:24  djr
 Correction.

 Revision 1.1.2.4  2000/01/27 10:55:44  djr
 Mods needed for powerpc_aix.  New macro OMNIORB_BASE_CTOR to provide
 fqname for base class constructor for some compilers.

 Revision 1.1.2.3  1999/10/21 10:59:20  djr
 *** empty log message ***

 Revision 1.1.2.2  1999/09/30 11:49:26  djr
 Implemented catching user-exceptions in GIOP_S for all compilers.

 Revision 1.1.2.1  1999/09/24 09:51:38  djr
 Moved from omniORB2 + some new files.

 Revision 1.39  1999/08/30 18:43:34  sll
 Removed ENABLE_CLIENT_IR_SUPPORT as the default define. Application code
 should define this macro if the code is to act as a client to an
 interface repository.

 Revision 1.38  1999/08/15 13:52:40  sll
 Updated compiler defines for SGI and AIX.

 Revision 1.37  1999/06/26 17:53:40  sll
 Use namespace if compiler is SGI MIPSpro 7.2.1 or HP aCC.
 Added support for Irix 6.5 in 64bit mode.

 Revision 1.36  1999/06/18 21:12:16  sll
 Updated copyright notice.

 Revision 1.35  1999/06/18 20:35:41  sll
 Replaced _LC_attr with _core_attr and _dyn_attr.

 Revision 1.34  1999/06/02 16:40:56  sll
 Enable IR client support on all platforms

 Revision 1.33  1999/05/31 14:07:34  sll
 Added flags for SUN C++ 5.0.

 Revision 1.32  1999/05/26 12:41:51  sll
 ENABLE_CLIENT_IR_SUPPORT again.

 Revision 1.31  1999/05/26 12:23:11  sll
 Enable IR client support when namespace is supported.

 Revision 1.30  1999/04/06 08:30:03  djr
 MSVC 6.0 is also not FD compliant regarding declaration of static const
 integral and enum types.

 Revision 1.29  1999/01/07 18:17:51  djr
 Enable namespaces on egcs 1.1.1
 MSVC and VMS platforms do not have strcasecmp and strncasecmp.

 Revision 1.28  1999/01/05 12:31:14  sll
 Correct typo in #error messages.

 Revision 1.27  1998/10/20 15:58:58  sll
 Added support for Alpha NT.

 Revision 1.26  1998/10/08 10:53:19  sll
 Disable MSVC++ warnings C4768 and C4250.

 Revision 1.25  1998/09/23 15:25:51  sll
 Workaround for SUN C++ compiler Template.DB code regeneration bug.

 Revision 1.24  1998/08/22 12:52:13  sll
 Added sysconfig for Irix + SGI C++ compiler.

 Revision 1.23  1998/08/19 16:03:43  sll
 Updated section for DEC C++ and MSVC++ to remove NO_Koenig_Lookup because
 this is no longer necessary with the new way of generating binary
 operators <<= and >>=.

 Revision 1.22  1998/08/15 14:23:26  sll
 Added macro No_Koenig_Lookup to MSVC++ and DEC C++ 6.0
 Remove NEED_DUMMY_RETURN if the compiler is DEC C++ > 5.7

 Revision 1.21  1998/08/14 13:56:21  sll
 Added HAS_pch if the compiler is DEC C++ v6.0

 Revision 1.20  1998/08/11 11:44:42  sll
 Added #include <stdlib.h> for pre 7.x VMS platform.

 Revision 1.19  1998/04/18 10:06:25  sll
 Added section for Borland C++. (Not actually working yet.)

 Revision 1.18  1998/04/09 19:17:59  sll
 Extra macros defined for specifying initializers in the declaration
 of integral constants.

 Revision 1.17  1998/04/08 16:11:20  sll
 Added support for Reliant UNIX 5.43.

 Revision 1.16  1998/04/07 20:07:06  sll
 Added the use of namespace.

 Revision 1.15  1998/03/25 14:24:12  sll
 Added #define EGCS_WORKAROUND for gcc compiler. This activate a
 workaround for a bug in post-1.0 egcs snapshots. Can be removed if
 the bug is fixed in future version.

 * Revision 1.14  1998/01/20  16:45:57  sll
 * Added support for OpenVMS.
 *
 Revision 1.13  1997/12/09 20:40:21  sll
 Various platform specific updates.

 Revision 1.12  1997/08/21 22:22:12  sll
 Support for AIX.

 * Revision 1.11  1997/07/21  17:07:38  tjr
 * use new OS / processor macros
 *
 * Revision 1.10  1997/05/06  16:06:03  sll
 * Public release.
 *
*/

#ifndef __CORBA_SYSDEP_H__
#define __CORBA_SYSDEP_H__

#include <omniORB4/local_config.h>

#define HAS_Cplusplus_const_cast
// Unset this define if the compiler does not support const_cast<T*>

#define HAS_Cplusplus_catch_exception_by_base
// Unset this define if the compiler does not support catching
// exceptions by base class.


#if defined(__GNUG__)
// GNU G++ compiler

#  if defined(__alpha) && !defined(__VMS)
#     define SIZEOF_LONG 8
#     define SIZEOF_INT  4
#     define SIZEOF_PTR  8
#  endif

#  if defined(__x86__)
#     define SIZEOF_LONG_DOUBLE 12
#  endif

#  if __GNUG__ == 2 && __GNUC_MINOR__ == 7
#     undef HAS_Cplusplus_catch_exception_by_base
#  endif

// Activate temporary workaround for a bug in post-1.0 egcs snapshots
// No side-effect on other gcc compilers. May be removed in future once
// the bug is fixed.
#define EGCS_WORKAROUND

#define NEED_DUMMY_RETURN

// Minor version number 91 is for egcs version 1.*  Some older
// versions of 1.* may not support namespaces properly - this is
// only tested for egcs 1.1.1
#  if (__GNUG__ == 2 && (__GNUC_MINOR__ >= 91 || __GNUC_MINOR__ == 9)) || \
      (__GNUG__ >= 3)
#     define HAS_Cplusplus_Namespace
#     define HAS_Cplusplus_Bool
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

#  if defined(__alpha) && !defined(__VMS)
#     define SIZEOF_LONG 8
#     define SIZEOF_INT  4
#     define SIZEOF_PTR  8
#  endif
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
//    Work-around for OpenVMS VAX Compaq C++ 5.6 compiler problem with
//    %CXX-W-CANTCOMPLETE messages.  Note that this cannot be disabled with a
//    compiler switch if the message occurs in a template instantiation (but
//    this pragma wasn't implemented until 6.something on Unix).
#     ifdef __VMS
#       pragma message disable CANTCOMPLETE
#     endif
#     define NEED_DUMMY_RETURN
#     undef  HAS_Cplusplus_const_cast
#     define OMNI_REQUIRES_FQ_BASE_CTOR
#     define OMNI_OPERATOR_REFPTR_REQUIRES_TYPEDEF
#     define OMNI_PREMATURE_INSTANTIATION
//    Extra macros from the Compaq C++ 5.x patch (in <top>/patches/) to be
//    added here
#     ifndef OMNI_OPERATOR_REFPTR
#       error "Patch for Compaq C++ 5.x has not been applied."
#     endif

#  endif

#elif defined(__SUNPRO_CC) 
// SUN C++ compiler
#  if __SUNPRO_CC < 0x420
#    define NEED_DUMMY_RETURN
#  elif __SUNPRO_CC >= 0x500
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
#  ifdef __x86__
#    define SIZEOF_LONG_DOUBLE 12
#  endif

// XXX
// This is a hack to work around a bug in SUN C++ compiler (seen on 4.2).
// When instantiating templates, the compiler may generate code in Template.DB.
// It stores in the directory the dependency and compiler options to
// regenerate the template code if necessary. Unfortunately, it stores the
// option -D__OSVERSION__=5 as -D__OSVERSION__='5'. This of course causes
// the compilation to fail because the source code assume that the CPP macro
// __OSVERSION__ is an interger. The following hack detects this condition
// and revert the macro to its expected form.
#  ifdef __OSVERSION__
#    if __OSVERSION__ != 5
#       if __OSVERSION__ == '5'
#          undef __OSVERSION__
#          define __OSVERSION__ 5
#       endif
#    endif
#  endif

#elif defined(_MSC_VER)
//  Microsoft Visual C++ compiler
#define NEED_DUMMY_RETURN
#if _MSC_VER >= 1000
#  ifndef NO_Cplusplus_Bool
#    define HAS_Cplusplus_Bool
#  endif
#define HAS_Cplusplus_Namespace
#define HAS_Std_Namespace
#endif
#define _HAS_NOT_GOT_strcasecmp
#define _HAS_NOT_GOT_strncasecmp

#define HAS_LongLong
#define _CORBA_LONGLONG_DECL   __int64
#define _CORBA_ULONGLONG_DECL  unsigned __int64
#define _CORBA_LONGLONG_CONST(x) (x)


#elif defined(__BCPLUSPLUS__)
#define HAS_Cplusplus_Namespace

#elif defined(__KCC)
// Kai C++
#define HAS_Cplusplus_Namespace
#define HAS_Std_Namespace
#define HAS_Cplusplus_Bool

#elif defined(__sgi)

#if _COMPILER_VERSION >= 721
#define HAS_Cplusplus_Namespace
#define HAS_Cplusplus_Bool
#define HAS_Cplusplus_const_cast
#define OMNI_REQUIRES_FQ_BASE_CTOR
#define HAS_LongLong
#define HAS_LongDouble
#define _CORBA_LONGLONG_DECL long long
#define _CORBA_ULONGLONG_DECL unsigned long long
#define _CORBA_LONGDOUBLE_DECL long double
#define _CORBA_LONGLONG_CONST(x) (x##LL)
#endif

#if  _MIPS_SZINT == 64
#     define SIZEOF_INT 8
#endif
 
#if _MIPS_SZLONG == 64
#     define SIZEOF_LONG 8
#endif
 
#if _MIPS_SZPTR == 64
#     define SIZEOF_PTR 8
#endif

#elif defined(__xlC__)
#  if (__xlC__ <= 0x0306)
#    undef HAS_Cplusplus_const_cast
#  elif (__xlC__ >= 0x0500) // added in xlC 5.0 (a.k.a. Visual Age 5.0)
#    define HAS_Cplusplus_Bool
#    define HAS_Cplusplus_Namespace
#    define HAS_Std_Namespace
#  endif

#elif defined(__hpux__)
// Recent versions of HP aCC (A01.18 and A.03.13) have an identifying macro.
// In the future, we should be able to remove the gcc test.
// In case this is an older compiler aCC, test if this is gcc, if not assume 
// it is aCC.
#if defined(__HP_aCC) || !defined(__GNUG__)
#define NEED_DUMMY_RETURN
#define HAS_Cplusplus_Namespace
#define HAS_Cplusplus_Bool
#define HAS_LongLong
#define _CORBA_LONGLONG_DECL   long long
#define _CORBA_ULONGLONG_DECL  unsigned long long
#define _CORBA_LONGLONG_CONST(x) (x##LL)

#endif

#endif


#if defined(arm)
// We don't have support for float.
#  define NO_FLOAT
#endif

#ifndef __cplusplus
#error "Cannot use this C++ header file for non C++ programs."
#endif

// Default flag values if not already overridden above

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

#ifndef _CORBA_WCHAR_DECL
#  define _CORBA_WCHAR_DECL wchar_t
   // Wide character size
#  if defined(__linux__) || defined(__sunos__) || defined(__freebsd__) || defined(__darwin__) || defined(__irix__)
#    define SIZEOF_WCHAR 4
#  elif defined(__WIN32__)
#    define SIZEOF_WCHAR 2
#  elif defined(__hpux__)
#    define SIZEOF_WCHAR 4
#  else
#    error "sizeof wchar_t not known for this platform"
#  endif

#else

// Define _CORBA_WCHAR_DECL to override the default mapping above.
// Instead of mapping to wchar_t, override the mapping with your own
// type declaration. 
// For instance, you want to use UTF16 and each wchar to be represented
// as a 16-bit entity. There is no guarantee that wchar_t would fit the bill,
// in fact, it may be a 32-bit entity!
//
// Should also define SIZEOF_WCHAR to indicate the size of the type you
// have chosen.
#ifndef SIZEOF_WCHAR
#    error "sizeof wchar_t not known for this platform"
#endif


#endif


#if defined(__arm__) && defined(__atmos__)
# define _OMNIORB_HOST_BYTE_ORDER_ 1
#elif defined(__alpha__) && !defined(__VMS)
# define _OMNIORB_HOST_BYTE_ORDER_ 1
# if !defined(__WIN32__)
#  define _HAS_SIGNAL 1
# endif
#elif defined(__sunos__) && defined(__sparc__)
# define _OMNIORB_HOST_BYTE_ORDER_ 0
# define _HAS_SIGNAL 1
# if __OSVERSION__ == 5 && (!defined(__GNUG__) || __GNUG__ < 3)
#  define NEED_GETHOSTNAME_PROTOTYPE
# endif
#elif defined(__x86__)
# define _OMNIORB_HOST_BYTE_ORDER_ 1
# if !defined(__WIN32__)
#  define _HAS_SIGNAL 1
# endif
# if defined(__nextstep__)
#  define _USE_MACH_SIGNAL 1
#  define _NO_STRDUP 1
#  define _USE_GETHOSTNAME 1
# endif
#elif defined(__s390__)
# define _OMNIORB_HOST_BYTE_ORDER_ 0
# define _HAS_SIGNAL 1
#elif defined(__aix__) && defined(__powerpc__)
# define _OMNIORB_HOST_BYTE_ORDER_ 0
# define _HAS_SIGNAL 1
# define OMNI_REQUIRES_FQ_BASE_CTOR 1
#elif defined(__hpux__) && defined(__hppa__)
# define _OMNIORB_HOST_BYTE_ORDER_ 0
# define _HAS_SIGNAL 1
#if __OSVERSION__ < 11
// Do we really need to include this here?   -SLL
//  - not for HPUX 11.0
//    need someone to check this for HPUX 10.20
#include <stdio.h>
#undef __ptr
#endif
#elif defined(__m68k__) && defined(__nextstep__)
# define _OMNIORB_HOST_BYTE_ORDER_ 0
# define _HAS_SIGNAL 1
# define _USE_MACH_SIGNAL 1
# define _NO_STRDUP 1
# define _USE_GETHOSTNAME 1
#elif defined(__VMS)
# define _OMNIORB_HOST_BYTE_ORDER_ 1
# if __VMS_VER >= 70000000
#  define _HAS_SIGNAL 1
# else
#  define _NO_STRDUP 1
#  define _HAS_NOT_GOT_strcasecmp
#  define _HAS_NOT_GOT_strncasecmp
# endif
#elif defined(__SINIX__)
# define _OMNIORB_HOST_BYTE_ORDER_ 0
# define _HAS_SIGNAL 1
#elif defined(__irix__)
# define _OMNIORB_HOST_BYTE_ORDER_ 0
# define _HAS_SIGNAL 1
# define _USE_GETHOSTNAME 1
#elif defined(__powerpc__)
# define _OMNIORB_HOST_BYTE_ORDER_ 0
#else
# error "The byte order of this platform is unknown"
#endif


#if defined(_MSC_VER)

//
// _OMNIORB_LIBRARY         is defined when the omniORB library is compiled.
// _OMNIORB_DYNAMIC_LIBRARY is defined when the dynamic library is compiled.
//  These are defined on the command line when compiling the libraries.
//
// _WINSTATIC                is defined when an application is compiled to
//                           use the static library.
//
// To package stubs into dlls:
//   1. Define the cpp macro _OMNIORB2_STUB_DLL when the stub _SK.cc is
//      compiled.
//   2. A .def file has to be created to export the symbols in the dll.
//      The .def file can be generated automatically based on the output
//      of dumpbin. For an example, look at how the omniORB2 dll is created.
//
// To use stubs that has been packaged into dlls:
//   1. Make sure that the cpp macro USE_stub_in_nt_dll is defined before
//      the stub header (.hh) is included.
//
// Use _OMNIORB_NTDLL_IMPORT to ensure that MSVC++ use the correct linkage
// for constants and variables exported by a DLL.
//

#  ifdef _WINSTATIC
#    define _OMNIORB_NTDLL_IMPORT
#  else
#    define _OMNIORB_NTDLL_IMPORT  __declspec(dllimport)
#  endif

#else

// For non-MSVC++ compiler, this macro expands to nothing.
#  define _OMNIORB_NTDLL_IMPORT

#endif

#if defined(_MSC_VER)
#if defined(_DEBUG)
// The type name instantiated from the sequence templates could exceeds the
// 255 char limit of the debug symbol names. It is harmless except that one
// cannot read their values with the debugger. Disable the warning about
// the symbol name truncation.
#pragma warning(disable: 4786)
#endif

// Disable warnings about a member function in a derived class overriding
// a member function in the base class.
#pragma warning(disable: 4250)
#endif

#ifdef HAS_Cplusplus_Namespace

#ifndef _CORBA_MODULE
#define _CORBA_MODULE namespace
#else
#error "Name conflict: _CORBA_MODULE is already defined."
#endif

#ifndef _CORBA_MODULE_BEG
#define _CORBA_MODULE_BEG {
#else
#error "Name conflict: _CORBA_MODULE_BEG is already defined."
#endif

#ifndef _CORBA_MODULE_END
#define _CORBA_MODULE_END }
#else
#error "Name conflict: _CORBA_MODULE_END is already defined."
#endif

#ifndef _CORBA_MODULE_OP
#define _CORBA_MODULE_OP
#else
#error "Name conflict: _CORBA_MODULE_OP is already defined."
#endif

#ifndef _CORBA_MODULE_FN
#define _CORBA_MODULE_FN
#else
#error "Name conflict: _CORBA_MODULE_FN is already defined."
#endif

#ifndef _CORBA_MODULE_VAR
#define _CORBA_MODULE_VAR extern
#else
#error "Name conflict: _CORBA_MODULE_VAR is already defined."
#endif

#ifndef _CORBA_GLOBAL_VAR
#define _CORBA_GLOBAL_VAR extern
#else
#error "Name conflict: _CORBA_GLOBAL_VAR is already defined."
#endif

#ifndef _CORBA_MODULE_VARINT
#define _CORBA_MODULE_VARINT
#else
#error "Name conflict: _CORBA_MODULE_VARINT is already defined."
#endif

#ifndef _CORBA_GLOBAL_VARINT
#define _CORBA_GLOBAL_VARINT
#else
#error "Name conflict: _CORBA_GLOBAL_VARINT is already defined."
#endif

#ifndef OMNI_NAMESPACE_BEGIN
#define OMNI_NAMESPACE_BEGIN(name) namespace name {
#else
#error "Name conflict: OMNI_NAME_SPACE_BEGIN is already defined."
#endif

#ifndef OMNI_NAMESPACE_END
#define OMNI_NAMESPACE_END(name) }
#else
#error "Name conflict: OMNI_NAME_SPACE_END is already defined."
#endif

#ifndef OMNI_USING_NAMESPACE
#define OMNI_USING_NAMESPACE(name) using namespace name;
#else
#error "Name conflict: OMNI_USING_NAMESPACE is already defined."
#endif

#ifndef _OMNI_NS
#define _OMNI_NS(x) omni::x
#else
#error "Name conflict: _OMNI_NS is already defined."
#endif

// Integral and enumeration constants are declared in the stub headers as:
//    e.g.  class A {
//              static const CORBA::Long AA _init_in_cldecl_( = 4 );
//          };
//          namespace B {
//              const CORBA::Long BB _init_in_decl_( = 5 );
//          };
// And defined in the SK.cc:
//    e.g.   const CORBA::Long A::AA _init_in_cldef_( = 4 );
//           _init_in_def_( const CORBA::Long B::BB = 5 );
//  
// Final Draft (FD) allows declaration of static const integral or enum type 
// be specified with a constant-initializer whereas ARM does not.
// The _init_in_decl_ and _init_in_def_ macros are defined so that the same 
// stub will compile on both FD and ARM compilers.
// MSVC++ 5.0 (and 6.0) is somewhere between FD and ARM.
//  _MSC_VER = 1100 for 5.0, 1200 for 6.0.
//
#ifndef _init_in_decl_
#define _init_in_decl_(x) x
#else
#error "Name conflict: _init_in_decl_ is already defined."
#endif

#ifndef _init_in_def_
#define _init_in_def_(x)
#else
#error "Name conflict: _init_in_def_ is already defined."
#endif

#ifndef _init_in_cldecl_
#  if !defined(_MSC_VER)
#    define _init_in_cldecl_(x) x
#  else
#    define _init_in_cldecl_(x) 
#  endif
#else
#error "Name conflict: _init_in_cldecl_ is already defined."
#endif

#ifndef _init_in_cldef_
#  if !defined(_MSC_VER)
#    define _init_in_cldef_(x)
#  else
#    define _init_in_cldef_(x) x 
#  endif
#else
#error "Name conflict: _init_in_cldef_ is already defined."
#endif

#else

#ifndef _CORBA_MODULE
#define _CORBA_MODULE class
#else
#error "Name conflict: _CORBA_MODULE is already defined."
#endif

#ifndef _CORBA_MODULE_BEG
#define _CORBA_MODULE_BEG { public:
#else
#error "Name conflict: _CORBA_MODULE_BEG is already defined."
#endif

#ifndef _CORBA_MODULE_END
#define _CORBA_MODULE_END };
#else
#error "Name conflict: _CORBA_MODULE_END is already defined."
#endif

#ifndef _CORBA_MODULE_OP
#define _CORBA_MODULE_OP friend
#else
#error "Name conflict: _CORBA_MODULE_OP is already defined."
#endif

#ifndef _CORBA_MODULE_FN
#define _CORBA_MODULE_FN static
#else
#error "Name conflict: _CORBA_MODULE_FN is already defined."
#endif

#ifndef _CORBA_MODULE_VAR
#define _CORBA_MODULE_VAR static
#else
#error "Name conflict: _CORBA_MODULE_VAR is already defined."
#endif

#ifndef _CORBA_GLOBAL_VAR
#define _CORBA_GLOBAL_VAR extern
#else
#error "Name conflict: _CORBA_GLOBAL_VAR is already defined."
#endif

#ifndef _CORBA_MODULE_VARINT
#define _CORBA_MODULE_VARINT static _core_attr
#else
#error "Name conflict: _CORBA_MODULE_VARINT is already defined."
#endif

#ifndef _CORBA_GLOBAL_VARINT
#define _CORBA_GLOBAL_VARINT extern _core_attr
#else
#error "Name conflict: _CORBA_GLOBAL_VARINT is already defined."
#endif

#ifndef OMNI_NAMESPACE_BEGIN
#define OMNI_NAMESPACE_BEGIN(name)
#else
#error "Name conflict: OMNI_NAME_SPACE_BEGIN is already defined."
#endif

#ifndef OMNI_NAMESPACE_END
#define OMNI_NAMESPACE_END(name)
#else
#error "Name conflict: OMNI_NAME_SPACE_END is already defined."
#endif

#ifndef OMNI_USING_NAMESPACE
#define OMNI_USING_NAMESPACE(name)
#else
#error "Name conflict: OMNI_USING_NAMESPACE is already defined."
#endif

#ifndef _OMNI_NS
#define _OMNI_NS(x) x
#else
#error "Name conflict: _OMNI_NS is already defined."
#endif

#ifndef _init_in_decl_
#define _init_in_decl_(x)
#else
#error "Name conflict: _init_in_decl_ is already defined."
#endif

#ifndef _init_in_def_
#define _init_in_def_(x) x
#else
#error "Name conflict: _init_in_def_ is already defined."
#endif

#ifndef _init_in_cldecl_
#define _init_in_cldecl_(x)
#else
#error "Name conflict: _init_in_cldecl_ is already defined."
#endif

#ifndef _init_in_cldef_
#define _init_in_cldef_(x) x
#else
#error "Name conflict: _init_in_cldef_ is already defined."
#endif


// Deprecated. Old stubs still need this. Should be removed.
#ifndef _CORBA_MODULE_PUBLIC
#define _CORBA_MODULE_PUBLIC public:
#endif


#endif // HAS_Cplusplus_Namespace


#ifdef OMNI_REQUIRES_FQ_BASE_CTOR
# define OMNIORB_BASE_CTOR(a)   a
#else
# define OMNIORB_BASE_CTOR(a)
#endif

#ifndef OMNI_OPERATOR_REFPTR
// Only used when the source tree is patched with DEC C++ 5.6 workarounds
#define OMNI_OPERATOR_REFPTR(T) inline operator T*&()
#endif

#ifndef OMNI_CONSTRTYPE_FIX_VAR_MEMBER
// Only used when the source tree is patched with DEC C++ 5.6 workarounds
#define OMNI_CONSTRTYPE_FIX_VAR_MEMBER(T) \
   typedef _CORBA_ConstrType_Fix_Var<T> _var_type;
#endif

#ifndef OMNI_CONSTRTYPE_FIX_VAR
// Only used when the source tree is patched with DEC C++ 5.6 workarounds
#define OMNI_CONSTRTYPE_FIX_VAR(T) typedef T::_var_type T##_var;
#endif

// #define ENABLE_CLIENT_IR_SUPPORT
// Define ENABLE_CLIENT_IR_SUPPORT to use as client to an Interface Repository


#endif // __CORBA_SYSDEP_H__
