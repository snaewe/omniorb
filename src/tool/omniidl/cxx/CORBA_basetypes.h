// -*- Mode: C++; -*-
//                            Package   : omniORB2
// CORBA_basetypes.h          Created on: 30/1/96
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 1996-1999 AT&T Laboratories - Cambridge Ltd
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
 Revision 1.5.2.1  2000/08/04 17:10:30  dpg1
 Long long support

 Revision 1.5  2000/07/13 15:25:54  dpg1
 Merge from omni3_develop for 3.0 release.

 Revision 1.2  2000/02/04 12:17:10  dpg1
 Support for VMS.

 Revision 1.1  1999/10/27 14:06:01  dpg1
 *** empty log message ***

 Revision 1.12  1999/08/15 13:52:17  sll
 New VMS float implementation.

 Revision 1.11  1999/06/18 21:11:24  sll
 Updated copyright notice.

 Revision 1.10  1999/06/18 20:34:22  sll
 New function _CORBA_bad_param_freebuf().

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

typedef unsigned short            _CORBA_WChar;

#if SIZEOF_LONG == 4
typedef long                      _CORBA_Long;

typedef unsigned long             _CORBA_ULong;
#elif SIZEOF_INT == 4
typedef int                       _CORBA_Long;

typedef unsigned int              _CORBA_ULong;
#else
# error "Can't map Long (32 bits) to a native type."
#endif

#ifdef HAS_LongLong
typedef _CORBA_LONGLONG_DECL      _CORBA_LongLong;
typedef _CORBA_ULONGLONG_DECL     _CORBA_ULongLong;
#endif


#ifndef NO_FLOAT

#ifndef __VMS

// This platform uses IEEE float
typedef float                     _CORBA_Float;
typedef double                    _CORBA_Double;

#ifdef HAS_LongDouble
typedef _CORBA_LONGDOUBLE_DECL    _CORBA_LongDouble;
#endif

#else	// VMS float test

// VMS now always uses proxies for float.
#define USING_PROXY_FLOAT

#undef cvt_
#if __D_FLOAT
#define cvt_ cvt_d_
#elif __G_FLOAT
#define cvt_ cvt_g_
#else
#define cvt_ cvt_ieee_
#endif

class _CORBA_Float {
  _CORBA_Long pd_f;
  void cvt_d_(float f);
  float cvt_d_() const;
  void cvt_g_(float f);
  float cvt_g_() const;
#ifndef __VAX
  void cvt_ieee_(float f);
  float cvt_ieee_() const;
#endif
public:
  // using compiler generated copy constructor and copy assignment
  inline _CORBA_Float() {cvt_(0.0f);}
  inline _CORBA_Float(float f) {cvt_(f);}
  inline operator float() const {return cvt_();}
};

class _CORBA_Double {
  _CORBA_Long pd_d[2];
  void cvt_d_(double d);
  double cvt_d_() const;
  void cvt_g_(double d);
  double cvt_g_() const;
#ifndef __VAX
  void cvt_ieee_(double d);
  double cvt_ieee_() const;
#endif
public:
  // using compiler generated copy constructor and copy assignment
  inline _CORBA_Double() {cvt_(0.0);}
  inline _CORBA_Double(double d) {cvt_(d);}
  inline operator double() const {return cvt_();}
};

#undef cvt_

//  Assume long double type is compatible with the CORBA standard.

#ifdef HAS_LongDouble
typedef _CORBA_LONGDOUBLE_DECL    _CORBA_LongDouble;
#endif

#endif   // VMS float test
#endif   // !defined(NO_FLOAT)

extern void _CORBA_new_operator_return_null();
extern void _CORBA_bound_check_error();
extern void _CORBA_marshal_error();
extern _CORBA_Boolean _CORBA_use_nil_ptr_as_nil_objref();
extern void _CORBA_null_string_ptr(_CORBA_Boolean);
extern void _CORBA_invoked_nil_pseudo_ref();
extern void _CORBA_invoked_nil_objref();
extern _CORBA_Boolean
_CORBA_use_nil_ptr_as_nil_pseudo_objref(const char* objType);
extern void _CORBA_bad_param_freebuf();


#endif // __CORBA_BASETYPES_H__
