// -*- Mode: C++; -*-
//                            Package   : omniORB
// giopStream.cc              Created on: 16/01/2001
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
//	

/*
  $Log$
  Revision 1.1.4.8  2001/05/01 17:15:17  sll
  Non-copy input now works correctly.

  Revision 1.1.4.7  2001/05/01 16:07:31  sll
  All GIOP implementations should now work with fragmentation and abitrary
  sizes non-copy transfer.

  Revision 1.1.4.6  2001/04/18 18:10:48  sll
  Big checkin with the brand new internal APIs.


  */

#include <omniORB4/CORBA.h>
#include <exceptiondefs.h>
#include <giopStream.h>
#include <giopStrand.h>
#include <giopStreamImpl.h>
#include <omniORB4/minorCode.h>
#include <stdio.h>

OMNI_NAMESPACE_BEGIN(omni)

static void dumpbuf(unsigned char* buf, size_t sz);

////////////////////////////////////////////////////////////////////////
CORBA::ULong giopStream::directSendCutOff = 1024;
CORBA::ULong giopStream::directReceiveCutOff = 1024;
CORBA::ULong giopStream::bufferSize = 8192;

////////////////////////////////////////////////////////////////////////
giopStream::giopStream(giopStrand* strand) : 
  pd_strand(strand),
  pd_rdlocked(0),
  pd_wrlocked(0),
  pd_impl(0),
  pd_currentInputBuffer(0),
  pd_input(0),
  pd_inputFullyBuffered(0),
  pd_inputMatchedId(0),
  pd_inputExpectAnotherFragment(0),
  pd_inputFragmentToCome(0),
  pd_inputMessageSize(0),
  pd_currentOutputBuffer(0),
  pd_outputFragmentSize(0),
  pd_outputMessageSize(0),
  pd_request_id(0)
{
}

////////////////////////////////////////////////////////////////////////
giopStream::~giopStream() {
  giopStream_Buffer* p = pd_input;
  while (p) {
    giopStream_Buffer* q = p->next;
    giopStream_Buffer::deleteBuffer(p);
    p = q;
  }
  pd_input = 0;
  if (pd_currentInputBuffer) {
    giopStream_Buffer::deleteBuffer(pd_currentInputBuffer);
    pd_currentInputBuffer = 0;
  }
  if (pd_currentOutputBuffer) {
    giopStream_Buffer::deleteBuffer(pd_currentOutputBuffer);
    pd_currentOutputBuffer = 0;
  }
}

////////////////////////////////////////////////////////////////////////
void
giopStream::reset() {
  giopStream_Buffer* p = pd_input;
  while (p) {
    giopStream_Buffer* q = p->next;
    giopStream_Buffer::deleteBuffer(p);
    p = q;
  }
  pd_input = 0;
  if (pd_currentInputBuffer) {
    giopStream_Buffer::deleteBuffer(pd_currentInputBuffer);
    pd_currentInputBuffer = 0;
  }
  inputFullyBuffered(0);
  inputMatchedId(0);
}


////////////////////////////////////////////////////////////////////////
GIOP::Version
giopStream::version() {
  return pd_impl->version();
}

////////////////////////////////////////////////////////////////////////
void
giopStream::rdLock() {

  ASSERT_OMNI_TRACEDMUTEX_HELD(*omniTransportLock,1);

  OMNIORB_ASSERT(!pd_rdlocked);

  while (pd_strand->rd_nwaiting < 0) {
    pd_strand->rd_nwaiting--;
    pd_strand->rdcond.wait();
    if (pd_strand->rd_nwaiting >= 0)
      pd_strand->rd_nwaiting--;
    else
      pd_strand->rd_nwaiting++;
  }
  pd_strand->rd_nwaiting = -pd_strand->rd_nwaiting - 1;

  pd_rdlocked = 1;

}

////////////////////////////////////////////////////////////////////////
void
giopStream::rdUnLock() {

  ASSERT_OMNI_TRACEDMUTEX_HELD(*omniTransportLock,1);

  if (!pd_rdlocked) return;

  OMNIORB_ASSERT(pd_strand->rd_nwaiting < 0);
  pd_strand->rd_nwaiting = -pd_strand->rd_nwaiting - 1;
  if (pd_strand->rd_nwaiting > 0)
    pd_strand->rdcond.signal();

  pd_rdlocked = 0;
}

