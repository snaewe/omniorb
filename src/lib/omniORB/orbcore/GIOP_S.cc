// -*- Mode: C++; -*-
//                            Package   : omniORB
// GIOP_S.cc                  Created on: 05/01/2001
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 2001 AT&T Laboratories Cambridge
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
//	*** PROPRIETORY INTERFACE ***
//

/*
  $Log$
  Revision 1.1.4.19  2002/03/18 12:38:25  dpg1
  Lower trace(0) to trace(1), propagate fatalException.

  Revision 1.1.4.18  2002/03/13 16:05:38  dpg1
  Transport shutdown fixes. Reference count SocketCollections to avoid
  connections using them after they are deleted. Properly close
  connections when in thread pool mode.

  Revision 1.1.4.17  2001/12/03 18:46:25  dpg1
  Race condition in giopWorker destruction.

  Revision 1.1.4.16  2001/11/12 13:47:09  dpg1
  Minor fixes.

  Revision 1.1.4.15  2001/10/17 16:33:27  dpg1
  New downcast mechanism for cdrStreams.

  Revision 1.1.4.14  2001/09/20 11:30:59  sll
  On the server, the final state of a GIOP_S is ReplyCompleted instead of
  Idle. This is necessary because the idle connection management code
  treats Idle as a state where the idle counter can be restarted.

  Revision 1.1.4.13  2001/09/10 17:44:34  sll
  Added stopIdleCounter() call inside dispatcher when the header has been
  received.

  Revision 1.1.4.12  2001/09/04 14:38:51  sll
  Added the boolean argument to notifyCommFailure to indicate if
  omniTransportLock is held by the caller.

  Revision 1.1.4.11  2001/08/15 10:26:11  dpg1
  New object table behaviour, correct POA semantics.

  Revision 1.1.4.10  2001/08/03 17:41:17  sll
  System exception minor code overhaul. When a system exeception is raised,
  a meaning minor code is provided.

  Revision 1.1.4.9  2001/07/31 16:28:01  sll
  Added GIOP BiDir support.

  Revision 1.1.4.8  2001/07/13 15:22:45  sll
  Call notifyWkPreUpCall at the right time and determine if there are
  buffered data to be served by another thread.

  Revision 1.1.4.7  2001/06/29 16:26:01  dpg1
  Reinstate tracing messages for new connections and handling locate
  requests.

  Revision 1.1.4.6  2001/05/31 16:18:12  dpg1
  inline string matching functions, re-ordered string matching in
  _ptrToInterface/_ptrToObjRef

  Revision 1.1.4.5  2001/05/29 17:03:50  dpg1
  In process identity.

  Revision 1.1.4.4  2001/05/04 13:55:27  sll
  When a system exception is raised, send the exception before skipping rest
  of the input message. Helpful if the client sends a message shorter than
  the header indicates. Otherwise the server will sit there waiting for the
  remaining bytes that will never come. Eventually the server side timeout
  mechanism kicks in but by then the server will just close the connection
  rather than telling the client it has detected a marshalling exception.

  Revision 1.1.4.3  2001/05/02 14:22:05  sll
  Cannot rely on the calldescriptor still being there when a user exception
  is raised.

  Revision 1.1.4.2  2001/05/01 16:07:32  sll
  All GIOP implementations should now work with fragmentation and abitrary
  sizes non-copy transfer.

  Revision 1.1.4.1  2001/04/18 18:10:51  sll
  Big checkin with the brand new internal APIs.

  */

#include <omniORB4/CORBA.h>
#include <omniORB4/callDescriptor.h>
#include <omniORB4/callHandle.h>
#include <initRefs.h>
#include <exceptiondefs.h>
#include <objectTable.h>
#include <objectAdapter.h>
#include <giopStrand.h>
#include <giopStream.h>
#include <giopStreamImpl.h>
#include <invoker.h>
#include <giopServer.h>
#include <giopWorker.h>
#include <GIOP_S.h>
#include <omniORB4/minorCode.h>
#include <omniORB4/omniInterceptors.h>
#include <poaimpl.h>

OMNI_NAMESPACE_BEGIN(omni)

////////////////////////////////////////////////////////////////////////
GIOP_S_Holder::GIOP_S_Holder(giopStrand* g, giopWorker* work) : pd_strand(g) {
  pd_iop_s = g->acquireServer(work);
}


