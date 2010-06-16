// -*- Mode: C++; -*-
//                            Package   : omniORB2
// giopImpl10.cc              Created on: 14/02/2001
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 2002-2008 Apasphere Ltd
//    Copyright (C) 2001 AT&T Laboratories, Cambridge
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
  Revision 1.1.6.9  2008/12/29 17:31:16  dgrisby
  Properly handle message size being exceeded in request header.

  Revision 1.1.6.8  2008/08/08 16:52:56  dgrisby
  Option to validate untransformed UTF-8; correct data conversion minor
  codes; better logging for MessageErrors.

  Revision 1.1.6.7  2006/09/20 13:36:31  dgrisby
  Descriptive logging for connection and GIOP errors.

  Revision 1.1.6.6  2006/06/05 11:28:04  dgrisby
  Change clientSendRequest interceptor members to a single GIOP_C.

  Revision 1.1.6.5  2005/12/08 14:22:31  dgrisby
  Better string marshalling performance; other minor optimisations.

  Revision 1.1.6.4  2005/04/11 12:09:42  dgrisby
  Another merge.

  Revision 1.1.6.3  2005/01/06 23:10:15  dgrisby
  Big merge from omni4_0_develop.

  Revision 1.1.6.2  2003/07/10 21:55:56  dgrisby
  Use re-entrant GIOP 1.0 size calc.

  Revision 1.1.6.1  2003/03/23 21:02:16  dgrisby
  Start of omniORB 4.1.x development branch.

  Revision 1.1.4.19  2003/01/22 11:40:12  dgrisby
  Correct serverSendException interceptor use.

  Revision 1.1.4.18  2002/11/26 16:54:34  dgrisby
  Fix exception interception.

  Revision 1.1.4.17  2002/11/26 14:51:50  dgrisby
  Implement missing interceptors.

  Revision 1.1.4.16  2002/07/04 15:14:40  dgrisby
  Correct usage of MessageErrors, fix log messages.

  Revision 1.1.4.15  2002/03/27 11:44:51  dpg1
  Check in interceptors things left over from last week.

  Revision 1.1.4.14  2002/03/18 12:38:25  dpg1
  Lower trace(0) to trace(1), propagate fatalException.

  Revision 1.1.4.13  2001/10/19 11:06:45  dpg1
  Principal support for GIOP 1.0. Correct some spelling mistakes.

  Revision 1.1.4.12  2001/09/12 19:43:19  sll
  Enforce GIOP message size limit.

  Revision 1.1.4.11  2001/09/10 17:46:09  sll
  When a connection is broken, check if it has been shutdown orderly. If so,
  do a retry.

  Revision 1.1.4.10  2001/09/04 14:38:51  sll
  Added the boolean argument to notifyCommFailure to indicate if
  omniTransportLock is held by the caller.

  Revision 1.1.4.9  2001/09/03 16:55:41  sll
  Modified to match the new signature of the giopStream member functions that
  previously accept explicit deadline parameters. The deadline is now
  implicit in the giopStream.

  Revision 1.1.4.8  2001/08/17 17:12:36  sll
  Modularise ORB configuration parameters.

  Revision 1.1.4.7  2001/07/31 16:20:30  sll
  New primitives to acquire read lock on a connection.

  Revision 1.1.4.6  2001/06/20 18:35:18  sll
  Upper case send,recv,connect,shutdown to avoid silly substutition by
  macros defined in socket.h to rename these socket functions
  to something else.

  Revision 1.1.4.5  2001/05/11 14:30:12  sll
  Message size limit is now enforced.

  Revision 1.1.4.4  2001/05/04 13:55:58  sll
  Silly mistake that causes non-copy marshalling to do the wrong thing
  in GIOP 1.0.

  Revision 1.1.4.3  2001/05/01 17:56:29  sll
  Remove user exception check in sendUserException. This has been done by
  the caller.

  Revision 1.1.4.2  2001/05/01 17:15:18  sll
  Non-copy input now works correctly.

  Revision 1.1.4.1  2001/04/18 18:10:51  sll
  Big checkin with the brand new internal APIs.

*/

#include <omniORB4/CORBA.h>
#include <giopStream.h>
#include <giopStreamImpl.h>
#include <giopStrand.h>
#include <giopRope.h>
#include <GIOP_S.h>
#include <GIOP_C.h>
#include <omniORB4/minorCode.h>
#include <initialiser.h>
#include <exceptiondefs.h>
#include <omniORB4/callDescriptor.h>
#include <omniORB4/omniInterceptors.h>
#include <interceptors.h>
#include <orbParameters.h>

// Define PRE_CALCULATE_MESSAGE_SIZE to force the code to pre-calculate
// the total size of the message before the actual marshalling.
// The implementation can actually avoid this calculation if full message
// can be fully buffered.
// The version without pre-calculation can only work if all
// marshalling code is re-entrant.

//#define PRE_CALCULATE_MESSAGE_SIZE


OMNI_NAMESPACE_BEGIN(omni)

class nonexistence;

class giopImpl10 {
public:
  static void outputMessageBegin(giopStream*,
				 void (*marshalHeader)(giopStream*));
  static void outputMessageEnd(giopStream*);
  static void inputMessageBegin(giopStream*,
				void (*unmarshalHeader)(giopStream*));
  static void inputMessageEnd(giopStream*,CORBA::Boolean skip = 0);
  static void sendMsgErrorMessage(giopStream*, const CORBA::SystemException*);
  static void marshalRequestHeader(giopStream*);
  static void sendLocateRequest(giopStream*);
  static void unmarshalReplyHeader(giopStream*);
  static void unmarshalLocateReply(giopStream*);
  static void unmarshalWildCardRequestHeader(giopStream*);
  static void unmarshalRequestHeader(giopStream*);
  static void unmarshalLocateRequest(giopStream*);
  static void marshalReplyHeader(giopStream*);
  static void sendSystemException(giopStream*,const CORBA::SystemException&);
  static void sendUserException(giopStream*,const CORBA::UserException&);
  static void sendLocationForwardReply(giopStream*,CORBA::Object_ptr,
				       CORBA::Boolean=0);
  static void sendLocateReply(giopStream*,GIOP::LocateStatusType,
			      CORBA::Object_ptr,CORBA::SystemException* p = 0);
  static size_t inputRemaining(giopStream*);
  static void getInputData(giopStream*,omni::alignment_t,size_t);
  static void skipInputData(giopStream*,size_t);
  static void copyInputData(giopStream*,void*, size_t,omni::alignment_t);
  static size_t outputRemaining(const giopStream*);
  static void getReserveSpace(giopStream*,omni::alignment_t,size_t);
  static void copyOutputData(giopStream*,void*, size_t,omni::alignment_t);
  static CORBA::ULong currentInputPtr(const giopStream*);
  static CORBA::ULong currentOutputPtr(const giopStream*);

  friend class nonexistence;  // Just to make gcc shut up.

  static CORBA::Boolean inputReplyBegin(giopStream*, 
					void (*unmarshalHeader)(giopStream*));

  static void inputTerminalProtocolError(giopStream* g,
					 const char* file, int line,
					 const char* message);
  // Helper function.  Call this function to indicate that a protocol
  // voilation was detected.  This function *always* raise a
  // giopStream::CommFailure exception.  Therefore the caller should not
  // expect this function to return.

  static void inputRaiseCommFailure(giopStream* g, const char* message);

  static void outputNewMessage(giopStream*);

  static void outputFlush(giopStream* g);

  static void outputSetMessageSize(giopStream*,CORBA::ULong);

private:
  giopImpl10();
  giopImpl10(const giopImpl10&);
  giopImpl10& operator=(const giopImpl10&);
};


