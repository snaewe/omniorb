// -*- Mode: C++; -*-
//                            Package   : omniORB2
// corbaBoa.cc                Created on: 6/2/96
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
//      Implementation of the BOA interface
//	

/*
  $Log$
  Revision 1.3  1997/05/06 15:09:04  sll
  Public release.

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
  omni::disposeObject(p->PR_getobj());
  return;
}

void 
CORBA::
BOA::obj_is_ready(Object_ptr op, ImplementationDef_ptr ip /* ignored */)
{
  omniObject *obj = op->PR_getobj();
  omni::objectIsReady(obj);
  return;
}

void
CORBA::
BOA::impl_is_ready(CORBA::ImplementationDef_ptr p,CORBA::Boolean NonBlocking)
{
  omni::orbIsReady();
  if (!NonBlocking) {
    omni_mutex m;
    omni_condition c(&m);
    m.lock();
    c.wait();	// block here forever
  }
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

