// -*- Mode: C++; -*-
//                            Package   : omniORB2
// CORBA_basetypes.h          Created on: 30/1/96
//                            Author    : Sai Lai Lo (sll)
//
// Copyright (C) Olivetti Research Limited, 1996
//
// Description:

/*
 $Log$
 Revision 1.1  1997/01/08 17:28:30  sll
 Initial revision

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

#endif // __CORBA_BASETYPES_H__
