// -*- Mode: C++; -*-
//                            Package   : omniORB
// exceptn.cc                 Created on: 9/1998
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
//  Implementation of the CORBA::Exception hierarchy.
//

#include <omniORB3/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <exception.h>
#include <string.h>
#include <stdlib.h>


_init_in_def_( const CORBA::ULong CORBA::OMGVMCID = 1330446336; )


//////////////////////////////////////////////////////////////////////
////////////////////////////// Exception /////////////////////////////
//////////////////////////////////////////////////////////////////////

CORBA::Exception::~Exception() { pd_magic = 0; }


CORBA::Exception*
CORBA::Exception::_duplicate(Exception* e)
{
  if( e )  return e->_NP_duplicate();
  else     return 0;
}


CORBA::Exception*
CORBA::
Exception::_NP_is_a(const Exception* e, const char* typeId)
{
  if( !e )  return 0;
  size_t len = strlen(typeId);
  if( strncmp(typeId, e->_NP_typeId(), len) )
    return 0;
  else
    return (Exception*) e;
}


size_t
CORBA::Exception::_NP_alignedSize(size_t msgsize) const
{
  return msgsize;
}

//////////////////////////////////////////////////////////////////////
/////////////////////////// SystemException //////////////////////////
//////////////////////////////////////////////////////////////////////

CORBA::SystemException::~SystemException() {}


CORBA::SystemException*
CORBA::SystemException::_downcast(Exception* e)
{
  return (SystemException*)_NP_is_a(e, "Exception/SystemException");
}

const CORBA::SystemException*
CORBA::SystemException::_downcast(const Exception* e)
{
  return (const SystemException*)_NP_is_a(e, "Exception/SystemException");
}


size_t
CORBA::SystemException::_NP_alignedSize(size_t msgsize) const
{
  return omni::align_to(msgsize, omni::ALIGN_4) + 8;
}


void
CORBA::SystemException::_NP_marshal(NetBufferedStream& s) const
{
  pd_minor >>= s;
  CORBA::ULong(pd_status) >>= s;
}