////////////////////////////////////////////////////////////////////////
void
giopImpl10::inputMessageBegin(giopStream* g,
			      void (*unmarshalHeader)(giopStream*)) {

 again:
  {
    omni_tracedmutex_lock sync(*omniTransportLock);

    while (!(g->inputFullyBuffered() || g->pd_rdlocked)) {
      if (!g->rdLockNonBlocking()) {
	g->sleepOnRdLock();
      }
    }
  }

  if (!g->pd_currentInputBuffer) {

    if (g->pd_input) {
      g->pd_currentInputBuffer = g->pd_input;
      g->pd_input = g->pd_input->next;
      g->pd_currentInputBuffer->next = 0;
    }
    else {
      g->pd_currentInputBuffer = g->inputMessage();
    }
  }

  char* hdr = (char*)g->pd_currentInputBuffer + 
                     g->pd_currentInputBuffer->start;

  if (hdr[4] != 1 || hdr[5] != 0) {
    // We accept a CloseConnection message with any GIOP version.
    if ((GIOP::MsgType)hdr[7] != GIOP::CloseConnection) {
      inputTerminalProtocolError(g, __FILE__, __LINE__,
				 "Invalid GIOP message version");
      // never reaches here.
    }
  }

  g->pd_unmarshal_byte_swap = (((hdr[6] & 0x1) == _OMNIORB_HOST_BYTE_ORDER_)
			       ? 0 : 1 );

  g->pd_inb_mkr = (void*)(hdr + 12);
  g->pd_inb_end = (void*)((omni::ptr_arith_t)g->pd_currentInputBuffer + 
			  g->pd_currentInputBuffer->last);


  g->inputExpectAnotherFragment(0);
  g->inputMessageSize(g->pd_currentInputBuffer->size);
  g->inputFragmentToCome(g->pd_currentInputBuffer->size - 
			 (g->pd_currentInputBuffer->last -
			  g->pd_currentInputBuffer->start));

  if (unmarshalHeader == unmarshalWildCardRequestHeader)
    unmarshalHeader(g);
  else {
    if (!inputReplyBegin(g,unmarshalHeader)) goto again;
  }
}

////////////////////////////////////////////////////////////////////////
CORBA::Boolean
giopImpl10::inputReplyBegin(giopStream* g, 
			    void (*unmarshalHeader)(giopStream*)) {

  if (g->inputMessageSize() > orbParameters::giopMaxMsgSize) {

    // This is not the perfect solution if we are multiplexing calls onto
    // this connection. The reply we are getting here may not be for the
    // call of this thread. If that is the case, we are sending the wrong
    // exception to the threads. On the otherhand, the server side is
    // quite doggy and it is not prudent to trust the message content.
    // Therefore, decoding the message further may not be doing any good.
    OMNIORB_THROW(MARSHAL,MARSHAL_MessageSizeExceedLimitOnClient,
		  CORBA::COMPLETED_YES);
  }

  char* hdr = (char*)g->pd_currentInputBuffer + 
                     g->pd_currentInputBuffer->start;

  GIOP::MsgType mtype = (GIOP::MsgType)hdr[7];

  switch (mtype) {
  case GIOP::LocateReply:
    {
      unmarshalLocateReply(g);
      break;
    }
  case GIOP::Reply:
    {
      unmarshalReplyHeader(g);
      break;
    }
  case GIOP::CloseConnection:
    {
      CORBA::ULong minor;
      CORBA::Boolean retry;
      g->pd_strand->orderly_closed = 1;
      g->notifyCommFailure(0,minor,retry);
      g->pd_strand->state(giopStrand::DYING);
      giopStream::CommFailure::_raise(minor,
				      CORBA::COMPLETED_NO,
				      retry,__FILE__,__LINE__,
				      "Orderly connection shutdown",
				      g->pd_strand);
      // never reach here.
      break;
    }
  default:
    inputTerminalProtocolError(g, __FILE__, __LINE__,
			       "Invalid reply message type");
    // never reaches here.
  }

  GIOP_C* source = (GIOP_C*) g;

  if (source->replyId() == source->requestId()) {

    if (mtype == GIOP::LocateReply && 
	unmarshalHeader != unmarshalLocateReply) {

      inputTerminalProtocolError(g, __FILE__, __LINE__,
				 "Unexpected LocateReply");
      // never reach here
    }
    else if (mtype == GIOP::Reply &&
	     unmarshalHeader != unmarshalReplyHeader) {

      inputTerminalProtocolError(g, __FILE__, __LINE__,
				 "Unexpected Reply");
      // never reach here
    }

    giopStream_Buffer* p = g->pd_input;
    while (p) {
      giopStream_Buffer* q = p->next;
      giopStream_Buffer::deleteBuffer(p);
      p = q;
    }
    g->pd_input = 0;
    g->inputMatchedId(1);
    return 1;
  }
  else {

    omni_tracedmutex_lock sync(*omniTransportLock);
    giopStreamList* gp = g->pd_strand->clients.next;
    for (; gp != &g->pd_strand->clients; gp = gp->next) {

      GIOP_C* target = (GIOP_C*)gp;

      if (target->state() != IOP_C::UnUsed &&
	  target->requestId() == source->replyId()) {

	if (target->inputMatchedId()) {
	  // a reply has already been received!
	  inputTerminalProtocolError(g, __FILE__, __LINE__,
				     "Duplicate Reply");
	}

	target->pd_input = source->pd_input;
	source->pd_input = 0;

	giopStream_Buffer** pp = &target->pd_input;
	while (*pp) {
	  pp = &((*pp)->next);
	}
	*pp = source->pd_currentInputBuffer;
	source->pd_currentInputBuffer = 0;

	target->inputFullyBuffered(source->inputFullyBuffered());
	source->inputFullyBuffered(0);
	target->pd_rdlocked = 1;
	source->pd_rdlocked = 0;
	target->inputMatchedId(1);
	giopStream::wakeUpRdLock(g->pd_strand);
	return 0;
      }
    }
    // reach here only if there is no match to the replyid.
    // Skip this message
    giopStream_Buffer* p = g->pd_input;
    while (p) {
      giopStream_Buffer* q = p->next;
      giopStream_Buffer::deleteBuffer(p);
      p = q;
    }
    g->pd_input = 0;
    if (g->pd_currentInputBuffer) {
      giopStream_Buffer::deleteBuffer(g->pd_currentInputBuffer);
      g->pd_currentInputBuffer = 0;
    }
    while (g->inputFragmentToCome()) {
      giopStream_Buffer* dummy = g->inputChunk(g->inputFragmentToCome());
      g->inputFragmentToCome( g->inputFragmentToCome() -
			      (dummy->last - dummy->start) );
      giopStream_Buffer::deleteBuffer(dummy);
    }
    return 0;
  }
}


////////////////////////////////////////////////////////////////////////
void
giopImpl10::inputMessageEnd(giopStream* g,CORBA::Boolean disgard) {

  if ( g->pd_strand->state() != giopStrand::DYING ) {

    if (!disgard && inputRemaining(g)) {
      if (omniORB::trace(15)) {
	omniORB::logger l;
	l << "Garbage left at the end of input message from "
	  << g->pd_strand->connection->peeraddress() << "\n";
      }
      if (!orbParameters::strictIIOP) {
	disgard = 1;
      }
      else {
	inputTerminalProtocolError(g, __FILE__, __LINE__,
				   "Garbage left at end of input message");
	// never reach here.
      }
    }

    if (disgard)
      skipInputData(g,inputRemaining(g));

    if (g->pd_currentInputBuffer) {
      g->releaseInputBuffer(g->pd_currentInputBuffer);
      g->pd_currentInputBuffer = 0;
    }
  }
  if (g->pd_rdlocked) {
    omni_tracedmutex_lock sync(*omniTransportLock);
    g->rdUnLock();
  }
}

