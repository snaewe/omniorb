// -*- Mode: C++; -*-
//                            Package   : omniORB2
// CORBA_sysdep.h             Created on: 30/1/96
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 1996, 1997 Olivetti & Oracle Research Laboratory
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

#elif defined(__DECCXX)
// DEC C++ compiler
#define NEED_DUMMY_RETURN

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
#  endif

#elif defined(__SUNPRO_CC) 
// SUN C++ compiler
#if __SUNPRO_CC < 0x420
#define NEED_DUMMY_RETURN
#endif

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

#elif defined(__BCPLUSPLUS__)
#define HAS_Cplusplus_Namespace

#endif

#if defined(__hpux__)
// HP aCC does not define a macro to identify itself.
// Test if this is gcc, if not assume it is aCC.
#if !defined(__GNUG__)
#define NEED_DUMMY_RETURN
#endif

// Do we really need to include this here?   -SLL
#include <stdio.h>
#undef __ptr

#endif


#if defined(arm)
// We don't have support for float.
#   define NO_FLOAT
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
#define _OMNIORB_HOST_BYTE_ORDER_ 1
#elif defined(__alpha__) && !defined(__VMS)
#define _OMNIORB_HOST_BYTE_ORDER_ 1
#define _HAS_SIGNAL 1
#elif defined(__sunos__) && defined(__sparc__)
#define _OMNIORB_HOST_BYTE_ORDER_ 0
#define _HAS_SIGNAL 1
#if __OSVERSION__ == 5
#define NEED_GETHOSTNAME_PROTOTYPE
#endif
#elif defined(__x86__)
#define _OMNIORB_HOST_BYTE_ORDER_ 1
#if !defined(__WIN32__)
#define _HAS_SIGNAL 1
#endif
#elif defined(__aix__) && defined(__powerpc__)
#define _OMNIORB_HOST_BYTE_ORDER_ 0
#define _HAS_SIGNAL 1
#elif defined(__hpux__) && defined(__hppa__)
#define _OMNIORB_HOST_BYTE_ORDER_ 0
#define _HAS_SIGNAL 1
#elif defined(__m68k__) && defined(__nextstep__)
#define _OMNIORB_HOST_BYTE_ORDER_ 0
#define _HAS_SIGNAL 1
#define _USE_MACH_SIGNAL 1
#define _NO_STRDUP 1
#define _USE_GETHOSTNAME 1
#elif defined(__VMS)
#define _OMNIORB_HOST_BYTE_ORDER_ 1
#if __VMS_VER >= 70000000
#define _HAS_SIGNAL 1
#else
#include <string.h>
// Pre 7.x VMS does not have strdup.
inline static char *
strdup (char* str)
{
  char *newstr;

  newstr = (char *) malloc (strlen (str) + 1);
  if (newstr)
    strcpy (newstr, str);
    return newstr;
}
#endif
#elif defined(__SINIX__)
#define _OMNIORB_HOST_BYTE_ORDER_ 0
#define _HAS_SIGNAL 1
#else
#error "The byte order of this platform is unknown"
#endif


#if defined(_MSC_VER)

// _OMNIORB2_DLL is defined when the omniORB2 dll is compiled.
// _WINSTATIC    is defined when an application is compiled to use the
//               static library.
//
// To package stubs into dlls:
//   1. Make sure that the cpp macro USE_stub_in_nt_dll is defined before
//      the stub header (.hh) is included.
//   2. Define the cpp macro _OMNIORB2_STUB_DLL when the stub _SK.cc is
//      compiled.
//   3. A .def file has to be created to export the symbols in the dll.
//      The .def file can be generated automatically based on the output
//      of dumpbin. For an example, look at how the omniORB2 dll is created.
//
// To use stubs that has been packaged into dlls:
//   1. Make sure that the cpp macro USE_stub_in_nt_dll is defined before
//      the stub header (.hh) is included.
//
// Use _OMNIORB_NTDLL_IMPORT to ensure that MSVC++ use the correct linkage
// for constants and variables exported by a DLL.

#  if defined(_OMNIORB2_DLL) && defined(_WINSTATIC)

#    error "Both _OMNIORB2_DLL and _WINSTATIC are defined."

#  elif defined(_OMNIORB2_DLL)

#    define _OMNIORB_NTDLL_IMPORT

#  elif defined(_WINSTATIC)

#    define _OMNIORB_NTDLL_IMPORT

#  elif defined(_OMNIORB2_STUB_DLL)

#    define _OMNIORB_NTDLL_IMPORT

#  else

#    define _OMNIORB_NTDLL_IMPORT __declspec(dllimport)

#  endif

#else

   // For non-MSVC++ compiler, this macro expands to nothing.
#  define _OMNIORB_NTDLL_IMPORT

#endif

#if defined(_MSC_VAR) && defined(_DEBUG)
// The type name instantiated from the sequence templates could exceeds the
// 255 char limit of the debug symbol names. It is harmless except that one
// cannot read their values with the debugger. Disable the warning about
// the symbol name truncation.
#pragma warning(disable: 4786)
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
#define _CORBA_MODULE_VAR extern _LC_attr
#else
#error "Name conflict: _CORBA_MODULE_VAR is already defined."
#endif

#ifndef _CORBA_GLOBAL_VAR
#define _CORBA_GLOBAL_VAR extern _LC_attr
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

// Integral and enumuration constants are declared in the stub headers as:
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
// MSVC++ 5.0 is somewhere between FD and ARM.
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
#  if !defined(_MSC_VER) || _MSC_VER > 1199
#    define _init_in_cldecl_(x) x
#  else
#    define _init_in_cldecl_(x) 
#  endif
#else
#error "Name conflict: _init_in_cldecl_ is already defined."
#endif

#ifndef _init_in_cldef_
#  if !defined(_MSC_VER) || _MSC_VER > 1199
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
#define _CORBA_MODULE_VAR static _LC_attr
#else
#error "Name conflict: _CORBA_MODULE_VAR is already defined."
#endif

#ifndef _CORBA_GLOBAL_VAR
#define _CORBA_GLOBAL_VAR extern _LC_attr
#else
#error "Name conflict: _CORBA_GLOBAL_VAR is already defined."
#endif

#ifndef _CORBA_MODULE_VARINT
#define _CORBA_MODULE_VARINT static _LC_attr
#else
#error "Name conflict: _CORBA_MODULE_INTCONST is already defined."
#endif

#ifndef _CORBA_GLOBAL_VARINT
#define _CORBA_GLOBAL_VARINT extern _LC_attr
#else
#error "Name conflict: _CORBA_GLOBAL_INTCONST is already defined."
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
 
// This implementation *DOES NOT* support the Dynamic Invocation Interface
// and the Dynamic Skeleton Interface. Hence some of the psuedo objects are
// not or only partially implemented. The declaration of the unimplemented
// classes and member functions are excluded by absence of the following
// macro. Uncomment the following line to make these declarations visible.

//#define SUPPORT_DII

#ifndef USE_omniORB_logStream
// New stubs use omniORB::logStream. Old stubs still need cerr. Include
// the necessary iostream header if that is the case.
#include <iostream.h>
#endif

#endif // __CORBA_SYSDEP_H__