////////////////////////////////////////////////////////////////////////
void
giopStream::wrLock() {

  ASSERT_OMNI_TRACEDMUTEX_HELD(*omniTransportLock,1);

  OMNIORB_ASSERT(!pd_wrlocked); 

  while (pd_strand->wr_nwaiting < 0) {
    pd_strand->wr_nwaiting--;
    pd_strand->wrcond.wait();
    if (pd_strand->wr_nwaiting >= 0)
      pd_strand->wr_nwaiting--;
    else
      pd_strand->wr_nwaiting++;
  }
  pd_strand->wr_nwaiting = -pd_strand->wr_nwaiting - 1;

  pd_wrlocked = 1;

}

////////////////////////////////////////////////////////////////////////
void
giopStream::wrUnLock() {

  ASSERT_OMNI_TRACEDMUTEX_HELD(*omniTransportLock,1);

  if (!pd_wrlocked) return;

  OMNIORB_ASSERT(pd_strand->wr_nwaiting < 0);
  pd_strand->wr_nwaiting = -pd_strand->wr_nwaiting - 1;
  if (pd_strand->wr_nwaiting > 0)
    pd_strand->wrcond.signal();

  pd_wrlocked = 0; 
}

////////////////////////////////////////////////////////////////////////
CORBA::Boolean
giopStream::rdLockNonBlocking(giopStrand* strand) {

  ASSERT_OMNI_TRACEDMUTEX_HELD(*omniTransportLock,1);

  if (strand->rd_nwaiting < 0)
    return 0;
  else {
    strand->rd_nwaiting = -strand->rd_nwaiting - 1;
    return 1;
  }
}

////////////////////////////////////////////////////////////////////////
void
giopStream::sleepOnRdLock(giopStrand* strand, CORBA::Boolean always) {

  ASSERT_OMNI_TRACEDMUTEX_HELD(*omniTransportLock,1);

  if (strand->rd_nwaiting < 0) {
    strand->rd_nwaiting--;
    strand->rdcond.wait();
    if (strand->rd_nwaiting >= 0)
      strand->rd_nwaiting--;
    else
      strand->rd_nwaiting++;
  }
  else if (always) {
    strand->rd_nwaiting++;
    strand->rdcond.wait();
    if (strand->rd_nwaiting >= 0)
      strand->rd_nwaiting--;
    else
      strand->rd_nwaiting++;
  }
}

////////////////////////////////////////////////////////////////////////
void
giopStream::wakeUpRdLock(giopStrand* strand) {

  ASSERT_OMNI_TRACEDMUTEX_HELD(*omniTransportLock,1);

  if (strand->rd_nwaiting < 0) {
#if 1
    strand->rdcond.broadcast();
#else
    // Do this if the platform's condition variable broadcast does not
    // work.
    for (int i= -strand->rd_nwaiting; i > 0; i--)
      strand->rdcond.signal();
#endif
  }

}

////////////////////////////////////////////////////////////////////////
CORBA::Boolean
giopStream::noLockWaiting(giopStrand* strand) {

  ASSERT_OMNI_TRACEDMUTEX_HELD(*omniTransportLock,1);

  return ((strand->rd_nwaiting == 0) && (strand->wr_nwaiting == 0));
}

////////////////////////////////////////////////////////////////////////
void
giopStream::markRdLock() {

  ASSERT_OMNI_TRACEDMUTEX_HELD(*omniTransportLock,1);

  OMNIORB_ASSERT(pd_rdlocked == 0);
  pd_rdlocked = 1;
}

////////////////////////////////////////////////////////////////////////
CORBA::Boolean
giopStream::RdLockIsHeld(giopStrand* strand) {

  ASSERT_OMNI_TRACEDMUTEX_HELD(*omniTransportLock,1);

  return ((strand->rd_nwaiting != 0));
}


////////////////////////////////////////////////////////////////////////
void
giopStream::notifyCommFailure(CORBA::ULong& minor,
			      CORBA::Boolean& retry)
{
  minor = 0;
  retry = 0;
}

