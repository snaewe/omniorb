// -*- Mode: C++; -*-
//                            Package   : omniORB2
// exceptn.cc                 Created on: 9/1998
//                            Author    : David Riddoch (djr)
//
//    Copyright (C) 1996-1999 Olivetti & Oracle Research Laboratory
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

#include <omniORB2/CORBA.h>
#include <string.h>


CORBA::Exception::~Exception() {}


void
CORBA::Exception::_raise()
{
  throw omniORB::fatalException(__FILE__,__LINE__,
				"_raise() not overriden in derived class"
				" of CORBA::Exception");
}


CORBA::Exception*
CORBA::Exception::_NP_duplicate() const
{
  throw omniORB::fatalException(__FILE__,__LINE__,
				"_NP_duplicate() not overriden in derived"
				" class of CORBA::Exception");
#ifdef NEED_DUMMY_RETURN
  return 0;
#endif
}


const char*
CORBA::Exception::_NP_mostDerivedTypeId() const
{
  return "Exception";
}


CORBA::Exception*
CORBA::Exception::_duplicate(Exception* e)
{
  if( e )  return e->_NP_duplicate();
  else     return 0;
}


CORBA::Exception*
CORBA::Exception::_narrow(Exception* e)
{
  return e;
}


CORBA::Exception*
CORBA::
Exception::_NP_is_a(Exception* e, const char* typeId)
{
  if( !e )  return 0;
  size_t len = strlen(typeId);
  if( strncmp(typeId, e->_NP_mostDerivedTypeId(), len) )
    return 0;
  else
    return e;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

CORBA::SystemException::~SystemException() {}


const char*
CORBA::SystemException::NP_RepositoryId() const
{
  return "";
}


CORBA::SystemException*
CORBA::SystemException::_narrow(Exception* e)
{
  return (SystemException*)_NP_is_a(e, "Exception/SystemException");
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

CORBA::UserException::~UserException() {}


CORBA::UserException*
CORBA::UserException::_narrow(Exception* e)
{
  return (UserException*)_NP_is_a(e, "Exception/UserException");
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
CORBA::Exception* \
CORBA::name::_NP_duplicate() const \
{ \
  return new name (pd_minor, pd_status); \
} \
 \
const char* \
CORBA::name::_NP_mostDerivedTypeId() const \
{ \
  return "Exception/SystemException/" #name; \
} \
 \
CORBA::name* \
CORBA::name::_narrow(Exception* e) \
{ \
  return (name*)_NP_is_a(e, "Exception/SystemException/" #name); \
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

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
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

USER_EXCEPTION (ORB,InvalidName)
#undef USER_EXCEPTION
