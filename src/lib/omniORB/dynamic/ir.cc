// -*- Mode: C++; -*-
//                            Package   : omniORB2
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

#include <omniORB2/CORBA.h>
#include <bootstrap_i.h>


#if defined(ENABLE_CLIENT_IR_SUPPORT)


CORBA::InterfaceDef_ptr
CORBA::
Object::_get_interface()
{
  if ( !PR_is_valid(this) ) 
    throw CORBA::BAD_PARAM(0,CORBA::COMPLETED_NO);

  if( !pd_obj )  _CORBA_invoked_nil_pseudo_ref();

  CORBA::Object_var o = omniInitialReferences::singleton()
    ->get("InterfaceRepository");
  CORBA::Repository_ptr repository = CORBA::Repository::_narrow(o);
  if( CORBA::is_nil(repository) )
    throw CORBA::INTF_REPOS(0, CORBA::COMPLETED_NO);

  CORBA::Contained_ptr interf =
    repository->lookup_id(pd_obj->NP_IRRepositoryId());
  return CORBA::InterfaceDef::_narrow(interf);
}


#else


CORBA::InterfaceDef_ptr
CORBA::
Object::_get_interface()
{
  throw NO_IMPLEMENT(0, COMPLETED_NO);
#ifdef NEED_DUMMY_RETURN
  return 0;
#endif
}


#endif
