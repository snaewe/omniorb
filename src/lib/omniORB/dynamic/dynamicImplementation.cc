// -*- Mode: C++; -*-
//                            Package   : omniORB
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

#define ENABLE_CLIENT_IR_SUPPORT
#include <omniORB4/CORBA.h>
#include <omniORB4/IOP_S.h>
#include <omniORB4/callDescriptor.h>
#include <omniORB4/callHandle.h>
#include <initRefs.h>
#include <dynamicLib.h>
#include <exceptiondefs.h>
#include <invoker.h>
#include <dynamicImplementation.h>
#include <poacurrentimpl.h>
#include <localIdentity.h>
#include <poaimpl.h>


OMNI_USING_NAMESPACE(omni)

//////////////////////////////////////////////////////////////////////
//////////////////////// DynamicImplementation ///////////////////////
//////////////////////////////////////////////////////////////////////

PortableServer::DynamicImplementation::~DynamicImplementation() {}


CORBA::Object_ptr
PortableServer::DynamicImplementation::_this()
{
  omniCurrent* current = omniCurrent::get();
  if (current) {
    omniCallDescriptor* call_desc = current->callDescriptor();

    if (call_desc &&
	call_desc->localId()->servant() == (omniServant*)this) {

      // In context of an invocation on this servant
      omniObjRef* ref = omniOrbPOACurrent::real_get_reference(call_desc);
      OMNIORB_ASSERT(ref);
      return (CORBA::Object_ptr)ref->_ptrToObjRef(CORBA::Object::_PD_repoId);
    }
  }
  // Not in context of invocation
  throw PortableServer::POA::WrongPolicy(); // WrongPolicy? eh?
}


CORBA::Boolean
PortableServer::DynamicImplementation::_is_a(const char* logical_type_id)
{
  omniCurrent* current = omniCurrent::get();
  if (current) {
    omniCallDescriptor* desc = current->callDescriptor();

    if (desc &&
	desc->poa() &&
	desc->localId()->servant() == (omniServant*)this) {

      PortableServer::ObjectId oid;
      desc->poa()->localId_to_ObjectId(desc->localId(), oid);

      const char* repoId = _primary_interface(oid, desc->poa());

      if( !repoId ) {
	omniORB::logs(1, "The _primary_interface() of a dynamic implementation"
		      " returned 0.");
	return 0;
      }
      return omni::ptrStrMatch(repoId, logical_type_id);
    }
  }
  return 0;
}


#ifdef HAS_Cplusplus_Namespace
namespace {
#endif
  class DSIPostInvoker {
  public:
    inline DSIPostInvoker(omniCallHandle::PostInvokeHook* hook)
      : pd_hook(hook) {}
    inline ~DSIPostInvoker() {
      if (pd_hook)
	pd_hook->postinvoke();
    }
  private:
    omniCallHandle::PostInvokeHook* pd_hook;
  };

  class DSIMainThreadTask : public omniTask {
  public:
    inline DSIMainThreadTask(PortableServer::DynamicImplementation* servant,
			     omniServerRequest& sreq,
			     omni_tracedmutex* mu, omni_tracedcondition* cond)
      : omniTask(omniTask::DedicatedThread),
	pd_servant(servant),
	pd_sreq(sreq),
	pd_mu(mu),
	pd_cond(cond),
	pd_except(0),
	pd_done(0)
    {
      if (omniORB::trace(25)) {
	omniORB::logger l;
	l << "Preparing to dispatch '" << sreq.calldesc()->op()
	  << "' to main thread\n";
      }
    }

    void execute();
    // Called by the async invoker. Performs the upcall. If an
    // exception occurs, places a copy in pd_except.

    void wait();
    // Wait for execute() to finish. Throws the exception in pd_except
    // if there is one.
    
  private:
    PortableServer::DynamicImplementation* pd_servant;
    omniServerRequest&                     pd_sreq;
    omni_tracedmutex*                      pd_mu;
    omni_tracedcondition*                  pd_cond;
    CORBA::Exception*                      pd_except;
    int                                    pd_done;
  };

#ifdef HAS_Cplusplus_Namespace
};
#endif



