// -*- Mode: C++; -*-
//                            Package   : omniORB3
// request.cc                 Created on: 9/1998
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
//   Implementation of CORBA::Request.
//

#include <omniORB3/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <request.h>
#include <deferredRequest.h>
#include <context.h>
#include <string.h>
#include <omniORB3/callDescriptor.h>
#include <remoteIdentity.h>
#include <exception.h>


#define INVOKE_DONE()  if( pd_state == RS_READY )  pd_state = RS_DONE;


RequestImpl::RequestImpl(CORBA::Object_ptr target, const char* operation)
{
  pd_target = CORBA::Object::_duplicate(target);

  pd_operation = CORBA::string_dup(operation);

  pd_arguments = new NVListImpl();

  pd_result = new NamedValueImpl(CORBA::Flags(0));
  pd_result->value()->replace(CORBA::_tc_void, (void*)0);

  pd_environment = new EnvironmentImpl;

  pd_state = RS_READY;
  pd_deferredRequest = 0;
  pd_sysExceptionToThrow = 0;

  if( CORBA::is_nil(target) )
    throw omniORB::fatalException(__FILE__,__LINE__,
	  "CORBA::RequestImpl::RequestImpl()");

  if( !operation || !*operation )
    OMNIORB_THROW(BAD_PARAM,0, CORBA::COMPLETED_NO);
}


RequestImpl::RequestImpl(CORBA::Object_ptr target, const char* operation,
			 CORBA::Context_ptr context,
			 CORBA::NVList_ptr arguments,
			 CORBA::NamedValue_ptr result)
{
  pd_target = CORBA::Object::_duplicate(target);

  pd_operation = CORBA::string_dup(operation);

  if( CORBA::is_nil(arguments) )
    pd_arguments = new NVListImpl();
  else
    pd_arguments = CORBA::NVList::_duplicate(arguments);

  if( CORBA::is_nil(result) ){
    pd_result = new NamedValueImpl(CORBA::Flags(0));
    pd_result->value()->replace(CORBA::_tc_void, (void*) 0);
  } else
    pd_result = CORBA::NamedValue::_duplicate(result);

  pd_environment = new EnvironmentImpl;

  pd_context     = CORBA::Context::_duplicate(context);

  pd_state = RS_READY;
  pd_deferredRequest = 0;
  pd_sysExceptionToThrow = 0;

  if( CORBA::is_nil(target) )
    throw omniORB::fatalException(__FILE__,__LINE__,
	  "CORBA::RequestImpl::RequestImpl()");

  if( !operation || !*operation )
    OMNIORB_THROW(BAD_PARAM,0, CORBA::COMPLETED_NO);
}


RequestImpl::RequestImpl(CORBA::Object_ptr target, const char* operation,
			 CORBA::Context_ptr context,
			 CORBA::NVList_ptr arguments,
			 CORBA::NamedValue_ptr result,
			 CORBA::ExceptionList_ptr exceptions,
			 CORBA::ContextList_ptr contexts)
{
  pd_target = CORBA::Object::_duplicate(target);

  pd_operation = CORBA::string_dup(operation);

  if( CORBA::is_nil(arguments) )
    pd_arguments = new NVListImpl();
  else
    pd_arguments = CORBA::NVList::_duplicate(arguments);

  if( CORBA::is_nil(result) ){
    pd_result = new NamedValueImpl(CORBA::Flags(0));
    pd_result->value()->replace(CORBA::_tc_void, (void*) 0);
  } else
    pd_result = CORBA::NamedValue::_duplicate(result);

  pd_environment = new EnvironmentImpl;

  pd_exceptions  = CORBA::ExceptionList::_duplicate(exceptions);
  pd_contexts    = CORBA::ContextList::_duplicate(contexts);
  pd_context     = CORBA::Context::_duplicate(context);

  pd_state = RS_READY;
  pd_deferredRequest = 0;
  pd_sysExceptionToThrow = 0;

  if( CORBA::is_nil(target) )
    throw omniORB::fatalException(__FILE__,__LINE__,
	  "CORBA::RequestImpl::RequestImpl()");

  if( !operation || !*operation )
    OMNIORB_THROW(BAD_PARAM,0, CORBA::COMPLETED_NO);
}


