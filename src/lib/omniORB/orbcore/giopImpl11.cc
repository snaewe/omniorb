// -*- Mode: C++; -*-
//                            Package   : omniORB2
// giopImpl11.cc              Created on: 14/02/2001
//                            Author    : Sai Lai Lo (sll)
//
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
  Revision 1.1.4.2  2001/05/01 16:07:32  sll
  All GIOP implementations should now work with fragmentation and abitrary
  sizes non-copy transfer.

  Revision 1.1.4.1  2001/04/18 18:10:50  sll
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

OMNI_NAMESPACE_BEGIN(omni)

class nonexistence;

class giopImpl11 {
public:
  static void outputMessageBegin(giopStream*,
				 void (*marshalHeader)(giopStream*));
  static void outputMessageEnd(giopStream*);
  static void inputMessageBegin(giopStream*,
				void (*unmarshalHeader)(giopStream*));
  static void inputMessageEnd(giopStream*,CORBA::Boolean disgard = 0);
  static void sendMsgErrorMessage(giopStream*);
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

  static void inputNewFragment(giopStream* g);

  static CORBA::Boolean inputReplyBegin(giopStream*, 
					void (*unmarshalHeader)(giopStream*));

  static void inputSkipWholeMessage(giopStream* g);

  static void inputTerminalProtocolError(giopStream* g);
  // Helper function.  Call this function to indicate that a protocol
  // voilation was detected.  This function *always* raise a
  // giopStream::CommFailure exception.  Therefore the caller should not
  // expect this function to return.

  static void outputNewMessage(giopStream* g);

  static void outputFlush(giopStream* g,CORBA::Boolean knownFragmentSize=0);

  static void outputSetFragmentSize(giopStream*,CORBA::ULong);

  static CORBA::Boolean outputHasReachedLimit(giopStream* g);

private:
  giopImpl11();
  giopImpl11(const giopImpl11&);
  giopImpl11& operator=(const giopImpl11&);
};