////////////////////////////////////////////////////////////////////////
void
giopStream::CommFailure::_raise(CORBA::ULong minor,
				CORBA::CompletionStatus status,
				CORBA::Boolean retry,
				const char* filename,
				CORBA::ULong linenumber)
{
#ifndef OMNIORB_NO_EXCEPTION_LOGGING
  if( omniORB::trace(10) ) {
    omniORB::logger l;
    l << "throw giopStream::CommFailure from "
      << omniExHelper::strip(filename) 	
      << ":" << linenumber << '\n';
  }
#endif
  if (status != CORBA::COMPLETED_NO) retry = 0;
  throw CommFailure(minor,status,retry,filename,linenumber);
}


////////////////////////////////////////////////////////////////////////
void
giopStream::get_octet_array(CORBA::Octet* b,int size,omni::alignment_t align) {

  OMNIORB_ASSERT(impl());
  impl()->copyInputData(this,b,size,align);
}

////////////////////////////////////////////////////////////////////////
void
giopStream::skipInput(CORBA::ULong size) {

  OMNIORB_ASSERT(impl());
  impl()->skipInputData(this,size);
}

////////////////////////////////////////////////////////////////////////
CORBA::Boolean
giopStream::checkInputOverrun(CORBA::ULong itemSize, 
			      CORBA::ULong nItems,
			      omni::alignment_t align) {

  OMNIORB_ASSERT(impl());
  size_t avail = impl()->inputRemaining(this);
  omni::ptr_arith_t p1 = omni::align_to((omni::ptr_arith_t)pd_inb_mkr,align);
  p1 += itemSize*nItems;
  if (avail < (size_t)(p1 - (omni::ptr_arith_t)pd_inb_mkr))
    return 0;
  else
    return 1;

}

////////////////////////////////////////////////////////////////////////
void
giopStream::fetchInputData(omni::alignment_t align,size_t required) {

  OMNIORB_ASSERT(impl());
  OMNIORB_ASSERT(required == 0 ||
		 (required <= 8 && ((size_t)align == required)));
  
  impl()->getInputData(this,align,required);
}


////////////////////////////////////////////////////////////////////////
void
giopStream::put_octet_array(const CORBA::Octet* b, int size,
			    omni::alignment_t align) {

  OMNIORB_ASSERT(impl());
  impl()->copyOutputData(this,(void*)b,size,align);
}


////////////////////////////////////////////////////////////////////////
CORBA::Boolean
giopStream::checkOutputOverrun(CORBA::ULong itemSize,
			       CORBA::ULong nItems,
			       omni::alignment_t align) {

  OMNIORB_ASSERT(impl());
  size_t avail = impl()->outputRemaining(this);
  if (avail != ULONG_MAX) {
    omni::ptr_arith_t p1=omni::align_to((omni::ptr_arith_t)pd_outb_mkr,align);
    p1 += itemSize*nItems;
    if (avail < (size_t)(p1 - (omni::ptr_arith_t)pd_outb_mkr))
      return 0;
  }
  return 1;
}

////////////////////////////////////////////////////////////////////////
CORBA::Boolean
giopStream::reserveOutputSpaceForPrimitiveType(omni::alignment_t align,
					       size_t required) {

  OMNIORB_ASSERT(impl());
  OMNIORB_ASSERT(required == 0 ||
		 (required <= 8 && ((size_t)align == required)));

  impl()->getReserveSpace(this,align,required);

  // Either we have the space or we've thrown an exception in getReserveSpace.
  return 1;

}

////////////////////////////////////////////////////////////////////////
CORBA::Boolean
giopStream::maybeReserveOutputSpace(omni::alignment_t align,
				    size_t required) {

  OMNIORB_ASSERT(impl());

  if (required > 8 || ((size_t)align != required)) {
    OMNIORB_THROW(BAD_PARAM,0,CORBA::COMPLETED_NO);
  }

  impl()->getReserveSpace(this,align,required);

  // Either we have the space or we've thrown an exception in getReserveSpace.
  return 1;

}


////////////////////////////////////////////////////////////////////////
CORBA::ULong
giopStream::currentInputPtr() const {

  OMNIORB_ASSERT(impl());
  return impl()->currentInputPtr(this);
}

