// -*- Mode: C++; -*-
//                            Package   : omniORB
// exceptiondefs.h            Created on: 27/5/99
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
  Revision 1.2  2000/07/04 15:23:18  dpg1
  Merge from omni3_develop.

  Revision 1.1.2.1  2000/06/22 10:37:49  dpg1
  Transport code now throws omniConnectionBroken exception rather than
  CORBA::COMM_FAILURE when things go wrong. This allows the invocation
  code to distinguish between transport problems and COMM_FAILURES
  propagated from the server side.

  exception.h renamed to exceptiondefs.h to avoid name clash on some
  platforms.

  Revision 1.1.2.3  1999/10/18 11:27:37  djr
  Centralised list of system exceptions.

  Revision 1.1.2.2  1999/10/14 16:21:54  djr
  Implemented logging when system exceptions are thrown.

  Revision 1.1.2.1  1999/09/22 14:26:25  djr
  Major rewrite of orbcore to support POA.

*/

#ifndef __OMNIORB_EXCEPTION_H__
#define __OMNIORB_EXCEPTION_H__


class omniConnectionBroken {
public:
  inline omniConnectionBroken(CORBA::ULong m, CORBA::CompletionStatus c)
    : pd_minor(m), pd_status(c) {}

  inline CORBA::ULong minor()                const { return pd_minor;  }
  inline CORBA::CompletionStatus completed() const { return pd_status; }

private:
  CORBA::ULong            pd_minor;
  CORBA::CompletionStatus pd_status;
};


#ifndef OMNIORB_NO_EXCEPTION_LOGGING


class omniExHelper {
public:

#define OMNIORB_EX(name) \
  static void name(const char*, int, CORBA::ULong, CORBA::CompletionStatus);

  OMNIORB_FOR_EACH_SYS_EXCEPTION(OMNIORB_EX)

#undef OMNIORB_EX

  static void omniConnectionBroken(const char*, int, CORBA::ULong,
				   CORBA::CompletionStatus);
};


#define OMNIORB_THROW(name, minor, completion) \
  omniExHelper::name(__FILE__, __LINE__, minor, completion)

#define OMNIORB_THROW_CONNECTION_BROKEN(minor, completion) \
  omniExHelper::omniConnectionBroken(__FILE__, __LINE__, minor, completion)

#else


#define OMNIORB_THROW(name, minor, completion) \
  throw CORBA::name(minor, completion)

#define OMNIORB_THROW_CONNECTION_BROKEN(minor, completion) \
  throw omniConnectionBroken(minor, completion)

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