////////////////////////////////////////////////////////////////////////
GIOP_S_Holder::~GIOP_S_Holder() {
  if (pd_iop_s) pd_strand->releaseServer(pd_iop_s);
}

////////////////////////////////////////////////////////////////////////
GIOP_S::GIOP_S(giopStrand* g) : giopStream(g),
				pd_state(UnUsed),
				pd_worker(0),
				pd_calldescriptor(0),
				pd_requestType(GIOP::MessageError),
				pd_operation((char*)pd_op_buffer),
				pd_principal(pd_pr_buffer),
				pd_principal_len(0),
				pd_response_expected(1),
				pd_result_expected(1)
{
}

////////////////////////////////////////////////////////////////////////
GIOP_S::GIOP_S(const GIOP_S& src) : giopStream(src.pd_strand),
				    pd_state(UnUsed),
				    pd_worker(0),
				    pd_calldescriptor(0),
				    pd_requestType(GIOP::MessageError),
				    pd_operation((char*)pd_op_buffer),
				    pd_principal(pd_pr_buffer),
				    pd_principal_len(0),
				    pd_response_expected(1),
				    pd_result_expected(1)
{
}

////////////////////////////////////////////////////////////////////////
GIOP_S::~GIOP_S() {
  if (pd_operation != (char*)pd_op_buffer) delete [] pd_operation;
  if (pd_principal != pd_pr_buffer) delete [] pd_principal;
}

////////////////////////////////////////////////////////////////////////
void*
GIOP_S::ptrToClass(int* cptr)
{
  if (cptr == &GIOP_S    ::_classid) return (GIOP_S*)    this;
  if (cptr == &giopStream::_classid) return (giopStream*)this;
  if (cptr == &cdrStream ::_classid) return (cdrStream*) this;

  return 0;
}
int GIOP_S::_classid;

////////////////////////////////////////////////////////////////////////
CORBA::Boolean
GIOP_S::dispatcher() {

  OMNIORB_ASSERT(pd_state == Idle);

  try {

    pd_state = WaitForRequestHeader;

    impl()->inputMessageBegin(this,impl()->unmarshalWildCardRequestHeader);

    {
      ASSERT_OMNI_TRACEDMUTEX_HELD(*omniTransportLock,0);
      omni_tracedmutex_lock sync(*omniTransportLock);
      pd_state = RequestHeaderIsBeingProcessed;
      if (!pd_strand->stopIdleCounter()) {
	// This strand has been expired by the scavenger. Don't
	// process this call.
	if (omniORB::trace(1)) {
	  omniORB::logger l;
	  l << "dispatcher cannot stop idle counter.\n";
	}
	pd_strand->state(giopStrand::DYING);
	return 0;
      }
    }

    if (pd_requestType == GIOP::Request) {
      return handleRequest();
    }
    else if (pd_requestType == GIOP::LocateRequest) {
      return handleLocateRequest();
    }
    else if (pd_requestType == GIOP::CancelRequest) {
      return handleCancelRequest();
    }
    else {
      if( omniORB::trace(1) ) {
	omniORB::logger l;
	l << "Unexpected message type (" << (CORBA::ULong) pd_requestType
	  << ")received by a server thread at "
	  << __FILE__ << ": line " << __LINE__ << "\n";
      }
      return 0;
    }
  }
  catch(const giopStream::CommFailure&) {
    return 0;
  }
  catch(const omniORB::fatalException& ex) {
    if( omniORB::trace(1) ) {
      omniORB::logger l;
      l << "omniORB fatalException caught by a server thread at "
	<< ex.file() << ": line "
	<< ex.line() << ", message: "
	<< ex.errmsg() << "\n";
    }
    impl()->sendMsgErrorMessage(this);
    return 0;
  }
  catch (...) {
    if ( omniORB::trace(1) ) {
      omniORB::logger l;
      l << "Unknown exception caught by a server thread at "
	<< __FILE__ << ": line " << __LINE__ << "\n";
    }
    impl()->sendMsgErrorMessage(this);
    return 0;
  }
}

