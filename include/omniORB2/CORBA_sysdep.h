// -*- Mode: C++; -*-
//                            Package   : omniORB2
// CORBA_sysdep.h             Created on: 30/1/96
//                            Author    : Sai Lai Lo (sll)
//
// Copyright (C) Olivetti Research Limited, 1996
//
// Description:
//	*** PROPRIETORY INTERFACE ***
//
// 	Define a set of flags in C++ macros. These flags provide information
//      about the system and the compiler used.
// 

/*
 $Log$
 Revision 1.5  1997/03/14 10:19:10  sll
 Use namespace instead of class for modules if the compiler supports it.

 * Revision 1.4  1997/03/09  14:35:59  sll
 * Minor cleanup.
 *
 * Revision 1.3  1997/02/19  11:13:32  ewc
 * Added support for Windows NT.
 *
 * Revision 1.2  1997/01/08  18:56:09  ewc
 * Added entry for NT byte - order
 *
 * Revision 1.1  1997/01/08  17:28:30  sll
 * Initial revision
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

#define _CORBA_MODULE namespace
#define _CORBA_MODULE_PUBLIC
#endif

#if defined(arm)
// We don't have support for float.
#   define NO_FLOAT
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
#elif defined(_X86_) || defined(__i386__)
#define _OMNIORB_HOST_BYTE_ORDER_ 1
#if !defined(__NT__)
#define _HAS_SIGNAL 1
#endif
#else
#error "The byte order of this platform is unknown"
#endif


// This implementation *DOES NOT* support the Dynamic Invocation Interface
// and the Dynamic Skeleton Interface. Hence some of the psuedo objects are
// not or only partially implemented. The declaration of the unimplemented
// classes and member functions are excluded by absence of the following
// macro. Uncomment the following line to make these declarations visible.

//#define SUPPORT_DII

#endif // __CORBA_SYSDEP_H__
