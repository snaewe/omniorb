// -*- Mode: C++; -*-
//                            Package   : omniORB2
// giopImpl12.cc              Created on: 14/02/2001
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
  Revision 1.1.2.1  2001/02/23 16:46:58  sll
  Added new files.

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

OMNI_NAMESPACE_BEGIN(omni)

class nonexistence;

class giopImpl12 {
public:
  static void outputMessageBegin(giopStream*,
				 void (*marshalHeader)(giopStream*));
  static void outputMessageEnd(giopStream*);
  static void inputMessageBegin(giopStream*,
				void (*unmarshalHeader)(giopStream*));
  static void inputMessageEnd(giopStream*,CORBA::Boolean disgard = 0);
  static void sendMsgErrorMessage(giopStream*);
  static void marshalRequestHeader(giopStream*);
  static void marshalLocateRequest(giopStream*);
  static void unmarshalReplyHeader(giopStream*);
  static void unmarshalLocateReply(giopStream*);
  static void unmarshalWildCardRequestHeader(giopStream*);
  static void unmarshalRequestHeader(giopStream*);
  static void unmarshalLocateRequest(giopStream*);
  static void marshalReplyHeader(giopStream*);
  static void sendSystemException(giopStream*,const CORBA::SystemException&);
  static void sendUserException(giopStream*,const CORBA::UserException&);
  static void sendLocationForwardReply(giopStream*,CORBA::Object_ptr);
  static void sendLocateReply(giopStream*,GIOP::LocateStatusType,
			      CORBA::Object_ptr,CORBA::SystemException* p = 0);
  static size_t inputRemaining(giopStream*);
  static void getInputData(giopStream*,omni::alignment_t,size_t);
  static size_t max_input_buffer_size(giopStream*,omni::alignment_t);
  static void skipInputData(giopStream*,size_t);
  static void copyInputData(giopStream*,void*, size_t,omni::alignment_t);
  static size_t outputRemaining(const giopStream*);
  static void getReserveSpace(giopStream*,omni::alignment_t,size_t);
  static void copyOutputData(giopStream*,void*, size_t,omni::alignment_t);
  static size_t max_reserve_buffer_size(const giopStream*,omni::alignment_t);
  static CORBA::ULong currentInputPtr(const giopStream*);
  static CORBA::ULong currentOutputPtr(const giopStream*);

  friend class nonexistence;  // Just to make gcc shut up.

  static void inputNewFragment(giopStream* g);

  static void inputNewServerMessage(giopStream* g);

  static void inputQueueMessage(giopStream*,giopStream::inputBuffer*);

  static void inputReplyBegin(giopStream*, 
			      void (*unmarshalHeader)(giopStream*));

  static void inputSkipWholeMessage(giopStream* g);

  static void inputTerminalProtocolError(giopStream* g);
  // Helper function.  Call this function to indicate that a protocol
  // voilation was detected.  This function *always* raise a
  // giopStream::CommFailure exception.  Therefore the caller should not
  // expect this function to return.

private:
  giopImpl12();
  giopImpl12(const giopImpl12&);
  giopImpl12& operator=(const giopImpl12&);
};