////////////////////////////////////////////////////////////////////////
void
giopImpl10::unmarshalReplyHeader(giopStream* g) {

  char* hdr = (char*)g->pd_currentInputBuffer + 
                     g->pd_currentInputBuffer->start;

  if ((GIOP::MsgType)hdr[7] != GIOP::Reply) {
    // Unexpected reply. The other end is terribly confused. Drop the
    // connection and died.
    inputTerminalProtocolError(g, __FILE__, __LINE__,
			       "Incorrect message type when expecting a "
			       "Reply");
    // Never reach here.
  }

  GIOP_C& giop_c = *((GIOP_C*) g);
  cdrStream& s = *((cdrStream*)g);

  IOP::ServiceContextList sctxts;

  if (omniInterceptorP::clientReceiveReply) {
    sctxts <<= s;
  }
  else {
    // Skip service context
    CORBA::ULong svcccount;
    CORBA::ULong svcctag;
    CORBA::ULong svcctxtsize;
    svcccount <<= s;
    while (svcccount-- > 0) {
      svcctag <<= s;
      svcctxtsize <<= s;
      s.skipInput(svcctxtsize);
    }
  }

  CORBA::ULong id;
  id <<= s;
  giop_c.replyId(id);

  CORBA::ULong v;
  v <<= s;

  switch (v) {
  case GIOP::SYSTEM_EXCEPTION:
  case GIOP::NO_EXCEPTION:
  case GIOP::USER_EXCEPTION:
  case GIOP::LOCATION_FORWARD:
    break;
  default:
    // Should never receive anything other that the above
    // Same treatment as wrong header
    inputTerminalProtocolError(g, __FILE__, __LINE__,
			       "Invalid status in Reply");
    // Never reach here.
    break;
  }
  giop_c.replyStatus((GIOP::ReplyStatusType)v);

  if (omniInterceptorP::clientReceiveReply) {
    omniInterceptors::clientReceiveReply_T::info_T info(giop_c, sctxts);
    omniInterceptorP::visit(info);
  }
}

////////////////////////////////////////////////////////////////////////
void
giopImpl10::unmarshalLocateReply(giopStream* g) {

  char* hdr = (char*)g->pd_currentInputBuffer + 
                     g->pd_currentInputBuffer->start;

  if ((GIOP::MsgType)hdr[7] != GIOP::LocateReply) {
    // Unexpected reply. The other end is terribly confused. Drop the
    // connection and died.
    inputTerminalProtocolError(g, __FILE__, __LINE__,
			       "Incorrect message type when expecting a "
			       "LocateReply");
    // Never reach here.
  }

  GIOP_C& giop_c = *((GIOP_C*) g);
  cdrStream& s = *((cdrStream*)g);

  CORBA::ULong id;
  id <<= s;
  giop_c.replyId(id);

  CORBA::ULong v;
  v <<= s;

  switch (v) {
  case GIOP::UNKNOWN_OBJECT:
  case GIOP::OBJECT_HERE:
  case GIOP::OBJECT_FORWARD:
    break;
  default:
    // Should never receive anything other that the above
    // Same treatment as wrong header
    inputTerminalProtocolError(g, __FILE__, __LINE__,
			       "Invalid status in LocateReply");
    // Never reach here.
    break;
  }
  giop_c.locateStatus((GIOP::LocateStatusType)v);
}

////////////////////////////////////////////////////////////////////////
void
giopImpl10::unmarshalWildCardRequestHeader(giopStream* g) {

  g->inputMatchedId(1);

  char* hdr = (char*)g->pd_currentInputBuffer + 
                     g->pd_currentInputBuffer->start;

  ((GIOP_S*)g)->requestType((GIOP::MsgType)hdr[7]);
  switch (((GIOP_S*)g)->requestType()) {
  case GIOP::CloseConnection:
    inputRaiseCommFailure(g, "Orderly connection shutdown");
    break;
  case GIOP::Request:
  case GIOP::LocateRequest:
  case GIOP::CancelRequest:
    if (g->inputMessageSize() <= orbParameters::giopMaxMsgSize) {
      break;
    }
    inputTerminalProtocolError(g, __FILE__, __LINE__,
			       "GIOP message size limit exceeded");
    break;
  default:
    inputTerminalProtocolError(g, __FILE__, __LINE__,
			       "Unknown GIOP message type");
    // Never reach here.
    break;
  }
}

////////////////////////////////////////////////////////////////////////
void
giopImpl10::unmarshalRequestHeader(giopStream* g) {

  GIOP_S& giop_s = *((GIOP_S*) g);
  cdrStream& s = *((cdrStream*)g);
  
  giop_s.service_contexts() <<= s;

  CORBA::ULong   vl;
  CORBA::Boolean vb;

  // request id
  vl <<= s;
  giop_s.requestId(vl);

  // response expected
  vb = s.unmarshalBoolean();
  giop_s.response_expected(vb);
  giop_s.result_expected(1);

  // object key
  vl <<= s;
  if (!s.checkInputOverrun(1,vl)) {
    OMNIORB_THROW(MARSHAL,MARSHAL_InvalidVariableLenComponentSize,
		  CORBA::COMPLETED_NO);
  }
  giop_s.keysize(vl);
  s.get_octet_array(giop_s.key(),vl);

  // operation
  vl <<= s;
  if (!vl || !s.checkInputOverrun(1,vl)) {
    OMNIORB_THROW(MARSHAL,MARSHAL_InvalidVariableLenComponentSize,
		  CORBA::COMPLETED_NO);
  }
  giop_s.set_operation_size(vl);
  char* op = giop_s.operation();
  s.get_octet_array((CORBA::Octet*)op,vl);
  op[vl-1] = '\0';

  // principal
  vl <<= s;
  if (!s.checkInputOverrun(1,vl)) {
    OMNIORB_THROW(MARSHAL,MARSHAL_InvalidVariableLenComponentSize,
		  CORBA::COMPLETED_NO);
  }
  giop_s.set_principal_size(vl);
  giop_s.get_octet_array(giop_s.principal(), vl);
}

////////////////////////////////////////////////////////////////////////
void
giopImpl10::unmarshalLocateRequest(giopStream* g) {

  GIOP_S& giop_s = *((GIOP_S*) g);
  cdrStream& s = *((cdrStream*)g);

  CORBA::ULong vl;

  // request ID
  vl <<= s;
  giop_s.requestId(vl);

  // object key
  vl <<= s;
  if (!s.checkInputOverrun(1,vl)) {
    OMNIORB_THROW(MARSHAL,MARSHAL_InvalidVariableLenComponentSize,
		  CORBA::COMPLETED_NO);
  }
  giop_s.keysize(vl);
  s.get_octet_array(giop_s.key(),vl);
}

////////////////////////////////////////////////////////////////////////
size_t
giopImpl10::inputRemaining(giopStream* g) {

  return (g->inputFragmentToCome() + ((omni::ptr_arith_t)g->pd_inb_end -
				      (omni::ptr_arith_t)g->pd_inb_mkr));
}