_CORBA_Boolean
PortableServer::DynamicImplementation::_dispatch(omniCallHandle& handle)
{
  const char* op = handle.operation_name();

  // We do not want to handle standard object operations ...
  if( omni::strMatch(op, "_is_a"          ) ||
      omni::strMatch(op, "_non_existent"  ) ||
      omni::strMatch(op, "_interface"     ) ||
      omni::strMatch(op, "_implementation") )
    return 0;

  omniServerRequest sreq(handle);
  sreq.calldesc()->poa(handle.poa());
  sreq.calldesc()->localId(handle.localId());

  {
    DSIPostInvoker postinvoker(handle.postinvoke_hook());

    if (!handle.mainthread_mu()) {
      // Upcall into application
      poaCurrentStackInsert insert(sreq.calldesc());
      invoke(&sreq);
    }
    else {
      // Main thread policy
      DSIMainThreadTask mtt(this, sreq,
			    handle.mainthread_mu(), handle.mainthread_cond());
      int i = orbAsyncInvoker->insert(&mtt); OMNIORB_ASSERT(i);
      mtt.wait();
    }
  }

  // It is legal for the caller to ask for no response even if the
  // operation is not <oneway>.  If no response is required, we do
  // nothing.

  switch( sreq.state() ){
  case omniServerRequest::SR_READY:
    if( omniORB::trace(1) ){
      omniORB::log <<
	"omniORB: WARNING -- A Dynamic Implementation Routine\n"
	" (DynamicImplementation::invoke()) failed to call arguments()\n"
	" on the ServerRequest object. BAD_INV_ORDER is thrown.\n";
      omniORB::log.flush();
    }
    OMNIORB_THROW(BAD_INV_ORDER,
		  BAD_INV_ORDER_ArgumentsNotCalled,
		  CORBA::COMPLETED_NO);


  case omniServerRequest::SR_GOT_PARAMS:
  case omniServerRequest::SR_GOT_RESULT:
  case omniServerRequest::SR_EXCEPTION:
    sreq.do_reply();
    break;

  case omniServerRequest::SR_DSI_ERROR:
    if( omniORB::trace(1) ){
      omniORB::log <<
	"omniORB: WARNING -- A Dynamic Implementation Routine\n"
	" (DynamicImplementation::invoke()) did not properly implement\n"
	" the Dynamic Skeleton Interface.\n";
      omniORB::log.flush();
    }
    OMNIORB_THROW(BAD_INV_ORDER,
		  BAD_INV_ORDER_ErrorInDynamicImplementation,
		  CORBA::COMPLETED_NO);

  case omniServerRequest::SR_ERROR:
    if( omniORB::trace(1) ) {
      omniORB::log <<
	"omniORB: WARNING -- A system exception was thrown when\n"
	" unmarshalling arguments for a DSI servant.  However the Dynamic\n"
	" Implementation Routine (DynamicImplementation::invoke()) did not\n"
	" propagate the exception or pass it to the server request.\n"
	" CORBA::MARSHAL is being passed back to the client anyway.\n";
      omniORB::log.flush();
    }
    OMNIORB_THROW(MARSHAL,
		  MARSHAL_ExceptionInDSINotPropagated,
		  CORBA::COMPLETED_MAYBE);
  }

  return 1;
}

//////////////////////////////////////////////////////////////////////

omniObjRef*
PortableServer::DynamicImplementation::_do_get_interface()
{
  CORBA::_objref_InterfaceDef* p = _get_interface();
  if( p )  return p->_PR_getobj();

  // If we get here then we assume that _get_interface has not
  // been overriden, and provide the default implementation.

  const char* repoId = 0;

  omniCurrent* current = omniCurrent::get();
  if (current) {
    omniCallDescriptor* desc = current->callDescriptor();

    if (desc &&
	desc->poa() &&
	desc->localId()->servant() == (omniServant*)this) {

      PortableServer::ObjectId oid;
      desc->poa()->localId_to_ObjectId(desc->localId(), oid);

      repoId = _primary_interface(oid, desc->poa());

      if( !repoId ) {
	omniORB::logs(1, "The _primary_interface() of a dynamic implementation"
		      " returned 0.");
	OMNIORB_THROW(INTF_REPOS,
		      INTF_REPOS_PrimaryInterfaceReturnedZero,
		      CORBA::COMPLETED_NO);
      }
    }
  }
  if (!repoId)
    OMNIORB_THROW(INTF_REPOS,
		  INTF_REPOS_NotAvailable,
		  CORBA::COMPLETED_NO);

  // Obtain the object reference for the interface repository.
  CORBA::Object_var repository =
    omniInitialReferences::resolve("InterfaceRepository");

  if( CORBA::is_nil(repository) )
    OMNIORB_THROW(INTF_REPOS,
		  INTF_REPOS_NotAvailable,
		  CORBA::COMPLETED_NO);

  // Make a call to the interface repository.
  omniStdCallDesc::_cCORBA_mObject_i_cstring
    call_desc(omniDynamicLib::ops->lookup_id_lcfn, "lookup_id", 10, repoId);
  repository->_PR_getobj()->_invoke(call_desc);

  return call_desc.result() ? call_desc.result()->_PR_getobj() : 0;
}



void
DSIMainThreadTask::execute()
{
  if (omniORB::traceInvocations) {
    omniORB::logger l;
    l << "Main thread dispatch '" << pd_sreq.calldesc()->op() << "'\n";
  }

  try {
    poaCurrentStackInsert insert(pd_sreq.calldesc());
    pd_servant->invoke(&pd_sreq);
  }
#ifdef HAS_Cplusplus_catch_exception_by_base
  catch (CORBA::Exception& ex) {
    pd_except = CORBA::Exception::_duplicate(&ex);
  }
#else
#  define DUPLICATE_AND_STORE(name) \
  catch (CORBA::name& ex) { \
    pd_except = CORBA::Exception::_duplicate(&ex); \
  }

  OMNIORB_FOR_EACH_SYS_EXCEPTION(DUPLICATE_AND_STORE)
#  undef DUPLICATE_AND_STORE

  catch (omniORB::StubUserException& uex) {
    pd_except = CORBA::Exception::_duplicate(uex.ex());
  }
#endif
  catch (...) {
    CORBA::UNKNOWN ex;
    pd_except = CORBA::Exception::_duplicate(&ex);
  }

  {
    // Wake up the dispatch thread
    omni_tracedmutex_lock l(*pd_mu);
    pd_done = 1;
    pd_cond->signal();
  }
}

void
DSIMainThreadTask::wait()
{
  {
    omni_tracedmutex_lock l(*pd_mu);
    while (!pd_done)
      pd_cond->wait();
  }
  if (pd_except) {
    // This interesting construction contrives to ask the
    // heap-allocated exception to throw a copy of itself, then
    // deletes it.
    try {
      pd_except->_raise();
    }
    catch (...) {
      delete pd_except;
      throw;
    }
  }
}