////////////////////////////////////////////////////////////////////////
CORBA::Boolean
GIOP_S::handleRequest() {

  try {

    impl()->unmarshalRequestHeader(this);

    pd_state = RequestIsBeingProcessed;

    {
      omniInterceptors::serverReceiveRequest_T::info_T info(*this);
      omniORB::getInterceptors()->serverReceiveRequest.visit(info);
    }

    // Create a callHandle object
    omniCallHandle call_handle(this);

    // Can we find the object in the local object table?
    if (keysize() < 0)
      OMNIORB_THROW(OBJECT_NOT_EXIST,OBJECT_NOT_EXIST_NoMatch,
		    CORBA::COMPLETED_NO);

    CORBA::ULong hash = omni::hash(key(), keysize());

    omni::internalLock->lock();
    omniLocalIdentity* id;
    id = omniObjTable::locateActive(key(), keysize(), hash, 1);

    if( id ) {
      id->dispatch(call_handle);
      return 1;
    }

    omni::internalLock->unlock();

    // Can we create a suitable object on demand?

    omniObjAdapter_var adapter(omniObjAdapter::getAdapter(key(),keysize()));

    if( adapter ) {
      adapter->dispatch(call_handle, key(), keysize());
      return 1;
    }

    // Or is it the bootstrap agent?

    if( keysize() == 4 && !memcmp(key(), "INIT", 4) &&
	omniInitialReferences::invoke_bootstrap_agentImpl(call_handle) )
      return 1;

    // Oh dear.

    OMNIORB_THROW(OBJECT_NOT_EXIST,OBJECT_NOT_EXIST_NoMatch,
		  CORBA::COMPLETED_NO);

  }
  catch(omniORB::LOCATION_FORWARD& ex) {
    // This is here to provide a convenient way to implement
    // location forwarding. The object implementation can throw
    // a location forward exception to re-direct the request
    // to another location.

    if( omniORB::traceInvocations )
      omniORB::logf("Implementation of \'%s\' generated LOCATION_FORWARD.",
		    operation());

    CORBA::Object_var release_it(ex.get_obj());
    if (pd_state == RequestIsBeingProcessed) {
      SkipRequestBody();
    }
    if (response_expected()) {
      impl()->sendLocationForwardReply(this,release_it,ex.is_permanent());
    }
    // If the client does not expect a response, we quietly drop
    // the location forward.
  }
  catch (const terminateProcessing&) {
  }

#define MARSHAL_USER_EXCEPTION() do { \
      OMNIORB_ASSERT(calldescriptor() != 0); \
      int i, repoid_size;  \
      const char* repoid = ex._NP_repoId(&repoid_size); \
      for( i = 0; i < pd_n_user_excns; i++ ) \
	if( omni::strMatch(pd_user_excns[i], repoid) ) { \
	  impl()->sendUserException(this,ex); \
	  break; \
	} \
      if( i == pd_n_user_excns ) { \
	if( omniORB::trace(1) ) { \
	  omniORB::logger l; \
	  l << "WARNING -- method \'" << operation() \
	    << "\' on: " << pd_key \
	    << "\n raised the exception: " << repoid << '\n'; \
	} \
	CORBA::UNKNOWN sex(UNKNOWN_UserException, \
			   (CORBA::CompletionStatus) completion()); \
	impl()->sendSystemException(this,sex); \
      } \
} while (0)

#define MARSHAL_SYSTEM_EXCEPTION() do { \
    if (pd_state == WaitForRequestHeader || \
	pd_state == RequestHeaderIsBeingProcessed ) { \
      impl()->sendMsgErrorMessage(this); \
      return 0; \
    } else if (response_expected()) { \
      impl()->sendSystemException(this,ex); \
    } \
    if (pd_state == RequestIsBeingProcessed) { \
      SkipRequestBody(); \
    } \
} while (0) 

# ifndef HAS_Cplusplus_catch_exception_by_base

  // We have to catch each type of system exception separately
  // here to support compilers which cannot catch more derived
  // types.
#   define CATCH_AND_MARSHAL(name)  \
  catch (CORBA::name& ex) {  \
    MARSHAL_SYSTEM_EXCEPTION(); \
  }

  OMNIORB_FOR_EACH_SYS_EXCEPTION(CATCH_AND_MARSHAL)

#    undef CATCH_AND_MARSHAL

  catch(omniORB::StubUserException& uex) {
    if (response_expected()) {
      CORBA::UserException& ex = *((CORBA::UserException*)uex.ex());
      MARSHAL_USER_EXCEPTION();
      delete uex.ex();  // ?? Possible memory leak?
    }
  }