////////////////////////////////////////////////////////////////////////
void
giopImpl10::getInputData(giopStream* g,omni::alignment_t align,size_t sz) {

  omni::ptr_arith_t last = omni::align_to((omni::ptr_arith_t)g->pd_inb_mkr,
					  align);

  omni::ptr_arith_t end = last + sz;

  if ( end <= (omni::ptr_arith_t) g->pd_inb_end) {
    return;
  }
  else {
    // Invariant check 
    // Either the full message is already in the buffer or the part that is
    // in the buffer always ends at an 8 byte aligned boundary.
    // Also remember that sz is <= 8 always!
    size_t extra = end - (omni::ptr_arith_t) g->pd_inb_end;
    if (extra != sz) {
      if (!g->inputFragmentToCome()) {
	// The full message is already in the buffer. The unmarshalling
	// code is asking for more. This is an error causes by the received
	// data. We'll let the code below to raise a MARSHAL exception
	sz = extra;            // in case sz == 0
      }
      else {
	// Very bad. Should never happen given our invariant.
	{
	  if( omniORB::trace(1) ) {
	    omniORB::logger l;
	    l << "Fatal error in unmarshalling message from "
	      << g->pd_strand->connection->peeraddress()
	      << ", invariant was violated at "
	      << __FILE__ 	
	      << ":" << __LINE__ << '\n';
	  }
	}
	OMNIORB_ASSERT(0);
	// never reach here.
      }
    }
  }

  if (g->inputFragmentToCome() < sz) {
    if (!g->inputMatchedId()) {
      // Because this error occurs when the id of the reply has not
      // been established. We have to treat this as an error on the
      // connection. Other threads that are reading from this connection
      // will notice this as well.
      g->pd_strand->state(giopStrand::DYING);
    }
    OMNIORB_THROW(MARSHAL,MARSHAL_PassEndOfMessage,
		  (CORBA::CompletionStatus)g->completion());
  }

  if (g->inputMatchedId()) {
    if (g->pd_currentInputBuffer) {
      g->releaseInputBuffer(g->pd_currentInputBuffer);
      g->pd_currentInputBuffer = 0;
    }
    if (!g->pd_input) {
      g->pd_currentInputBuffer = g->inputChunk(g->inputFragmentToCome());
    }
    else {
      g->pd_currentInputBuffer = g->pd_input;
      g->pd_input = g->pd_currentInputBuffer->next;
      g->pd_currentInputBuffer->next = 0;
    }
  }
  else {
    // We keep the buffer around until the id of the reply is established.
    giopStream_Buffer** pp = &g->pd_input;
    while (*pp) {
      pp = &((*pp)->next);
    }
    *pp = g->pd_currentInputBuffer;
    g->pd_currentInputBuffer = 0;
    g->pd_currentInputBuffer = g->inputChunk(g->inputFragmentToCome());
  }

  g->pd_inb_mkr = (void*)((omni::ptr_arith_t)g->pd_currentInputBuffer + 
			  g->pd_currentInputBuffer->start);
  g->pd_inb_end = (void*)((omni::ptr_arith_t)g->pd_currentInputBuffer + 
			  g->pd_currentInputBuffer->last);
  g->inputFragmentToCome(g->inputFragmentToCome() - 
			 (g->pd_currentInputBuffer->last -
			  g->pd_currentInputBuffer->start));
}

////////////////////////////////////////////////////////////////////////
void
giopImpl10::skipInputData(giopStream* g,size_t sz) {
  copyInputData(g,0,sz,omni::ALIGN_1);
}

////////////////////////////////////////////////////////////////////////
void
giopImpl10::copyInputData(giopStream* g,void* b, size_t sz,
			  omni::alignment_t align) {

  // If b == 0, we don't actually copy the data but just skip <sz> bytes.

  omni::ptr_arith_t last = omni::align_to((omni::ptr_arith_t)g->pd_inb_mkr,
					  align);

  if ( last > (omni::ptr_arith_t) g->pd_inb_end ) {
    // Invariant check 
    // Either the full message is already in the buffer or the part that is
    // in the buffer always ends at an 8 byte aligned boundary.
    if (!g->inputFragmentToCome()) {
      // The full message is already in the buffer. The unmarshalling
      // code is asking for more. This is an error causes by the received
      // data.
      if (!g->inputMatchedId()) {
	// Because this error occurs when the id of the reply has not
	// been established. We have to treat this as an error on the
	// connection. Other threads that are reading from this connection
	// will notice this as well.
	g->pd_strand->state(giopStrand::DYING);
      }
      OMNIORB_THROW(MARSHAL,MARSHAL_PassEndOfMessage,
		    (CORBA::CompletionStatus)g->completion());
    }
    else {
      // Very bad. Should never happen given our invariant.
      {
	if( omniORB::trace(1) ) {
	  omniORB::logger l;
	  l << "Fatal error in unmarshalling message from "
	    << g->pd_strand->connection->peeraddress()
	    << ", invariant was violated at "
	    << __FILE__ 	
	    << ":" << __LINE__ << '\n';
	}
      }
      OMNIORB_ASSERT(0);
      // never reach here.
    }
  }

  g->pd_inb_mkr = (void*) last;
  while (sz) {

    size_t avail = (omni::ptr_arith_t) g->pd_inb_end - 
                   (omni::ptr_arith_t) g->pd_inb_mkr;
    if (avail >= sz) {
      avail = sz;
    }
    if (avail) {
      if (b) memcpy(b,g->pd_inb_mkr,avail);
      g->pd_inb_mkr = (void*)((omni::ptr_arith_t)g->pd_inb_mkr + avail);
    }
    sz -= avail;
    if (b) b = (void*)((omni::ptr_arith_t)b + avail);

    if (!sz) break;

    if (g->pd_inb_mkr == g->pd_inb_end) {

      if (g->inputFragmentToCome() < sz) {
	if (!g->inputMatchedId()) {
	  // Because this error occurs when the id of the reply has not
	  // been established. We have to treat this as an error on the
	  // connection. Other threads that are reading from this connection
	  // will notice this as well.
	  g->pd_strand->state(giopStrand::DYING);
	}
	OMNIORB_THROW(MARSHAL,MARSHAL_PassEndOfMessage,
		      (CORBA::CompletionStatus)g->completion());
	// never reach here
      }


      if (g->inputMatchedId()) {

	if (g->pd_currentInputBuffer) {
	  g->releaseInputBuffer(g->pd_currentInputBuffer);
	  g->pd_currentInputBuffer = 0;
	}
	if (g->pd_input) {
	  g->pd_currentInputBuffer = g->pd_input;
	  g->pd_input = g->pd_currentInputBuffer->next;
	  g->pd_currentInputBuffer->next = 0;
	}
	else {

	  if ( b && sz >= giopStream::directReceiveCutOff ) {
	  
	    CORBA::ULong transz = g->inputFragmentToCome();
	    if (transz > sz) transz = sz;
	    transz = (transz >> 3) << 3;
	    g->inputCopyChunk(b,transz);
	    sz -= transz;
	    b = (void*)((omni::ptr_arith_t)b + transz);
	    g->inputFragmentToCome(g->inputFragmentToCome() - transz);
	    continue;
	  }
	  else {
	    g->pd_currentInputBuffer = g->inputChunk(g->inputFragmentToCome());
	  }
	}
      }
      else {
	// We keep the buffer around until the id of the reply is established.

	if (g->pd_currentInputBuffer) {
	  giopStream_Buffer** pp = &g->pd_input;
	  while (*pp) {
	    pp = &((*pp)->next);
	  }
	  *pp = g->pd_currentInputBuffer;
	  g->pd_currentInputBuffer = 0;
	  g->pd_currentInputBuffer = g->inputChunk(g->inputFragmentToCome());
	}
      }

      g->pd_inb_mkr = (void*)((omni::ptr_arith_t)g->pd_currentInputBuffer + 
			      g->pd_currentInputBuffer->start);
      g->pd_inb_end = (void*)((omni::ptr_arith_t)g->pd_currentInputBuffer + 
			      g->pd_currentInputBuffer->last);
      g->inputFragmentToCome(g->inputFragmentToCome() - 
			     (g->pd_currentInputBuffer->last -
			      g->pd_currentInputBuffer->start));
    }
  }	
}

