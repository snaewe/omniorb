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
  Revision 1.1.2.1  1999/09/22 14:26:25  djr
  Major rewrite of orbcore to support POA.

*/

#ifndef __OMNIORB_EXCEPTION_H__
#define __OMNIORB_EXCEPTION_H__


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
