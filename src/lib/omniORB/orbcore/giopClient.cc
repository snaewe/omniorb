// -*- Mode: C++; -*-
//                            Package   : omniORB
// giopClient.cc              Created on: 26/3/96
//                            Author    : Sai Lai Lo (sll)
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
//      Client side GIOP
//      
 
/*
  $Log$
  Revision 1.13.2.4  2000/10/10 14:48:33  sll
  RequestCompleted() should not call inputMessageEnd if no response is
  expected.

  Revision 1.13.2.3  2000/10/09 16:21:54  sll
  Removed reference to omniConnectionBroken.

  Revision 1.13.2.2  2000/09/27 18:21:35  sll
  Use the new GIOP engine to drive a remote call.

  Revision 1.13.2.1  2000/07/17 10:35:53  sll
  Merged from omni3_develop the diff between omni3_0_0_pre3 and omni3_0_0.

  Revision 1.14  2000/07/13 15:25:57  dpg1
  Merge from omni3_develop for 3.0 release.

  Revision 1.12.6.4  2000/06/22 10:37:50  dpg1
  Transport code now throws omniConnectionBroken exception rather than
  CORBA::COMM_FAILURE when things go wrong. This allows the invocation
  code to distinguish between transport problems and COMM_FAILURES
  propagated from the server side.

  exception.h renamed to exceptiondefs.h to avoid name clash on some
  platforms.

  Revision 1.12.6.3  1999/10/18 11:27:39  djr
  Centralised list of system exceptions.

  Revision 1.12.6.2  1999/10/14 16:22:09  djr
  Implemented logging when system exceptions are thrown.

  Revision 1.12.6.1  1999/09/22 14:26:49  djr
  Major rewrite of orbcore to support POA.

  Revision 1.12  1999/08/30 16:50:17  sll
  Added call to Strand::Sync::clicksSet to control how long a call is
  allowed to progress or how long an idle connection is to stay open.

  Revision 1.11  1999/03/11 16:25:53  djr
  Updated copyright notice

  Revision 1.10  1999/01/07 15:51:17  djr
  Changes needed due to changes to Net/MemBufferedStream.

  Revision 1.9  1998/04/07 19:34:11  sll
  Replace cerr with omniORB::log.

  Revision 1.8  1997/12/09 17:37:15  sll
  Updated to use the new rope and strand interface.
  Now unmarshal context properly.

  Revision 1.7  1997/08/21 21:56:40  sll
  Added system exception TRANSACTION_REQUIRED, TRANSACTION_ROLLEDBACK,
  INVALID_TRANSACTION, WRONG_TRANSACTION.

// Revision 1.6  1997/05/06  15:19:49  sll
// Public release.
//
  */

#include <omniORB4/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <exceptiondefs.h>


GIOP_C::GIOP_C(omniIOR* ior, Rope *r) : 
  pd_state(GIOP_C::Idle), pd_ior(ior)
{
  pd_cdrStream = giopStream::acquireClient(r,ior->iiop.version);
}


GIOP_C::~GIOP_C()
{
  if (pd_state == GIOP_C::Zombie) {
    if (omniORB::traceLevel >= 15) {
      omniORB::log << "GIOP_C dtor re-entered.\n";
      omniORB::log.flush();
    }
    return;
  }
  if (pd_state != GIOP_C::Idle) {
    pd_cdrStream->setTerminalError();
  }
  pd_state = GIOP_C::Zombie;
  pd_cdrStream->release();
}

void 
GIOP_C::InitialiseRequest(const char*          opname,
			  const size_t         opnamesize,
			  const _CORBA_Boolean oneway,
			  const _CORBA_Boolean response_expected,
			  giopMarshaller&      marshaller)
{
  if (pd_state != GIOP_C::Idle)
    throw omniORB::fatalException(__FILE__,__LINE__,
      "GIOP_C::InitialiseRequest() entered with the wrong state.");
 
  pd_state = GIOP_C::RequestInProgress;

  pd_response_expected = response_expected;

  // Setup callback in case the giopStream have to calculate the
  // message body size.
  pd_cdrStream->outputMessageBodyMarshaller(marshaller);


  // Marshal request header
  (void) pd_cdrStream->outputRequestMessageBegin(pd_ior,
						 opname,
						 opnamesize,
						 oneway,
						 response_expected);

  // Marshal request body
  marshaller.marshal(*pd_cdrStream);
}