////////////////////////////////////////////////////////////////////////
CORBA::ULong
giopImpl10::currentInputPtr(const giopStream* g) {

  return  g->inputMessageSize() - 
          g->inputFragmentToCome() -
         ((omni::ptr_arith_t) g->pd_inb_end - 
	  (omni::ptr_arith_t) g->pd_inb_mkr);
}

////////////////////////////////////////////////////////////////////////
void
giopImpl10::inputTerminalProtocolError(giopStream* g,
				       const char* file, int line,
				       const char* message) {

  if (omniORB::trace(1)) {
    omniORB::logger l;
    l << "From endpoint: " << g->pd_strand->connection->peeraddress()
      << ". Detected GIOP 1.0 protocol error in input message. "
      << omniExHelper::strip(file) << ":" << line
      << ". Connection is closed.\n";
  }

  sendMsgErrorMessage(g, 0);

  inputRaiseCommFailure(g, message);
}

////////////////////////////////////////////////////////////////////////
void
giopImpl10::inputRaiseCommFailure(giopStream* g, const char* message) {

  CORBA::ULong minor;
  CORBA::Boolean retry;
  g->notifyCommFailure(0,minor,retry);
  g->pd_strand->state(giopStrand::DYING);
  giopStream::CommFailure::_raise(minor,
				  (CORBA::CompletionStatus)g->completion(),
				  0,__FILE__,__LINE__,message,g->pd_strand);
}

////////////////////////////////////////////////////////////////////////
void
giopImpl10::outputNewMessage(giopStream* g) {

  if (!g->pd_wrlocked) {
    omni_tracedmutex_lock sync(*omniTransportLock);
    g->wrLock();
  }

  if (!g->pd_currentOutputBuffer) {
    g->pd_currentOutputBuffer = giopStream_Buffer::newBuffer();
  }
  g->pd_currentOutputBuffer->alignStart(omni::ALIGN_8);

  char* hdr = (char*)g->pd_currentOutputBuffer + 
                     g->pd_currentOutputBuffer->start;

  hdr[0] = 'G'; hdr[1] = 'I'; hdr[2] = 'O'; hdr[3] = 'P';
  hdr[4] = 1;   hdr[5] = 0;   hdr[6] = _OMNIORB_HOST_BYTE_ORDER_;

  g->pd_outb_mkr = (void*)(hdr + 12);
  g->pd_outb_end = (void*)((omni::ptr_arith_t)g->pd_currentOutputBuffer + 
			   g->pd_currentOutputBuffer->end);
  g->outputFragmentSize(0);
  g->outputMessageSize(0);
}

////////////////////////////////////////////////////////////////////////
void
giopImpl10::outputMessageBegin(giopStream* g,
			       void (*marshalHeader)(giopStream*)) {

  outputNewMessage(g);
  marshalHeader(g);
}

////////////////////////////////////////////////////////////////////////
void
giopImpl10::outputMessageEnd(giopStream* g) {

  if (g->pd_currentOutputBuffer) {

    omni::ptr_arith_t outbuf_begin = ((omni::ptr_arith_t) 
				    g->pd_currentOutputBuffer + 
				    g->pd_currentOutputBuffer->start);

    if ( outbuf_begin != (omni::ptr_arith_t)g->pd_outb_mkr ) {

      if (!g->outputMessageSize()) {

	CORBA::ULong sz = (omni::ptr_arith_t)g->pd_outb_mkr - outbuf_begin -12;
	*((CORBA::ULong*)(outbuf_begin + 8)) = sz;
	outputSetMessageSize(g,sz);
      }

      g->pd_currentOutputBuffer->last = (omni::ptr_arith_t) g->pd_outb_mkr - 
					(omni::ptr_arith_t) 
                                        g->pd_currentOutputBuffer;

      g->sendChunk(g->pd_currentOutputBuffer);
    
    }
    // Notice that we do not release the buffer. Next time this giopStream
    // is re-used, the buffer will be reused as well.
  }

  {
    omni_tracedmutex_lock sync(*omniTransportLock);
    g->wrUnLock();
  }
}


////////////////////////////////////////////////////////////////////////
void
giopImpl10::sendMsgErrorMessage(giopStream* g,
				const CORBA::SystemException* ex) {

  if (!g->pd_wrlocked) {
    omni_tracedmutex_lock sync(*omniTransportLock);
    g->wrLock();
  }

  if (omniORB::trace(1)) {
    omniORB::logger l;

    l << "To endpoint: " << g->pd_strand->connection->peeraddress() << ". ";

    if (ex) {
      l << "System exception " << *ex << " while (un)marshalling. "
	<< "Send GIOP 1.0 MessageError.\n";
    }
    else {
      l << "Send GIOP 1.0 MessageError because a protocol error has "
	<< "been detected. Connection is closed.\n";
    }
  }

  if (!g->pd_currentOutputBuffer) {
    g->pd_currentOutputBuffer = giopStream_Buffer::newBuffer();
  }
  g->pd_currentOutputBuffer->alignStart(omni::ALIGN_8);

  char* hdr = (char*)g->pd_currentOutputBuffer + 
                     g->pd_currentOutputBuffer->start;

  hdr[0] = 'G'; hdr[1] = 'I'; hdr[2] = 'O'; hdr[3] = 'P';
  hdr[4] = 1;   hdr[5] = 0;   hdr[6] = _OMNIORB_HOST_BYTE_ORDER_;

  g->pd_outb_mkr = (void*)(hdr + 12);
  g->pd_outb_end = (void*)((omni::ptr_arith_t)g->pd_currentOutputBuffer + 
			   g->pd_currentOutputBuffer->end);
  g->outputFragmentSize(0);
  g->outputMessageSize(0);

  hdr[7] = (char)GIOP::MessageError;
  hdr[8] = hdr[9] = hdr[10] = hdr[11] = 0;

  (void)  g->pd_strand->connection->Send(hdr,12);

  g->pd_strand->state(giopStrand::DYING);

  {
    omni_tracedmutex_lock sync(*omniTransportLock);
    g->wrUnLock();
  }
}

////////////////////////////////////////////////////////////////////////
void
giopImpl10::marshalRequestHeader(giopStream* g) {

  char* hdr = (char*) g->pd_currentOutputBuffer + 
                      g->pd_currentOutputBuffer->start;

  hdr[7] = (char) GIOP::Request;

  GIOP_C& giop_c = *(GIOP_C*)g;
  cdrStream& s = (cdrStream&) *g;
  omniCallDescriptor& calldesc = *giop_c.calldescriptor();
  CORBA::Boolean response_expected = !calldesc.is_oneway();
  

  omniInterceptors::clientSendRequest_T::info_T info(giop_c);
  omniInterceptorP::visit(info);

  {
    // calculate the request header size
    cdrCountingStream cs(g->TCS_C(),g->TCS_W(),12);
    info.service_contexts >>= cs;
    giop_c.requestId() >>= cs;
    cs.marshalBoolean(response_expected);
    giop_c.keysize() >>= cs;
    cs.put_octet_array(giop_c.key(),giop_c.keysize());
    operator>>= ((CORBA::ULong)calldesc.op_len(),cs);
    cs.put_octet_array((CORBA::Octet*) calldesc.op(), calldesc.op_len());
    omni::myPrincipalID >>= cs;
    *((CORBA::ULong*)(hdr+8)) = cs.total();

#if defined(PRE_CALCULATE_MESSAGE_SIZE)
    omniORB::logs(30, "Pre-calculating GIOP 1.0 message size.");
    giop_c.calldescriptor()->marshalArguments(cs);
    CORBA::ULong msgsz = cs.total() - 12;
    *((CORBA::ULong*)(hdr + 8)) = msgsz;
    outputSetMessageSize(g,msgsz);
#endif
  }

  // service context
  info.service_contexts >>= s;

  // request id
  giop_c.requestId() >>= s;

  // response expected flag
  s.marshalBoolean(response_expected);

  // object key
  giop_c.keysize() >>= s;
  s.put_small_octet_array(giop_c.key(),giop_c.keysize());

  // operation
  operator>>= ((CORBA::ULong)calldesc.op_len(),s);
  s.put_small_octet_array((CORBA::Octet*) calldesc.op(), calldesc.op_len());

  // principal
  omni::myPrincipalID >>= s;
}

