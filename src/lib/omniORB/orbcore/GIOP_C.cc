// -*- Mode: C++; -*-
//                            Package   : omniORB
// GIOP_C.cc                  Created on: 08/02/2001
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
  Revision 1.1.4.10  2004/04/07 17:37:31  dgrisby
  Fix bug with retries when location forwarding.

  Revision 1.1.4.9  2001/10/17 16:33:27  dpg1
  New downcast mechanism for cdrStreams.

  Revision 1.1.4.8  2001/09/10 17:43:44  sll
  Stop idle counter inside initialise.

  Revision 1.1.4.7  2001/09/04 14:38:51  sll
  Added the boolean argument to notifyCommFailure to indicate if
  omniTransportLock is held by the caller.

  Revision 1.1.4.6  2001/09/03 16:54:06  sll
  In initialise(), set deadline from the parameters in calldescriptor.

  Revision 1.1.4.5  2001/09/03 13:28:09  sll
  In the calldescriptor, in addition to the first address, record the current
  address in use.

  Revision 1.1.4.4  2001/08/03 17:41:17  sll
  System exception minor code overhaul. When a system exeception is raised,
  a meaning minor code is provided.

  Revision 1.1.4.3  2001/07/31 16:28:01  sll
  Added GIOP BiDir support.

  Revision 1.1.4.2  2001/05/11 14:30:56  sll
  first_use status of the strand is now in use.

  Revision 1.1.4.1  2001/04/18 18:10:52  sll
  Big checkin with the brand new internal APIs.

*/

#include <omniORB4/CORBA.h>
#include <giopRope.h>
#include <giopStrand.h>
#include <giopStream.h>
#include <giopStreamImpl.h>
#include <GIOP_C.h>
#include <exceptiondefs.h>
#include <omniORB4/callDescriptor.h>
#include <omniORB4/minorCode.h>

OMNI_NAMESPACE_BEGIN(omni)

////////////////////////////////////////////////////////////////////////
GIOP_C::GIOP_C(giopRope* r,giopStrand* s) : giopStream(s), 
					    pd_state(IOP_C::UnUsed),
					    pd_calldescriptor(0),
					    pd_ior(0),
					    pd_rope(r),
					    pd_replyStatus(GIOP::NO_EXCEPTION),
					    pd_locateStatus(GIOP::OBJECT_HERE)
{
}

////////////////////////////////////////////////////////////////////////
GIOP_C::~GIOP_C() {
}

////////////////////////////////////////////////////////////////////////
void*
GIOP_C::ptrToClass(int* cptr)
{
  if (cptr == &GIOP_C    ::_classid) return (GIOP_C*)    this;
  if (cptr == &giopStream::_classid) return (giopStream*)this;
  if (cptr == &cdrStream ::_classid) return (cdrStream*) this;

  return 0;
}
int GIOP_C::_classid;

////////////////////////////////////////////////////////////////////////
void
GIOP_C::initialise(const omniIOR* i, 
		   const CORBA::Octet* k,
		   int ksz,
		   omniCallDescriptor* calldesc) {
  giopStream::reset();
  pd_strand->stopIdleCounter();
  state(IOP_C::Idle);
  ior(i);
  calldescriptor(calldesc);
  unsigned long secs,nanosecs;
  calldesc->getDeadline(secs,nanosecs);
  setDeadline(secs,nanosecs);
  key(k);
  keysize(ksz);
  requestId(pd_strand->newSeqNumber());
  TCS_C(0);
  TCS_W(0);
}

////////////////////////////////////////////////////////////////////////
void
GIOP_C::cleanup() {
  giopStream::reset();
  state(IOP_C::UnUsed);
  ior(0);
  calldescriptor(0);
}

////////////////////////////////////////////////////////////////////////
void
GIOP_C::InitialiseRequest() {

  OMNIORB_ASSERT(pd_state  == IOP_C::Idle);
  OMNIORB_ASSERT(pd_calldescriptor);
  OMNIORB_ASSERT(pd_ior);

  pd_state = IOP_C::RequestInProgress;
  impl()->outputMessageBegin(this,impl()->marshalRequestHeader);
  calldescriptor()->marshalArguments(*this);
  impl()->outputMessageEnd(this);
  pd_state = IOP_C::WaitingForReply;
}

////////////////////////////////////////////////////////////////////////
GIOP::ReplyStatusType
GIOP_C::ReceiveReply() {

  OMNIORB_ASSERT(pd_state == IOP_C::WaitingForReply);

  if (calldescriptor()->is_oneway()) {
    pd_state = IOP_C::ReplyIsBeingProcessed;
    return GIOP::NO_EXCEPTION;
  }

  impl()->inputMessageBegin(this,impl()->unmarshalReplyHeader);

  pd_state = IOP_C::ReplyIsBeingProcessed;

  GIOP::ReplyStatusType rc = replyStatus();
  if (rc == GIOP::SYSTEM_EXCEPTION) { 
    UnMarshallSystemException();
    // never reaches here
  }
  return rc;
}