////////////////////////////////////////////////////////////////////////
CORBA::ULong
giopStream::currentOutputPtr() const {

  OMNIORB_ASSERT(impl());
  return impl()->currentOutputPtr(this);
}

////////////////////////////////////////////////////////////////////////
giopStream_Buffer* 
giopStream_Buffer::newBuffer(CORBA::ULong sz) {
  if (!sz) sz = giopStream::bufferSize;

  //  giopStream_Buffer* b = (giopStream_Buffer*)
  //                         (new char[sz + sizeof(giopStream_Buffer) + 8]);
  char* p = new char[sz + sizeof(giopStream_Buffer) + 8];
  giopStream_Buffer* b = (giopStream_Buffer*)p;
  b->alignStart(omni::ALIGN_8);
  b->end = b->start + sz;
  b->last = b->start;
  b->size = 0;
  b->next = 0;
  return b;
}

////////////////////////////////////////////////////////////////////////
void 
giopStream_Buffer::deleteBuffer(giopStream_Buffer* b) {
  char* p = (char*)b;
  delete [] p;
}

////////////////////////////////////////////////////////////////////////
void 
giopStream_Buffer::alignStart(omni::alignment_t align) {

  omni::ptr_arith_t p = omni::align_to((omni::ptr_arith_t)this + 
				       sizeof(giopStream_Buffer),align);
  start = p - (omni::ptr_arith_t) this;
}


////////////////////////////////////////////////////////////////////////
void
giopStream::releaseInputBuffer(giopStream_Buffer* p) {

  if (!pd_rdlocked || pd_strand->spare || (p->end - p->start) < giopStream::bufferSize ) {
    delete p;
    return;
  }
  p->next = pd_strand->spare;
  pd_strand->spare = p;
}

////////////////////////////////////////////////////////////////////////
void
giopStream::errorOnReceive(int rc, const char* filename, CORBA::ULong lineno,
			   giopStream_Buffer* buf) {

  CORBA::ULong minor;
  CORBA::Boolean retry;

  notifyCommFailure(minor,retry);
  if (rc == 0) {
    // Timeout.
    // We do not use the return code from the function.
    if (buf->end != buf->start) {
      // partially received a buffer, we assume the other end
      // is in serious trouble.
      pd_strand->state(giopStrand::DYING);
    }
    retry = 0;
    minor = TRANSIENT_CallTimedout;
  }
  else {
    pd_strand->state(giopStrand::DYING);
  }
  if (buf) giopStream_Buffer::deleteBuffer(buf);
  CommFailure::_raise(minor,(CORBA::CompletionStatus)completion(),retry,
		      filename,lineno);
  // never reaches here.
}


////////////////////////////////////////////////////////////////////////
CORBA::ULong
giopStream::ensureSaneHeader(const char* filename, CORBA::ULong lineno,
			     giopStream_Buffer* buf,
			     CORBA::ULong begin) {

  CORBA::ULong minor;
  CORBA::Boolean retry;

  char* hdr = (char*)buf + begin;
  if (hdr[0] != 'G' || hdr[1] != 'I' || hdr[2] != 'O' || hdr[3] != 'P') {
    // Terrible! This is not a GIOP header.
    pd_strand->state(giopStrand::DYING);
    notifyCommFailure(minor,retry);
    giopStream_Buffer::deleteBuffer(buf);
    CommFailure::_raise(minor,(CORBA::CompletionStatus)completion(),retry,
			filename,lineno);
    // never reaches here.
  }
  // Get the message size from the buffer
  CORBA::ULong msz = *(CORBA::ULong*)(hdr + 8);
  if ((hdr[6] & 0x1) != _OMNIORB_HOST_BYTE_ORDER_) {
    CORBA::ULong bsz = msz;
    msz = ((((bsz) & 0xff000000) >> 24) |
	   (((bsz) & 0x00ff0000) >> 8)  |
	   (((bsz) & 0x0000ff00) << 8)  |
	   (((bsz) & 0x000000ff) << 24));
  }
  return msz + 12;
}