#endif


  catch(CORBA::SystemException& ex) {
    MARSHAL_SYSTEM_EXCEPTION();
    // If the client does not expect a response, we quietly drop
    // the system exception.
  }
  catch(CORBA::UserException& ex) {
    if (response_expected()) {
      MARSHAL_USER_EXCEPTION();
    }
  }
#undef MARSHAL_USER_EXCEPTION
#undef MARSHAL_SYSTEM_EXCEPTION

  catch(const giopStream::CommFailure&) {
    throw;
  }

  catch(...) {
    if( omniORB::traceLevel > 1 ) {
      omniORB::logger l;
      l << "WARNING -- method \'" << operation() << "\' raised an unexpected\n"
	" exception (not a CORBA exception).\n";
    }
    CORBA::UNKNOWN ex(UNKNOWN_UserException,
		      (CORBA::CompletionStatus) completion());
    impl()->sendSystemException(this,ex);
  }
  pd_state = ReplyCompleted;
  return 1;
}

////////////////////////////////////////////////////////////////////////
CORBA::Boolean
GIOP_S::handleLocateRequest() {
  try {

    impl()->unmarshalLocateRequest(this);

    pd_state = RequestIsBeingProcessed;

    // Here we notify the giopServer that this thread has finished
    // reading the request. The server may want to keep a watch on
    // any more request coming in on the connection while this
    // thread does the upcall.

    CORBA::Boolean data_in_buffer = 0;
    if (pd_rdlocked) {
      // This is the thread that is reading from the connection. We
      // check if we have previously queued giopStream_Buffers on the
      // connection. In other words, we might have previously read
      // too much stuff out of the connection and these data belong to
      // other requests. If that is the case, we notify the giopServer
      // that there are already buffers waiting to be read.
      giopStrand& s = (giopStrand&) *this;
      data_in_buffer = ((s.head) ? 1 : 0);
    }
    pd_worker->server()->notifyWkPreUpCall(pd_worker,data_in_buffer);

    impl()->inputMessageEnd(this,0);
    
    pd_state = WaitingForReply;

    omniORB::logs(10, "Handling a GIOP LOCATE_REQUEST.");

    GIOP::LocateStatusType status = GIOP::UNKNOWN_OBJECT;

    if (keysize() > 0) {
      CORBA::ULong hash = omni::hash(key(), keysize());
      omni_tracedmutex_lock sync(*omni::internalLock);
      omniLocalIdentity* id;
      id = omniObjTable::locateActive(key(), keysize(), hash, 1);
      if( id )  status = GIOP::OBJECT_HERE;
     }
    if ( status == GIOP::UNKNOWN_OBJECT && keysize() > 0 ) {
      // We attempt to find the object adapter (activate it if necassary)
      // and ask it if the object exists, or if it has the *capability*
      // to activate such an object.  ie. is it able to do object loading
      // on demand?

      omniObjAdapter_var adapter(omniObjAdapter::getAdapter(key(),keysize()));
      if( adapter && adapter->objectExists(key(),keysize()) )
	status = GIOP::OBJECT_HERE;
    }
    if ( status == GIOP::UNKNOWN_OBJECT && 
	 keysize() == 4 && !memcmp(key(), "INIT", 4) &&
	 omniInitialReferences::is_bootstrap_agentImpl_initialised() ) {
      status = GIOP::OBJECT_HERE;
    }

    impl()->sendLocateReply(this,status,CORBA::Object::_nil(),0);
  }
  catch (omniORB::LOCATION_FORWARD& lf) {
    CORBA::Object_var release_it(lf.get_obj());
    impl()->sendLocateReply(this,
			    lf.is_permanent() ? GIOP::OBJECT_FORWARD_PERM :
			                        GIOP::OBJECT_FORWARD,
                            release_it,0);
  }

#define MARSHAL_SYSTEM_EXCEPTION() do { \
    if (pd_state == WaitForRequestHeader || \
        pd_state == RequestHeaderIsBeingProcessed) { \
      impl()->sendMsgErrorMessage(this); \
      return 0; \
    } else if (response_expected()) { \
      impl()->sendSystemException(this,ex); \
    } \
    if (pd_state == RequestIsBeingProcessed) { \
      SkipRequestBody(); \
    } \
} while (0) 

# ifndef HAS_Cplusplus_catch_exception_by_base

  // We have to catch each type of system exception separately
  // here to support compilers which cannot catch more derived
  // types.