////////////////////////////////////////////////////////////////////////
void
GIOP_C::RequestCompleted(CORBA::Boolean skip) {

  OMNIORB_ASSERT(pd_state == IOP_C::ReplyIsBeingProcessed);

  if (!calldescriptor() || !calldescriptor()->is_oneway()) {
    impl()->inputMessageEnd(this,skip);
  }
  pd_strand->first_use = 0;
  pd_state = IOP_C::Idle;
}

////////////////////////////////////////////////////////////////////////
GIOP::LocateStatusType
GIOP_C::IssueLocateRequest() {

  OMNIORB_ASSERT(pd_state  == IOP_C::Idle);
  OMNIORB_ASSERT(pd_ior);

  pd_state = IOP_C::RequestInProgress;
  impl()->sendLocateRequest(this);
  pd_state = IOP_C::WaitingForReply;
  impl()->inputMessageBegin(this,impl()->unmarshalLocateReply);

  pd_state = IOP_C::ReplyIsBeingProcessed;

  GIOP::LocateStatusType rc = locateStatus();
  if (rc == GIOP::LOC_SYSTEM_EXCEPTION) {
    UnMarshallSystemException();
    // never reaches here
  }
  return rc;
}


////////////////////////////////////////////////////////////////////////
void
GIOP_C::UnMarshallSystemException()
{

#define CHECK_AND_IF_MATCH_THROW_SYSTEM_EXCEPTION(_ex) \
  if (strcmp("IDL:omg.org/CORBA/" #_ex ":1.0",(const char*)repoid) == 0) \
    OMNIORB_THROW(_ex, minorcode, (CORBA::CompletionStatus) status);

  // Real code begins here
  cdrStream& s = *this;

  CORBA::String_var repoid;

  repoid = s.unmarshalRawString();

  CORBA::ULong minorcode;
  CORBA::ULong status;
  minorcode <<= s;
  status <<= s;

  impl()->inputMessageEnd(this,0);
  pd_strand->first_use = 0;
  pd_state = IOP_C::Idle;

  switch (status) {
  case CORBA::COMPLETED_YES:
  case CORBA::COMPLETED_NO:
  case CORBA::COMPLETED_MAYBE:
    break;
  default:
    OMNIORB_THROW(UNKNOWN,UNKNOWN_SystemException,CORBA::COMPLETED_YES);
  };

  OMNIORB_FOR_EACH_SYS_EXCEPTION(CHECK_AND_IF_MATCH_THROW_SYSTEM_EXCEPTION)

  // If none of the above matched
  OMNIORB_THROW(UNKNOWN,UNKNOWN_SystemException,CORBA::COMPLETED_YES);

#undef CHECK_AND_IF_MATCH_THROW_SYSTEM_EXCEPTION
}

////////////////////////////////////////////////////////////////////////
void
GIOP_C::notifyCommFailure(CORBA::Boolean heldlock,
			  CORBA::ULong& minor,
			  CORBA::Boolean& retry) {

  OMNIORB_ASSERT(pd_calldescriptor);

  if (pd_strand->first_use) {
    const giopAddress* firstaddr = pd_calldescriptor->firstAddressUsed();
    const giopAddress* currentaddr; 
    if (!firstaddr) {
      firstaddr = pd_strand->address;
      pd_calldescriptor->firstAddressUsed(firstaddr);
      currentaddr = firstaddr;
      pd_calldescriptor->currentAddress(currentaddr);
    }
    else {
      currentaddr = pd_calldescriptor->currentAddress();
    }
    currentaddr = pd_rope->notifyCommFailure(currentaddr,heldlock);
    pd_calldescriptor->currentAddress(currentaddr);

    if (currentaddr == firstaddr) {
      // Run out of addresses to try.
      retry = 0;
      pd_calldescriptor->firstAddressUsed(0);
      pd_calldescriptor->currentAddress(0);
    }
    else {
      // Retry will use the next address in the list.
      retry = 1;
    }
  }
  else if (pd_strand->biDir && 
	   pd_strand->isClient() && 
	   pd_strand->biDir_has_callbacks) {

    // when the connection is used bidirectionally, the call back
    // objects at the other end will not be able to call us.
    // The application may want to know this. We
    // should not silently retry and reconnect again because the
    // callback objects would not the new connection.
    retry = 0;
  }
  else {
    // Strand has been re-used from a previous invocation. Have
    // another go with a new strand in case something was broken in
    // the current one.
    retry = 1;
  }

  switch (pd_state) {
  case IOP_C::RequestInProgress:
    minor = COMM_FAILURE_MarshalArguments;
    break;
  case IOP_C::WaitingForReply:
    minor = COMM_FAILURE_WaitingForReply;
    break;
  case IOP_C::ReplyIsBeingProcessed:
    minor = COMM_FAILURE_UnMarshalResults;
    break;
  default:
    minor = TRANSIENT_ConnectionClosed;
    break;
  }
}

////////////////////////////////////////////////////////////////////////
CORBA::ULong
GIOP_C::completion() {
  if (pd_state == IOP_C::WaitingForReply) {
    return (CORBA::ULong)CORBA::COMPLETED_MAYBE;
  }
  else if (pd_state == IOP_C::ReplyIsBeingProcessed) {
    return (CORBA::ULong)CORBA::COMPLETED_YES;
  }
  else {
    return (CORBA::ULong)CORBA::COMPLETED_NO;
  }
}

OMNI_NAMESPACE_END(omni)


