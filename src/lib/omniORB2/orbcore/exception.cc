// -*- Mode: C++; -*-
//                            Package   : omniORB2
// exception.cc               Created on: 13/5/96
//                            Author    : Sai Lai Lo (sll)
//
// Copyright (C) Olivetti Research Limited, 1996
//
// Description:
//      
//      
 
/*
  $Log$
  Revision 1.1  1997/01/08 17:26:01  sll
  Initial revision

  */

#include <omniORB2/CORBA.h>

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