#   define CATCH_AND_MARSHAL(name)  \
  catch (CORBA::name& ex) {  \
    MARSHAL_SYSTEM_EXCEPTION(); \
  }

  OMNIORB_FOR_EACH_SYS_EXCEPTION(CATCH_AND_MARSHAL)

#    undef CATCH_AND_MARSHAL
#endif

  catch(CORBA::SystemException& ex) {
    MARSHAL_SYSTEM_EXCEPTION();
  }

  pd_state = ReplyCompleted;
  return 1;
}

////////////////////////////////////////////////////////////////////////
CORBA::Boolean
GIOP_S::handleCancelRequest() {
  // We do not have the means to asynchronously abort the execution of
  // an upcall by another thread. Therefore it is not possible to
  // cancel a request that has already been in progress. 
  pd_state = ReplyCompleted;
  return 1;
}

////////////////////////////////////////////////////////////////////////
void
GIOP_S::ReceiveRequest(omniCallDescriptor& desc) {

  OMNIORB_ASSERT(pd_state == RequestIsBeingProcessed);

  calldescriptor(&desc);

  // When a user exception is throw by the stub code, the
  // call descriptor could have been deallocated before the
  // catch frame for the user exception is reached. Therefore
  // we store the user exception signatures inside our own
  // private states.
  pd_n_user_excns = desc.n_user_excns();
  pd_user_excns = desc.user_excns();

  cdrStream& s = *this;
  desc.unmarshalArguments(s);
  pd_state = WaitingForReply;

  // Here we notify the giopServer that this thread has finished
  // reading the request. The server may want to keep a watch on
  // any more request coming in on the connection while this
  // thread does the upcall.

  CORBA::Boolean data_in_buffer = 0;
  if (pd_rdlocked) {
    // This is the thread that is reading from the connection. We
    // check if we have previously queued giopStream_Buffers on the
    // connection. In other words, we might have previously read
    // too much stuff out of the connection and these data belong to
    // other requests. If that is the case, we notify the giopServer
    // that there are already buffers waiting to be read.
    giopStrand& s = (giopStrand&) *this;
    data_in_buffer = ((s.head) ? 1 : 0);
  }
  pd_worker->server()->notifyWkPreUpCall(pd_worker,data_in_buffer);

  impl()->inputMessageEnd(this,0);


  // Check if this call comes in from a bidirectional connection.
  // If so check if the servant's POA policy allows this.
  giopStrand& g = (giopStrand&)((giopStream&)(*this));
  if (g.biDir && g.isClient()) {
    if (!(pd_calldescriptor->poa() &&
	  pd_calldescriptor->poa()->acceptBiDirectional())) {
      OMNIORB_THROW(OBJ_ADAPTER,OBJ_ADAPTER_BiDirNotAllowed,
		    CORBA::COMPLETED_NO);
    }
  }
}

////////////////////////////////////////////////////////////////////////
void
GIOP_S::SkipRequestBody() {

  OMNIORB_ASSERT(pd_state == RequestIsBeingProcessed);

  pd_state = WaitingForReply;
  impl()->inputMessageEnd(this,1);
}

////////////////////////////////////////////////////////////////////////
void
GIOP_S::SendReply() {

  OMNIORB_ASSERT(pd_state == WaitingForReply);

  if (!response_expected()) throw terminateProcessing();

  pd_state = ReplyIsBeingComposed;
  impl()->outputMessageBegin(this,impl()->marshalReplyHeader);
  calldescriptor()->marshalReturnedValues((cdrStream&)*this);
  impl()->outputMessageEnd(this);
  pd_state = ReplyCompleted;
}