////////////////////////////////////////////////////////////////////////
void
giopImpl11::inputMessageBegin(giopStream* g,
			      void (*unmarshalHeader)(giopStream*)) {

 again:
  {
    omni_tracedmutex_lock sync(*omniTransportLock);

    while (!(g->inputFullyBuffered() || g->pd_rdlocked)) {
      if (giopStream::rdLockNonBlocking(g->pd_strand)) {
	g->markRdLock();
      }
      else {
	giopStream::sleepOnRdLock(g->pd_strand);
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
      g->pd_currentInputBuffer = g->inputMessage(0,0);
      // XXX timeout value not set.
    }
  }

  char* hdr = (char*)g->pd_currentInputBuffer + 
                     g->pd_currentInputBuffer->start;

  if (hdr[4] != 1 || hdr[5] != 1) {

    if (hdr[5] == 0 && unmarshalHeader == unmarshalWildCardRequestHeader) {
      // This is a GIOP 1.0 message, switch to the implementation of giop 1.0
      // and dispatch again.
      GIOP::Version v = { 1, 0 };
      g->impl(giopStreamImpl::matchVersion(v));
      OMNIORB_ASSERT(g->impl());
      g->impl()->inputMessageBegin(g,g->impl()->unmarshalWildCardRequestHeader);
      return;
    }
    inputTerminalProtocolError(g);
    // never reaches here.
  }

  g->pd_unmarshal_byte_swap = (((hdr[6] & 0x1) == _OMNIORB_HOST_BYTE_ORDER_)
			       ? 0 : 1 );

  g->pd_inb_mkr = (void*)(hdr + 12);
  g->pd_inb_end = (void*)((omni::ptr_arith_t)g->pd_currentInputBuffer + 
			  g->pd_currentInputBuffer->last);


  g->inputExpectAnotherFragment(((hdr[6] & 0x2) ? 1 : 0));
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
giopImpl11::inputReplyBegin(giopStream* g, 
			    void (*unmarshalHeader)(giopStream*)) {

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
      g->notifyCommFailure(minor,retry);
      g->pd_strand->state(giopStrand::DYING);
      giopStream::CommFailure::_raise(minor,
				      (CORBA::CompletionStatus)g->completion(),
				      retry,__FILE__,__LINE__);
      // never reach here.
      break;
    }
  default:
    inputTerminalProtocolError(g);
    // never reaches here.
  }

  GIOP_C* source = (GIOP_C*) g;

  if (source->replyId() == source->requestId()) {

    if (mtype == GIOP::LocateReply && 
	unmarshalHeader != unmarshalLocateReply) {

      inputTerminalProtocolError(g);
      // never reach here
    }
    else if (mtype == GIOP::Reply &&
	     unmarshalHeader != unmarshalReplyHeader) {

      inputTerminalProtocolError(g);
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
	  inputTerminalProtocolError(g);
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
    inputSkipWholeMessage(g);
    return 0;
  }
}

////////////////////////////////////////////////////////////////////////
void
giopImpl11::inputSkipWholeMessage(giopStream* g) {

  giopStream_Buffer* p = g->pd_input;
  while (p) {
    giopStream_Buffer* q = p->next;
    giopStream_Buffer::deleteBuffer(p);
    p = q;
  }
  g->pd_input = 0;

  do {
    if (g->pd_currentInputBuffer) {
      giopStream_Buffer::deleteBuffer(g->pd_currentInputBuffer);
      g->pd_currentInputBuffer = 0;
    }
    while (g->inputFragmentToCome()) {
      giopStream_Buffer* dummy= g->inputChunk(g->inputFragmentToCome(),
						    0,0);
      // XXX no deadline set.	
      g->inputFragmentToCome( g->inputFragmentToCome() -
			      (dummy->last - dummy->start) );
      giopStream_Buffer::deleteBuffer(dummy);
    }

    if (g->inputExpectAnotherFragment()) {
      inputNewFragment(g);
    }
    else {
      break;
    }
  } while (1);
  g->pd_inb_mkr = g->pd_inb_end;
}

////////////////////////////////////////////////////////////////////////
void
giopImpl11::inputMessageEnd(giopStream* g,CORBA::Boolean disgard) {

  if ( g->pd_strand->state() != giopStrand::DYING ) {

    while ( g->inputExpectAnotherFragment() &&
	    g->inputFragmentToCome() == 0   && 
	    g->pd_inb_end == g->pd_inb_mkr     ) {

      // If there are more fragments to come and we do not have any
      // data left in our buffer, we keep fetching the next
      // fragment until one of the conditions is false.
      // This will cater for the case where the remote end is sending
      // the last fragment(s) with 0 body size to indicate the end of
      // a message.
      inputNewFragment(g);
    }

    if (!disgard && inputRemaining(g)) {
      if (omniORB::trace(15)) {
	omniORB::logger l;
	l << "Garbage left at the end of input message from "
	  << g->pd_strand->connection->peeraddress() << "\n";
      }
      if (!omniORB::strictIIOP) {
	disgard = 1;
      }
      else {
	inputTerminalProtocolError(g);
	// never reach here.
      }
    }

    if (disgard)
      inputSkipWholeMessage(g);

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
giopImpl11::unmarshalReplyHeader(giopStream* g) {

  char* hdr = (char*)g->pd_currentInputBuffer + 
                     g->pd_currentInputBuffer->start;

  if ((GIOP::MsgType)hdr[7] != GIOP::Reply) {
    // Unexpected reply. The other end is terribly confused. Drop the
    // connection and died.
    inputTerminalProtocolError(g);
    // Never reach here.
  }

  GIOP_C& giop_c = *((GIOP_C*) g);
  cdrStream& s = *((cdrStream*)g);

  // Service context
  CORBA::ULong svcccount;
  CORBA::ULong svcctag;
  CORBA::ULong svcctxtsize;
  svcccount <<= s;
  while (svcccount-- > 0) {
    svcctag <<= s;
    svcctxtsize <<= s;
    s.skipInput(svcctxtsize);
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
    inputTerminalProtocolError(g);
    // Never reach here.
    break;
  }
  giop_c.replyStatus((GIOP::ReplyStatusType)v);

}

////////////////////////////////////////////////////////////////////////
void
giopImpl11::unmarshalLocateReply(giopStream* g) {

  char* hdr = (char*)g->pd_currentInputBuffer + 
                     g->pd_currentInputBuffer->start;

  if ((GIOP::MsgType)hdr[7] != GIOP::LocateReply) {
    // Unexpected reply. The other end is terribly confused. Drop the
    // connection and died.
    inputTerminalProtocolError(g);
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
    inputTerminalProtocolError(g);
    // Never reach here.
    break;
  }
  giop_c.locateStatus((GIOP::LocateStatusType)v);
}

////////////////////////////////////////////////////////////////////////
void
giopImpl11::unmarshalWildCardRequestHeader(giopStream* g) {

  g->inputMatchedId(1);

  char* hdr = (char*)g->pd_currentInputBuffer + 
                     g->pd_currentInputBuffer->start;

  ((GIOP_S*)g)->requestType((GIOP::MsgType)hdr[7]);
  switch (((GIOP_S*)g)->requestType()) {
  case GIOP::Request:
  case GIOP::LocateRequest:
  case GIOP::CancelRequest:
    break;
  default:
    inputTerminalProtocolError(g);
    // Never reach here.
    break;
  }
}

////////////////////////////////////////////////////////////////////////
void
giopImpl11::unmarshalRequestHeader(giopStream* g) {

  GIOP_S& giop_s = *((GIOP_S*) g);
  cdrStream& s = *((cdrStream*)g);
  
  giop_s.receive_service_contexts() <<= s;

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
giopImpl11::unmarshalLocateRequest(giopStream* g) {

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
giopImpl11::inputRemaining(giopStream* g) {

  if (g->inputExpectAnotherFragment()) {
    return omniORB::MaxMessageSize() - currentInputPtr(g);
  }
  else {
    return (g->inputFragmentToCome() + ((omni::ptr_arith_t)g->pd_inb_end -
					(omni::ptr_arith_t)g->pd_inb_mkr));
  }
}

////////////////////////////////////////////////////////////////////////
void
giopImpl11::inputNewFragment(giopStream* g) {

  if (g->inputMatchedId()) {

    g->releaseInputBuffer(g->pd_currentInputBuffer);
    g->pd_currentInputBuffer = 0;

    if (!g->pd_input) {
      g->pd_currentInputBuffer = g->inputMessage(0,0);
      // XXX no deadline set.
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
    g->pd_currentInputBuffer = g->inputMessage(0,0);
    // XXX no deadline set.
  }

  char* hdr = (char*)g->pd_currentInputBuffer + 
                     g->pd_currentInputBuffer->start;

  if (hdr[4] != 1 || hdr[5] != 1) {
    inputTerminalProtocolError(g);
    // never reach here.
  }
      
  CORBA::Boolean bswap = (((hdr[6] & 0x1) == _OMNIORB_HOST_BYTE_ORDER_)
			  ? 0 : 1 );

  if (hdr[7] != (char)GIOP::Fragment || bswap != g->pd_unmarshal_byte_swap) {
    inputTerminalProtocolError(g);
    // never reach here
  }

  g->pd_inb_mkr = (void*)(hdr + 12);
  g->pd_inb_end = (void*)((omni::ptr_arith_t)g->pd_currentInputBuffer + 
			  g->pd_currentInputBuffer->last);
  g->inputExpectAnotherFragment(((hdr[6] & 0x2) ? 1 : 0));
  g->inputMessageSize(g->inputMessageSize() + 
		      g->pd_currentInputBuffer->size - 12);
  g->inputFragmentToCome(g->pd_currentInputBuffer->size - 
			 (g->pd_currentInputBuffer->last -
			  g->pd_currentInputBuffer->start));
}

////////////////////////////////////////////////////////////////////////
void
giopImpl11::getInputData(giopStream* g,omni::alignment_t align,size_t sz) {

 again:
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
      if ( !(g->inputFragmentToCome() || g->inputExpectAnotherFragment()) ) {
	// The full message is already in the buffer. The unmarshalling
	// code is asking for more. This is an error causes by the received
	// data. We'll let the code below to raise a MARSHAL exception
	sz = extra;            // in case sz == 0
      }
      else {

	if (g->inputExpectAnotherFragment()) {
	  // The incoming message is fragmented at the wrong boundary!!!
	  inputTerminalProtocolError(g);
	  // never reach here
	}
	// Very bad. Should never happen given our invariant.
	{
	  if( omniORB::trace(0) ) {
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

  if (g->inputFragmentToCome() < sz && !g->inputExpectAnotherFragment()) {
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

  if (!g->inputFragmentToCome()) {
    inputNewFragment(g);
    goto again;
  }

  // Reach here if we have some bytes to fetch for the current fragment

  if (g->inputMatchedId()) {
    g->releaseInputBuffer(g->pd_currentInputBuffer);
    g->pd_currentInputBuffer = 0;
    if (!g->pd_input) {
      g->pd_currentInputBuffer = g->inputChunk(g->inputFragmentToCome(),
					       0,0);
      // XXX no deadline set.
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
    g->pd_currentInputBuffer = g->inputChunk(g->inputFragmentToCome(),
					     0,0);
    // XXX no deadline set.
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
giopImpl11::skipInputData(giopStream* g,size_t sz) {
  copyInputData(g,0,sz,omni::ALIGN_1);
}

////////////////////////////////////////////////////////////////////////
void
giopImpl11::copyInputData(giopStream* g,void* b, size_t sz,
			  omni::alignment_t align) {

  // If b == 0, we don't actually copy the data but just skip <sz> bytes.

  omni::ptr_arith_t last = omni::align_to((omni::ptr_arith_t)g->pd_inb_mkr,
					  align);

  if ( last > (omni::ptr_arith_t) g->pd_inb_end ) {
    // Invariant check 
    // Either the full message is already in the buffer or the part that is
    // in the buffer always ends at an 8 byte aligned boundary.
    if ( !(g->inputFragmentToCome() || g->inputExpectAnotherFragment()) ) {
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

      if (g->inputExpectAnotherFragment()) {
	// The incoming message is fragmented at the wrong boundary!!!
	inputTerminalProtocolError(g);
	// never reach here
      }
      // Very bad. Should never happen given our invariant.
      {
	if( omniORB::trace(0) ) {
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

      if (g->inputFragmentToCome() < sz && !g->inputExpectAnotherFragment()) {
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

      if (!g->inputFragmentToCome()) {
	inputNewFragment(g);
	continue;
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
	  
	    CORBA::ULong transz = (sz >> 3) << 3;
	    g->inputCopyChunk(b,transz,0,0);
	    // XXX no deadline set
	    sz -= transz;
	    b = (void*)((omni::ptr_arith_t)b + transz);
	    continue;
	  }
	  else {
	    g->pd_currentInputBuffer = g->inputChunk(g->inputFragmentToCome(),
						     0,0);
	    // XXX no deadline set.
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
	  g->pd_currentInputBuffer = g->inputChunk(g->inputFragmentToCome(),
						   0,0);
	  // XXX no deadline set.
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
giopImpl11::currentInputPtr(const giopStream* g) {

  return  g->inputMessageSize() - 
          g->inputFragmentToCome() -
         ((omni::ptr_arith_t) g->pd_inb_end - 
	  (omni::ptr_arith_t) g->pd_inb_mkr);
}

////////////////////////////////////////////////////////////////////////
void
giopImpl11::inputTerminalProtocolError(giopStream* g) {

  // XXX We may choose to send a message error to the other end.
  if (omniORB::trace(1)) {
    omniORB::logger l;
    l << "From endpoint: " << g->pd_strand->connection->peeraddress()
      <<". Detected GIOP 1.1 protocol error in input message. "
      << "Connection is closed.\n";
  }

  CORBA::ULong minor;
  CORBA::Boolean retry;
  g->notifyCommFailure(minor,retry);
  g->pd_strand->state(giopStrand::DYING);
  giopStream::CommFailure::_raise(minor,
				  (CORBA::CompletionStatus)g->completion(),
				  0,__FILE__,__LINE__);
}

////////////////////////////////////////////////////////////////////////
void
giopImpl11::outputNewMessage(giopStream* g) {

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
  hdr[4] = 1;   hdr[5] = 1;   hdr[6] = _OMNIORB_HOST_BYTE_ORDER_;

  g->pd_outb_mkr = (void*)(hdr + 12);
  g->pd_outb_end = (void*)((omni::ptr_arith_t)g->pd_currentOutputBuffer + 
			   g->pd_currentOutputBuffer->end);
  g->outputFragmentSize(0);
  g->outputMessageSize(0);
}

////////////////////////////////////////////////////////////////////////
void
giopImpl11::outputMessageBegin(giopStream* g,
			       void (*marshalHeader)(giopStream*)) {
  outputNewMessage(g);
  marshalHeader(g);
}

////////////////////////////////////////////////////////////////////////
void
giopImpl11::outputMessageEnd(giopStream* g) {

  if (g->pd_currentOutputBuffer) {

    omni::ptr_arith_t outbuf_begin = ((omni::ptr_arith_t) 
				    g->pd_currentOutputBuffer + 
				    g->pd_currentOutputBuffer->start);

    if ( outbuf_begin != (omni::ptr_arith_t)g->pd_outb_mkr ) {

      if (!g->outputFragmentSize()) {

	CORBA::ULong sz = (omni::ptr_arith_t)g->pd_outb_mkr - outbuf_begin -12;
	*((CORBA::ULong*)(outbuf_begin + 8)) = sz;
	g->outputMessageSize(g->outputMessageSize()+sz);
      }

      g->pd_currentOutputBuffer->last = (omni::ptr_arith_t) g->pd_outb_mkr - 
					(omni::ptr_arith_t) 
                                        g->pd_currentOutputBuffer;

      g->sendChunk(g->pd_currentOutputBuffer,0,0);
      // XXX deadline not set
    
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
giopImpl11::sendMsgErrorMessage(giopStream* g) {

  if (!g->pd_wrlocked) {
    omni_tracedmutex_lock sync(*omniTransportLock);
    g->wrLock();
  }

  if (omniORB::trace(1)) {
    omniORB::logger l;
    l << "To endpoint: " << g->pd_strand->connection->peeraddress()
      <<". Send GIOP 1.1 ErrorMessage because a protocol error has been detected. "
      << "Connection is closed.\n";
  }

  if (!g->pd_currentOutputBuffer) {
    g->pd_currentOutputBuffer = giopStream_Buffer::newBuffer();
  }
  g->pd_currentOutputBuffer->alignStart(omni::ALIGN_8);

  char* hdr = (char*)g->pd_currentOutputBuffer + 
                     g->pd_currentOutputBuffer->start;

  hdr[0] = 'G'; hdr[1] = 'I'; hdr[2] = 'O'; hdr[3] = 'P';
  hdr[4] = 1;   hdr[5] = 1;   hdr[6] = _OMNIORB_HOST_BYTE_ORDER_;

  g->pd_outb_mkr = (void*)(hdr + 12);
  g->pd_outb_end = (void*)((omni::ptr_arith_t)g->pd_currentOutputBuffer + 
			   g->pd_currentOutputBuffer->end);
  g->outputFragmentSize(0);
  g->outputMessageSize(0);

  hdr[7] = (char)GIOP::MessageError;
  hdr[8] = hdr[9] = hdr[10] = hdr[11] = 0;

  (void)  g->pd_strand->connection->send(hdr,12, 0, 0);
  // XXX no deadline set.

  g->pd_strand->state(giopStrand::DYING);

  {
    omni_tracedmutex_lock sync(*omniTransportLock);
    g->wrUnLock();
  }
}

////////////////////////////////////////////////////////////////////////
void
giopImpl11::marshalRequestHeader(giopStream* g) {

  char* hdr = (char*) g->pd_currentOutputBuffer + 
                      g->pd_currentOutputBuffer->start;

  hdr[7] = (char) GIOP::Request;

  GIOP_C& giop_c = *(GIOP_C*)g;
  cdrStream& s = (cdrStream&) *g;
  omniCallDescriptor& calldesc = *giop_c.calldescriptor();
  CORBA::Boolean response_expected = !calldesc.is_oneway();

  omniInterceptors::clientSendRequest_T::info_T info(*g,
				                     *(giop_c.ior()),
						     calldesc.op(),
						     response_expected,
						     !response_expected);

  omniORB::getInterceptors()->clientSendRequest.visit(info);

  // service context
  info.service_contexts >>= s;

  // request id
  giop_c.requestId() >>= s;

  // response expected flag
  s.marshalBoolean(response_expected);

  // object key
  giop_c.keysize() >>= s;
  s.put_octet_array(giop_c.key(),giop_c.keysize());

  // operation
  operator>>= ((CORBA::ULong) calldesc.op_len(),s);
  s.put_octet_array((CORBA::Octet*) calldesc.op(), calldesc.op_len());

  // principal
  operator>>= ((CORBA::ULong)0,s);
}

////////////////////////////////////////////////////////////////////////
void
giopImpl11::sendLocateRequest(giopStream* g) {

  outputNewMessage(g);

  char* hdr = (char*) g->pd_currentOutputBuffer + 
                      g->pd_currentOutputBuffer->start;

  hdr[7] = (char)GIOP::LocateRequest;

  GIOP_C& giop_c = *(GIOP_C*)g;
  cdrStream& s = (cdrStream&) *g;

  // Compute and initialise the message size field
  {
    cdrCountingStream cs(g->TCS_C(),g->TCS_W(),12);
    operator>>= ((CORBA::ULong)0,cs);
    giop_c.keysize() >>= cs;
    cs.put_octet_array(giop_c.key(),giop_c.keysize());
    outputSetFragmentSize(g,cs.total()-12);
    *((CORBA::ULong*)(hdr + 8)) = cs.total() - 12;
  }

  // request id
  giop_c.requestId() >>= s;
  
  // object key
  giop_c.keysize() >>= s;
  s.put_octet_array(giop_c.key(),giop_c.keysize());

  outputMessageEnd(g);
}

////////////////////////////////////////////////////////////////////////
void
giopImpl11::marshalReplyHeader(giopStream* g) {

  char* hdr = (char*) g->pd_currentOutputBuffer + 
                      g->pd_currentOutputBuffer->start;

  hdr[7] = (char) GIOP::Reply;

  GIOP_S& giop_s = *(GIOP_S*)g;
  cdrStream& s = (cdrStream&) *g;

  CORBA::ULong rc = GIOP::NO_EXCEPTION;

  // Service context
  operator>>= ((CORBA::ULong)0,s);

  // request id
  giop_s.requestId() >>= s;

  // reply status
  rc >>= s;
}

////////////////////////////////////////////////////////////////////////
void
giopImpl11::sendSystemException(giopStream* g,const CORBA::SystemException& ex) {
  GIOP_S& giop_s = *(GIOP_S*)g;
  cdrStream& s = (cdrStream&) *g;

  if (giop_s.state() == GIOP_S::ReplyIsBeingComposed) {
    // This system exception is raised during the marshalling of the reply.
    // We cannot marshal the exception. Can only indicate that something
    // fatal about this request.
    sendMsgErrorMessage(g);

    CORBA::ULong minor;
    CORBA::Boolean retry;
    giop_s.notifyCommFailure(minor,retry);
    giopStream::CommFailure::_raise(minor,(CORBA::CompletionStatus)
				    giop_s.completion(),
				    retry,__FILE__,__LINE__);
  }

  outputNewMessage(g);

  *((char*)g->pd_currentOutputBuffer + 
    g->pd_currentOutputBuffer->start + 7) = (char)GIOP::Reply;


  // Service context
  operator>>= ((CORBA::ULong)0,s);

  // request id
  giop_s.requestId() >>= s;

  // reply status
  CORBA::ULong rc = GIOP::SYSTEM_EXCEPTION;
  rc >>= s;

  // system exception value
  int repoid_size;
  const char* repoid = ex._NP_repoId(&repoid_size);
  CORBA::ULong(repoid_size) >>= s;
  s.put_octet_array((const CORBA::Octet*) repoid, repoid_size);
  ex.minor() >>= s;
  CORBA::ULong(ex.completed()) >>= s;

  outputMessageEnd(g);

}

////////////////////////////////////////////////////////////////////////
void
giopImpl11::sendUserException(giopStream* g,const CORBA::UserException& ex) {

  GIOP_S& giop_s = *(GIOP_S*)g;
  cdrStream& s = (cdrStream&) *g;

  int i, repoid_size;
  const char* repoid = ex._NP_repoId(&repoid_size);

  const char*const* user_exns = giop_s.calldescriptor()->user_excns();
  int n_user_exns = giop_s.calldescriptor()->n_user_excns();

  // Could turn this into a binary search (list is sorted).
  // Usually a short list though -- probably not worth it.
  for( i = 0; i < n_user_exns; i++ )
    if( !strcmp(user_exns[i], repoid) ) {
      break;
    }

  if( i == n_user_exns ) {
    if( omniORB::trace(1) ) {
      omniORB::logger l;
      l << "WARNING -- to "
	<< g->pd_strand->connection->peeraddress()
	<< " method \'" << giop_s.calldescriptor()->op()
	<< "\' on: " << giop_s.keyobj()
	<< "\n raised the exception: " << repoid 
	<< " which is not in the operation signature\n";
    }
    CORBA::UNKNOWN ex(UNKNOWN_UserException, CORBA::COMPLETED_YES);
    sendSystemException(g,ex);
    return;
  }

  outputNewMessage(g);

  char* hdr = (char*)g->pd_currentOutputBuffer + 
                     g->pd_currentOutputBuffer->start;

  hdr[7] = (char)GIOP::Reply;


  // user exception value

  // Compute and initialise the message size field
  {
    cdrCountingStream cs(g->TCS_C(),g->TCS_W(),12);
    operator>>= ((CORBA::ULong)0,cs);
    operator>>= ((CORBA::ULong)0,cs);
    operator>>= ((CORBA::ULong)0,cs);
    CORBA::ULong(repoid_size) >>= cs;
    cs.put_octet_array((const CORBA::Octet*) repoid, repoid_size);
    ex._NP_marshal(cs);

    outputSetFragmentSize(g,cs.total()-12);
    *((CORBA::ULong*)(hdr + 8)) = cs.total() - 12;
  }

  // Service context
  operator>>= ((CORBA::ULong)0,s);

  // request id
  giop_s.requestId() >>= s;

  // reply status
  CORBA::ULong rc = GIOP::USER_EXCEPTION;
  rc >>= s;

  CORBA::ULong(repoid_size) >>= s;
  s.put_octet_array((const CORBA::Octet*) repoid, repoid_size);
  ex._NP_marshal(s);

  outputMessageEnd(g);
}

////////////////////////////////////////////////////////////////////////
void
giopImpl11::sendLocationForwardReply(giopStream* g,CORBA::Object_ptr obj,
				     CORBA::Boolean) {

  outputNewMessage(g);
  
  char* hdr = (char*)g->pd_currentOutputBuffer + 
                     g->pd_currentOutputBuffer->start;

  hdr[7] = (char)GIOP::Reply;

  GIOP_S& giop_s = *(GIOP_S*)g;
  cdrStream& s = (cdrStream&) *g;

  // Compute and initialise the message size field
  {
    cdrCountingStream cs(g->TCS_C(),g->TCS_W(),12);
    operator>>= ((CORBA::ULong)0,cs);
    operator>>= ((CORBA::ULong)0,cs);
    operator>>= ((CORBA::ULong)0,cs);
    CORBA::Object::_marshalObjRef(obj,cs);
    outputSetFragmentSize(g,cs.total()-12);
    *((CORBA::ULong*)(hdr + 8)) = cs.total() - 12;
  }

  // Service context
  operator>>= ((CORBA::ULong)0,s);

  // request id
  giop_s.requestId() >>= s;

  // reply status
  CORBA::ULong rc = GIOP::LOCATION_FORWARD;
  rc >>= s;


  // object reference
  CORBA::Object::_marshalObjRef(obj,s);
  outputMessageEnd(g);
}

////////////////////////////////////////////////////////////////////////
void
giopImpl11::sendLocateReply(giopStream* g,GIOP::LocateStatusType rc,
			    CORBA::Object_ptr obj,CORBA::SystemException* p) {


  outputNewMessage(g);

  char* hdr = (char*)g->pd_currentOutputBuffer + 
                     g->pd_currentOutputBuffer->start;

  hdr[7] = (char)GIOP::LocateReply;

  GIOP_S& giop_s = *(GIOP_S*)g;
  cdrStream& s = (cdrStream&) *g;

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


  // Compute and initialise the message size field
  {
    cdrCountingStream cs(g->TCS_C(),g->TCS_W(),12);
    operator>>= ((CORBA::ULong)0,cs);
    operator>>= ((CORBA::ULong)0,cs);
    if (!CORBA::is_nil(extra)) {
      CORBA::Object::_marshalObjRef(extra,cs);
    }
    outputSetFragmentSize(g,cs.total()-12);
    *((CORBA::ULong*)(hdr + 8)) = cs.total() - 12;
  }


  // request id
  giop_s.requestId() >>= s;

  operator>>= ((CORBA::ULong)rc,s);

  if (!CORBA::is_nil(extra)) {
    CORBA::Object::_marshalObjRef(extra,s);
  }
  outputMessageEnd(g);
}

////////////////////////////////////////////////////////////////////////
size_t
giopImpl11::outputRemaining(const giopStream* g) {

  CORBA::ULong total = g->outputFragmentSize();
  if (!total) {
    CORBA::ULong avail = (CORBA::Long)omniORB::MaxMessageSize() -
                         (CORBA::Long)currentOutputPtr(g);
    
    // Adjust avail to exactly the same value as calculated in outputFlush().
    // See the comment in outputFlush() for the reason why.
    avail = ((avail + 4) >> 3) << 3;
    avail = (avail ? avail - 4 : 0);
    return avail;
  }
  else {
    return total - currentOutputPtr(g);
  }
}

////////////////////////////////////////////////////////////////////////
void
giopImpl11::outputFlush(giopStream* g,CORBA::Boolean knownFragmentSize) {

  // Note: g->outputFragmentSize() == 0 implies that the full message
  //       size has been pre-calculated and no GIOP Fragment should be
  //       sent! This also means that the message size limit has been
  //       checked and there is no need to check against
  //       omniORB::MaxMessageSize().
  //       

  omni::ptr_arith_t outbuf_begin = ((omni::ptr_arith_t) 
				    g->pd_currentOutputBuffer + 
				    g->pd_currentOutputBuffer->start);

  CORBA::ULong fsz =   (omni::ptr_arith_t) g->pd_outb_mkr - outbuf_begin;

  if (!g->outputFragmentSize()) {
    ((char*)outbuf_begin)[6] |= 0x2;
    CORBA::ULong sz;
    if (!knownFragmentSize) {
      sz = fsz - 12;
      *((CORBA::ULong*)((omni::ptr_arith_t)outbuf_begin + 8)) = sz;
    }
    else {
      sz = *((CORBA::ULong*)((omni::ptr_arith_t)outbuf_begin + 8));
    }
    g->outputMessageSize(g->outputMessageSize()+sz);
    if (g->outputMessageSize() > omniORB::MaxMessageSize()) {
      OMNIORB_THROW(MARSHAL,MARSHAL_MessageSizeExceedLimit,
		    (CORBA::CompletionStatus)g->completion());
    }
  }
  else {
    CORBA::Long msz = g->outputMessageSize();
    if (msz) {
      g->outputMessageSize(msz+fsz);
    }
    else {
      g->outputMessageSize(fsz-12);
    }
  }

  g->pd_currentOutputBuffer->last = g->pd_currentOutputBuffer->start + fsz;
  g->sendChunk(g->pd_currentOutputBuffer,0,0);
  // XXX deadline not set

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

  if (!g->outputFragmentSize()) {
    char* hdr = (char*) outbuf_begin;

    hdr[0] = 'G'; hdr[1] = 'I'; hdr[2] = 'O'; hdr[3] = 'P';
    hdr[4] = 1;   hdr[5] = 1;   hdr[6] = _OMNIORB_HOST_BYTE_ORDER_;
    hdr[7] = (char)GIOP::Fragment;
    g->pd_outb_mkr = (void*)(outbuf_begin + 12);
    
    // Now determine how much space we have left.
    // If the message size has already reach omniORB::MaxMessageSize(),
    // outputHasReachedLimit() will return TRUE.
    CORBA::ULong avail = omniORB::MaxMessageSize() - g->outputMessageSize();

    // Adjust avail to make sure that either it is 0 or when the header
    // size (12) is added the result is a multiple of 8.
    // This preserves our invariant: g->pd_outb_end always align on 8 bytes
    // boundary.
    // The value should be one of 0, 4, 12, 20, ....
    avail = ((avail + 4) >> 3) << 3;
    avail = (avail ? avail - 4 : 0);

    omni::ptr_arith_t newmkr = (omni::ptr_arith_t) g->pd_outb_mkr + avail;
    if (newmkr < (omni::ptr_arith_t)g->pd_outb_end) {
      g->pd_outb_end = (void*) newmkr;
    }
  }
}

////////////////////////////////////////////////////////////////////////
void
giopImpl11::getReserveSpace(giopStream* g,omni::alignment_t align,size_t sz) {

  // The caller has already checked that align == sz, or sz == 0.

  if (outputHasReachedLimit(g)) {
    // Already reached the message size limit
    goto overflow;
  }

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
  if (!outputHasReachedLimit(g))
    return;

 overflow:
    OMNIORB_THROW(MARSHAL,MARSHAL_MessageSizeExceedLimit,
		  (CORBA::CompletionStatus)g->completion());
}

////////////////////////////////////////////////////////////////////////
void
giopImpl11::copyOutputData(giopStream* g,void* b, size_t sz,
			   omni::alignment_t align) {

  if (outputHasReachedLimit(g)) {
    // Already reached the message size limit
    OMNIORB_THROW(MARSHAL,MARSHAL_MessageSizeExceedLimit,
		  (CORBA::CompletionStatus)g->completion());
  }

  omni::ptr_arith_t newmkr = omni::align_to((omni::ptr_arith_t)g->pd_outb_mkr,
					    align);
  OMNIORB_ASSERT(newmkr <= (omni::ptr_arith_t)g->pd_outb_end);

  if (sz >= giopStream::directSendCutOff) {

    g->pd_outb_mkr = (void*)newmkr;

    if (!g->outputFragmentSize()) {

      omni::ptr_arith_t outbuf_begin = ((omni::ptr_arith_t) 
					g->pd_currentOutputBuffer + 
					g->pd_currentOutputBuffer->start);

      CORBA::ULong fsz = (omni::ptr_arith_t) g->pd_outb_mkr - 
            	         (omni::ptr_arith_t) outbuf_begin + sz - 12;

      *((CORBA::ULong*)((omni::ptr_arith_t)outbuf_begin + 8)) = fsz;
    }

    outputFlush(g,1);

    g->sendCopyChunk(b,sz,0,0);
    // XXX no deadline set.

    if (g->outputFragmentSize()) {
      size_t leftover = (newmkr + sz) % 0x7;
      if (leftover) {
	g->pd_currentOutputBuffer->start += leftover;
	g->pd_outb_mkr = (void*) ((omni::ptr_arith_t) 
				  g->pd_currentOutputBuffer + 
				  g->pd_currentOutputBuffer->start);
      }
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
      if (g->pd_outb_mkr == g->pd_outb_end) {
	outputFlush(g);
	if (sz && outputHasReachedLimit(g)) {
	  // Already reached the message size limit
	  OMNIORB_THROW(MARSHAL,MARSHAL_MessageSizeExceedLimit,
			(CORBA::CompletionStatus)g->completion());
	}
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////
CORBA::ULong
giopImpl11::currentOutputPtr(const giopStream* g) {

  CORBA::ULong fsz = (omni::ptr_arith_t) g->pd_outb_mkr - 
                     ((omni::ptr_arith_t) g->pd_currentOutputBuffer + 
		      g->pd_currentOutputBuffer->start);

  if (g->outputMessageSize()) {
    return fsz + g->outputMessageSize();
  }
  else {
    return fsz - 12;
  }
}


////////////////////////////////////////////////////////////////////////
void
giopImpl11::outputSetFragmentSize(giopStream* g,CORBA::ULong msz) {

  if (msz > omniORB::MaxMessageSize()) {
    OMNIORB_THROW(MARSHAL,MARSHAL_MessageSizeExceedLimit,
		  (CORBA::CompletionStatus)g->completion());
  }
  g->outputFragmentSize(msz);
}

////////////////////////////////////////////////////////////////////////
CORBA::Boolean
giopImpl11::outputHasReachedLimit(giopStream* g) {

  return ( (omni::ptr_arith_t) g->pd_outb_end == 
          ((omni::ptr_arith_t) g->pd_currentOutputBuffer + 
		  	       g->pd_currentOutputBuffer->start + 12) );
}


////////////////////////////////////////////////////////////////////////
static giopStreamImpl* giop_1_1_singleton = 0;

class omni_giopImpl11_initialiser : public omniInitialiser {
public:

  void attach() {
    if (!giop_1_1_singleton) {
      GIOP::Version ver = { 1, 1};
      giopStreamImpl* p;
      giop_1_1_singleton = p = new giopStreamImpl(ver);
      
      // Shared by the client and server side
      // Process message header
      p->outputMessageBegin             = giopImpl11::outputMessageBegin;
      p->outputMessageEnd               = giopImpl11::outputMessageEnd;
      p->inputMessageBegin              = giopImpl11::inputMessageBegin;
      p->inputMessageEnd                = giopImpl11::inputMessageEnd;
      p->sendMsgErrorMessage            = giopImpl11::sendMsgErrorMessage;

      // Client side
      // Process message header
      p->marshalRequestHeader           = giopImpl11::marshalRequestHeader;
      p->sendLocateRequest              = giopImpl11::sendLocateRequest;
      p->unmarshalReplyHeader           = giopImpl11::unmarshalReplyHeader;
      p->unmarshalLocateReply           = giopImpl11::unmarshalLocateReply;

      // Server side
      // Process message header
      p->unmarshalWildCardRequestHeader = giopImpl11::unmarshalWildCardRequestHeader;
      p->unmarshalRequestHeader         = giopImpl11::unmarshalRequestHeader;
      p->unmarshalLocateRequest         = giopImpl11::unmarshalLocateRequest;
      p->marshalReplyHeader             = giopImpl11::marshalReplyHeader;
      p->sendSystemException            = giopImpl11::sendSystemException;
      p->sendUserException              = giopImpl11::sendUserException;
      p->sendLocationForwardReply       = giopImpl11::sendLocationForwardReply;
      p->sendLocateReply                = giopImpl11::sendLocateReply;


      // Shared by the client and the server side
      // Process message body
      p->inputRemaining                 = giopImpl11::inputRemaining;
      p->getInputData                   = giopImpl11::getInputData;
      p->skipInputData                  = giopImpl11::skipInputData;
      p->copyInputData                  = giopImpl11::copyInputData;
      p->outputRemaining                = giopImpl11::outputRemaining;
      p->getReserveSpace                = giopImpl11::getReserveSpace;
      p->copyOutputData                 = giopImpl11::copyOutputData;
      p->currentInputPtr                = giopImpl11::currentInputPtr;
      p->currentOutputPtr               = giopImpl11::currentOutputPtr;

      giopStreamImpl::registerImpl(giop_1_1_singleton);
    }
  }

  void detach() { 
    if (giop_1_1_singleton) {
      delete giop_1_1_singleton;
      giop_1_1_singleton = 0;
    }
  }
};

static omni_giopImpl11_initialiser initialiser;

omniInitialiser& omni_giopImpl11_initialiser_ = initialiser;

OMNI_NAMESPACE_END(omni)
