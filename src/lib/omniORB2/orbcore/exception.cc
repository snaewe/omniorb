// -*- Mode: C++; -*-
//                            Package   : omniORB2
// exception.cc               Created on: 13/5/96
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
//      
//      
 
/*
  $Log$
  Revision 1.3  1997/08/21 22:03:56  sll
  Added system exception TRANSACTION_REQUIRED, TRANSACTION_ROLLEDBACK,
  INVALID_TRANSACTION, WRONG_TRANSACTION.
  INVALID_TRANSACTION, WRONG_TRANSACTION.

// Revision 1.2  1997/05/06  15:12:21  sll
// Public release.
//
  */

#include <omniORB2/CORBA.h>


CORBA::Boolean 
_CORBA_use_nil_ptr_as_nil_objref()
{
  if (omniORB::traceLevel > 10) {
    cerr << "Warning: omniORB2 detects that a nil pointer is wrongly used as a nil object reference."
	 << endl;
  }
  return 1;
}

void
_CORBA_new_operator_return_null()
{
  throw CORBA::NO_MEMORY(0,CORBA::COMPLETED_NO);
}

void
_CORBA_bound_check_error()
{
  throw CORBA::BAD_PARAM(0,CORBA::COMPLETED_NO);
}

void
_CORBA_marshal_error()
{
  throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);
}

const char *
CORBA::
UNKNOWN::NP_RepositoryId() const
{
  return (const char *) GIOP_Basetypes::SysExceptRepoID::UNKNOWN.id;
}

const char *
CORBA::
BAD_PARAM::NP_RepositoryId() const
{
  return (const char *) GIOP_Basetypes::SysExceptRepoID::BAD_PARAM.id;
}

const char *
CORBA::
NO_MEMORY::NP_RepositoryId() const
{
  return (const char *) GIOP_Basetypes::SysExceptRepoID::NO_MEMORY.id;
}

const char *
CORBA::
IMP_LIMIT::NP_RepositoryId() const
{
  return (const char *) GIOP_Basetypes::SysExceptRepoID::IMP_LIMIT.id;
}

const char *
CORBA::
COMM_FAILURE::NP_RepositoryId() const
{
  return (const char *) GIOP_Basetypes::SysExceptRepoID::COMM_FAILURE.id;
}

const char *
CORBA::
INV_OBJREF::NP_RepositoryId() const
{
  return (const char *) GIOP_Basetypes::SysExceptRepoID::INV_OBJREF.id;
}

const char *
CORBA::
OBJECT_NOT_EXIST::NP_RepositoryId() const
{
  return (const char *) GIOP_Basetypes::SysExceptRepoID::OBJECT_NOT_EXIST.id;
}

const char *
CORBA::
NO_PERMISSION::NP_RepositoryId() const
{
  return (const char *) GIOP_Basetypes::SysExceptRepoID::NO_PERMISSION.id;
}

const char *
CORBA::
INTERNAL::NP_RepositoryId() const
{
  return (const char *) GIOP_Basetypes::SysExceptRepoID::INTERNAL.id;
}

const char *
CORBA::
MARSHAL::NP_RepositoryId() const
{
  return (const char *) GIOP_Basetypes::SysExceptRepoID::MARSHAL.id;
}

const char *
CORBA::
INITIALIZE::NP_RepositoryId() const
{
  return (const char *) GIOP_Basetypes::SysExceptRepoID::INITIALIZE.id;
}

const char *
CORBA::
NO_IMPLEMENT::NP_RepositoryId() const
{
  return (const char *) GIOP_Basetypes::SysExceptRepoID::NO_IMPLEMENT.id;
}

const char *
CORBA::
BAD_TYPECODE::NP_RepositoryId() const
{
  return (const char *) GIOP_Basetypes::SysExceptRepoID::BAD_TYPECODE.id;
}

const char *
CORBA::
BAD_OPERATION::NP_RepositoryId() const
{
  return (const char *) GIOP_Basetypes::SysExceptRepoID::BAD_OPERATION.id;
}

const char *
CORBA::
NO_RESOURCES::NP_RepositoryId() const
{
  return (const char *) GIOP_Basetypes::SysExceptRepoID::NO_RESOURCES.id;
}

const char *
CORBA::
NO_RESPONSE::NP_RepositoryId() const
{
  return (const char *) GIOP_Basetypes::SysExceptRepoID::NO_RESPONSE.id;
}

const char *
CORBA::
PERSIST_STORE::NP_RepositoryId() const
{
  return (const char *) GIOP_Basetypes::SysExceptRepoID::PERSIST_STORE.id;
}

const char *
CORBA::
BAD_INV_ORDER::NP_RepositoryId() const
{
  return (const char *) GIOP_Basetypes::SysExceptRepoID::BAD_INV_ORDER.id;
}

const char *
CORBA::
TRANSIENT::NP_RepositoryId() const
{
  return (const char *) GIOP_Basetypes::SysExceptRepoID::TRANSIENT.id;
}

const char *
CORBA::
FREE_MEM::NP_RepositoryId() const
{
  return (const char *) GIOP_Basetypes::SysExceptRepoID::FREE_MEM.id;
}

const char *
CORBA::
INV_IDENT::NP_RepositoryId() const
{
  return (const char *) GIOP_Basetypes::SysExceptRepoID::INV_IDENT.id;
}

const char *
CORBA::
INV_FLAG::NP_RepositoryId() const
{
  return (const char *) GIOP_Basetypes::SysExceptRepoID::INV_FLAG.id;
}

const char *
CORBA::
INTF_REPOS::NP_RepositoryId() const
{
  return (const char *) GIOP_Basetypes::SysExceptRepoID::INTF_REPOS.id;
}

const char *
CORBA::
BAD_CONTEXT::NP_RepositoryId() const
{
  return (const char *) GIOP_Basetypes::SysExceptRepoID::BAD_CONTEXT.id;
}

const char *
CORBA::
OBJ_ADAPTER::NP_RepositoryId() const
{
  return (const char *) GIOP_Basetypes::SysExceptRepoID::OBJ_ADAPTER.id;
}

const char *
CORBA::
DATA_CONVERSION::NP_RepositoryId() const
{
  return (const char *) GIOP_Basetypes::SysExceptRepoID::DATA_CONVERSION.id;
}

const char *
CORBA::
TRANSACTION_REQUIRED::NP_RepositoryId() const
{
  return (const char *) GIOP_Basetypes::SysExceptRepoID::TRANSACTION_REQUIRED.id;
}
const char *
CORBA::
TRANSACTION_ROLLEDBACK::NP_RepositoryId() const
{
  return (const char *) GIOP_Basetypes::SysExceptRepoID::TRANSACTION_ROLLEDBACK.id;
}
const char *
CORBA::
INVALID_TRANSACTION::NP_RepositoryId() const
{
  return (const char *) GIOP_Basetypes::SysExceptRepoID::INVALID_TRANSACTION.id;
}
const char *
CORBA::
WRONG_TRANSACTION::NP_RepositoryId() const
{
  return (const char *) GIOP_Basetypes::SysExceptRepoID::WRONG_TRANSACTION.id;
}