////////////////////////////////////////////////////////////////////////
void
giopImpl10::sendLocateRequest(giopStream* g) {

  outputNewMessage(g);

  char* hdr = (char*) g->pd_currentOutputBuffer + 
                      g->pd_currentOutputBuffer->start;

  hdr[7] = (char)GIOP::LocateRequest;

  GIOP_C& giop_c = *(GIOP_C*)g;
  cdrStream& s = (cdrStream&) *g;

  // Compute and initialise the message size field
  {
    CORBA::ULong totalsz = (omni::ptr_arith_t)g->pd_outb_mkr - 
                           (omni::ptr_arith_t)hdr;

    cdrCountingStream cs(g->TCS_C(),g->TCS_W(),totalsz);
    giop_c.requestId() >>= cs;
    giop_c.keysize() >>= cs;
    cs.put_octet_array(giop_c.key(),giop_c.keysize());
    outputSetMessageSize(g,cs.total()-12);
    *((CORBA::ULong*)(hdr + 8)) = cs.total() - 12;
  }

  // request id
  giop_c.requestId() >>= s;
  
  // object key
  giop_c.keysize() >>= s;
  s.put_small_octet_array(giop_c.key(),giop_c.keysize());

  outputMessageEnd(g);
}



////////////////////////////////////////////////////////////////////////
void
giopImpl10::marshalReplyHeader(giopStream* g) {

  char* hdr = (char*) g->pd_currentOutputBuffer + 
                      g->pd_currentOutputBuffer->start;

  hdr[7] = (char) GIOP::Reply;

  GIOP_S& giop_s = *(GIOP_S*)g;
  cdrStream& s = (cdrStream&) *g;

  CORBA::ULong rc = GIOP::NO_EXCEPTION;
  {
    // calculate the request header size
    cdrCountingStream cs(g->TCS_C(),g->TCS_W(),12);
    giop_s.service_contexts() >>= cs;
    giop_s.requestId() >>= cs;
    rc >>= cs;
    *((CORBA::ULong*)(hdr+8)) = cs.total();

#if defined(PRE_CALCULATE_MESSAGE_SIZE)
    omniORB::logs(30, "Pre-calculating GIOP 1.0 message size.");
    giop_s.calldescriptor()->marshalReturnedValues(cs);
    CORBA::ULong msgsz = cs.total() - 12;
    *((CORBA::ULong*)(hdr + 8)) = msgsz;
    outputSetMessageSize(g,msgsz);
#endif
  }

  // Service context
  giop_s.service_contexts() >>= s;

  // request id
  giop_s.requestId() >>= s;

  // reply status
  rc >>= s;
}

////////////////////////////////////////////////////////////////////////
void
giopImpl10::sendSystemException(giopStream* g,const CORBA::SystemException& ex) {
  GIOP_S& giop_s = *(GIOP_S*)g;
  cdrStream& s = (cdrStream&) *g;

  if (giop_s.state() == GIOP_S::ReplyIsBeingComposed) {
    // This system exception is raised during the marshalling of the reply.
    // If we have already send part of the message out, it is too late
    // to marshal the exception. Can only indicate that something
    // fatal about this request.

#if defined(PRE_CALCULATE_MESSAGE_SIZE)
    if (0)
#else
    if (g->outputMessageSize())
#endif
      {
	sendMsgErrorMessage(g, &ex);

	CORBA::ULong minor;
	CORBA::Boolean retry;
	giop_s.notifyCommFailure(0,minor,retry);
	giopStream::CommFailure::_raise(minor,(CORBA::CompletionStatus)
					giop_s.completion(),
					retry,__FILE__,__LINE__,
					"System Exception occurred while "
					"marshalling reply. Sending a "
					"MessageError", g->pd_strand);
      }
  }

  giop_s.state(GIOP_S::ReplyIsBeingComposed);

  int repoid_size;
  const char* repoid = ex._NP_repoId(&repoid_size);

  outputNewMessage(g);

  char* hdr = (char*) g->pd_currentOutputBuffer + 
                      g->pd_currentOutputBuffer->start;

  hdr[7] = (char) GIOP::Reply;

  giop_s.service_contexts().length(0);

  if (omniInterceptorP::serverSendException) {
    omniInterceptors::serverSendException_T::info_T info(giop_s, &ex);
    omniInterceptorP::visit(info);

    if (giop_s.service_contexts().length() > 0) {

      // Compute and initialise the message size field. Only necessary
      // if there are service contexts, since we know a message without
      // service contexts will fit in a single buffer.

      cdrCountingStream cs(g->TCS_C(),g->TCS_W(),12);
      giop_s.service_contexts() >>= cs;
      operator>>= ((CORBA::ULong)0,cs);
      operator>>= ((CORBA::ULong)0,cs);
      CORBA::ULong(repoid_size) >>= cs;
      cs.put_octet_array((const CORBA::Octet*) repoid, repoid_size);
      ex.minor() >>= cs;
      operator>>= ((CORBA::ULong)0,cs);

      outputSetMessageSize(g,cs.total()-12);
      *((CORBA::ULong*)(hdr + 8)) = cs.total() - 12;
    }
  }

  // Service context
  giop_s.service_contexts() >>= s;

  // request id
  giop_s.requestId() >>= s;

  // reply status
  CORBA::ULong rc = GIOP::SYSTEM_EXCEPTION;
  rc >>= s;

  // system exception value
  CORBA::ULong(repoid_size) >>= s;
  s.put_small_octet_array((const CORBA::Octet*) repoid, repoid_size);
  ex.minor() >>= s;
  CORBA::ULong(ex.completed()) >>= s;

  outputMessageEnd(g);

}

////////////////////////////////////////////////////////////////////////
void
giopImpl10::sendUserException(giopStream* g,const CORBA::UserException& ex) {

  GIOP_S& giop_s = *(GIOP_S*)g;
  cdrStream& s = (cdrStream&) *g;

  giop_s.state(GIOP_S::ReplyIsBeingComposed);

  int i, repoid_size;
  const char* repoid = ex._NP_repoId(&repoid_size);

  outputNewMessage(g);

  char* hdr = (char*)g->pd_currentOutputBuffer + 
                     g->pd_currentOutputBuffer->start;

  hdr[7] = (char)GIOP::Reply;

  giop_s.service_contexts().length(0);

  if (omniInterceptorP::serverSendException) {
    omniInterceptors::serverSendException_T::info_T info(giop_s, &ex);
    omniInterceptorP::visit(info);
  }

  // Compute and initialise the message size field
  {
    cdrCountingStream cs(g->TCS_C(),g->TCS_W(),12);
    giop_s.service_contexts() >>= cs;
    operator>>= ((CORBA::ULong)0,cs);
    operator>>= ((CORBA::ULong)0,cs);
    CORBA::ULong(repoid_size) >>= cs;
    cs.put_octet_array((const CORBA::Octet*) repoid, repoid_size);
    ex._NP_marshal(cs);

    outputSetMessageSize(g,cs.total()-12);
    *((CORBA::ULong*)(hdr + 8)) = cs.total() - 12;
  }

  // Service context
  giop_s.service_contexts() >>= s;

  // request id
  giop_s.requestId() >>= s;

  // reply status
  CORBA::ULong rc = GIOP::USER_EXCEPTION;
  rc >>= s;

  // user exception value
  CORBA::ULong(repoid_size) >>= s;
  s.put_small_octet_array((const CORBA::Octet*) repoid, repoid_size);
  ex._NP_marshal(s);

  outputMessageEnd(g);
}