GIOP::ReplyStatusType 
GIOP_C::ReceiveReply()
{
  if (pd_state != GIOP_C::RequestInProgress)
    throw omniORB::fatalException(__FILE__,__LINE__,
      "GIOP_C::ReceiveReply() entered with the wrong state.");

  pd_cdrStream->outputMessageEnd();
  pd_state = GIOP_C::WaitingForReply;

  if (!pd_response_expected) {
    pd_state = GIOP_C::ReplyIsBeingProcessed;
    return GIOP::NO_EXCEPTION;
  }
  
  GIOP::ReplyStatusType rc;

  rc = pd_cdrStream->inputReplyMessageBegin();

  pd_state = GIOP_C::ReplyIsBeingProcessed;

  switch (rc) {
  case GIOP::SYSTEM_EXCEPTION:
    {
      UnMarshallSystemException();
      // never reaches here
    }
    break;
  case GIOP::NO_EXCEPTION:
  case GIOP::USER_EXCEPTION:
  case GIOP::LOCATION_FORWARD:
  case GIOP::LOCATION_FORWARD_PERM:
  case GIOP::NEEDS_ADDRESSING_MODE:
    break;
  default:
    // Should never receive anything other that the above
    // Same treatment as wrong header
    pd_cdrStream->setTerminalError();
    OMNIORB_THROW(COMM_FAILURE,0,CORBA::COMPLETED_MAYBE);
  }
  return rc;
}

void
GIOP_C::RequestCompleted(CORBA::Boolean skip_msg)
{
  if (pd_state != GIOP_C::ReplyIsBeingProcessed)
    throw omniORB::fatalException(__FILE__,__LINE__,
      "GIOP_C::RequestCompleted() entered with the wrong state.");

  if (pd_response_expected) {
    pd_cdrStream->inputMessageEnd(skip_msg);
  }

  pd_state = GIOP_C::Idle;
}

GIOP::LocateStatusType
GIOP_C::IssueLocateRequest()
{
  if (pd_state != GIOP_C::Idle)
    throw omniORB::fatalException(__FILE__,__LINE__,
      "GIOP_C::IssueLocateRequest() entered with the wrong state.");

  pd_state = GIOP_C::RequestInProgress;

  // XXX Should rewite to use giopMarshaller
  (void) pd_cdrStream->outputLocateMessageBegin(pd_ior);

  pd_cdrStream->outputMessageEnd();

  pd_state = GIOP_C::WaitingForReply;

  GIOP::LocateStatusType rc;

  rc = pd_cdrStream->inputLocateReplyMessageBegin();

  pd_state = GIOP_C::ReplyIsBeingProcessed;


  switch (rc) {
  case GIOP::UNKNOWN_OBJECT:
  case GIOP::OBJECT_HERE:
  case GIOP::OBJECT_FORWARD:
  case GIOP::OBJECT_FORWARD_PERM:
  case GIOP::LOC_NEEDS_ADDRESSING_MODE:
    break;
  case GIOP::LOC_SYSTEM_EXCEPTION:
    UnMarshallSystemException();
    break;
  default:
    // Should never receive anything other that the above
    // Same treatment as wrong header
    pd_cdrStream->setTerminalError();
    OMNIORB_THROW(COMM_FAILURE,0,CORBA::COMPLETED_MAYBE);
  }
  return rc;
}



void
GIOP_C::UnMarshallSystemException()
{

#define CHECK_AND_IF_MATCH_THROW_SYSTEM_EXCEPTION(_ex) \
  if (strncmp((const char *)repoid,(const char *) \
	      GIOP_Basetypes::SysExceptRepoID:: _ex .id, \
	      GIOP_Basetypes::SysExceptRepoID:: _ex .len)==0) \
    OMNIORB_THROW(_ex, m, (CORBA::CompletionStatus) s);

  // Real code begins here
  CORBA::ULong len;
  len <<= *pd_cdrStream;
  if (len > omniORB_GIOP_Basetypes_SysExceptRepoID_maxIDLen)
    OMNIORB_THROW(UNKNOWN,0,CORBA::COMPLETED_MAYBE);

  CORBA::Char repoid[omniORB_GIOP_Basetypes_SysExceptRepoID_maxIDLen];

  pd_cdrStream->get_char_array(repoid, len);
  CORBA::ULong m;
  CORBA::ULong s;
  m <<= *pd_cdrStream;
  s <<= *pd_cdrStream;
  RequestCompleted();
  switch (s) {
  case CORBA::COMPLETED_YES:
  case CORBA::COMPLETED_NO:
  case CORBA::COMPLETED_MAYBE:
    break;
  default:
    OMNIORB_THROW(UNKNOWN,0,CORBA::COMPLETED_MAYBE);
  };

  OMNIORB_FOR_EACH_SYS_EXCEPTION(CHECK_AND_IF_MATCH_THROW_SYSTEM_EXCEPTION)

  // If none of the above matched
  OMNIORB_THROW(UNKNOWN,0,CORBA::COMPLETED_MAYBE);

#undef CHECK_AND_IF_MATCH_THROW_SYSTEM_EXCEPTION
}