RequestImpl::~RequestImpl()
{
  if( pd_deferredRequest && omniORB::traceLevel > 0 ){
    omniORB::log <<
      "omniORB: WARNING -- The application has not collected the reponse of\n"
      " a deferred DII request.  Use Request::get_response() or\n"
      " poll_response().\n";
    omniORB::log.flush();
  }
  if( pd_sysExceptionToThrow )  delete pd_sysExceptionToThrow;
}


CORBA::Object_ptr
RequestImpl::target() const
{
  return pd_target;
}


const char*
RequestImpl::operation() const
{
  return pd_operation;
}


CORBA::NVList_ptr
RequestImpl::arguments()
{
  if( pd_sysExceptionToThrow )  pd_sysExceptionToThrow->_raise();

  return pd_arguments;
}


CORBA::NamedValue_ptr
RequestImpl::result()
{
  if( pd_sysExceptionToThrow )  pd_sysExceptionToThrow->_raise();

  return pd_result;
}


CORBA::Environment_ptr
RequestImpl::env()
{
  if( pd_sysExceptionToThrow )  pd_sysExceptionToThrow->_raise();

  return pd_environment;
}


CORBA::ExceptionList_ptr
RequestImpl::exceptions()
{
  if( CORBA::is_nil(pd_exceptions) )
    pd_exceptions = new ExceptionListImpl();

  return pd_exceptions;
}


CORBA::ContextList_ptr
RequestImpl::contexts()
{
  if( CORBA::is_nil(pd_contexts) )
    pd_contexts = new ContextListImpl();

  return pd_contexts;
}


CORBA::Context_ptr
RequestImpl::ctx() const
{
  return pd_context;
}


void
RequestImpl::ctx(CORBA::Context_ptr context)
{
  if (!CORBA::Context::PR_is_valid(context))
    OMNIORB_THROW(BAD_PARAM,0,CORBA::COMPLETED_NO);
  if( pd_state != RS_READY )
    OMNIORB_THROW(BAD_INV_ORDER,0, CORBA::COMPLETED_NO);

  pd_context = context;
}


CORBA::Any&
RequestImpl::add_in_arg()
{
  if( pd_state != RS_READY )
    OMNIORB_THROW(BAD_INV_ORDER,0, CORBA::COMPLETED_NO);

  return *(pd_arguments->add(CORBA::ARG_IN)->value());
}


CORBA::Any&
RequestImpl::add_in_arg(const char* name)
{
  if( pd_state != RS_READY )
    OMNIORB_THROW(BAD_INV_ORDER,0, CORBA::COMPLETED_NO);

  return *(pd_arguments->add_item(name, CORBA::ARG_IN)->value());
}


CORBA::Any&
RequestImpl::add_inout_arg()
{
  if( pd_state != RS_READY )
    OMNIORB_THROW(BAD_INV_ORDER,0, CORBA::COMPLETED_NO);

  return *(pd_arguments->add(CORBA::ARG_INOUT)->value());
}


CORBA::Any&
RequestImpl::add_inout_arg(const char* name)
{
  if( pd_state != RS_READY )
    OMNIORB_THROW(BAD_INV_ORDER,0, CORBA::COMPLETED_NO);

  return *(pd_arguments->add_item(name, CORBA::ARG_INOUT)->value());
}


CORBA::Any&
RequestImpl::add_out_arg()
{
  if( pd_state != RS_READY )
    OMNIORB_THROW(BAD_INV_ORDER,0, CORBA::COMPLETED_NO);

  return *(pd_arguments->add(CORBA::ARG_OUT)->value());
}