////////////////////////////////////////////////////////////////////////
void
giopImpl12::inputQueueMessage(giopStream* g,giopStream::inputBuffer* b) {

  unsigned char* hdr = (unsigned char*)b + b->start;
  GIOP::MsgType mtype = (GIOP::MsgType)hdr[7];

  if (hdr[4] != 1 || hdr[5] != 2 || mtype > GIOP::Fragment) {

    // Notice that we only accept GIOP 1.2 packets here. That is, any
    // GIOP 1.0 or 1.1. messages will be rejected.
    // While the spec. say that a GIOP 1.2 message can interleave with
    // another 1.2 message. It does not say if a GIOP 1.0 or 1.1 message
    // can interleave with a GIOP 1.2 message. Our interpretation is to
    // disallow this.
    inputTerminalProtocolError(g);
    // never reach here
  }


  CORBA::ULong reqid;

  if ( mtype != GIOP::CloseConnection && mtype != GIOP::MessageError) {

    // unmarshal request id.
    reqid = *((CORBA::ULong*)(hdr + 12));
    if ((hdr[6] & 0x1) != _OMNIORB_HOST_BYTE_ORDER_) {
      CORBA::ULong v = reqid;
      reqid = ((((v) & 0xff000000) >> 24) |
	       (((v) & 0x00ff0000) >> 8)  |
	       (((v) & 0x0000ff00) << 8)  |
	       (((v) & 0x000000ff) << 24));
    }
  }
  else if ( mtype == GIOP::MessageError) {
    inputTerminalProtocolError(g);
    // never reach here
  }
  else if ( g->pd_strand->isClient() || g->pd_strand->biDir) {
    // orderly shutdown. What to do?
    inputTerminalProtocolError(g);
    // never reach here
  }
  else {
    inputTerminalProtocolError(g);
    // never reach here
  }

  giopStream*    matched_target = 0;
  CORBA::Boolean matched_target_is_client = 0;

  omniTransportLock->lock();

  // Look at clients
  switch (mtype) {
  case GIOP::Reply:
  case GIOP::LocateReply:
    if (!(g->pd_strand->isClient() || g->pd_strand->biDir)) {
      omniTransportLock->unlock();
      inputTerminalProtocolError(g);
      // never reach here
    }
    // falls through
  case GIOP::Fragment:
    {
      // search for a matching id in clients.

      giopStreamList* gp = g->pd_strand->clients.next;
      for (; gp != &g->pd_strand->clients; gp = gp->next) {

	GIOP_C* target = (GIOP_C*)gp;

	if (target->state() != IOP_C::UnUsed && 
	    target->requestId() == reqid) {

	  // sanity check
	  if (target->inputFullyBuffered()) {
	    // a reply has already been received!
	    omniTransportLock->unlock();
	    inputTerminalProtocolError(g);
	    // never reach here
	  }

	  if (target->inputMatchedId()) {
	    if (mtype != GIOP::Fragment) {
	      // already got the header
	      omniTransportLock->unlock();
	      inputTerminalProtocolError(g);
	      // never reach here
	    }
	  }
	  else if (mtype == GIOP::Fragment) {
	    // receive body before the header
	    omniTransportLock->unlock();
	    inputTerminalProtocolError(g);
	    // never reach here
	  }
	  else {
	    target->inputMatchedId(1);
	  }
	  matched_target = (giopStream*)target;
	  matched_target_is_client = 1;
	  break;
	}
      }
    }
  default:
    break;
  }

  if (!matched_target) {

    // look at servers
    switch (mtype) {
    case GIOP::Request:
    case GIOP::LocateRequest:
      {
	// Make sure this request id has not been seen before
	GIOP_S* unused = 0;
	giopStreamList* gp = g->pd_strand->servers.next;
	for (; gp != &g->pd_strand->servers; gp = gp->next) {

	  GIOP_S* target = (GIOP_S*)gp;
	  if (target->state() == IOP_S::UnUsed) {
	    unused = target;
	  }
	  else if (target->requestId() == reqid) {
	    // already have a request with the same id.
	    omniTransportLock->unlock();
	    inputTerminalProtocolError(g);
	    // never reach here
	  }
	}
	if (!unused) {
	  OMNIORB_ASSERT(g->pd_strand->servers.next != &g->pd_strand->servers);
	  GIOP_S* copy = (GIOP_S*)g->pd_strand->servers.next;
	  unused = new GIOP_S(*copy);
	  unused->giopStreamList::insert(g->pd_strand->servers);
	}
	unused->state(IOP_S::InputPartiallyBuffered);
	unused->requestId(reqid);
      }
    case GIOP::CancelRequest:
      if (g->pd_strand->isClient() && !g->pd_strand->biDir) {
	omniTransportLock->unlock();
	inputTerminalProtocolError(g);
	// never reach here
      }
      // falls through
    case GIOP::Fragment:
      {
	// search for a matching id in servers.
	
	giopStreamList* gp = g->pd_strand->servers.next;
	for (; gp != &g->pd_strand->servers; gp = gp->next) {

	  GIOP_S* target = (GIOP_S*)gp;

	  if (target->state() != IOP_S::UnUsed && 
	      target->requestId() == reqid) {
	    
	    // sanity check
	    if (target->inputFullyBuffered()) {
	      // a reply has already been received!
	      omniTransportLock->unlock();
	      inputTerminalProtocolError(g);
	      // never reach here
	    }

	    matched_target = (giopStream*)target;
	    break;
	  }
	}
      }
    default:
      break;
    }
  }

  omniTransportLock->unlock();

  if (matched_target) {

    if (matched_target == g) {
      OMNIORB_ASSERT(g->pd_input == 0);
      if (g->pd_currentInputBuffer) {
	g->releaseInputBuffer(g->pd_currentInputBuffer);
	g->pd_currentInputBuffer = 0;
      }
      g->pd_input = b;
      return;
    }

    OMNIORB_ASSERT(matched_target->pd_currentInputBuffer == 0);
    giopStream::inputBuffer** pp = &matched_target->pd_input;
    while (*pp) {
      pp = &((*pp)->next);
    }
    *pp = b;

    CORBA::ULong fetchsz = b->size - (b->last - b->start);
    while (fetchsz) {
      // fetch the rest of the message;
      giopStream::inputBuffer* p = g->inputChunk(fetchsz,0,0);
      // XXX no deadline set.
      pp = &((*pp)->next);
      *pp = p;
      fetchsz -= (p->last - p->start);
    }
      
    CORBA::Boolean isfull = ((hdr[6] & 0x2) ? 0 : 1);
    if (mtype == GIOP::CancelRequest) isfull = 1;
    if (isfull) {
      omni_tracedmutex_lock sync(*omniTransportLock);
      matched_target->inputFullyBuffered(isfull);
      if (!matched_target_is_client) {
	((GIOP_S*)matched_target)->state(IOP_S::InputFullyBuffered);
      }
      giopStream::wakeUpRdLock(g->pd_strand);
    }
    return;
  }

  // reach here if we have no match.
  {
    CORBA::ULong fetchsz = b->size - (b->last - b->start);
    giopStream::inputBuffer::deleteBuffer(b);
    while (fetchsz) {
      // fetch the rest of the message;
      b = g->inputChunk(fetchsz,0,0);
      // XXX no deadline set.
      fetchsz -= (b->last - b->start);
      giopStream::inputBuffer::deleteBuffer(b);
    }
  }
}


