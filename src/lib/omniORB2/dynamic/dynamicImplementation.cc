// -*- Mode: C++; -*-
//                            Package   : omniORB2
// dynamicImplementation.cc   Created on: 11/1998
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
//   Dynamic Skeleton Interface (DSI).
//

#include <dynamicImplementation.h>
#include <pseudo.h>
#include <string.h>
#include <dynException.h>

//////////////////////////////////////////////////////////////////////
////////////////////////////// DsiObject /////////////////////////////
//////////////////////////////////////////////////////////////////////

DsiObject::DsiObject(CORBA::BOA::DynamicImplementation_ptr dynImpl,
		     const char* intfRepoId)
{
  omniObject::PR_IRRepositoryId(intfRepoId);
  this->PR_setobj(this);
  pd_dynImpl = dynImpl;
}


DsiObject::~DsiObject()
{
  delete pd_dynImpl;
}


CORBA::Boolean
DsiObject::dispatch(GIOP_S& s,const char *op, CORBA::Boolean response_expected)
{
  ServerRequestImpl server_request(op, &s, response_expected);
  CORBA::Environment_var env = new EnvironmentImpl();
  if( !env )  throw CORBA::NO_MEMORY(0, CORBA::COMPLETED_MAYBE);

  // Systems exceptions are passed via <env>. We throw them on to the
  // next level to deal with. BAD_OPERATION just returns 0, so that
  // GIOP_S::dispatch() can go on to check if <op> is one of the
  // standard Object operations.
  try{
    // Upcall into application supplied implementation.
    pd_dynImpl->invoke(&server_request, *env);

    if( env->exception() )
      env->exception()->_raise();
  }
  catch(CORBA::BAD_OPERATION&) {
    return 0;
  }

  // Check that server_request has gotten to a legal state.
  if( server_request.state() == ServerRequestImpl::SR_READY ){
    if( omniORB::traceLevel > 0 ){
      omniORB::log <<
	"Warning: omniORB2 has detected that a Dynamic Implementation\n"
	" Routine (DynamicImplementation::invoke()) failed to call\n"
	" params() on the ServerRequest object.\n";
      omniORB::log.flush();
    }
    throw CORBA::UNKNOWN(0, CORBA::COMPLETED_NO);
  }
  if( server_request.state() == ServerRequestImpl::SR_ERROR ) {
    if( omniORB::traceLevel > 0 ){
      omniORB::log <<
	"Warning: omniORB2 has detected that a Dynamic Implementation\n"
	" Routine (DynamicImplementation::invoke()) did not properly\n"
	" implement the Dynamic Skeleton Interface.\n";
      omniORB::log.flush();
    }
    throw CORBA::UNKNOWN(0, CORBA::COMPLETED_NO);
  }

  // It is legal for the caller to ask for no response even if the
  // operation is not <oneway>. If no response is required, we do
  // nothing.

  if( response_expected ){
    switch( server_request.state() ){
    case ServerRequestImpl::SR_GOT_PARAMS:
    case ServerRequestImpl::SR_GOT_CTX:
    case ServerRequestImpl::SR_GOT_RESULT:
      {
	// Calculate the message size.
	CORBA::ULong msgsize = (CORBA::ULong) GIOP_S::ReplyHeaderSize();
	if( server_request.result() )
	  msgsize = server_request.result()->NP_alignedDataOnlySize(msgsize);
	CORBA::ULong num_args = server_request.params()->count();
	for( CORBA::ULong i = 0; i < num_args; i++ ){
	  CORBA::NamedValue_ptr arg = server_request.params()->item(i);
	  if( arg->flags() & CORBA::ARG_OUT ||
	      arg->flags() & CORBA::ARG_INOUT )
	    msgsize = arg->value()->NP_alignedDataOnlySize(msgsize);
	}

	s.InitialiseReply(GIOP::NO_EXCEPTION, msgsize);

	// Marshal the result and OUT/INOUT parameters.
	if( server_request.result() )
	  server_request.result()->NP_marshalDataOnly(s);
	for( CORBA::ULong j = 0; j < num_args; j++ ){
	  CORBA::NamedValue_ptr arg = server_request.params()->item(j);
	  if( arg->flags() & CORBA::ARG_OUT ||
	      arg->flags() & CORBA::ARG_INOUT )
	    arg->value()->NP_marshalDataOnly(s);
	}
	break;
      }

    case ServerRequestImpl::SR_EXCEPTION:  // User & System exception
      {
	CORBA::ULong msgsize = (CORBA::ULong) GIOP_S::ReplyHeaderSize();

	CORBA::TypeCode_var tc = server_request.exception()->type();

	// Exception TypeCodes are guarenteed to have a non-empty id().
	const char* intfRepoId = tc->id();
	CORBA::ULong len = strlen(intfRepoId) + 1;
	msgsize = omni::align_to(msgsize, omni::ALIGN_4) + 4 + len;
	msgsize = server_request.exception()->NP_alignedDataOnlySize(msgsize);

	if (isaSystemException(server_request.exception())) {
	  s.InitialiseReply(GIOP::SYSTEM_EXCEPTION, msgsize);
	}
	else {
	  s.InitialiseReply(GIOP::USER_EXCEPTION, msgsize);
	}

	len >>= s;
	s.put_char_array((CORBA::Char*)intfRepoId, len);
	server_request.exception()->NP_marshalDataOnly(s);
	break;
      }

    default:
      // Never get here
      break;
    }
  }

  s.ReplyCompleted();

  return 1;
}


void*
DsiObject::_widenFromTheMostDerivedIntf(const char* type_id,
					_CORBA_Boolean is_cxx_type_id)
{
  if( is_cxx_type_id )  return 0;
  if( !type_id )  return (CORBA::Object_ptr) this;
  if( strcmp(type_id, NP_IRRepositoryId()) )  return 0;
  return (DsiObject_ptr) this;
}

//////////////////////////////////////////////////////////////////////
//////////////////////// DynamicImplementation ///////////////////////
//////////////////////////////////////////////////////////////////////


#define pd_boa         (((DynamicImplementation_pd*)pd)->boa)
#define pd_object      (((DynamicImplementation_pd*)pd)->object)


CORBA::BOA::
DynamicImplementation::DynamicImplementation()
{
  pd = new DynamicImplementation_pd;
  pd_boa = 0;
  pd_object = 0;
}


CORBA::BOA::
DynamicImplementation::~DynamicImplementation()
{
  delete (DynamicImplementation_pd*)pd;
}


CORBA::Object_ptr
CORBA::BOA::
DynamicImplementation::_this()
{
  // This must only be called from within invoke().
  return pd_object->_this();
}


CORBA::BOA_ptr
CORBA::BOA::
DynamicImplementation::_boa()
{
  // This must only be called from within invoke().
  return pd_boa;
}

//////////////////////////////////////////////////////////////////////
///////////////////////////// CORBA::BOA /////////////////////////////
//////////////////////////////////////////////////////////////////////

CORBA::Object_ptr
CORBA::
BOA::create_dynamic_object(DynamicImplementation_ptr dir,
			   const char* intfRepoId)
{
  if( !dir )  throw BAD_PARAM(0, COMPLETED_NO);

  DsiObject_ptr obj = new DsiObject(dir, intfRepoId);
  if( !obj )  _CORBA_new_operator_return_null();

  ((DynamicImplementation_pd*)dir->NP_pd())->object = obj;
  ((DynamicImplementation_pd*)dir->NP_pd())->boa = this;

  return obj;
}
