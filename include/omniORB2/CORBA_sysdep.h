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
 Revision 1.39.2.1  1999/09/22 13:44:40  sll
 Updated for AIX 4.2 and 4.3

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


#define HAS_Cplusplus_const_cast
// Unset this define if the compiler does not support const_cast<T*>

#if defined(__GNUG__)
// GNU G++ compiler

#  if defined(__alpha) && !defined(__VMS)
#     define SIZEOF_LONG 8
#     define SIZEOF_INT  4
#     define SIZEOF_PTR  8
#  endif

// Activate temporary workaround for a bug in post-1.0 egcs snapshots
// No side-effect on other gcc compilers. May be removed in future once
// the bug is fixed.
#define EGCS_WORKAROUND

// Minor version number 91 is for egcs version 1.*  Some older
// versions of 1.* may not support namespaces properly - this is
// only tested for egcs 1.1.1
#  if __GNUC_MINOR__ >= 91
#     define HAS_Cplusplus_Namespace
#     define HAS_Cplusplus_Bool
#  endif

#elif defined(__DECCXX)
// DEC C++ compiler

#  if defined(__alpha) && !defined(__VMS)
#     define SIZEOF_LONG 8
#     define SIZEOF_INT  4
#     define SIZEOF_PTR  8
#  endif
#  if __DECCXX_VER >= 60000000
#     ifndef NO_Cplusplus_Bool
#       define HAS_Cplusplus_Bool
#     endif
#     define HAS_Cplusplus_Namespace
#     define HAS_Std_Namespace
#     define HAS_pch
// Uncomment the following lines to enable the use of namespace with cxx v5.6
// Notice that the source code may have to be patched to compile.
//#  elif __DECCXX_VER >= 50600000
//#     define HAS_Cplusplus_Namespace
//#     define NEED_DUMMY_RETURN
#  else
#     define NEED_DUMMY_RETURN
#     undef  HAS_Cplusplus_const_cast
#  endif

#elif defined(__SUNPRO_CC) 
// SUN C++ compiler
#  if __SUNPRO_CC < 0x420
#    define NEED_DUMMY_RETURN
#  elif __SUNPRO_CC >= 0x500
#    define HAS_Cplusplus_Namespace
#    define HAS_Std_Namespace
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

#elif defined(__BCPLUSPLUS__)
#define HAS_Cplusplus_Namespace

#elif defined(__sgi)

#if _COMPILER_VERSION >= 721
#define HAS_Cplusplus_Namespace
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

#elif defined(__xlC__) && (__xlC__ <= 0x0306)

#undef HAS_Cplusplus_const_cast

#endif

#if defined(__hpux__)
// Recent versions of HP aCC (A01.18 and A.03.13) have an identifying macro.
// In the future, we should be able to remove the gcc test.
// In case this is an older compiler aCC, test if this is gcc, if not assume 
// it is aCC.
#if defined(__HP_aCC) || !defined(__GNUG__)
#define NEED_DUMMY_RETURN
#define HAS_Cplusplus_Namespace
#define HAS_Cplusplus_Bool
#endif

#if __OSVERSION__ < 11
// Do we really need to include this here?   -SLL
//  - not for HPUX 11.0
//    need someone to check this for HPUX 10.20
#include <stdio.h>
#undef __ptr
#endif

#endif


#if defined(arm)
// We don't have support for float.
#  define NO_FLOAT
#endif

#ifndef __cplusplus
#error "Cannot use this C++ header file for non C++ programs."
#endif

#ifdef Status
#error "Name conflict: Status is defined as a macro in a header file include before this."
// X11's Xlib.h (and may be others) define Status as a macro. This name
// conflicts with the Status type defined in the CORBA namespace.
// To remove this error, make sure that the offending header file is included
// after omniORB2/CORBA.h.
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
# if __OSVERSION__ == 5
#  define NEED_GETHOSTNAME_PROTOTYPE
# endif
#elif defined(__x86__)
# define _OMNIORB_HOST_BYTE_ORDER_ 1
# if !defined(__WIN32__)
#  define _HAS_SIGNAL 1
# endif
#elif defined(__aix__) && defined(__powerpc__)
# define _OMNIORB_HOST_BYTE_ORDER_ 0
# define _HAS_SIGNAL 1
#elif defined(__hpux__) && defined(__hppa__)
# define _OMNIORB_HOST_BYTE_ORDER_ 0
# define _HAS_SIGNAL 1
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
#  include <string.h>
#  include <stdlib.h>
// Pre 7.x VMS does not have strdup.
inline static char*
strdup (char* str)
{
  char* newstr;

  newstr = (char*) malloc(strlen(str) + 1);
  if( newstr )  strcpy(newstr, str);
  return newstr;
}
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
#else
# error "The byte order of this platform is unknown"
#endif


#if defined(_MSC_VER)

//
// _OMNIORB2_LIBRARY         is defined when the omniORB2 library is compiled.
// _OMNIORB2_DYNAMIC_LIBRARY is defined when the dynamic library is compiled.
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

#ifndef USE_omniORB_logStream
// New stubs use omniORB::logStream. Old stubs still need cerr. Include
// the necessary iostream header if that is the case.
#include <iostream.h>
#endif

// #define ENABLE_CLIENT_IR_SUPPORT
// Define ENABLE_CLIENT_IR_SUPPORT to use as client to an Interface Repository


#endif // __CORBA_SYSDEP_H__
