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
  Revision 1.1.2.2  2001/02/23 19:25:19  sll
  Merged interim FT client stuff.

  Revision 1.1.2.1  2001/02/23 16:46:59  sll
  Added new files.

  */

#include <omniORB4/CORBA.h>
#include <omniORB4/callDescriptor.h>
#include <initRefs.h>
#include <exceptiondefs.h>
#include <localIdentity.h>
#include <objectAdapter.h>
#include <giopStrand.h>
#include <giopStream.h>
#include <giopStreamImpl.h>
#include <giopServer.h>
#include <GIOP_S.h>
#include <omniORB4/minorCode.h>

OMNI_NAMESPACE_BEGIN(omni)

////////////////////////////////////////////////////////////////////////
GIOP_S_Holder::GIOP_S_Holder(giopStrand* g, giopServer* serv) : pd_strand(g) {
  pd_iop_s = g->acquireServer(serv);
}


////////////////////////////////////////////////////////////////////////
GIOP_S_Holder::~GIOP_S_Holder() {
  pd_strand->releaseServer(pd_iop_s);
}

////////////////////////////////////////////////////////////////////////
GIOP_S::GIOP_S(giopStrand* g,giopServer*server) : giopStream(g),
					    pd_state(UnUsed),
					    pd_server(server),
					    pd_calldescriptor(0),
					    pd_requestType(GIOP::MessageError),
		                            pd_operation((char*)pd_op_buffer),
					    pd_principal(pd_pr_buffer),
					    pd_principal_len(0),
					    pd_request_id(0),
					    pd_response_expected(1),
					    pd_result_expected(1)
{
}

