// -*- Mode: C++; -*-
//                            Package   : omniORB
// ir.cc                      Created on: 12/1998
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
//   Client side of CORBA::Object::_get_interface(). Only available if
//   the Interface Repository stubs have been compiled in.
//

#define ENABLE_CLIENT_IR_SUPPORT
#include <omniORB4/CORBA.h>
#include <initRefs.h>
#include <exceptiondefs.h>

OMNI_USING_NAMESPACE(omni)

CORBA::InterfaceDef_ptr
CORBA::
Object::_get_interface()
{
  if( _NP_is_nil() )  _CORBA_invoked_nil_objref();
  if( !_PR_is_valid(this) )  OMNIORB_THROW(BAD_PARAM,
					   BAD_PARAM_InvalidObjectRef,
					   CORBA::COMPLETED_NO);

  //?? No - we ought to contact the implementation first, and only
  // if that fails to provide an answer should we try to go direct
  // to the repository.

  CORBA::Object_var o(omniInitialReferences::resolve("InterfaceRepository"));
  CORBA::Repository_ptr repository = CORBA::Repository::_narrow(o);

  if( CORBA::is_nil(repository) )
    OMNIORB_THROW(INTF_REPOS, INTF_REPOS_NotAvailable, CORBA::COMPLETED_NO);

  CORBA::Contained_ptr interf =
    repository->lookup_id(pd_obj->_mostDerivedRepoId());
  return CORBA::InterfaceDef::_narrow(interf);
}