////////////////////////////////////////////////////////////////////////
giopStream_Buffer*
giopStream::inputMessage(unsigned long deadline_secs,
			 unsigned long deadline_nanosecs) {

  OMNIORB_ASSERT(pd_rdlocked);


  if (pd_strand->state() == giopStrand::DYING) {
    CORBA::ULong minor;
    CORBA::Boolean retry;
    notifyCommFailure(minor,retry);
    CommFailure::_raise(minor,(CORBA::CompletionStatus)completion(),retry,
			__FILE__,__LINE__);
    // never reaches here.
  }

  giopStream_Buffer* buf;

  if (pd_strand->head) {
    buf = pd_strand->head;
    pd_strand->head = buf->next;
    buf->next = 0;
  }
  else if (pd_strand->spare) {
    buf = pd_strand->spare;
    pd_strand->spare = buf->next;
    buf->next = 0;
    buf->last = buf->start;
  }
  else {
    buf = giopStream_Buffer::newBuffer();
  }

  while ((buf->last - buf->start) < 12) {

    int rsz = pd_strand->connection->recv((void*)
					  ((omni::ptr_arith_t)buf+buf->last),
					  (size_t) (buf->end - buf->last),
					  deadline_secs, deadline_nanosecs);
    if (rsz > 0) {
      buf->last += rsz;
    }
    else {
      errorOnReceive(rsz,__FILE__,__LINE__,buf);
      // never reaches here.
    }
  }

  if (omniORB::trace(30)) {
    fprintf(stderr,"omniORB: inputMessage: from %s %lu bytes\n",
	    pd_strand->connection->peeraddress(),buf->last - buf->start);
    dumpbuf((unsigned char*)buf+buf->start,buf->last - buf->start);
  }

  buf->size = ensureSaneHeader(__FILE__,__LINE__,buf,buf->start);

  if (buf->size > (buf->last - buf->start)) {
    // Not enough data in the buffer. Try to fetch as much as can be fit
    // into the buffer.
    CORBA::ULong total = buf->size;
    if (total > (buf->end - buf->start)) {
      total = buf->end - buf->start;
    }
    total -= (buf->last - buf->start);
    while (total) {
      int rsz = pd_strand->connection->recv((void*)
					    ((omni::ptr_arith_t)buf+buf->last),
					    (size_t) total,
					    deadline_secs, deadline_nanosecs);
      if (rsz > 0) {
	if (omniORB::trace(30)) {
	  fprintf(stderr,"omniORB: inputMessage: (body) from %s %d bytes\n",
		  pd_strand->connection->peeraddress(),rsz);
	  dumpbuf((unsigned char*)buf+buf->last,rsz);
	}
	buf->last += rsz;
	total -= rsz;
      }
      else {
	errorOnReceive(rsz,__FILE__,__LINE__,buf);
	// never reaches here.
      }
    }
  }
  else if (buf->size < (buf->last - buf->start)) {
    // Too much data in the buffer. Locate the beginning of the next
    // message header(s) and uses a separate Buffer for each message.
    CORBA::ULong first = buf->start + buf->size;
    giopStream_Buffer** tail = &pd_strand->head;
    while (*tail)
      tail = &(*tail)->next;

    do {
      CORBA::ULong sz = buf->last - first;
      giopStream_Buffer* newbuf;
      if (sz >= 12) {
	CORBA::ULong msz = ensureSaneHeader(__FILE__,__LINE__,buf,first);
	if (msz < sz) sz = msz;
	newbuf = giopStream_Buffer::newBuffer(sz);
      }
      else {
	// incomplete header, we don't know the size of the message.
	// allocate a normal buffer to accomodate the rest of the message
	newbuf = giopStream_Buffer::newBuffer();
      }
      memcpy((void*)((omni::ptr_arith_t)newbuf+newbuf->start),
	     (void*)((omni::ptr_arith_t)buf + first),
	     sz);
      *tail = newbuf;
      tail = &(newbuf->next);
      first += sz;
    } while (first != buf->last);
    buf->last = buf->start + buf->size;
  }
  return buf;
}