CORBA::Any&
RequestImpl::add_out_arg(const char* name)
{
  if( pd_state != RS_READY )
    OMNIORB_THROW(BAD_INV_ORDER,0, CORBA::COMPLETED_NO);

  return *(pd_arguments->add_item(name, CORBA::ARG_OUT)->value());
}


void
RequestImpl::set_return_type(CORBA::TypeCode_ptr tc)
{
  if (!CORBA::TypeCode::PR_is_valid(tc))
    OMNIORB_THROW(BAD_PARAM,0,CORBA::COMPLETED_NO);
  if( pd_state != RS_READY )
    OMNIORB_THROW(BAD_INV_ORDER,0, CORBA::COMPLETED_NO);

  pd_result->value()->replace(tc, (void*)0);
}


CORBA::Any&
RequestImpl::return_value()
{
  if( pd_sysExceptionToThrow )  pd_sysExceptionToThrow->_raise();

  return *(pd_result->value());
}


CORBA::Status
RequestImpl::invoke()
{
  if( pd_state != RS_READY && pd_state != RS_DEFERRED )
    OMNIORB_THROW(BAD_INV_ORDER,0, CORBA::COMPLETED_NO);

  try{
    int retries = 0;
    int fwd;

    CORBA::ULong operation_len = strlen(pd_operation) + 1;
    omniObjRef* o = pd_target->_PR_getobj();

#ifndef EGCS_WORKAROUND
  _again:
#else
    while(1) {
#endif
      o->_assertExistsAndTypeVerified();

      try{

	omniRopeAndKey rak;
	fwd = o->_getRopeAndKey(rak);

	CORBA::Boolean reuse = 0; //?? move this up into transport

	try {
	  // Get a GIOP driven strand.
	  GIOP_C giop_c(rak.rope());
	  reuse = giop_c.isReUsingExistingConnection();

	  ContextListImpl* context_list = 0;
	  if( !CORBA::is_nil(pd_contexts) )
	    context_list =
	      (ContextListImpl*)(CORBA::ContextList_ptr)pd_contexts;

	  // Calculate the size of the message.
	  CORBA::ULong message_size =
	    GIOP_C::RequestHeaderSize(rak.keysize(), operation_len);

	  message_size = calculateArgDataSize(message_size);
	  if( context_list )
	    message_size =
	      CORBA::Context::_NP_alignedSize(pd_context,
					      context_list->NP_list(),
					      context_list->count(),
					      message_size);

	  giop_c.InitialiseRequest(rak.key(), rak.keysize(),
				   pd_operation, operation_len,
				   message_size, 0);

	  // Marshal the arguments to the operation.
	  marshalArgs(giop_c);

	  // If context strings are supplied, marshal them.
	  if( context_list )
	    CORBA::Context::marshalContext(pd_context,
					   context_list->NP_list(),
					   context_list->count(),
					   giop_c);

	  // Wait for the reply.

	  switch( giop_c.ReceiveReply() ) {
	  case GIOP::NO_EXCEPTION:
	    // Unmarshal the result.
	    pd_result->value()->NP_unmarshalDataOnly(giop_c);
	    // This defaults to unmarshaling (void) if the return type
	    // has not been set.

	    // Unmarshal any out and inout arguments.
	    unmarshalArgs(giop_c);
	    giop_c.RequestCompleted();
	    INVOKE_DONE();
	    RETURN_CORBA_STATUS;

	  case GIOP::USER_EXCEPTION:
	    {
	      // Retrieve the Interface Repository ID of the exception.
	      CORBA::ULong repoIdLen;
	      repoIdLen <<= giop_c;
	      CORBA::String_var repoId(CORBA::string_alloc(repoIdLen - 1));
	      giop_c.get_char_array((CORBA::Char*)(char*)repoId,
					 repoIdLen);

	      CORBA::ULong exListLen = CORBA::is_nil(pd_exceptions) ?
		0 : pd_exceptions->count();

	      // Search for a match in the exception list.
	      for( CORBA::ULong i = 0; i < exListLen; i++ ){
		CORBA::TypeCode_ptr exType = pd_exceptions->item(i);

		if( !strcmp(repoId, exType->id()) ){
		  // Unmarshal the exception into an Any.
		  CORBA::Any* newAny = new CORBA::Any(exType, 0);
		  try {
		    newAny->NP_unmarshalDataOnly(giop_c);
		  }
		  catch(...) {
		    delete newAny;
		    throw;
		  }
		  // Encapsulate this in an UnknownUserException, which is
		  // placed into pd_environment.
		  CORBA::UnknownUserException* ex =
		    new CORBA::UnknownUserException(newAny);
		  pd_environment->exception(ex);
		  giop_c.RequestCompleted();
		  INVOKE_DONE();
		  RETURN_CORBA_STATUS;
		}
	      }
	      // The exception didn't match any in the list, or a list
	      // was not supplied.
	      giop_c.RequestCompleted(1);
	      OMNIORB_THROW(MARSHAL,0, CORBA::COMPLETED_MAYBE);
	    }

	  case GIOP::LOCATION_FORWARD:
	    {
	      CORBA::Object_var obj(CORBA::Object::_unmarshalObjRef(giop_c));
	      giop_c.RequestCompleted();
	      throw omniORB::LOCATION_FORWARD(obj._retn());
	    }

	  case GIOP::SYSTEM_EXCEPTION:
	    giop_c.RequestCompleted(1);
	    throw omniORB::fatalException(__FILE__,__LINE__,
					  "GIOP::SYSTEM_EXCEPTION should not"
					  " be returned by"
					  " GIOP_C::ReceiveReply()");
	  }
	}
	catch(const CORBA::COMM_FAILURE& ex) {
	  if( reuse ){
	    CORBA::TRANSIENT ex2(ex.minor(), ex.completed());
	    throw ex2;
	  }
	  else throw;
	}
      }
      catch(const CORBA::COMM_FAILURE& ex) {
	if( fwd ){
	  omni::revertToOriginalProfile(o);
	  CORBA::TRANSIENT ex2(ex.minor(), ex.completed());
	  if( !_omni_callTransientExceptionHandler(o, retries++, ex2) )
	    throw ex2;
	}
	else if( !_omni_callCommFailureExceptionHandler(o, retries++, ex) )
	  throw;
      }
      catch(const CORBA::TRANSIENT& ex) {
	if( !_omni_callTransientExceptionHandler(o, retries++, ex) )
	  throw;
      }
      catch(const CORBA::OBJECT_NOT_EXIST& ex){
	if( fwd ){
	  omni::revertToOriginalProfile(o);
	  CORBA::TRANSIENT ex2(ex.minor(), ex.completed());
	  if( !_omni_callTransientExceptionHandler(o, retries++, ex2) )
	    throw ex2;
	}
	else if( !_omni_callSystemExceptionHandler(o, retries++, ex) )
	  throw;
      }
      catch(const CORBA::SystemException& ex){
	if( !_omni_callSystemExceptionHandler(o, retries++, ex) )
	  throw;
      }
      catch(omniORB::LOCATION_FORWARD& ex) {
	if( CORBA::is_nil(ex.get_obj()) ) {
	  CORBA::COMM_FAILURE ex2(0, CORBA::COMPLETED_NO);
	  if( omniORB::traceLevel > 10 ){
	    omniORB::log << "Received GIOP::LOCATION_FORWARD message that"
	      " contains a nil object reference.\n";
	    omniORB::log.flush();
	  }
	  if( !_omni_callCommFailureExceptionHandler(o, retries++, ex2) )
	    throw ex2;
	}
	omni::locationForward(o, ex.get_obj()->_PR_getobj());
      }

#ifndef EGCS_WORKAROUND
      goto _again;
#else
    }
#endif
  }
  // Either throw system exceptions, or store in pd_environment.
  catch(CORBA::SystemException& ex){
    INVOKE_DONE();
    if( omniORB::diiThrowsSysExceptions ) {
      pd_sysExceptionToThrow = CORBA::Exception::_duplicate(&ex);
      throw;
    } else
      pd_environment->exception(CORBA::Exception::_duplicate(&ex));
  }
  INVOKE_DONE();
  RETURN_CORBA_STATUS;
}