////////////////////////////////////////////////////////////////////////
void
giopImpl10::sendLocationForwardReply(giopStream* g,CORBA::Object_ptr obj,
				     CORBA::Boolean permanent) {

  outputNewMessage(g);

  char* hdr = (char*)g->pd_currentOutputBuffer + 
                     g->pd_currentOutputBuffer->start;

  hdr[7] = (char)GIOP::Reply;

  GIOP_S& giop_s = *(GIOP_S*)g;
  cdrStream& s = (cdrStream&) *g;

  giop_s.state(GIOP_S::ReplyIsBeingComposed);

  // Service context
  operator>>= ((CORBA::ULong)0,s);

  // request id
  giop_s.requestId() >>= s;

  // reply status
  CORBA::ULong rc = GIOP::LOCATION_FORWARD;
  rc >>= s;

  // Compute and initialise the message size field
  {
    CORBA::ULong totalsz = (omni::ptr_arith_t)g->pd_outb_mkr - 
                           (omni::ptr_arith_t)hdr;

    cdrCountingStream cs(g->TCS_C(),g->TCS_W(),totalsz);
    CORBA::Object::_marshalObjRef(obj,cs);
    outputSetMessageSize(g,cs.total()-12);
    *((CORBA::ULong*)(hdr + 8)) = cs.total() - 12;
  }

  // object reference
  CORBA::Object::_marshalObjRef(obj,s);
  outputMessageEnd(g);
}

////////////////////////////////////////////////////////////////////////
void
giopImpl10::sendLocateReply(giopStream* g,GIOP::LocateStatusType rc,
			    CORBA::Object_ptr obj,CORBA::SystemException*) {

  outputNewMessage(g);

  char* hdr = (char*)g->pd_currentOutputBuffer + 
                     g->pd_currentOutputBuffer->start;

  hdr[7] = (char)GIOP::LocateReply;

  GIOP_S& giop_s = *(GIOP_S*)g;
  cdrStream& s = (cdrStream&) *g;

  giop_s.state(GIOP_S::ReplyIsBeingComposed);

  // request id
  giop_s.requestId() >>= s;

  CORBA::Object_ptr extra = CORBA::Object::_nil();

  // reply status
  switch (rc) {
  case GIOP::UNKNOWN_OBJECT:
  case GIOP::OBJECT_HERE:
    break;
  case GIOP::OBJECT_FORWARD_PERM:
    rc = GIOP::OBJECT_FORWARD;
    // falls through
  case GIOP::OBJECT_FORWARD:
    extra = obj;
    break;
  default:
    // None of these status types are supported.
    rc = GIOP::UNKNOWN_OBJECT;
    break;
  }

  operator>>= ((CORBA::ULong)rc ,s);

  if (!CORBA::is_nil(extra)) {

    // Compute and initialise the message size field
    {
      CORBA::ULong totalsz = (omni::ptr_arith_t)g->pd_outb_mkr - 
           	             (omni::ptr_arith_t)hdr;

      cdrCountingStream cs(g->TCS_C(),g->TCS_W(),totalsz);
      CORBA::Object::_marshalObjRef(extra,cs);
      outputSetMessageSize(g,cs.total()-12);
      *((CORBA::ULong*)(hdr + 8)) = cs.total() - 12;
    }

    // object reference
    CORBA::Object::_marshalObjRef(extra,s);
  }
  outputMessageEnd(g);
}

////////////////////////////////////////////////////////////////////////
size_t
giopImpl10::outputRemaining(const giopStream* g) {
  CORBA::ULong total = g->outputMessageSize();
  if (!total) {
    return orbParameters::giopMaxMsgSize - currentOutputPtr(g);
  }
  else {
    return total - currentOutputPtr(g);
  }
}

////////////////////////////////////////////////////////////////////////
void
giopImpl10::outputFlush(giopStream* g) {

  omni::ptr_arith_t outbuf_begin = ((omni::ptr_arith_t) 
				    g->pd_currentOutputBuffer + 
				    g->pd_currentOutputBuffer->start);

  CORBA::ULong fsz =   (omni::ptr_arith_t) g->pd_outb_mkr - outbuf_begin;

  if (!g->outputMessageSize()) {

    char* hdr = (char*)outbuf_begin;

    CORBA::ULong msgsz = *((CORBA::ULong*)(hdr + 8));
    // the header size including the request/reply header is stored in the 
    // GIOP header's size field.

    if (hdr[7] == (char) GIOP::Request) {

      GIOP_C& giop_c = *(GIOP_C*)g;

      cdrCountingStream cs(g->TCS_C(),g->TCS_W(),msgsz);
      
      giop_c.calldescriptor()->marshalArguments(cs);

      msgsz = cs.total() - 12;
    }
    else if (hdr[7] == (char) GIOP::Reply) {

      GIOP_S& giop_s = *(GIOP_S*)g;

      cdrCountingStream cs(g->TCS_C(),g->TCS_W(),msgsz);
      
      giop_s.calldescriptor()->marshalReturnedValues(cs);

      msgsz = cs.total() - 12;
    }
    else {
      // Any other message type should never caused this function to be called.
      if( omniORB::trace(1) ) {
	omniORB::logger l;
	l << "Fatal error in sending message to "
	  << g->pd_strand->connection->peeraddress()
	  << ", invariant was violated at "
	  << __FILE__ 	
	  << ":" << __LINE__ << '\n';
      }
      OMNIORB_ASSERT(0);
      // never reach here.
    }
    *((CORBA::ULong*)(hdr + 8)) = msgsz;
    outputSetMessageSize(g,msgsz);
  }
  if (g->outputFragmentSize()) {
    g->outputFragmentSize(g->outputFragmentSize()+fsz);
  }
  else {
    g->outputFragmentSize(fsz - 12);
  }
  g->pd_currentOutputBuffer->last = g->pd_currentOutputBuffer->start + fsz;
  g->sendChunk(g->pd_currentOutputBuffer);

  if (outbuf_begin & 0x7) {
    // start has previously been changed to non 8-bytes aligned
    g->pd_currentOutputBuffer->alignStart(omni::ALIGN_8);
    outbuf_begin = ((omni::ptr_arith_t) g->pd_currentOutputBuffer + 
		                        g->pd_currentOutputBuffer->start);
  }

  g->pd_outb_mkr = (void*) outbuf_begin;
  g->pd_outb_end = (void*)((omni::ptr_arith_t)g->pd_currentOutputBuffer + 
			   g->pd_currentOutputBuffer->end);
  g->pd_currentOutputBuffer->last = g->pd_currentOutputBuffer->start;
}

