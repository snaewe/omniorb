// -*- Mode: C++; -*-
//                            Package   : omniORB
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

#include <omniORB4/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <request.h>
#include <deferredRequest.h>
#include <context.h>
#include <string.h>
#include <omniORB4/callDescriptor.h>
#include <remoteIdentity.h>
#include <exceptiondefs.h>


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


class omni_RequestImpl_callDesc : public omniCallDescriptor
{
public:
  inline omni_RequestImpl_callDesc(const char* op,
				   size_t oplen,
				   _CORBA_Boolean oneway,
				   RequestImpl& impl) :
    omniCallDescriptor(0,op,oplen,oneway,0,0,0), pd_impl(impl) {}

  void marshalArguments(cdrStream& s) {
    pd_impl.marshalArgs(s);
  }

  void unmarshalReturnedValues(cdrStream& s) {
    pd_impl.unmarshalArgs(s);
  }
private:
  RequestImpl& pd_impl;
};

				    
CORBA::Status
RequestImpl::invoke()
{
  if( pd_state != RS_READY && pd_state != RS_DEFERRED )
    OMNIORB_THROW(BAD_INV_ORDER,0, CORBA::COMPLETED_NO);

  try {

    CORBA::ULong operation_len = strlen(pd_operation) + 1;
    omniObjRef* o = pd_target->_PR_getobj();

    omni_RequestImpl_callDesc call_desc(pd_operation,operation_len,0,*this);

    omniCallDescriptor::ContextInfo ctxt_info;
    if( !CORBA::is_nil(pd_contexts) ) {
      ContextListImpl* context_list = (ContextListImpl*)
	                               (CORBA::ContextList_ptr)pd_contexts;
      ctxt_info.context = pd_context;
      ctxt_info.expected = context_list->NP_list();
      ctxt_info.num_expected = context_list->count();
      call_desc.set_context_info(&ctxt_info);
    }

    o->_invoke(call_desc);
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
    CORBA::ULong operation_len = strlen(pd_operation) + 1;
    omniObjRef* o = pd_target->_PR_getobj();

    omni_RequestImpl_callDesc call_desc(pd_operation,operation_len,1,*this);

    omniCallDescriptor::ContextInfo ctxt_info;
    if( !CORBA::is_nil(pd_contexts) ) {
      ContextListImpl* context_list = (ContextListImpl*)
	                              (CORBA::ContextList_ptr)pd_contexts;
      ctxt_info.context = pd_context;
      ctxt_info.expected = context_list->NP_list();
      ctxt_info.num_expected = context_list->count();
      call_desc.set_context_info(&ctxt_info);
    }

    o->_invoke(call_desc);
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


void
RequestImpl::marshalArgs(cdrStream& s)
{
  CORBA::ULong num_args = pd_arguments->count();

  for( CORBA::ULong i = 0; i < num_args; i++ ){
    CORBA::NamedValue_ptr arg = pd_arguments->item(i);
    if( arg->flags() & CORBA::ARG_IN || arg->flags() & CORBA::ARG_INOUT )
      arg->value()->NP_marshalDataOnly(s);
  }
}


void
RequestImpl::unmarshalArgs(cdrStream& s)
{
  CORBA::ULong num_args = pd_arguments->count();

  for( CORBA::ULong i = 0; i < num_args; i++){
    CORBA::NamedValue_ptr arg = pd_arguments->item(i);
    if( arg->flags() & CORBA::ARG_OUT || arg->flags() & CORBA::ARG_INOUT )
      arg->value()->NP_unmarshalDataOnly(s);
  }
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

static CORBA::Any dummy_any;

class omniNilRequest : public CORBA::Request {
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
  static omniNilRequest* _the_nil_ptr = 0;
  if( !_the_nil_ptr ) {
    omni::nilRefLock().lock();
    if( !_the_nil_ptr )  _the_nil_ptr = new omniNilRequest;
    omni::nilRefLock().unlock();
  }
  return _the_nil_ptr;
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