////////////////////////////////////////////////////////////////////////
void
giopImpl12::inputNewServerMessage(giopStream* g) {

  OMNIORB_ASSERT(g->pd_currentInputBuffer == 0);

 again:

  g->pd_currentInputBuffer = g->inputMessage(0,0);
  // XXX no deadline set

  unsigned char* hdr = (unsigned char*)g->pd_currentInputBuffer + 
                                       g->pd_currentInputBuffer->start;

  if (hdr[4] != 1 || hdr[5] > 2 || hdr[7] > (unsigned char) GIOP::Fragment) {
    inputTerminalProtocolError(g);
    // never reach here
  }

  switch ((GIOP::MsgType)hdr[7]) {
  case GIOP::Request:
  case GIOP::LocateRequest:
  case GIOP::MessageError:
  case GIOP::CloseConnection:
    return;
  case GIOP::Reply:
  case GIOP::LocateReply:
    if (g->pd_strand->biDir) {
      break;
    }
    else {
      inputTerminalProtocolError(g);
      // Never reach here.
    }
  case GIOP::Fragment:
  case GIOP::CancelRequest:
    break;
  }

  // reach here if the message is destine to some other call in progress.
  giopStream::inputBuffer* p = g->pd_currentInputBuffer;
  g->pd_currentInputBuffer = 0;
  inputQueueMessage(g,p);
}