////////////////////////////////////////////////////////////////////////
giopStream_Buffer*
giopStream::inputChunk(CORBA::ULong maxsize,
		       unsigned long deadline_secs,
		       unsigned long deadline_nanosecs) {

  OMNIORB_ASSERT(pd_rdlocked);

  giopStream_Buffer* buf;

  if (pd_strand->head) {
    // We are expecting a chunk of a message and yet what comes
    // in is another message. This indicates something seriously
    // wrong with the data sent by the other end.
    pd_strand->state(giopStrand::DYING);
    CORBA::ULong minor;
    CORBA::Boolean retry;
    notifyCommFailure(minor,retry);
    CommFailure::_raise(minor,(CORBA::CompletionStatus)completion(),retry,
			__FILE__,__LINE__);
    // never reaches here.
  }
  else if (pd_strand->spare) {
    buf = pd_strand->spare;
    pd_strand->spare = buf->next;
    buf->next = 0;
    buf->last = buf->start;
  }
  else {
    buf = giopStream_Buffer::newBuffer();
  }

  if (maxsize > (buf->end - buf->start)) {
    maxsize = buf->end - buf->start;
  }

  while (maxsize) {
    int rsz = pd_strand->connection->recv((void*)
					  ((omni::ptr_arith_t)buf+buf->last),
					  (size_t) maxsize,
					  deadline_secs, deadline_nanosecs);
    if (rsz > 0) {
      buf->last += rsz;
      maxsize -= rsz;
    }
    else {
      errorOnReceive(rsz,__FILE__,__LINE__,buf);
      // never reaches here.
    }
  }
  if (omniORB::trace(30)) {
    fprintf(stderr,"omniORB: inputChunk: from %s %lu bytes\n",
	    pd_strand->connection->peeraddress(),buf->last - buf->start);
    dumpbuf((unsigned char*)buf+buf->start,buf->last - buf->start);
  }
  return buf;
}

////////////////////////////////////////////////////////////////////////
void
giopStream::inputCopyChunk(void* dest,
			   CORBA::ULong size,
			   unsigned long deadline_secs,
			   unsigned long deadline_nanosecs) {

  OMNIORB_ASSERT(pd_rdlocked);

  if (pd_strand->head) {
    // We are expecting a chunk of a message and yet what comes
    // in is another message. This indicates something seriously
    // wrong with the data sent by the other end.
    pd_strand->state(giopStrand::DYING);
    CORBA::ULong minor;
    CORBA::Boolean retry;
    notifyCommFailure(minor,retry);
    CommFailure::_raise(minor,(CORBA::CompletionStatus)completion(),retry,
			__FILE__,__LINE__);
    // never reaches here.
  }

  char* p = (char*) dest;

  if (omniORB::trace(30)) {
    fprintf(stderr,"omniORB: inputCopyChunk: from %s %lu bytes\n",
	    pd_strand->connection->peeraddress(),size);
  }

  while (size) {
    int rsz = pd_strand->connection->recv((void*)p,
					  (size_t) size,
					  deadline_secs, deadline_nanosecs);
    if (rsz > 0) {
      if (omniORB::trace(30)) {
	dumpbuf((unsigned char*)p,rsz);
      }
      p += rsz;
      size -= rsz;

    }
    else {
      errorOnReceive(rsz,__FILE__,__LINE__,0);
      // never reaches here.
    }
  }
}

////////////////////////////////////////////////////////////////////////
void
giopStream::sendChunk(giopStream_Buffer* buf,
		      unsigned long deadline_secs,
		      unsigned long deadline_nanosecs) {

  if (!pd_strand->connection) {
    OMNIORB_ASSERT(pd_strand->address);
    pd_strand->connection = pd_strand->address->connect(deadline_secs,
							deadline_nanosecs);
    if (!pd_strand->connection) {
      errorOnSend(TRANSIENT_ConnectFailed,__FILE__,__LINE__);
    }
  }

  CORBA::ULong first = buf->start;
  size_t total;

  if (omniORB::trace(30)) {
    fprintf(stderr,"omniORB: sendChunk: to %s %lu bytes\n",
	    pd_strand->connection->peeraddress(),buf->last - buf->start);
    dumpbuf((unsigned char*)buf+buf->start,buf->last-buf->start);
  }

  while ((total = buf->last - first)) {
    int ssz = pd_strand->connection->send((void*)
					  ((omni::ptr_arith_t)buf+first),
					  total,
					  deadline_secs, deadline_nanosecs);
    if (ssz > 0) {
      first += ssz;
    }
    else {
      errorOnSend(ssz,__FILE__,__LINE__);
      // never reaches here.
    }
  }
}

