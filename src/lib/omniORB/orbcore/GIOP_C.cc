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
void
GIOP_C::initialise(const omniIOR* i, 
		   const CORBA::Octet* k,
		   int ksz,
		   omniCallDescriptor* calldesc) {
  giopStream::reset();
  state(IOP_C::Idle);
  ior(i);
  calldescriptor(calldesc);
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
GIOP_C::notifyCommFailure(CORBA::ULong& minor,
			  CORBA::Boolean& retry) {

  OMNIORB_ASSERT(pd_calldescriptor);

  if (pd_strand->first_use) {
    const giopAddress* addr = pd_calldescriptor->firstAddressUsed();
    if (!addr) {
      addr = pd_strand->address;
      pd_calldescriptor->firstAddressUsed(addr);
    }
    retry =  (pd_rope->notifyCommFailure(addr) != addr);
  }
  else {
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


