// -*- Mode: C++; -*-
//                            Package   : omniORB2
// CORBA_sysdep.h             Created on: 30/1/96
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 2003-2008 Apasphere Ltd
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
// 	Define a set of flags in C++ macros. These flags provide information
//      about the system and the compiler used.
// 

/*
 $Log$
 Revision 1.5.2.11  2009/05/06 16:16:14  dgrisby
 Update lots of copyright notices.

 Revision 1.5.2.10  2008/03/24 20:17:20  dgrisby
 Sun's compiler doesn't like the <: in const_cast<::CORBA...>.

 Revision 1.5.2.9  2007/02/05 17:39:17  dgrisby
 Later Sun compilers require a dummy return.

 Revision 1.5.2.8  2007/01/12 10:19:51  dgrisby
 Support for MontaVista ARM Linux.

 Revision 1.5.2.7  2006/04/28 18:40:46  dgrisby
 Merge from omni4_0_develop.

 Revision 1.5.2.6  2006/01/10 12:24:04  dgrisby
 Merge from omni4_0_develop pre 4.0.7 release.

 Revision 1.5.2.5  2005/11/17 17:03:27  dgrisby
 Merge from omni4_0_develop.

 Revision 1.5.2.4  2005/07/22 17:18:40  dgrisby
 Another merge from omni4_0_develop.

 Revision 1.5.2.3  2005/07/11 17:47:31  dgrisby
 VMS changes from Bruce Visscher.

 Revision 1.5.2.2  2005/01/06 23:08:07  dgrisby
 Big merge from omni4_0_develop.

 Revision 1.5.2.1  2003/03/23 21:04:22  dgrisby
 Start of omniORB 4.1.x development branch.

 Revision 1.2.2.23  2003/03/02 17:10:40  dgrisby
 AIX patches integrated in main tree.

 Revision 1.2.2.22  2003/02/17 02:03:07  dgrisby
 vxWorks port. (Thanks Michael Sturm / Acterna Eningen GmbH).

 Revision 1.2.2.21  2003/01/16 12:47:08  dgrisby
 Const cast macro. Thanks Matej Kenda.

 Revision 1.2.2.20  2003/01/16 11:08:26  dgrisby
 Patches to support Digital Mars C++. Thanks Christof Meerwald.

 Revision 1.2.2.19  2002/02/18 11:59:12  dpg1
 Full autoconf support.

 Revision 1.2.2.18  2002/01/15 16:38:09  dpg1
 On the road to autoconf. Dependencies refactored, configure.ac
 written. No makefiles yet.


 Log truncated.

*/

#ifndef __CORBA_SYSDEP_H__
#define __CORBA_SYSDEP_H__

#ifndef __cplusplus
#error "Cannot use this C++ header file for non C++ programs."
#endif


#include <omniORB4/local_config.h>


//
// Most system dependencies either come from autoconf or from
// hard-coded values in CORBA_sysdep_trad.h
//

#ifndef OMNI_CONFIG_EXTERNAL
#  include <omniconfig.h>
#endif

#ifdef OMNI_CONFIG_TRADITIONAL
#  include <omniORB4/CORBA_sysdep_trad.h>
#else
#  include <omniORB4/CORBA_sysdep_auto.h>
#endif


//
// Dependencies that are always hard-coded
//

#if defined(__WIN32__)
#  define NTArchitecture 1
#elif defined(__VMS)
#else
#  define UnixArchitecture 1
#endif

//
// Processor dependencies
//

#if defined(__arm__)
#  if defined(__armv5teb__)
#    define NO_OMNI_MIXED_ENDIAN_DOUBLE
#  else
#    define OMNI_MIXED_ENDIAN_DOUBLE
#  endif
#endif


//
// Macro to provide const_cast functionality on all platforms.
//
#ifdef HAS_Cplusplus_const_cast
#  define OMNI_CONST_CAST(_t, _v) const_cast< _t >(_v)
#  define OMNI_CONST_VOID_CAST(_v) const_cast<void*>(static_cast<const void*>(_v))
#else
#  define OMNI_CONST_CAST(_t, _v) (_t)(_v)
#  define OMNI_CONST_VOID_CAST(_v) (void*)(_t)
#endif

#ifdef HAS_Cplusplus_reinterpret_cast
#  define OMNI_REINTERPRET_CAST(_t, _v) reinterpret_cast< _t const& >(_v)
#else
#  define OMNI_REINTERPRET_CAST(_t, _v) (*(_t*)(&_v))
#endif


#if defined(__GNUG__)
// GNU G++ compiler
#  define EGCS_WORKAROUND
#  define NEED_DUMMY_RETURN
#endif

