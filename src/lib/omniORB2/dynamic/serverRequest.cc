// -*- Mode: C++; -*-
//                            Package   : omniORB2
// serverRequest.cc           Created on: 9/1998
//                            Author    : David Riddoch (djr)
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
//   Implementation of CORBA::ServerRequest.
//

#include <dynamicImplementation.h>
#include <pseudo.h>
#include <context.h>


CORBA::ServerRequest::~ServerRequest() {}


const char*
ServerRequestImpl::op_name()
{
  return pd_opName;
}


CORBA::OperationDef_ptr
ServerRequestImpl::op_def()
{
  throw CORBA::NO_IMPLEMENT(0, CORBA::COMPLETED_NO);
#ifdef NEED_DUMMY_RETURN
  return 0;
#endif
}


CORBA::Context_ptr
ServerRequestImpl::ctx()
{
  // Returns an empty context if no context information supplied.
  //  NB. This will change for CORBA 2.2, which specifies that a
  // nil context is returned if no context is specified in the
  // IDL.

  if( pd_state != SR_GOT_PARAMS ) {
    pd_state = SR_ERROR;
    throw CORBA::BAD_INV_ORDER(0, CORBA::COMPLETED_NO);
  }

  pd_context = new ContextImpl("", CORBA::Context::_nil());

  if( pd_giopS->RdMessageUnRead() >= 4 ){
    CORBA::ULong num_strings;
    num_strings <<= *pd_giopS;
    // This gives the number of strings - names and values - so must be
    // an even number.
    if( num_strings % 1 ) {
      pd_state = SR_ERROR;
      throw CORBA::MARSHAL(0, CORBA::COMPLETED_MAYBE);
    }
    CORBA::ULong num_ctxts = num_strings / 2;
    CORBA::ULong len;

    for( CORBA::ULong i = 0; i < num_ctxts; i++ ){
      len <<= *pd_giopS;
      CORBA::String_var name(CORBA::string_alloc(len - 1));
      pd_giopS->get_char_array((CORBA::Char*)(char*)name, len);

      len <<= *pd_giopS;
      CORBA::String_var value(CORBA::string_alloc(len - 1));
      pd_giopS->get_char_array((CORBA::Char*)(char*)value, len);

      ((ContextImpl*)(CORBA::Context_ptr)pd_context)->
	insert_single_consume(name._retn(), value._retn());
    }
  }

  return pd_context;
}


void
ServerRequestImpl::params(CORBA::NVList_ptr parameters)
{
  if( pd_state != SR_READY ) {
    pd_state = SR_ERROR;
    throw CORBA::BAD_INV_ORDER(0, CORBA::COMPLETED_NO);
  }
  if( CORBA::is_nil(parameters) ) {
    pd_state = SR_ERROR;
    throw CORBA::BAD_PARAM(0, CORBA::COMPLETED_NO);
  }

  pd_params = parameters;

  { // unmarshal the arguments
    CORBA::ULong num_args = pd_params->count();

    for( CORBA::ULong i = 0; i < num_args; i++){
      CORBA::NamedValue_ptr arg = pd_params->item(i);
      if( arg->flags() & CORBA::ARG_IN || arg->flags() & CORBA::ARG_INOUT )
	arg->value()->NP_unmarshalDataOnly(*pd_giopS);
    }
  }

  pd_giopS->RequestReceived();
  pd_state = SR_GOT_PARAMS;
}


void
ServerRequestImpl::result(CORBA::Any* value)
{
  if( !(pd_state == SR_GOT_PARAMS || pd_state == SR_GOT_CTX) ) {
    pd_state = SR_ERROR;
    throw CORBA::BAD_INV_ORDER(0, CORBA::COMPLETED_NO);
  }
  if( !value ) {
    pd_state = SR_ERROR;
    throw CORBA::BAD_PARAM(0, CORBA::COMPLETED_NO);
  }

  pd_result = value;
  pd_state = SR_GOT_RESULT;
}


void
ServerRequestImpl::exception(CORBA::Any* value)
{
  if( !(pd_state == SR_GOT_PARAMS || pd_state == SR_GOT_CTX) ) {
    pd_state = SR_ERROR;
    throw CORBA::BAD_INV_ORDER(0, CORBA::COMPLETED_NO);
  }
  if( !value ) {
    pd_state = SR_ERROR;
    throw CORBA::BAD_PARAM(0, CORBA::COMPLETED_NO);
  }

  pd_exception = value;
  pd_state = SR_EXCEPTION;
}