void
CORBA::SystemException::_NP_marshal(MemBufferedStream& s) const
{
  pd_minor >>= s;
  CORBA::ULong(pd_status) >>= s;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////// UserException ///////////////////////////
//////////////////////////////////////////////////////////////////////

CORBA::UserException::~UserException() {}


CORBA::UserException*
CORBA::UserException::_downcast(Exception* e)
{
  return (UserException*)_NP_is_a(e, "Exception/UserException");
}

const CORBA::UserException*
CORBA::UserException::_downcast(const Exception* e)
{
  return (const UserException*)_NP_is_a(e, "Exception/UserException");
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

#define  STD_EXCEPTION(name) \
CORBA::name::~name() {} \
 \
void \
CORBA::name::_raise() \
{ \
  throw *this; \
} \
 \
CORBA::name* \
CORBA::name::_downcast(Exception* e) \
{ \
  return (name*)_NP_is_a(e, "Exception/SystemException/" #name); \
} \
 \
const CORBA::name* \
CORBA::name::_downcast(const Exception* e) \
{ \
  return (const name*)_NP_is_a(e, "Exception/SystemException/" #name); \
} \
 \
const char* \
CORBA::name::_NP_repoId(int* size) const \
{ \
  *size = sizeof("IDL:omg.org/CORBA/" #name ":1.0"); \
  return "IDL:omg.org/CORBA/" #name ":1.0"; \
} \
 \
CORBA::Exception* \
CORBA::name::_NP_duplicate() const \
{ \
  return new name (*this); \
} \
 \
const char* \
CORBA::name::_NP_typeId() const \
{ \
  return "Exception/SystemException/" #name; \
} \
 \
CORBA::Exception::insertExceptionToAny CORBA::name::insertToAnyFn = 0; \
CORBA::Exception::insertExceptionToAnyNCP CORBA::name::insertToAnyFnNCP = 0; \


STD_EXCEPTION (UNKNOWN)
STD_EXCEPTION (BAD_PARAM)
STD_EXCEPTION (NO_MEMORY)
STD_EXCEPTION (IMP_LIMIT)
STD_EXCEPTION (COMM_FAILURE)
STD_EXCEPTION (INV_OBJREF)
STD_EXCEPTION (OBJECT_NOT_EXIST)
STD_EXCEPTION (NO_PERMISSION)
STD_EXCEPTION (INTERNAL)
STD_EXCEPTION (MARSHAL)
STD_EXCEPTION (INITIALIZE)
STD_EXCEPTION (NO_IMPLEMENT)
STD_EXCEPTION (BAD_TYPECODE)
STD_EXCEPTION (BAD_OPERATION)
STD_EXCEPTION (NO_RESOURCES)
STD_EXCEPTION (NO_RESPONSE)
STD_EXCEPTION (PERSIST_STORE)
STD_EXCEPTION (BAD_INV_ORDER)
STD_EXCEPTION (TRANSIENT)
STD_EXCEPTION (FREE_MEM)
STD_EXCEPTION (INV_IDENT)
STD_EXCEPTION (INV_FLAG)
STD_EXCEPTION (INTF_REPOS)
STD_EXCEPTION (BAD_CONTEXT)
STD_EXCEPTION (OBJ_ADAPTER)
STD_EXCEPTION (DATA_CONVERSION)
STD_EXCEPTION (TRANSACTION_REQUIRED)
STD_EXCEPTION (TRANSACTION_ROLLEDBACK)
STD_EXCEPTION (INVALID_TRANSACTION)
STD_EXCEPTION (WRONG_TRANSACTION)
#undef STD_EXCEPTION

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

OMNIORB_DEFINE_USER_EX_WITHOUT_MEMBERS(CORBA::ORB, InvalidName,
				       "IDL:omg.org/CORBA/ORB/InvalidName:1.0")

//////////////////////////////////////////////////////////////////////
//////////////////////////// omniExHelper ////////////////////////////
//////////////////////////////////////////////////////////////////////

#ifndef OMNIORB_NO_EXCEPTION_LOGGING

static const char* strip(const char* fn)
{
  const char* p = fn + strlen(fn);

  while( p > fn && *p != '/' && *p != '\\' && *p != ':' )  p--;

  if( *p == '/' || *p == '\\' || *p == ':' )  p++;

  return p;
}


#define STD_EXCEPTION(name) \
  void omniExHelper::name(const char* file, int line, \
		  CORBA::ULong minor , CORBA::CompletionStatus status) \
  { \
    if( omniORB::trace(10) ) { \
      omniORB::logger l; \
      l << "throw " #name << " from " << strip(file) << ":" << line << '\n'; \
    } \
    throw CORBA::name(minor, status); \
  }

STD_EXCEPTION (UNKNOWN)
STD_EXCEPTION (BAD_PARAM)
STD_EXCEPTION (NO_MEMORY)
STD_EXCEPTION (IMP_LIMIT)
STD_EXCEPTION (COMM_FAILURE)
STD_EXCEPTION (INV_OBJREF)
STD_EXCEPTION (OBJECT_NOT_EXIST)
STD_EXCEPTION (NO_PERMISSION)
STD_EXCEPTION (INTERNAL)
STD_EXCEPTION (MARSHAL)
STD_EXCEPTION (INITIALIZE)
STD_EXCEPTION (NO_IMPLEMENT)
STD_EXCEPTION (BAD_TYPECODE)
STD_EXCEPTION (BAD_OPERATION)
STD_EXCEPTION (NO_RESOURCES)
STD_EXCEPTION (NO_RESPONSE)
STD_EXCEPTION (PERSIST_STORE)
STD_EXCEPTION (BAD_INV_ORDER)
STD_EXCEPTION (TRANSIENT)
STD_EXCEPTION (FREE_MEM)
STD_EXCEPTION (INV_IDENT)
STD_EXCEPTION (INV_FLAG)
STD_EXCEPTION (INTF_REPOS)
STD_EXCEPTION (BAD_CONTEXT)
STD_EXCEPTION (OBJ_ADAPTER)
STD_EXCEPTION (DATA_CONVERSION)
STD_EXCEPTION (TRANSACTION_REQUIRED)
STD_EXCEPTION (TRANSACTION_ROLLEDBACK)
STD_EXCEPTION (INVALID_TRANSACTION)
STD_EXCEPTION (WRONG_TRANSACTION)
#undef STD_EXCEPTION

#endif  // ifndef OMNIORB_NO_EXCEPTION_LOGGING
