// -*- Mode: C++; -*-
//                            Package   : omniORB2
// CORBA_basetypes.h          Created on: 30/1/96
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

/*
 $Log$
 Revision 1.9  1999/01/07 18:11:58  djr
 New functions: _CORBA_invoked_nil_pseudo_ref()
 and            _CORBA_use_nil_ptr_as_nil_pseudo_objref()

 Revision 1.8  1998/08/11 16:34:47  sll
 Changed Float and Double representation on VMS.

 Revision 1.7  1998/03/02 14:05:02  ewc
 Patch to fix IDL unions containing structs which contain floats or doubles
 (was broken on OpenVMS).

 * Revision 1.6  1998/01/21  12:12:17  sll
 * New function _CORBA_null_string_ptr.
 *
 * Revision 1.5  1998/01/20  16:45:45  sll
 * Added support for OpenVMS.
 *
 Revision 1.4  1997/08/21 22:21:38  sll
 New extern function _CORBA_use_nil_ptr_as_nil_objref().

 * Revision 1.3  1997/05/06  16:05:20  sll
 * Public release.
 *
 */

#ifndef __CORBA_BASETYPES_H__
#define __CORBA_BASETYPES_H__

#ifdef HAS_Cplusplus_Bool
typedef bool                      _CORBA_Boolean;
#else
typedef unsigned char             _CORBA_Boolean;
#endif

typedef unsigned char             _CORBA_Char;

typedef unsigned char             _CORBA_Octet;

typedef short                     _CORBA_Short;

typedef unsigned short            _CORBA_UShort;

#if SIZEOF_LONG == 4
typedef long                      _CORBA_Long;

typedef unsigned long             _CORBA_ULong;
#elif SIZEOF_INT == 4
typedef int                       _CORBA_Long;

typedef unsigned int              _CORBA_ULong;
#else
# error "Can't map Long (32 bits) to a native type."
#endif

#ifndef NO_FLOAT

#if defined(__VMS)

// VMS now always uses proxies for float.
#define USING_PROXY_FLOAT

class _CORBA_Float {
  _CORBA_Long pd_f;
  void cvt_(float f);
  float cvt_() const;
public:
  // using compiler generated copy constructor and copy assignment
  _CORBA_Float();
  _CORBA_Float(float f);
  operator float() const;
};

class _CORBA_Double {
  _CORBA_Long pd_d[2];
  void cvt_(double d);
  double cvt_() const;
public:
  // using compiler generated copy constructor and copy assignment
  _CORBA_Double();
  _CORBA_Double(double d);
  operator double() const;
};

#else


// This platform uses IEEE float
typedef float                     _CORBA_Float;
typedef double                    _CORBA_Double;

#endif   // VMS float test
#endif   // !defined(NO_FLOAT)

extern void _CORBA_new_operator_return_null();
extern void _CORBA_bound_check_error();
extern void _CORBA_marshal_error();
extern _CORBA_Boolean _CORBA_use_nil_ptr_as_nil_objref();
extern void _CORBA_null_string_ptr(_CORBA_Boolean);
extern void _CORBA_invoked_nil_pseudo_ref();
extern _CORBA_Boolean
_CORBA_use_nil_ptr_as_nil_pseudo_objref(const char* objType);


#endif // __CORBA_BASETYPES_H__