////////////////////////////////////////////////////////////////////////
void
giopImpl12::inputNewFragment(giopStream* g) {

  g->releaseInputBuffer(g->pd_currentInputBuffer);
  g->pd_currentInputBuffer = 0;

 again:
  if (!g->pd_input) {
    giopStream::inputBuffer* p = g->inputMessage(0,0);
    inputQueueMessage(g,p);
    goto again;
  }
  else {
    g->pd_currentInputBuffer = g->pd_input;
    g->pd_input = g->pd_currentInputBuffer->next;
    g->pd_currentInputBuffer->next = 0;
  }

  char* hdr = (char*)g->pd_currentInputBuffer + 
                     g->pd_currentInputBuffer->start;


  if (hdr[7] == GIOP::CancelRequest) {
    if (g->pd_strand->biDir || !g->pd_strand->isClient()) {
      throw GIOP_S::terminateProcessing();
    }
    else {
      inputTerminalProtocolError(g);
      // never reach here.
    }
  }

  CORBA::Boolean bswap = (((hdr[6] & 0x1) == _OMNIORB_HOST_BYTE_ORDER_)
			  ? 0 : 1 );

  if (bswap != g->pd_unmarshal_byte_swap) {
    inputTerminalProtocolError(g);
    // never reach here
  }

  g->pd_inb_mkr = (void*)(hdr + 16);
  g->pd_inb_end = (void*)((omni::ptr_arith_t)g->pd_currentInputBuffer + 
			  g->pd_currentInputBuffer->last);
  g->inputExpectAnotherFragment(((hdr[6] & 0x2) ? 1 : 0));
  g->inputMessageSize(g->inputMessageSize() + 
		      g->pd_currentInputBuffer->size - 16);
  g->inputFragmentToCome(g->pd_currentInputBuffer->size - 
			 (g->pd_currentInputBuffer->last -
			  g->pd_currentInputBuffer->start));
}


////////////////////////////////////////////////////////////////////////
void
giopImpl12::inputReplyBegin(giopStream* g, 
			    void (*unmarshalHeader)(giopStream*)) {

  {
    omni_tracedmutex_lock sync(*omniTransportLock);

    if (!g->pd_strand->biDir) {

      while (!(g->inputFullyBuffered() || g->pd_rdlocked)) {
	if (giopStream::rdLockNonBlocking(g->pd_strand)) {
	  g->markRdLock();
	}
	else {
	  giopStream::sleepOnRdLock(g->pd_strand);
	}
      }
    }
    else {
      // If the strand is used for bidirectional GIOP, we
      // let the server thread to read the incoming message and to
      // demultiplex any reply back to us. Therefore, we do not
      // try to acquire the read lock but just sleep on the read
      // lock instead.
      while (!g->inputFullyBuffered()) {
	OMNIORB_ASSERT(g->pd_rdlocked == 0);
	giopStream::sleepOnRdLock(g->pd_strand,1);
      }
    }
  }

  if (!g->pd_currentInputBuffer) {
  again:
    if (!g->pd_input) {
      giopStream::inputBuffer* p = g->inputMessage(0,0);
      inputQueueMessage(g,p);
      goto again;
    }
    else {
      g->pd_currentInputBuffer = g->pd_input;
      g->pd_input = g->pd_input->next;
      g->pd_currentInputBuffer->next = 0;
    }
  }

  char* hdr = (char*)g->pd_currentInputBuffer + 
                     g->pd_currentInputBuffer->start;

  g->pd_unmarshal_byte_swap = (((hdr[6] & 0x1) == _OMNIORB_HOST_BYTE_ORDER_)
			       ? 0 : 1 );

  g->pd_inb_mkr = (void*)(hdr + 16);
  g->pd_inb_end = (void*)((omni::ptr_arith_t)g->pd_currentInputBuffer + 
			  g->pd_currentInputBuffer->last);


  g->inputExpectAnotherFragment(((hdr[6] & 0x2) ? 1 : 0));
  g->inputMessageSize(g->pd_currentInputBuffer->size);
  g->inputFragmentToCome(g->pd_currentInputBuffer->size - 
			 (g->pd_currentInputBuffer->last -
			  g->pd_currentInputBuffer->start));

  unmarshalHeader(g);
}


