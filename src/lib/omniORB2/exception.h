// -*- Mode: C++; -*-
//                            Package   : omniORB
// exception.h                Created on: 27/5/99
//                            Author    : David Riddoch (djr)
//
//    Copyright (C) 1996-1999 AT&T Research Cambridge
//
//    This file is part of the omniORB library.
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
//

/*
  $Log$
  Revision 1.1.2.2  1999/10/14 16:21:54  djr
  Implemented logging when system exceptions are thrown.

  Revision 1.1.2.1  1999/09/22 14:26:25  djr
  Major rewrite of orbcore to support POA.

*/

#ifndef __OMNIORB_EXCEPTION_H__
#define __OMNIORB_EXCEPTION_H__


#ifndef OMNIORB_NO_EXCEPTION_LOGGING


class omniExHelper {
public:

#define OMNIORB_EX(name) \
  static void name(const char*, int, CORBA::ULong, CORBA::CompletionStatus)

  OMNIORB_EX (UNKNOWN);
  OMNIORB_EX (BAD_PARAM);
  OMNIORB_EX (NO_MEMORY);
  OMNIORB_EX (IMP_LIMIT);
  OMNIORB_EX (COMM_FAILURE);
  OMNIORB_EX (INV_OBJREF);
  OMNIORB_EX (OBJECT_NOT_EXIST);
  OMNIORB_EX (NO_PERMISSION);
  OMNIORB_EX (INTERNAL);
  OMNIORB_EX (MARSHAL);
  OMNIORB_EX (INITIALIZE);
  OMNIORB_EX (NO_IMPLEMENT);
  OMNIORB_EX (BAD_TYPECODE);
  OMNIORB_EX (BAD_OPERATION);
  OMNIORB_EX (NO_RESOURCES);
  OMNIORB_EX (NO_RESPONSE);
  OMNIORB_EX (PERSIST_STORE);
  OMNIORB_EX (BAD_INV_ORDER);
  OMNIORB_EX (TRANSIENT);
  OMNIORB_EX (FREE_MEM);
  OMNIORB_EX (INV_IDENT);
  OMNIORB_EX (INV_FLAG);
  OMNIORB_EX (INTF_REPOS);
  OMNIORB_EX (BAD_CONTEXT);
  OMNIORB_EX (OBJ_ADAPTER);
  OMNIORB_EX (DATA_CONVERSION);
  OMNIORB_EX (TRANSACTION_REQUIRED);
  OMNIORB_EX (TRANSACTION_ROLLEDBACK);
  OMNIORB_EX (INVALID_TRANSACTION);
  OMNIORB_EX (WRONG_TRANSACTION);

#undef OMNIORB_EX


};


#define OMNIORB_THROW(name, minor, completion) \
  omniExHelper::name(__FILE__, __LINE__, minor, completion)


#else


#define OMNIORB_THROW(name, minor, completion) \
  throw CORBA::name(minor, completion)


#endif


#define OMNIORB_DEFINE_USER_EX_COMMON_FNS(scope, name, repoid) \
 \
CORBA::Exception::insertExceptionToAny scope::name::insertToAnyFn = 0; \
CORBA::Exception::insertExceptionToAnyNCP scope::name::insertToAnyFnNCP = 0; \
 \
scope::name::~name() {} \
 \
void scope::name::_raise() { throw *this; } \
 \
scope::name* scope::name::_downcast(CORBA::Exception* e) { \
  return (name*)_NP_is_a(e, "Exception/UserException/"#scope"::"#name); \
} \
const scope::name* \
scope::name::_downcast(const CORBA::Exception* e) \
{ \
  return (const name*)_NP_is_a(e,"Exception/UserException/"#scope"::"#name); \
} \
 \
const char* scope::name::_PD_repoId = repoid; \
 \
CORBA::Exception* \
scope::name::_NP_duplicate() const { \
  return new name(*this); \
} \
 \
const char* \
scope::name::_NP_typeId() const { \
  return "Exception/UserException/" #scope "::" #name; \
} \
 \
const char* \
scope::name::_NP_repoId(int* size) const { \
  *size = sizeof(repoid); \
  return repoid; \
} \
 \
void \
scope::name::_NP_marshal(NetBufferedStream& s) const { \
  *this >>= s; \
} \
 \
void \
scope::name::_NP_marshal(MemBufferedStream& s) const { \
  *this >>= s; \
} \


#define OMNIORB_DEFINE_USER_EX_WITHOUT_MEMBERS(scope, name, repoid) \
 \
OMNIORB_DEFINE_USER_EX_COMMON_FNS(scope, name, repoid) \


#endif  // __OMNIORB_EXCEPTION_H__
