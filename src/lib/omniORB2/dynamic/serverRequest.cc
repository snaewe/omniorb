// -*- Mode: C++; -*-
//                            Package   : omniORB3
// serverRequest.cc           Created on: 9/1998
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
//   Implementation of CORBA::ServerRequest.
//

/*
 $Log$
 Revision 1.7.6.1  1999/09/22 14:26:36  djr
 Major rewrite of orbcore to support POA.

 Revision 1.6  1999/06/18 20:59:12  sll
 Allow system exception to be returned inside exception().

 Revision 1.5  1999/04/21 13:40:10  djr
 Use CORBA::Context::unmarshalContext() for unmarshalling context ...

*/

#include <omniORB3/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <dynamicImplementation.h>
#include <pseudo.h>
#include <context.h>
#include <dynException.h>


CORBA::ServerRequest::~ServerRequest()  {}


omniServerRequest::~omniServerRequest()  {}


const char*
omniServerRequest::operation()
{
  return pd_giop_s.operation();
}


void
omniServerRequest::arguments(CORBA::NVList_ptr& parameters)
{
  if( pd_state != SR_READY ) {
    pd_state = SR_DSI_ERROR;
    throw CORBA::BAD_INV_ORDER(0, CORBA::COMPLETED_NO);
  }
  if( CORBA::is_nil(parameters) ) {
    pd_state = SR_DSI_ERROR;
    throw CORBA::BAD_PARAM(0, CORBA::COMPLETED_NO);
  }

  pd_params = parameters;
  pd_state = SR_ERROR;

  // unmarshal the arguments
  CORBA::ULong num_args = pd_params->count();

  for( CORBA::ULong i = 0; i < num_args; i++){
    CORBA::NamedValue_ptr arg = pd_params->item(i);
    if( arg->flags() & CORBA::ARG_IN || arg->flags() & CORBA::ARG_INOUT )
      arg->value()->NP_unmarshalDataOnly(pd_giop_s);
  }

  // If there is no space left for context info...
  if( pd_giop_s.RdMessageUnRead() < 4 )
    pd_giop_s.RequestReceived();

  pd_state = SR_GOT_PARAMS;
}


CORBA::Context_ptr
omniServerRequest::ctx()
{
  // Returns a nil context if no context information supplied.

  if( pd_state != SR_GOT_PARAMS ) {
    pd_state = SR_DSI_ERROR;
    throw CORBA::BAD_INV_ORDER(0, CORBA::COMPLETED_NO);
  }

  if( pd_giop_s.RdMessageUnRead() >= 4 ) {
    pd_state = SR_ERROR;
    pd_context = CORBA::Context::unmarshalContext(pd_giop_s);
    pd_giop_s.RequestReceived();
    pd_state = SR_GOT_CTX;
  }

  return pd_context;
}


void
omniServerRequest::set_result(const CORBA::Any& value)
{
  if( !(pd_state == SR_GOT_PARAMS || pd_state == SR_GOT_CTX) ) {
    pd_state = SR_DSI_ERROR;
    throw CORBA::BAD_INV_ORDER(0, CORBA::COMPLETED_NO);
  }
  if( pd_state == SR_GOT_PARAMS && pd_giop_s.RdMessageUnRead() > 0 )
    pd_giop_s.RequestReceived();

  pd_result = value;
  pd_state = SR_GOT_RESULT;
}


void
omniServerRequest::set_exception(const CORBA::Any& value)
{
  CORBA::TypeCode_var tc = value.type();
  while( tc->kind() == CORBA::tk_alias )
    tc = tc->content_type();
  if( tc->kind() != CORBA::tk_except )
    throw CORBA::BAD_PARAM(0, CORBA::COMPLETED_NO);

  switch( pd_state ) {
  case SR_READY:
    if( isaSystemException(&value) )
      pd_giop_s.RequestReceived(1);
    else {
      pd_state = SR_DSI_ERROR;
      throw CORBA::BAD_INV_ORDER(0, CORBA::COMPLETED_NO);
    }
    break;

  case SR_GOT_PARAMS:
  case SR_GOT_CTX:
  case SR_GOT_RESULT:
  case SR_EXCEPTION:
  case SR_ERROR:
    break;

  case SR_DSI_ERROR:
    throw CORBA::BAD_INV_ORDER(0, CORBA::COMPLETED_NO);
  }

  pd_exception = value;
  pd_state = SR_EXCEPTION;
}
