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

#  if defined(__alpha)
#     define SIZEOF_LONG 8
#     define SIZEOF_INT  4
#     define SIZEOF_PTR  8
#  endif

#elif defined(__DECCXX)
// DEC C++ compiler

#  if defined(__alpha)
#     define SIZEOF_LONG 8
#     define SIZEOF_INT  4
#     define SIZEOF_PTR  8
#  endif

#elif defined(__SUNPRO_CC)
// SUN C++ compiler

#elif defined(_MSC_VER)
//  Microsoft Visual C++ compiler
 
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

#ifndef _CORBA_MODULE
#define _CORBA_MODULE class
#endif

#ifndef _CORBA_MODULE_PUBLIC
#define _CORBA_MODULE_PUBLIC public:
#endif

#if defined(__arm__) && defined(__atmos__)
#define _OMNIORB_HOST_BYTE_ORDER_ 1
#elif defined(__alpha__)
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
#else
#error "The byte order of this platform is unknown"
#endif


// Define macro for NT DLL import/export:
// Note that if an application is being compiled (using MSVC++ on NT or '95) 
// to use the static library, the macro _WINSTATIC should be defined.

#if defined(__WIN32__) && defined(_MSC_VER)

#if defined(_OMNIORB2_DLL) && defined(_WINSTATIC)
#error "Both _OMNIORB2_DLL and _WINSTATIC are defined."
#elif defined(_OMNIORB2_DLL)
#define _OMNIORB2_NTDLL_ __declspec(dllexport) 
#pragma warning(disable: 4251)
// Disable this warning, as myPrincipalID is defined by a template, which
// can't be exported using __declspec
#elif !defined(_WINSTATIC)
#define _OMNIORB2_NTDLL_ __declspec(dllimport)
#pragma warning(disable: 4251)
// Disable this warning, as myPrincipalID is defined by a template, which
// can't be imported using __declspec
#elif defined(_WINSTATIC)
#define _OMNIORB2_NTDLL_
#endif 
 // _OMNIORB2_DLL && _WINSTATIC

#else
#define _OMNIORB2_NTDLL_
#endif
   // __WIN32__ && _MSC_VER
 
// This implementation *DOES NOT* support the Dynamic Invocation Interface
// and the Dynamic Skeleton Interface. Hence some of the psuedo objects are
// not or only partially implemented. The declaration of the unimplemented
// classes and member functions are excluded by absence of the following
// macro. Uncomment the following line to make these declarations visible.

//#define SUPPORT_DII

#endif // __CORBA_SYSDEP_H__