#if defined(__DECCXX)
// DEC C++ compiler
#  define NEED_DUMMY_RETURN
#  if __DECCXX_VER < 60000000
//    Compaq C++ 5.x
//    Work-around for OpenVMS VAX Compaq C++ 5.6 compiler problem with
//    %CXX-W-CANTCOMPLETE messages.  Note that this cannot be disabled with a
//    compiler switch if the message occurs in a template instantiation (but
//    this pragma wasn't implemented until 6.something on Unix).
#     ifdef __VMS
#       pragma message disable CANTCOMPLETE
#     endif
#     define OMNI_OPERATOR_REFPTR_REQUIRES_TYPEDEF
#     define OMNI_PREMATURE_INSTANTIATION
//    Extra macros from the Compaq C++ 5.x patch (in <top>/patches/) to be
//    added here
#     ifndef OMNI_OPERATOR_REFPTR
#       error "Patch for Compaq C++ 5.x has not been applied."
#     endif
#  endif
#endif

#if defined(__SUNPRO_CC) 
// SUN C++ compiler
#  define NEED_DUMMY_RETURN

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
#endif

#if defined(__HP_aCC) || ( defined(__hpux__) && !defined(__GNUG__) )
#  define NEED_DUMMY_RETURN
#endif

#if defined(__hpux__) && __OSVERSION__ < 11
// *** Is this really needed?
#  include <stdio.h>
#  undef __ptr
#endif


#if defined(__sgi)
#  define OMNI_NEED_STATIC_FUNC_TO_FORCE_LINK
#endif


#if defined(__aix__) && defined(__xlC__)
#  define OMNI_NO_INLINE_FRIENDS
#endif


//
// Windows DLL hell
//

#if defined(_MSC_VER)

#  define NEED_DUMMY_RETURN

// VC.NET 2003 (v. 7.1) has problems recognizing inline friend
// operators.

#  if (_MSC_VER >= 1310)
#    define OMNI_NO_INLINE_FRIENDS
#  endif

//
// _OMNIORB_LIBRARY         is defined when the omniORB library is compiled.
// _OMNIORB_DYNAMIC_LIBRARY is defined when the dynamic library is compiled.
//  These are defined on the command line when compiling the libraries.
//
// _WINSTATIC                is defined when an application is compiled to
//                           use the static library.
//
// To package stubs into dlls:
//   1. Define the cpp macro _OMNIORB_STUB_DLL when the stub _SK.cc is
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

#  if defined(_DEBUG)
// The type name instantiated from the sequence templates could exceeds the
// 255 char limit of the debug symbol names. It is harmless except that one
// cannot read their values with the debugger. Disable the warning about
// the symbol name truncation.
#    pragma warning(disable: 4786)
#  endif

// Disable warnings about a member function in a derived class overriding
// a member function in the base class.
#  pragma warning(disable: 4250)

#elif defined(__DMC__)
#  define NEED_DUMMY_RETURN

#  ifdef _WINSTATIC
#    define _OMNIORB_NTDLL_IMPORT
#  else
#    define _OMNIORB_NTDLL_IMPORT  __declspec(dllimport)
#  endif

#elif defined(__MINGW32__)

#  ifdef _WINSTATIC
#    define _OMNIORB_NTDLL_IMPORT
#  else
#    define _OMNIORB_NTDLL_IMPORT  __attribute__((dllimport))
#  endif

#else

// For non-MSVC++ compiler, this macro expands to nothing.
#  define _OMNIORB_NTDLL_IMPORT

#endif


//
// Module mapping using namespaces or classes
//

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

#ifndef _CORBA_MODULE_INLINE
#define _CORBA_MODULE_INLINE inline
#else
#error "Name conflict: _CORBA_MODULE_INLINE is already defined."
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
#error "Name conflict: OMNI_NAMESPACE_BEGIN is already defined."
#endif

#ifndef OMNI_NAMESPACE_END
#define OMNI_NAMESPACE_END(name) }
#else
#error "Name conflict: OMNI_NAMESPACE_END is already defined."
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
#  if !defined(_MSC_VER) || defined(__INTEL_COMPILER)
#    define _init_in_cldecl_(x) x
#  else
#    define _init_in_cldecl_(x) 
#  endif
#else
#error "Name conflict: _init_in_cldecl_ is already defined."
#endif

#ifndef _init_in_cldef_
#  if !defined(_MSC_VER) || defined(__INTEL_COMPILER)
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

#ifndef _CORBA_MODULE_INLINE
#define _CORBA_MODULE_INLINE static inline
#else
#error "Name conflict: _CORBA_MODULE_INLINE is already defined."
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
#error "Name conflict: OMNI_NAMESPACE_BEGIN is already defined."
#endif

#ifndef OMNI_NAMESPACE_END
#define OMNI_NAMESPACE_END(name)
#else
#error "Name conflict: OMNI_NAMESPACE_END is already defined."
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
