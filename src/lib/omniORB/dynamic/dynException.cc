// -*- Mode: C++; -*-
//                            Package   : omniORB2
// dynException.cc            Created on: 10/1998
//                            Author    : David Riddoch (djr)
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
//   Exceptions used in the Dynamic library.
//

#include <omniORB2/CORBA.h>


//////////////////////////////////////////////////////////////////////
////////////// Implementation of standard UserException //////////////
//////////////////////////////////////////////////////////////////////

#define USER_EXCEPTION(scope,name) \
 \
CORBA::scope::name::~name() {} \
 \
void \
CORBA::scope::name::_raise() \
{ \
  throw *this; \
} \
 \
CORBA::scope::name* \
CORBA::scope::name::_narrow(Exception* e) \
{ \
  return (name*)_NP_is_a(e, "Exception/UserException/" #scope "::" #name); \
} \
 \
CORBA::Exception* \
CORBA::scope::name::_NP_duplicate() const \
{ \
  return new name (); \
} \
 \
const char* \
CORBA::scope::name::_NP_mostDerivedTypeId() const \
{ \
  return "Exception/UserException/" #scope "::" #name; \
}


USER_EXCEPTION (TypeCode,Bounds)
USER_EXCEPTION (TypeCode,BadKind)
USER_EXCEPTION (DynAny,Invalid)
USER_EXCEPTION (DynAny,InvalidValue)
USER_EXCEPTION (DynAny,TypeMismatch)
USER_EXCEPTION (DynAny,InvalidSeq)
USER_EXCEPTION (ORB,InconsistentTypeCode)
#undef USER_EXCEPTION