////////////////////////////////////////////////////////////////////////
void
giopImpl12::inputMessageBegin(giopStream* g,
			      void (*unmarshalHeader)(giopStream*)) {

  if (unmarshalHeader != unmarshalWildCardRequestHeader) {
    inputReplyBegin(g,unmarshalHeader);
    return;
  }

  {
    omni_tracedmutex_lock sync(*omniTransportLock);

    if (!(g->inputFullyBuffered() || g->pd_rdlocked)) {
      g->rdLock();
    }
  }
 again:
  if (!g->pd_currentInputBuffer) {

    if (g->pd_input) {
      g->pd_currentInputBuffer = g->pd_input;
      g->pd_input = g->pd_input->next;
      g->pd_currentInputBuffer->next = 0;
    }
    else {
      inputNewServerMessage(g);
      goto again;
    }
  }

  char* hdr = (char*)g->pd_currentInputBuffer + 
                     g->pd_currentInputBuffer->start;

  if (hdr[5] <= 1) {
    // This is a GIOP 1.0 or 1.1 message, switch to the implementation
    // and dispatch again.
    GIOP::Version v;
    v.major = 1;
    v.minor = hdr[5];
    g->impl(giopStreamImpl::matchVersion(v));
    OMNIORB_ASSERT(g->impl());
    g->impl()->inputMessageBegin(g,unmarshalHeader);
    return;
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

  unmarshalHeader(g);
}

////////////////////////////////////////////////////////////////////////
void
giopImpl12::inputSkipWholeMessage(giopStream* g) {

  do {
    if (g->pd_currentInputBuffer) {
      giopStream::inputBuffer::deleteBuffer(g->pd_currentInputBuffer);
      g->pd_currentInputBuffer = 0;
    }
    if (g->inputFragmentToCome()) {

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
      g->pd_inb_mkr = (void*)((omni::ptr_arith_t)g->pd_currentInputBuffer + 
			      g->pd_currentInputBuffer->start);
      g->pd_inb_end = (void*)((omni::ptr_arith_t)g->pd_currentInputBuffer + 
			      g->pd_currentInputBuffer->last);
      g->inputFragmentToCome(g->inputFragmentToCome() - 
			     (g->pd_currentInputBuffer->last -
			      g->pd_currentInputBuffer->start));
    }
    else if (g->inputExpectAnotherFragment()) {
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
giopImpl12::inputMessageEnd(giopStream* g,CORBA::Boolean disgard) {

  if ( g->pd_strand->state() != giopStrand::DYING ) {

    if (!disgard && inputRemaining(g)) {
      if (omniORB::trace(15)) {
	omniORB::logger l;
	l << "Garbage left at the end of input message.\n";
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
    g->rdUnLock();
  }
}

////////////////////////////////////////////////////////////////////////
void
giopImpl12::unmarshalReplyHeader(giopStream* g) {

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

  // We have already verified the request id in the header and the stream
  // have been setup to go pass it

  CORBA::ULong v;
  v <<= s;

  switch (v) {
  case GIOP::SYSTEM_EXCEPTION:
  case GIOP::NO_EXCEPTION:
  case GIOP::USER_EXCEPTION:
  case GIOP::LOCATION_FORWARD:
  case GIOP::LOCATION_FORWARD_PERM:
  case GIOP::NEEDS_ADDRESSING_MODE:
    break;
  default:
    // Should never receive anything other that the above
    // Same treatment as wrong header
    inputTerminalProtocolError(g);
    // Never reach here.
  }

  giop_c.replyStatus((GIOP::ReplyStatusType)v);

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

  if (inputRemaining(g))
    s.alignInput(omni::ALIGN_8);
}

////////////////////////////////////////////////////////////////////////
void
giopImpl12::unmarshalLocateReply(giopStream* g) {

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

  // We have already verified the request id in the header and the stream
  // have been setup to go pass it

  CORBA::ULong v;
  v <<= s;

  switch (v) {
  case GIOP::UNKNOWN_OBJECT:
  case GIOP::OBJECT_HERE:
  case GIOP::OBJECT_FORWARD:
  case GIOP::OBJECT_FORWARD_PERM:
  case GIOP::LOC_SYSTEM_EXCEPTION:
  case GIOP::LOC_NEEDS_ADDRESSING_MODE:
    break;
  default:
    // Should never receive anything other that the above
    // Same treatment as wrong header
    inputTerminalProtocolError(g);
    // never reach here.
  }
  giop_c.locateStatus((GIOP::LocateStatusType)v);
}

////////////////////////////////////////////////////////////////////////
void
giopImpl12::unmarshalWildCardRequestHeader(giopStream* g) {

  g->inputMatchedId(1);

  char* hdr = (char*)g->pd_currentInputBuffer + 
                     g->pd_currentInputBuffer->start;

  ((GIOP_S*)g)->requestType((GIOP::MsgType)hdr[7]);
  switch (((GIOP_S*)g)->requestType()) {
  case GIOP::Request:
  case GIOP::LocateRequest:
  case GIOP::CancelRequest:
    CORBA::ULong reqid;
    reqid <<= (cdrStream&)(*g);
    ((GIOP_S*)g)->requestId(reqid);
    break;
  case GIOP::CloseConnection:
    if (g->pd_strand->biDir) {
      // proper shutdown of a connection.
      // XXX what to do?
    }
    // falls through
  default:
    inputTerminalProtocolError(g);
    // Never reach here.
    break;
  }
}

////////////////////////////////////////////////////////////////////////
void
giopImpl12::unmarshalRequestHeader(giopStream* g) {

  GIOP_S& giop_s = *((GIOP_S*) g);
  cdrStream& s = *((cdrStream*)g);

  // We have already verified the request id in the header and the stream
  // have been setup to go pass it

  CORBA::ULong   vl;
  CORBA::Octet   vb;

  // response flag
  vb = s.unmarshalOctet();
  giop_s.response_expected((vb & 0x1));
  giop_s.result_expected((vb & 0x2) ? 1 : 0);

  // padding
  vb = s.unmarshalOctet();
  vb = s.unmarshalOctet();
  vb = s.unmarshalOctet();

  // object key
  giop_s.unmarshalIORAddressingInfo();

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

  // Service context
  giop_s.receive_service_contexts() <<= s;

  if (inputRemaining(g))
    s.alignInput(omni::ALIGN_8);
}

////////////////////////////////////////////////////////////////////////
void
giopImpl12::unmarshalLocateRequest(giopStream* g) {

  GIOP_S& giop_s = *((GIOP_S*) g);
  cdrStream& s = *((cdrStream*)g);


  // We have already verified the request id in the header and the stream
  // have been setup to go pass it

  giop_s.unmarshalIORAddressingInfo();
}

////////////////////////////////////////////////////////////////////////
size_t
giopImpl12::inputRemaining(giopStream* g) {

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
giopImpl12::getInputData(giopStream* g,omni::alignment_t align,size_t sz) {

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
	    l << "Fatal error in unmarshalling, invariant was violated at "
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
    OMNIORB_THROW(MARSHAL,MARSHAL_PassEndOfMessage,
		  (CORBA::CompletionStatus)g->completion());
  }

  if (!g->inputFragmentToCome()) {
    inputNewFragment(g);
    goto again;
  }

  // Reach here if we have some bytes to fetch for the current fragment

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

  g->pd_inb_mkr = (void*)((omni::ptr_arith_t)g->pd_currentInputBuffer + 
			  g->pd_currentInputBuffer->start);
  g->pd_inb_end = (void*)((omni::ptr_arith_t)g->pd_currentInputBuffer + 
			  g->pd_currentInputBuffer->last);
  g->inputFragmentToCome(g->inputFragmentToCome() - 
			 (g->pd_currentInputBuffer->last -
			  g->pd_currentInputBuffer->start));
}

////////////////////////////////////////////////////////////////////////
size_t
giopImpl12::max_input_buffer_size(giopStream* g,omni::alignment_t align) {

 again:
  omni::ptr_arith_t last = omni::align_to((omni::ptr_arith_t)g->pd_inb_mkr,
					  align);

  if (last == (omni::ptr_arith_t) g->pd_inb_end) {
    if (g->inputFragmentToCome())
      return g->inputFragmentToCome();
    else if (g->inputExpectAnotherFragment()) {
      inputNewFragment(g);
      goto again;
    }
  }
  else if (last < (omni::ptr_arith_t) g->pd_inb_end) {
    return ((omni::ptr_arith_t)g->pd_inb_end - last);
  }
  return 0;
}

////////////////////////////////////////////////////////////////////////
void
giopImpl12::skipInputData(giopStream* g,size_t sz) {
  copyInputData(g,0,sz,omni::ALIGN_1);
}

////////////////////////////////////////////////////////////////////////
void
giopImpl12::copyInputData(giopStream* g,void* b, size_t sz,
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
	  l << "Fatal error in unmarshalling, invariant was violated at "
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
	OMNIORB_THROW(MARSHAL,MARSHAL_PassEndOfMessage,
		      (CORBA::CompletionStatus)g->completion());
	// never reach here
      }

      if (!g->inputFragmentToCome()) {
	inputNewFragment(g);
	continue;
      }

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
giopImpl12::currentInputPtr(const giopStream* g) {

  return  g->inputMessageSize() - 
          g->inputFragmentToCome() -
         ((omni::ptr_arith_t) g->pd_inb_end - 
	  (omni::ptr_arith_t) g->pd_inb_mkr);
}

////////////////////////////////////////////////////////////////////////
void
giopImpl12::inputTerminalProtocolError(giopStream* g) {

  // XXX We may choose to send a message error to the other end.

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
giopImpl12::outputMessageBegin(giopStream* g,
			       void (*marshalHeader)(giopStream*)) {
}

////////////////////////////////////////////////////////////////////////
void
giopImpl12::outputMessageEnd(giopStream* g) {
}

////////////////////////////////////////////////////////////////////////
void
giopImpl12::sendMsgErrorMessage(giopStream* g) {
}

////////////////////////////////////////////////////////////////////////
void
giopImpl12::marshalRequestHeader(giopStream* g) {
}

////////////////////////////////////////////////////////////////////////
void
giopImpl12::marshalLocateRequest(giopStream* g) {
}

////////////////////////////////////////////////////////////////////////
void
giopImpl12::marshalReplyHeader(giopStream* g) {
}

////////////////////////////////////////////////////////////////////////
void
giopImpl12::sendSystemException(giopStream* g,const CORBA::SystemException& ex) {
}

////////////////////////////////////////////////////////////////////////
void
giopImpl12::sendUserException(giopStream* g,const CORBA::UserException& ex) {
}

////////////////////////////////////////////////////////////////////////
void
giopImpl12::sendLocationForwardReply(giopStream* g,CORBA::Object_ptr obj) {
}

////////////////////////////////////////////////////////////////////////
void
giopImpl12::sendLocateReply(giopStream* g,GIOP::LocateStatusType rc,
			    CORBA::Object_ptr obj,CORBA::SystemException* p) {
}

////////////////////////////////////////////////////////////////////////
size_t
giopImpl12::outputRemaining(const giopStream* g) {
  return 0;
}

////////////////////////////////////////////////////////////////////////
void
giopImpl12::getReserveSpace(giopStream* g,omni::alignment_t align,size_t sz) {
}

////////////////////////////////////////////////////////////////////////
void
giopImpl12::copyOutputData(giopStream* g,void* b, size_t sz,
			   omni::alignment_t align) {
}

////////////////////////////////////////////////////////////////////////
size_t
giopImpl12::max_reserve_buffer_size(const giopStream* g,
				    omni::alignment_t align) {
  return 0;
}

////////////////////////////////////////////////////////////////////////
CORBA::ULong
giopImpl12::currentOutputPtr(const giopStream* g) {
  return 0;
}



////////////////////////////////////////////////////////////////////////
static giopStreamImpl* giop_1_2_singleton = 0;

class omni_giopImpl12_initialiser : public omniInitialiser {
public:

  void attach() {
    if (!giop_1_2_singleton) {
      GIOP::Version ver = { 1, 2};
      giopStreamImpl* p;
      giop_1_2_singleton = p = new giopStreamImpl(ver);
      
      // Shared by the client and server side
      // Process message header
      p->outputMessageBegin             = giopImpl12::outputMessageBegin;
      p->outputMessageEnd               = giopImpl12::outputMessageEnd;
      p->inputMessageBegin              = giopImpl12::inputMessageBegin;
      p->inputMessageEnd                = giopImpl12::inputMessageEnd;
      p->sendMsgErrorMessage            = giopImpl12::sendMsgErrorMessage;

      // Client side
      // Process message header
      p->marshalRequestHeader           = giopImpl12::marshalRequestHeader;
      p->marshalLocateRequest           = giopImpl12::marshalLocateRequest;
      p->unmarshalReplyHeader           = giopImpl12::unmarshalReplyHeader;
      p->unmarshalLocateReply           = giopImpl12::unmarshalLocateReply;

      // Server side
      // Process message header
      p->unmarshalWildCardRequestHeader = giopImpl12::unmarshalWildCardRequestHeader;
      p->unmarshalRequestHeader         = giopImpl12::unmarshalRequestHeader;
      p->unmarshalLocateRequest         = giopImpl12::unmarshalLocateRequest;
      p->marshalReplyHeader             = giopImpl12::marshalReplyHeader;
      p->sendSystemException            = giopImpl12::sendSystemException;
      p->sendUserException              = giopImpl12::sendUserException;
      p->sendLocationForwardReply       = giopImpl12::sendLocationForwardReply;
      p->sendLocateReply                = giopImpl12::sendLocateReply;


      // Shared by the client and the server side
      // Process message body
      p->inputRemaining                 = giopImpl12::inputRemaining;
      p->getInputData                   = giopImpl12::getInputData;
      p->max_input_buffer_size          = giopImpl12::max_input_buffer_size;
      p->skipInputData                  = giopImpl12::skipInputData;
      p->copyInputData                  = giopImpl12::copyInputData;
      p->outputRemaining                = giopImpl12::outputRemaining;
      p->getReserveSpace                = giopImpl12::getReserveSpace;
      p->copyOutputData                 = giopImpl12::copyOutputData;
      p->max_reserve_buffer_size        = giopImpl12::max_reserve_buffer_size;
      p->currentInputPtr                = giopImpl12::currentInputPtr;
      p->currentOutputPtr               = giopImpl12::currentOutputPtr;

      giopStreamImpl::registerImpl(giop_1_2_singleton);
    }
  }

  void detach() { 
    if (giop_1_2_singleton) {
      delete giop_1_2_singleton;
      giop_1_2_singleton = 0;
    }
  }
};

static omni_giopImpl12_initialiser initialiser;

omniInitialiser& omni_giopImpl12_initialiser_ = initialiser;

OMNI_NAMESPACE_END(omni)