CORBA::Status
RequestImpl::send_oneway()
{
  if( pd_state != RS_READY )
    OMNIORB_THROW(BAD_INV_ORDER,0, CORBA::COMPLETED_NO);

  try{
    int retries = 0;
    int fwd;

    CORBA::ULong operation_len = strlen(pd_operation) + 1;
    omniObjRef* o = pd_target->_PR_getobj();

#ifndef EGCS_WORKAROUND
  _again:
#else
    while(1) {
#endif
      o->_assertExistsAndTypeVerified();

      try{

	omniRopeAndKey rak;
	fwd = o->_getRopeAndKey(rak);

	CORBA::Boolean reuse = 0; //?? move this up into transport

	try {
	  // Get a GIOP driven strand.
	  GIOP_C giop_c(rak.rope());
	  reuse = giop_c.isReUsingExistingConnection();

	  ContextListImpl* context_list = 0;
	  if( !CORBA::is_nil(pd_contexts) )
	    context_list =
	      (ContextListImpl*)(CORBA::ContextList_ptr)pd_contexts;

	  // Calculate the size of the message.
	  CORBA::ULong message_size =
	    GIOP_C::RequestHeaderSize(rak.keysize(), operation_len);

	  message_size = calculateArgDataSize(message_size);
	  if( context_list )
	    message_size =
	      CORBA::Context::_NP_alignedSize(pd_context,
					      context_list->NP_list(),
					      context_list->count(),
					      message_size);

	  giop_c.InitialiseRequest(rak.key(), rak.keysize(),
				   pd_operation, operation_len,
				   message_size, 1);

	  // Marshal the arguments to the operation.
	  marshalArgs(giop_c);

	  // If context strings are supplied, marshal them.
	  if( context_list )
	    CORBA::Context::marshalContext(pd_context,
					   context_list->NP_list(),
					   context_list->count(),
					   giop_c);

	  // Wait for the reply.

	  switch( giop_c.ReceiveReply() ) {
	  case GIOP::NO_EXCEPTION:
	    giop_c.RequestCompleted();
	    INVOKE_DONE();
	    RETURN_CORBA_STATUS;

	  case GIOP::USER_EXCEPTION:
	  case GIOP::LOCATION_FORWARD:
	  case GIOP::SYSTEM_EXCEPTION:
	    giop_c.RequestCompleted(1);
	    throw omniORB::fatalException(__FILE__,__LINE__,
					  "GIOP::SYSTEM_EXCEPTION returned"
					  " unexpected code for oneway call");
	  }
	}
	catch(const CORBA::COMM_FAILURE& ex) {
	  if( reuse ){
	    CORBA::TRANSIENT ex2(ex.minor(), ex.completed());
	    throw ex2;
	  }
	  else throw;
	}
      }
      catch(const CORBA::COMM_FAILURE& ex) {
	if( fwd ){
	  omni::revertToOriginalProfile(o);
	  CORBA::TRANSIENT ex2(ex.minor(), ex.completed());
	  if( !_omni_callTransientExceptionHandler(o, retries++, ex2) )
	    throw ex2;
	}
	else if( !_omni_callCommFailureExceptionHandler(o, retries++, ex) )
	  throw;
      }
      catch(const CORBA::TRANSIENT& ex) {
	if( !_omni_callTransientExceptionHandler(o, retries++, ex) )
	  throw;
      }
      catch(const CORBA::OBJECT_NOT_EXIST& ex){
	if( fwd ){
	  omni::revertToOriginalProfile(o);
	  CORBA::TRANSIENT ex2(ex.minor(), ex.completed());
	  if( !_omni_callTransientExceptionHandler(o, retries++, ex2) )
	    throw ex2;
	}
	else if( !_omni_callSystemExceptionHandler(o, retries++, ex) )
	  throw;
      }
      catch(const CORBA::SystemException& ex){
	if( !_omni_callSystemExceptionHandler(o, retries++, ex) )
	  throw;
      }
      catch(omniORB::LOCATION_FORWARD& ex) {
	if( CORBA::is_nil(ex.get_obj()) ) {
	  CORBA::COMM_FAILURE ex2(0, CORBA::COMPLETED_NO);
	  if( omniORB::traceLevel > 10 ){
	    omniORB::log << "Received GIOP::LOCATION_FORWARD message that"
	      " contains a nil object reference.\n";
	    omniORB::log.flush();
	  }
	  if( !_omni_callCommFailureExceptionHandler(o, retries++, ex2) )
	    throw ex2;
	}
	omni::locationForward(o, ex.get_obj()->_PR_getobj());
      }

#ifndef EGCS_WORKAROUND
      goto _again;
#else
    }
#endif
  }
  // Either throw system exceptions, or store in pd_environment.
  catch(CORBA::SystemException& ex){
    INVOKE_DONE();
    if( omniORB::diiThrowsSysExceptions ) {
      pd_sysExceptionToThrow = CORBA::Exception::_duplicate(&ex);
      throw;
    } else
      pd_environment->exception(CORBA::Exception::_duplicate(&ex));
  }
  INVOKE_DONE();
  RETURN_CORBA_STATUS;
}