////////////////////////////////////////////////////////////////////////
void
giopStream::sendCopyChunk(void* buf,
			  CORBA::ULong size,
			  unsigned long deadline_secs,
			  unsigned long deadline_nanosecs) {

  if (!pd_strand->connection) {
    OMNIORB_ASSERT(pd_strand->address);
    pd_strand->connection = pd_strand->address->connect(deadline_secs,
							deadline_nanosecs);
    if (!pd_strand->connection) {
      errorOnSend(TRANSIENT_ConnectFailed,__FILE__,__LINE__);
    }
  }

  if (omniORB::trace(30)) {
    fprintf(stderr,"omniORB: sendCopyChunk: to %s %lu bytes\n",
	    pd_strand->connection->peeraddress(),size);
    dumpbuf((unsigned char*)buf,size);
  }

  while (size) {
    int ssz = pd_strand->connection->send(buf,
					  size,
					  deadline_secs, deadline_nanosecs);
    if (ssz > 0) {
      size -= ssz;
      buf = (void*)((omni::ptr_arith_t)buf + ssz);
    }
    else {
      errorOnSend(ssz,__FILE__,__LINE__);
      // never reaches here.
    }
  }

}

////////////////////////////////////////////////////////////////////////
void
giopStream::errorOnSend(int rc, const char* filename, CORBA::ULong lineno) {

  CORBA::ULong minor;
  CORBA::Boolean retry;

  notifyCommFailure(minor,retry);
  if (rc == 0) {
    // Timeout.
    // We do not use the return code from the function.
    pd_strand->state(giopStrand::DYING);
    retry = 0;
    minor = TRANSIENT_CallTimedout;
  }
  else if (rc == TRANSIENT_ConnectFailed) {
    pd_strand->state(giopStrand::DYING);
    retry = 0;
    minor = rc;
  }
  else {
    pd_strand->state(giopStrand::DYING);
  }
  CommFailure::_raise(minor,(CORBA::CompletionStatus)completion(),retry,
		      filename,lineno);
  // never reaches here.
}


/////////////////////////////////////////////////////////////////////////
static inline char printable_char(char c) {
  return (c < 32 || c > 126) ? '.' : c;
}

/////////////////////////////////////////////////////////////////////////
static void dumpbuf(unsigned char* buf, size_t sz)
{
  static omni_mutex lock;
  omni_mutex_lock sync(lock);
  unsigned i;
  char row[80];

  const size_t dumplimit = 128;
  if (!omniORB::trace(40) && sz > dumplimit) {
    fprintf(stderr,"%u bytes out of %u\n",
	    dumplimit, sz);
    sz = dumplimit;
  }


  // Do complete rows of 16 octets.
  while( sz >= 16u ) {
    sprintf(row, "%02x%02x %02x%02x %02x%02x %02x%02x "
	         "%02x%02x %02x%02x %02x%02x %02x%02x ",
            (int) buf[0], (int) buf[1], (int) buf[2], (int) buf[3],
            (int) buf[4], (int) buf[5], (int) buf[6], (int) buf[7],
            (int) buf[8], (int) buf[9], (int) buf[10], (int) buf[11],
            (int) buf[12], (int) buf[13], (int) buf[14], (int) buf[15]);
    fprintf(stderr, "%s", row);
    char* p = row;
    for( i = 0u; i < 16u; i++ )  *p++ = printable_char(*buf++);
    *p++ = '\0';
    fprintf(stderr,"%s\n", row);
    sz -= 16u;
  }

  if( sz ) {
    // The final part-row.
    for( i = 0u; i < sz; i++ )
      fprintf(stderr, (i & 1u) ? "%02x ":"%02x", (int) buf[i]);
    for( ; i < 16u; i++ )
      fprintf(stderr, (i & 1u) ? "   ":"  ");
    for( i = 0u; i < sz; i++ )
      fprintf(stderr, "%c", printable_char(buf[i]));
    fprintf(stderr,"\n");
  }
}

OMNI_NAMESPACE_END(omni)
