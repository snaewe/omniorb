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
typedef float                     _CORBA_Float;

typedef double                    _CORBA_Double;
#endif

extern void _CORBA_new_operator_return_null();
extern void _CORBA_bound_check_error();
extern void _CORBA_marshal_error();
extern _CORBA_Boolean _CORBA_use_nil_ptr_as_nil_objref();

#endif // __CORBA_BASETYPES_H__