CORBA::Status
RequestImpl::send_deferred()
{
  if( pd_state != RS_READY )
    OMNIORB_THROW(BAD_INV_ORDER,0, CORBA::COMPLETED_NO);

  pd_state = RS_DEFERRED;
  pd_deferredRequest = new DeferredRequest(this);
  RETURN_CORBA_STATUS;
}


CORBA::Status
RequestImpl::get_response()
{
  if( pd_sysExceptionToThrow )  pd_sysExceptionToThrow->_raise();

  if( pd_state == RS_DONE )  RETURN_CORBA_STATUS;

  if( pd_state != RS_DEFERRED || !pd_deferredRequest )
    OMNIORB_THROW(BAD_INV_ORDER,0, CORBA::COMPLETED_NO);

  pd_deferredRequest->get_response();
  pd_sysExceptionToThrow = pd_deferredRequest->get_exception();
  pd_deferredRequest->die();
  pd_deferredRequest = 0;
  pd_state = RS_DONE;
  if( pd_sysExceptionToThrow )  pd_sysExceptionToThrow->_raise();
  RETURN_CORBA_STATUS;
}


CORBA::Boolean
RequestImpl::poll_response()
{
  if( pd_state == RS_DONE )  return CORBA::Boolean(1);

  if( pd_state != RS_DEFERRED || !pd_deferredRequest )
    OMNIORB_THROW(BAD_INV_ORDER,0, CORBA::COMPLETED_NO);

  CORBA::Boolean result = pd_deferredRequest->poll_response();

  if( result ) {
    pd_sysExceptionToThrow = pd_deferredRequest->get_exception();
    pd_deferredRequest->die();
    pd_deferredRequest = 0;
    pd_state = RS_DONE;

    // XXX Opengroup vsOrb tests for poll_response to raise an
    //     exception when the invocation results in a system exception.
    if( omniORB::diiThrowsSysExceptions ) {
      if (pd_sysExceptionToThrow) pd_sysExceptionToThrow->_raise();
    }
  }
  else {
    omni_thread::yield();
  }

  return result;
}