////////////////////////////////////////////////////////////////////////
void
giopImpl10::getReserveSpace(giopStream* g,omni::alignment_t align,size_t sz) {

  // The caller has already checked that align == sz, or sz == 0.

  g->pd_outb_mkr = (void*) omni::align_to((omni::ptr_arith_t)g->pd_outb_mkr,
					  align);

  if (sz == 0)
    return;

  if ((omni::ptr_arith_t)g->pd_outb_mkr < (omni::ptr_arith_t)g->pd_outb_end) {
    omni::ptr_arith_t newmkr = ((omni::ptr_arith_t)g->pd_outb_mkr + sz); 
    if (newmkr <= (omni::ptr_arith_t)g->pd_outb_end)
      return;
    // Should never happen!!
    OMNIORB_ASSERT(0);
  }

  // Reach here only if the buffer has been filled up completely.
  outputFlush(g);
}

////////////////////////////////////////////////////////////////////////
void
giopImpl10::copyOutputData(giopStream* g,void* b, size_t sz,
			   omni::alignment_t align) {

  omni::ptr_arith_t newmkr = omni::align_to((omni::ptr_arith_t)g->pd_outb_mkr,
					    align);
  OMNIORB_ASSERT(newmkr <= (omni::ptr_arith_t)g->pd_outb_end);

  if (sz >= giopStream::directSendCutOff) {

    g->pd_outb_mkr = (void*)newmkr;
    if (  (omni::ptr_arith_t)g->pd_outb_mkr !=
	 ((omni::ptr_arith_t)g->pd_currentOutputBuffer + 
	                     g->pd_currentOutputBuffer->start) ) {
      outputFlush(g);
    }
    // After this vector of bytes is sent, the stream may or may not be
    // 8 bytes aligned. But our output buffer is now emptied and hence
    // is 8 bytes aligned. Since we send the whole vector out, we have
    // to make sure that the next byte will be marshalled at the correct
    // alignment by adjusting the start of the currentOutputBuffer.
    g->sendCopyChunk(b,sz);

    size_t leftover = (newmkr + sz) & 0x7;
    if (leftover) {
      g->pd_currentOutputBuffer->start += leftover;
      g->pd_outb_mkr = (void*) ((omni::ptr_arith_t) 
				g->pd_currentOutputBuffer + 
				g->pd_currentOutputBuffer->start);
    }
  }
  else {
    g->pd_outb_mkr = (void*)newmkr;
    while (sz) {
      size_t avail = (omni::ptr_arith_t) g->pd_outb_end	- 
	             (omni::ptr_arith_t) g->pd_outb_mkr;
      if (avail > sz) avail = sz;
      memcpy(g->pd_outb_mkr,b,avail);
      sz -= avail;
      g->pd_outb_mkr = (void*)((omni::ptr_arith_t) g->pd_outb_mkr + avail);
      b = (void*)((omni::ptr_arith_t) b + avail);
      if (g->pd_outb_mkr == g->pd_outb_end)
	outputFlush(g);
    }
  }
}

////////////////////////////////////////////////////////////////////////
CORBA::ULong
giopImpl10::currentOutputPtr(const giopStream* g) {

  CORBA::ULong fsz = (omni::ptr_arith_t) g->pd_outb_mkr - 
                     ((omni::ptr_arith_t) g->pd_currentOutputBuffer + 
		      g->pd_currentOutputBuffer->start);

  if (g->outputFragmentSize()) {
    return fsz + g->outputFragmentSize();
  }
  else {
    // subtract the header that is still in the buffer.
    return fsz - 12;
  }
}

////////////////////////////////////////////////////////////////////////
void
giopImpl10::outputSetMessageSize(giopStream* g,CORBA::ULong msz) {

  if (msz > orbParameters::giopMaxMsgSize) {
    char* hdr = (char*)((omni::ptr_arith_t) g->pd_currentOutputBuffer + 
			g->pd_currentOutputBuffer->start);

    switch ((GIOP::MsgType)hdr[7]) {
    case GIOP::Request:
    case GIOP::LocateRequest:
      {
	// We have detected a limit error on the client side, since
	// we have not sent any part of the message yet, we can
	// safely relinquish this giopStream so that another request
	// can proceed. There is no need to close the connection.
	((GIOP_C*)g)->state(IOP_C::Idle);
	omni_tracedmutex_lock sync(*omniTransportLock);
	g->wrUnLock();
	OMNIORB_THROW(MARSHAL,MARSHAL_MessageSizeExceedLimitOnClient,
		      (CORBA::CompletionStatus)g->completion());
      }
      break;
    case GIOP::Reply:
    case GIOP::LocateReply:
      {
	// We have detected a limit error on the server side
	OMNIORB_THROW(MARSHAL,MARSHAL_MessageSizeExceedLimitOnServer,
		      (CORBA::CompletionStatus)g->completion());
      }
      break;
    default:
      OMNIORB_ASSERT(0);
    }
  }
  g->outputMessageSize(msz);
}


////////////////////////////////////////////////////////////////////////
static giopStreamImpl* giop_1_0_singleton = 0;

class omni_giopImpl10_initialiser : public omniInitialiser {
public:

  void attach() {
    if (!giop_1_0_singleton) {
      GIOP::Version ver = { 1, 0};
      giopStreamImpl* p;
      giop_1_0_singleton = p = new giopStreamImpl(ver);
      
      // Shared by the client and server side
      // Process message header
      p->outputMessageBegin             = giopImpl10::outputMessageBegin;
      p->outputMessageEnd               = giopImpl10::outputMessageEnd;
      p->inputMessageBegin              = giopImpl10::inputMessageBegin;
      p->inputMessageEnd                = giopImpl10::inputMessageEnd;
      p->sendMsgErrorMessage            = giopImpl10::sendMsgErrorMessage;

      // Client side
      // Process message header
      p->marshalRequestHeader           = giopImpl10::marshalRequestHeader;
      p->sendLocateRequest              = giopImpl10::sendLocateRequest;
      p->unmarshalReplyHeader           = giopImpl10::unmarshalReplyHeader;
      p->unmarshalLocateReply           = giopImpl10::unmarshalLocateReply;

      // Server side
      // Process message header
      p->unmarshalWildCardRequestHeader = giopImpl10::unmarshalWildCardRequestHeader;
      p->unmarshalRequestHeader         = giopImpl10::unmarshalRequestHeader;
      p->unmarshalLocateRequest         = giopImpl10::unmarshalLocateRequest;
      p->marshalReplyHeader             = giopImpl10::marshalReplyHeader;
      p->sendSystemException            = giopImpl10::sendSystemException;
      p->sendUserException              = giopImpl10::sendUserException;
      p->sendLocationForwardReply       = giopImpl10::sendLocationForwardReply;
      p->sendLocateReply                = giopImpl10::sendLocateReply;


      // Shared by the client and the server side
      // Process message body
      p->inputRemaining                 = giopImpl10::inputRemaining;
      p->getInputData                   = giopImpl10::getInputData;
      p->skipInputData                  = giopImpl10::skipInputData;
      p->copyInputData                  = giopImpl10::copyInputData;
      p->outputRemaining                = giopImpl10::outputRemaining;
      p->getReserveSpace                = giopImpl10::getReserveSpace;
      p->copyOutputData                 = giopImpl10::copyOutputData;
      p->currentInputPtr                = giopImpl10::currentInputPtr;
      p->currentOutputPtr               = giopImpl10::currentOutputPtr;

      giopStreamImpl::registerImpl(giop_1_0_singleton);
    }
  }

  void detach() { 
    if (giop_1_0_singleton) {
      delete giop_1_0_singleton;
      giop_1_0_singleton = 0;
    }
  }
};

static omni_giopImpl10_initialiser initialiser;

omniInitialiser& omni_giopImpl10_initialiser_ = initialiser;

OMNI_NAMESPACE_END(omni)
