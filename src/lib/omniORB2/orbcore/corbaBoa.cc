// -*- Mode: C++; -*-
//                            Package   : omniORB2
// corbaBoa.cc                Created on: 6/2/96
//                            Author    : Sai Lai Lo (sll)
//
// Copyright (C) Olivetti Research Limited, 1996
//
// Description:
//      Implementation of the BOA interface
//	

/*
  $Log$
  Revision 1.1  1997/01/08 17:26:01  sll
  Initial revision

 */

#include <omniORB2/CORBA.h>

CORBA::
BOA::BOA() 
{
  return;
}

CORBA::
BOA::~BOA()
{
  return;
}

void
CORBA::
BOA::dispose(CORBA::Object_ptr p)
{
  omniORB::disposeObject(p->PR_getobj());
  return;
}

void 
CORBA::
BOA::obj_is_ready(Object_ptr op, ImplementationDef_ptr ip /* ignored */)
{
  omniObject *obj = op->PR_getobj();
  omniORB::objectIsReady(obj);
  return;
}

void
CORBA::
BOA::impl_is_ready(CORBA::ImplementationDef_ptr p)
{
  omniORB::orbIsReady();
  return;
}

CORBA::BOA_ptr 
CORBA::
BOA::_duplicate(CORBA::BOA_ptr p)
{
  return p;
}

CORBA::BOA_ptr
CORBA::
BOA::_nil()
{
  return 0;
}

CORBA::Boolean
CORBA::is_nil(CORBA::BOA_ptr p)
{
  return (p==0) ? 1 : 0;
}

void
CORBA::release(CORBA::BOA_ptr p)
{
  return;
}

CORBA::Object_ptr
CORBA::
BOA::create(const CORBA::ReferenceData& ref,
	    CORBA::InterfaceDef_ptr intf,
	    CORBA::ImplementationDef_ptr impl)
{
  // XXX not implemented yet
  return 0;
}

CORBA::ReferenceData *
CORBA::
BOA::get_id(CORBA::Object_ptr obj)
{
  // XXX not implemented yet
  return 0;
}

void
CORBA::
BOA::change_implementation(CORBA::Object_ptr obj,
			   CORBA::ImplementationDef_ptr impl)
{
  // XXX not implemented yet
  return;
}

CORBA::Principal_ptr
CORBA::
BOA::get_principal(CORBA::Object_ptr obj, CORBA::Environment_ptr env)
{
  // XXX not implemented yet
  return 0;
}

void
CORBA::
BOA::deactivate_impl(CORBA::ImplementationDef_ptr impl)
{
  // XXX not implemented yet
  return;
}

void
CORBA::
BOA::deactivate_obj(CORBA::Object_ptr obj)
{
  // XXX not implemented yet
  return;
}