CORBA::Boolean
RequestImpl::NP_is_nil() const
{
  return 0;
}


CORBA::Request_ptr
RequestImpl::NP_duplicate()
{
  incrRefCount();
  return this;
}


CORBA::ULong
RequestImpl::calculateArgDataSize(CORBA::ULong msize)
{
  CORBA::ULong num_args = pd_arguments->count();

  for( CORBA::ULong i = 0; i < num_args; i++ ){
    CORBA::NamedValue_ptr arg = pd_arguments->item(i);
    if( arg->flags() & CORBA::ARG_IN || arg->flags() & CORBA::ARG_INOUT )
      msize = arg->value()->NP_alignedDataOnlySize(msize);
  }
  return msize;
}


void
RequestImpl::marshalArgs(GIOP_C& giop_c)
{
  CORBA::ULong num_args = pd_arguments->count();

  for( CORBA::ULong i = 0; i < num_args; i++ ){
    CORBA::NamedValue_ptr arg = pd_arguments->item(i);
    if( arg->flags() & CORBA::ARG_IN || arg->flags() & CORBA::ARG_INOUT )
      arg->value()->NP_marshalDataOnly(giop_c);
  }
}


void
RequestImpl::unmarshalArgs(GIOP_C& giop_c)
{
  CORBA::ULong num_args = pd_arguments->count();

  for( CORBA::ULong i = 0; i < num_args; i++){
    CORBA::NamedValue_ptr arg = pd_arguments->item(i);
    if( arg->flags() & CORBA::ARG_OUT || arg->flags() & CORBA::ARG_INOUT )
      arg->value()->NP_unmarshalDataOnly(giop_c);
  }
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

static CORBA::Any dummy_any;

class NilRequest : public CORBA::Request {
public:
  virtual CORBA::Object_ptr target() const {
    _CORBA_invoked_nil_pseudo_ref();
    return CORBA::Object::_nil();
  }
  virtual const char* operation() const {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
  virtual CORBA::NVList_ptr arguments() {
    _CORBA_invoked_nil_pseudo_ref();
    return CORBA::NVList::_nil();
  }
  virtual CORBA::NamedValue_ptr result() {
    _CORBA_invoked_nil_pseudo_ref();
    return CORBA::NamedValue::_nil();
  }
  virtual CORBA::Environment_ptr env() {
    _CORBA_invoked_nil_pseudo_ref();
    return CORBA::Environment::_nil();
  }
  virtual CORBA::ExceptionList_ptr exceptions() {
    _CORBA_invoked_nil_pseudo_ref();
    return CORBA::ExceptionList::_nil();
  }
  virtual CORBA::ContextList_ptr contexts() {
    _CORBA_invoked_nil_pseudo_ref();
    return CORBA::ContextList::_nil();
  }
  virtual CORBA::Context_ptr ctx() const {
    _CORBA_invoked_nil_pseudo_ref();
    return CORBA::Context::_nil();
  }
  virtual void ctx(CORBA::Context_ptr) {
    _CORBA_invoked_nil_pseudo_ref();
  }
  virtual CORBA::Any& add_in_arg() {
    _CORBA_invoked_nil_pseudo_ref();
    return dummy_any;
  }
  virtual CORBA::Any& add_in_arg(const char* name) {
    _CORBA_invoked_nil_pseudo_ref();
    return dummy_any;
  }
  virtual CORBA::Any& add_inout_arg() {
    _CORBA_invoked_nil_pseudo_ref();
    return dummy_any;
  }
  virtual CORBA::Any& add_inout_arg(const char* name) {
    _CORBA_invoked_nil_pseudo_ref();
    return dummy_any;
  }
  virtual CORBA::Any& add_out_arg() {
    _CORBA_invoked_nil_pseudo_ref();
    return dummy_any;
  }
  virtual CORBA::Any& add_out_arg(const char* name) {
    _CORBA_invoked_nil_pseudo_ref();
    return dummy_any;
  }
  virtual void set_return_type(CORBA::TypeCode_ptr tc) {
    _CORBA_invoked_nil_pseudo_ref();
  }
  virtual CORBA::Any& return_value() {
    _CORBA_invoked_nil_pseudo_ref();
    return dummy_any;
  }
  virtual CORBA::Status  invoke() {
    _CORBA_invoked_nil_pseudo_ref();
    RETURN_CORBA_STATUS;
  }
  virtual CORBA::Status  send_oneway() {
    _CORBA_invoked_nil_pseudo_ref();
    RETURN_CORBA_STATUS;
  }
  virtual CORBA::Status  send_deferred() {
    _CORBA_invoked_nil_pseudo_ref();
    RETURN_CORBA_STATUS;
  }
  virtual CORBA::Status  get_response() {
    _CORBA_invoked_nil_pseudo_ref();
    RETURN_CORBA_STATUS;
  }
  virtual CORBA::Boolean poll_response() {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
  virtual CORBA::Boolean NP_is_nil() const {
    return 1;
  }
  virtual CORBA::Request_ptr NP_duplicate() {
    return _nil();
  }
};

static NilRequest _nilRequest;

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

CORBA::Request::~Request() { pd_magic = 0; }


CORBA::Request_ptr
CORBA::
Request::_duplicate(Request_ptr p)
{
  if (!PR_is_valid(p))
    OMNIORB_THROW(BAD_PARAM,0,CORBA::COMPLETED_NO);
  if( !CORBA::is_nil(p) )  return p->NP_duplicate();
  else     return _nil();
}


CORBA::Request_ptr
CORBA::
Request::_nil()
{
  return &_nilRequest;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////// CORBA::Object ///////////////////////////
//////////////////////////////////////////////////////////////////////

void
CORBA::release(CORBA::Request_ptr p)
{
  if( CORBA::Request::PR_is_valid(p) && !CORBA::is_nil(p) )
    ((RequestImpl*)p)->decrRefCount();
}


CORBA::Status
CORBA::Object::_create_request(CORBA::Context_ptr ctx,
			       const char* operation,
			       CORBA::NVList_ptr arg_list,
			       CORBA::NamedValue_ptr result,
			       CORBA::Request_out request,
			       CORBA::Flags req_flags)
{
  if( _NP_is_pseudo() )  OMNIORB_THROW(NO_IMPLEMENT,0, CORBA::COMPLETED_NO);

  // NB. req_flags is ignored - ref. CORBA 2.2 section 20.28
  if( !CORBA::Context::PR_is_valid(ctx) ||
      !operation ||
      !CORBA::NVList::PR_is_valid(arg_list) ||
      !CORBA::NamedValue::PR_is_valid(result) )
    OMNIORB_THROW(BAD_PARAM,0, CORBA::COMPLETED_NO);

  request = new RequestImpl(this, operation, ctx, arg_list, result);
  RETURN_CORBA_STATUS;
}


CORBA::Status 
CORBA::Object::_create_request(CORBA::Context_ptr ctx,
                               const char* operation,
			       CORBA::NVList_ptr arg_list,
			       CORBA::NamedValue_ptr result,
			       CORBA::ExceptionList_ptr exceptions,
			       CORBA::ContextList_ptr ctxlist,
			       CORBA::Request_out request,
			       CORBA::Flags req_flags)
{
  if( _NP_is_pseudo() )  OMNIORB_THROW(NO_IMPLEMENT,0, CORBA::COMPLETED_NO);

  // NB. req_flags is ignored - ref. CORBA 2.2 section 20.28
  if( !CORBA::Context::PR_is_valid(ctx) ||
      !operation ||
      !CORBA::NVList::PR_is_valid(arg_list) ||
      !CORBA::NamedValue::PR_is_valid(result) ||
      !CORBA::ExceptionList::PR_is_valid(exceptions) ||
      !CORBA::ContextList::PR_is_valid(ctxlist))
    OMNIORB_THROW(BAD_PARAM,0, CORBA::COMPLETED_NO);

  request = new RequestImpl(this, operation, ctx, arg_list, result,
			    exceptions, ctxlist);
  RETURN_CORBA_STATUS;
}


CORBA::Request_ptr
CORBA::Object::_request(const char* operation) 
{
  if( _NP_is_pseudo() )  OMNIORB_THROW(NO_IMPLEMENT,0, CORBA::COMPLETED_NO);
  if( !operation )  OMNIORB_THROW(BAD_PARAM,0, CORBA::COMPLETED_NO);

  return new RequestImpl(this, operation);
}