////////////////////////////////////////////////////////////////////////
GIOP_S::GIOP_S(const GIOP_S& src) : giopStream(src.pd_strand),
				    pd_state(UnUsed),
				    pd_server(src.pd_server),
				    pd_calldescriptor(0),
				    pd_requestType(GIOP::MessageError),
				    pd_operation((char*)pd_op_buffer),
				    pd_principal(pd_pr_buffer),
				    pd_principal_len(0),
				    pd_request_id(0),
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
CORBA::Boolean
GIOP_S::dispatcher() {

  OMNIORB_ASSERT(pd_state == Idle);

  try {
    pd_state = RequestIsBeingProcessed;
    impl()->inputMessageBegin(this,impl()->unmarshalWildCardRequestHeader);

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
      if( omniORB::trace(0) ) {
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
    if( omniORB::trace(0) ) {
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
    if ( omniORB::trace(0) ) {
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

    // Can we find the object in the local object table?
    if (keysize() < 0)
      OMNIORB_THROW(OBJECT_NOT_EXIST,0, CORBA::COMPLETED_NO);

    CORBA::ULong hash = omni::hash(key(), keysize());

    omni::internalLock->lock();
    omniLocalIdentity* id;
    id = omni::locateIdentity(key(), keysize(), hash);

    if( id && id->servant() ) {
      id->dispatch(*this);
      return 1;
    }

    omni::internalLock->unlock();

    // Can we create a suitable object on demand?

    omniObjAdapter_var adapter(omniObjAdapter::getAdapter(key(),keysize()));

    if( adapter ) {
      adapter->dispatch(*this, key(), keysize());
      return 1;
    }

    // Or is it the bootstrap agent?

    if( keysize() == 4 && !memcmp(key(), "INIT", 4) &&
	omniInitialReferences::invoke_bootstrap_agentImpl(*this) )
      return 1;

    // Oh dear.

    OMNIORB_THROW(OBJECT_NOT_EXIST,0, CORBA::COMPLETED_NO);

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
      impl()->sendLocationForwardReply(this,release_it);
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
      const char*const* user_exns = calldescriptor()->user_excns(); \
      int n_user_exns = calldescriptor()->n_user_excns(); \
      for( i = 0; i < n_user_exns; i++ ) \
	if( !strcmp(user_exns[i], repoid) ) { \
	  impl()->sendUserException(this,ex); \
	  break; \
	} \
      if( i == n_user_exns ) { \
	if( omniORB::trace(1) ) { \
	  omniORB::logger l; \
	  l << "WARNING -- method \'" << operation() \
	    << "\' on: " << key() \
	    << "\n raised the exception: " << repoid << '\n'; \
	} \
	CORBA::UNKNOWN sex(UNKNOWN_UserException, \
			   (CORBA::CompletionStatus) completion()); \
	impl()->sendSystemException(this,sex); \
      } \
} while (0)

# ifdef HAS_Cplusplus_catch_exception_by_base

  catch(CORBA::SystemException& ex) {
    if (pd_state == RequestIsBeingProcessed) {
      SkipRequestBody();
    }
    if (response_expected()) {
      impl()->sendSystemException(this,ex);
    }
    // If the client does not expect a response, we quietly drop
    // the system exception.
  }
  catch(CORBA::UserException& ex) {
    if (response_expected()) {
      MARSHAL_USER_EXCEPTION();
    }
  }

# else

  // We have to catch each type of system exception separately
  // here to support compilers which cannot catch more derived
  // types.
#   define CATCH_AND_MARSHAL(name)  \
  catch (CORBA::name& ex) {  \
    if (pd_state == RequestIsBeingProcessed) { \
      SkipRequestBody(); \
    } \
    if (response_expected()) { \
      impl()->sendSystemException(this,ex); \
    } \
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

#undef MARSHAL_USER_EXCEPTION

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
  pd_state = Idle;
  return 1;
}

////////////////////////////////////////////////////////////////////////
CORBA::Boolean
GIOP_S::handleLocateRequest() {
  try {

    impl()->unmarshalLocateRequest(this);
    
    pd_state = WaitingForReply;

    GIOP::LocateStatusType status = GIOP::UNKNOWN_OBJECT;

    if (keysize() > 0) {
      CORBA::ULong hash = omni::hash(key(), keysize());
      omni_tracedmutex_lock sync(*omni::internalLock);
      omniLocalIdentity* id;
      id = omni::locateIdentity(key(), keysize(), hash);
      if( id && id->servant() )  status = GIOP::OBJECT_HERE;
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

    impl()->sendLocateReply(this,status,CORBA::Object::_nil());
  }
  catch (omniORB::LOCATION_FORWARD& lf) {
    CORBA::Object_var release_it(lf.get_obj());
    impl()->sendLocateReply(this,GIOP::OBJECT_FORWARD,release_it);
  }
# ifdef HAS_Cplusplus_catch_exception_by_base

  catch(CORBA::SystemException& ex) {
    if (pd_state == RequestIsBeingProcessed) {
      SkipRequestBody();
    }
    impl()->sendLocateReply(this,GIOP::LOC_SYSTEM_EXCEPTION,
			    CORBA::Object::_nil(),&ex);
  }

# else

  // We have to catch each type of system exception separately
  // here to support compilers which cannot catch more derived
  // types.
#   define CATCH_AND_MARSHAL(name)  \
  catch (CORBA::name& ex) {  \
    if (pd_state == RequestIsBeingProcessed) { \
      SkipRequestBody(); \
    } \
    impl()->sendLocateReply(this,GIOP::LOC_SYSTEM_EXECPTION, \
			    CORBA::Object::_nil(),&ex); \
  }

  OMNIORB_FOR_EACH_SYS_EXCEPTION(CATCH_AND_MARSHAL)

#    undef CATCH_AND_MARSHAL
#endif

  pd_state = Idle;
  return 1;
}

////////////////////////////////////////////////////////////////////////
CORBA::Boolean
GIOP_S::handleCancelRequest() {
  // We do not have the means to asynchronously abort the execution of
  // an upcall by another thread. Therefore it is not possible to
  // cancel a request that has already been in progress. 
  pd_state = Idle;
  return 1;
}

////////////////////////////////////////////////////////////////////////
void
GIOP_S::ReceiveRequest(omniCallDescriptor& desc) {

  OMNIORB_ASSERT(pd_state == RequestIsBeingProcessed);

  calldescriptor(&desc);
  cdrStream& s = (cdrStream&)*this;
  desc.unmarshalArguments(s);
  pd_state = WaitingForReply;

  // Here we notify the giopServer that this thread has finished
  // reading the request. The server may want to keep a watch on
  // any more request coming in on the connection while this
  // thread does the upcall.
  pd_server->notifyWkPreUpCall(pd_strand);

  impl()->inputMessageEnd(this);
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
  pd_state = Idle;
}

////////////////////////////////////////////////////////////////////////
void
GIOP_S::notifyCommFailure(CORBA::ULong& minor,
			  CORBA::Boolean& retry) {
  retry = 0;

  if (pd_state == RequestIsBeingProcessed) {
    minor = COMM_FAILURE_UnMarshalArguments;
  }
  else if (pd_state == WaitingForReply) {
    minor = COMM_FAILURE_WaitingForReply;
  }
  else if (pd_state == ReplyIsBeingComposed) {
    minor = COMM_FAILURE_MarshalResults;
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
      throw CORBA::MARSHAL(0,(CORBA::CompletionStatus)completion());
    }
    keysize(vl);
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
      throw CORBA::MARSHAL(0,(CORBA::CompletionStatus)completion());
    }

    // Extract the repository ID and the profile(s) from the addressinginfo.
    char* repoid = ta.ior.type_id._retn();
    CORBA::ULong nprofiles = ta.ior.profiles.length();
    IOP::TaggedProfile* profiles = ta.ior.profiles.get_buffer(1);


    omniIOR_var ior = new omniIOR(repoid,profiles,nprofiles,
				  ta.selected_profile_index);

    Rope* rope;
    CORBA::Boolean is_local;

    // XXX The following code does not have the desired effect yet.
    //     because the IOR is decoded as a side effect of selectRope ATM.
    //     Therefore ior->is_IOGR() always return 0 which is the default
    //     value.
    if ( !ior->is_IOGR() && ior->selectRope(rope,is_local)) {
      if (is_local) {
	keysize(ior->getIIOPprofile().object_key.length());
	memcpy((void*)key(),
	       (void*)(ior->getIIOPprofile().object_key.get_buffer()),
	       keysize());
      }
      else {
	rope->decrRefCount();
      }
    }

    // Return the repository ID and the profile(s) back to addressinginfo.
    IOP::TaggedProfileList& iop = (IOP::TaggedProfileList&)ior->iopProfiles();
    ta.ior.profiles.replace(nprofiles,nprofiles,iop.get_buffer(1),1);

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