////////////////////////////////////////////////////////////////////////
void
GIOP_S::SendException(CORBA::Exception* ex) {

  OMNIORB_ASSERT(pd_state == WaitingForReply);

  if (!response_expected()) throw terminateProcessing();

  int idsize;
  const char* repoid = ex->_NP_repoId(&idsize);

# define TEST_AND_MARSHAL_SYSEXCEPTION(name) \
  if ( strcmp("IDL:omg.org/CORBA/" #name ":1.0",repoid) == 0 ) { \
    impl()->sendSystemException(this,*((CORBA::SystemException*)ex)); \
    pd_state = ReplyCompleted; \
    return; \
  }

  OMNIORB_FOR_EACH_SYS_EXCEPTION(TEST_AND_MARSHAL_SYSEXCEPTION)
# undef TEST_AND_MARSHAL_SYSEXCEPTION

  // this is not a system exception, treat it as a user exception
  // we do not check if this is a valid user exception for this operation.
  // caller should have checked this or else the user exception should have
  // been thrown as a C++ exception and got handled by the catch clause in
  // handleRequest.
  impl()->sendUserException(this,*((CORBA::UserException*)ex));
  pd_state = ReplyCompleted;
}

////////////////////////////////////////////////////////////////////////
void
GIOP_S::notifyCommFailure(CORBA::Boolean,
			  CORBA::ULong& minor,
			  CORBA::Boolean& retry) {
  retry = 0;

  if (pd_state == WaitForRequestHeader ||
      pd_state == RequestIsBeingProcessed) {
    minor = COMM_FAILURE_UnMarshalArguments;
  }
  else if (pd_state == WaitingForReply) {
    minor = COMM_FAILURE_WaitingForReply;
  }
  else if (pd_state == ReplyIsBeingComposed) {
    minor = COMM_FAILURE_MarshalResults;
  }
  else {
    minor = TRANSIENT_ConnectionClosed;
  }
}

////////////////////////////////////////////////////////////////////////
CORBA::ULong
GIOP_S::completion() {
  if (pd_state == WaitingForReply) {
    return (CORBA::ULong)CORBA::COMPLETED_MAYBE;
  }
  else if (pd_state == ReplyIsBeingComposed) {
    return (CORBA::ULong)CORBA::COMPLETED_YES;
  }
  else {
    return (CORBA::ULong)CORBA::COMPLETED_NO;
  }
}

////////////////////////////////////////////////////////////////////////
const char*
GIOP_S::operation_name() const {
  return operation();
}

////////////////////////////////////////////////////////////////////////
void
GIOP_S::unmarshalIORAddressingInfo() {
  GIOP::AddressingDisposition vp;
  CORBA::ULong   vl;

  resetKey();

  cdrStream& s = (cdrStream&)*this;

  vp <<= s;
  if (vp == GIOP::KeyAddr) {
    vl <<= s;
    if (!s.checkInputOverrun(1,vl)) {
      OMNIORB_THROW(MARSHAL,MARSHAL_SequenceIsTooLong,
		    (CORBA::CompletionStatus)completion());
    }
    keysize((int)vl);
    s.get_octet_array(key(),vl);
  }
  else {
    
    GIOP::IORAddressingInfo& ta = pd_target_address;
    if (vp == GIOP::ProfileAddr) {
      ta.ior.profiles.length(1);
      ta.ior.profiles[0] <<= s;
      ta.selected_profile_index = 0;
    }
    else {
      // GIOP::ReferenceAddr
      ta.selected_profile_index <<= s;
      ta.ior <<= s;
    }
    if (ta.selected_profile_index >= ta.ior.profiles.length() ||
	ta.ior.profiles[ta.selected_profile_index].tag != 
	                                              IOP::TAG_INTERNET_IOP) {
      if ( omniORB::trace(25) ) {
	omniORB::logger l;
	l << "unmarshal corrupted targetAddress at "
	  << __FILE__ << " line no. " << __LINE__ << "\n";
      }
      OMNIORB_THROW(BAD_PARAM,BAD_PARAM_IndexOutOfRange,
		    (CORBA::CompletionStatus)completion());
    }

    IIOP::ProfileBody decodedBody;
    IIOP::unmarshalProfile(ta.ior.profiles[ta.selected_profile_index],
			decodedBody);

#if 0 // XXX Not finalise yet
    _OMNI_NS(giopAddressList) addresses;
    IIOP::extractAddresses(decodedBody,addresses);
    if ( isLocal(addresses) ) {
      keysize((int)decodedBody.object_key.length());
      memcpy((void*)key(),decodedBody.object_key.get_buffer(),keysize());
    }
    // XXX delete all the addresses.
#else
    OMNIORB_ASSERT(0);
#endif

    // Reach here either we have got the key of the target object
    // or we have the target address info in targetAddress().
    //
    if (keysize() < 0) {
      // We couldn't decode the target address to a local object key. Unless
      // an interceptor can decode it further, this request will be rejected.
      if ( omniORB::trace(25) ) {
	omniORB::logger l;
	l << "ProfileAddr or ReferenceAddr addresses unknown target at "
	  << __FILE__ << " line no. " << __LINE__ << "\n";
      }
    }
  }

}

OMNI_NAMESPACE_END(omni)
