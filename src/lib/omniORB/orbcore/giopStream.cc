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
  Revision 1.1.4.4.2.2  2001/02/23 16:46:57  sll
  Added new files.

  */

#include <omniORB4/CORBA.h>
#include <exceptiondefs.h>
#include <giopStream.h>
#include <giopStrand.h>
#include <giopStreamImpl.h>
#include <omniORB4/minorCode.h>

OMNI_NAMESPACE_BEGIN(omni)

////////////////////////////////////////////////////////////////////////
CORBA::ULong giopStream::directSendCutOff = 1024;
CORBA::ULong giopStream::directReceiveCutOff = 1024;
CORBA::ULong giopStream::receiveBufferSize = 8192;

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
  pd_inputMessageSize(0)
{
}

////////////////////////////////////////////////////////////////////////
giopStream::~giopStream() {
  inputBuffer* p = pd_input;
  while (p) {
    inputBuffer* q = p->next;
    giopStream::inputBuffer::deleteBuffer(p);
    p = q;
  }
  pd_input = 0;
  if (pd_currentInputBuffer) {
    giopStream::inputBuffer::deleteBuffer(pd_currentInputBuffer);
    pd_currentInputBuffer = 0;
  }
}

////////////////////////////////////////////////////////////////////////
void
giopStream::reset() {
  inputBuffer* p = pd_input;
  while (p) {
    inputBuffer* q = p->next;
    giopStream::inputBuffer::deleteBuffer(p);
    p = q;
  }
  pd_input = 0;
  if (pd_currentInputBuffer) {
    giopStream::inputBuffer::deleteBuffer(pd_currentInputBuffer);
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
  OMNIORB_ASSERT(required <= 8 && ((size_t)align == required));
  
  impl()->getInputData(this,align,required);
}

////////////////////////////////////////////////////////////////////////
size_t
giopStream::maxFetchInputData(omni::alignment_t align) const {

  OMNIORB_ASSERT(impl());
  return impl()->max_input_buffer_size((giopStream*)this,align);
}


////////////////////////////////////////////////////////////////////////
void
giopStream::put_octet_array(const CORBA::Octet* b, int size,
			    omni::alignment_t align) {

  OMNIORB_ASSERT(impl());
  if ((CORBA::ULong)size >= directSendCutOff || 
      (size_t)size > impl()->max_reserve_buffer_size(this,align)) {

    impl()->copyOutputData(this,(void*)b,size,align);
  }
  else {
  again:
    omni::ptr_arith_t p1 = omni::align_to((omni::ptr_arith_t)pd_outb_mkr,
					  align);
    omni::ptr_arith_t p2 = p1 + size;
    if ((void *)p2 > pd_outb_end) {

      if ((void*)(p1 + align) > pd_outb_end)
	reserveOutputSpace(align,size);
      else {
	// This is a bit messy, we make sure that we fill up all the
	// space left before we call reserveOutputSpace. This is
	// necessary because reserveOutputSpace may start a new
	// GIOP fragment. AND with GIOP 1.2 and above, padding may
	// be inserted at the end and  depending on the alignment,
	// may be incorrectly taken as data by the other end.
	// The following code make sure that this cannot happen.
	int avail = (omni::ptr_arith_t)pd_outb_end - p1;
	avail = avail / align * align;
	memcpy((void*)p1,(void*)b,avail);
	size -= avail;
	b += avail;
	pd_outb_mkr = (void*)(p1 + avail);
      }

      goto again;
    }
    pd_outb_mkr = (void*) p2;
    if (size)
      memcpy((void*)p1,(void*)b,size);
  }
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
giopStream::reserveOutputSpace(omni::alignment_t align,size_t required) {

  OMNIORB_ASSERT(impl());
  impl()->getReserveSpace(this,align,required);

  // Either we have the space or we've thrown an exception in getReserveSpace.
  return 1;

}

////////////////////////////////////////////////////////////////////////
size_t
giopStream::maxReserveOutputSpace(omni::alignment_t align) const {

  OMNIORB_ASSERT(impl());
  size_t reqsize = impl()->outputRemaining(this);
  if (reqsize > impl()->max_reserve_buffer_size(this,align)) {
    reqsize = impl()->max_reserve_buffer_size(this,align);
  }
  return reqsize;
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
static giopStream::inputBuffer* newInputBuffer(CORBA::ULong sz=0) {
  if (!sz) sz = giopStream::receiveBufferSize;

  char* p = new char[sz + sizeof(giopStream::inputBuffer) + 8];
  char* q = (char*) omni::align_to((omni::ptr_arith_t)
				   (p + sizeof(giopStream::inputBuffer)),
				   omni::ALIGN_8);

  giopStream::inputBuffer* b = (giopStream::inputBuffer*)p;
  b->start = (q - p);
  b->end = b->start + sz;
  b->last = b->start;
  b->size = 0;
  b->next = 0;
  return b;
}

////////////////////////////////////////////////////////////////////////
void 
giopStream_inputBuffer::deleteBuffer(giopStream_inputBuffer* b) {
  char* p = (char*)b;
  delete [] p;
}

////////////////////////////////////////////////////////////////////////
void
giopStream::releaseInputBuffer(giopStream::inputBuffer* p) {

  if (!pd_rdlocked || pd_strand->spare || (p->end - p->start) < giopStream::receiveBufferSize )
    delete p;
  pd_strand->spare = p;
  p->next = pd_strand->spare;
}

////////////////////////////////////////////////////////////////////////
void
giopStream::errorOnReceive(int rc, const char* filename, CORBA::ULong lineno,
			   giopStream::inputBuffer* buf) {

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
  if (buf) giopStream::inputBuffer::deleteBuffer(buf);
  CommFailure::_raise(minor,(CORBA::CompletionStatus)completion(),retry,
		      filename,lineno);
  // never reaches here.
}


////////////////////////////////////////////////////////////////////////
CORBA::ULong
giopStream::ensureSaneHeader(const char* filename, CORBA::ULong lineno,
			     giopStream::inputBuffer* buf,
			     CORBA::ULong begin) {

  CORBA::ULong minor;
  CORBA::Boolean retry;

  char* hdr = (char*)buf + begin;
  if (hdr[0] != 'G' || hdr[1] != 'I' || hdr[2] != 'O' || hdr[3] != 'P') {
    // Terrible! This is not a GIOP header.
    pd_strand->state(giopStrand::DYING);
    notifyCommFailure(minor,retry);
    giopStream::inputBuffer::deleteBuffer(buf);
    CommFailure::_raise(minor,(CORBA::CompletionStatus)completion(),retry,
			filename,lineno);
    // never reaches here.
  }
  // Get the message size from the buffer
  CORBA::ULong msz = *(CORBA::ULong*)(hdr + 8) + 12;
  if ((hdr[6] & 0x1) != _OMNIORB_HOST_BYTE_ORDER_) {
    CORBA::ULong bsz = msz;
    msz = ((((bsz) & 0xff000000) >> 24) |
	   (((bsz) & 0x00ff0000) >> 8)  |
	   (((bsz) & 0x0000ff00) << 8)  |
	   (((bsz) & 0x000000ff) << 24));
  }
  return msz;
}

////////////////////////////////////////////////////////////////////////
giopStream::inputBuffer*
giopStream::inputMessage(unsigned long deadline_secs = 0,
			 unsigned long deadline_nanosecs = 0) {

  OMNIORB_ASSERT(pd_rdlocked);


  if (pd_strand->state() == giopStrand::DYING) {
    CORBA::ULong minor;
    CORBA::Boolean retry;
    notifyCommFailure(minor,retry);
    CommFailure::_raise(minor,(CORBA::CompletionStatus)completion(),retry,
			__FILE__,__LINE__);
    // never reaches here.
  }

  giopStream::inputBuffer* buf;

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
    buf = newInputBuffer();
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

  buf->size = ensureSaneHeader(__FILE__,__LINE__,buf,buf->start);

  if (buf->size > (buf->last - buf->start)) {
    // Not enough data in the buffer. Try to fetch as much as can be fit
    // into the buffer.
    CORBA::ULong total = buf->size;
    if (total > (buf->end - buf->start)) {
      total = buf->end - buf->start;
    }
    total -= (buf->last - buf->start);
    do {
      int rsz = pd_strand->connection->recv((void*)
					    ((omni::ptr_arith_t)buf+buf->last),
					    (size_t) total,
					    deadline_secs, deadline_nanosecs);
      if (rsz > 0) {
	buf->last += rsz;
	total -= rsz;
      }
      else {
	errorOnReceive(rsz,__FILE__,__LINE__,buf);
	// never reaches here.
      }
    } while (total);
  }
  else if (buf->size < (buf->last - buf->start)) {
    // Too much data in the buffer. Locate the beginning of the next
    // message header(s) and uses a separate inputBuffer for each message.
    CORBA::ULong first = buf->start + buf->size;
    giopStream::inputBuffer** tail = &pd_strand->head;
    while (*tail)
      tail = &(*tail)->next;

    do {
      CORBA::ULong sz = buf->last - first;
      giopStream::inputBuffer* newbuf;
      if (sz >= 12) {
	CORBA::ULong msz = ensureSaneHeader(__FILE__,__LINE__,buf,first);
	if (msz < sz) sz = msz;
	newbuf = newInputBuffer(sz);
      }
      else {
	// incomplete header, we don't know the size of the message.
	// allocate a normal buffer to accomodate the rest of the message
	newbuf = newInputBuffer();
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
giopStream::inputBuffer*
giopStream::inputChunk(CORBA::ULong maxsize,
		       unsigned long deadline_secs,
		       unsigned long deadline_nanosecs) {

  OMNIORB_ASSERT(pd_rdlocked);

  giopStream::inputBuffer* buf;

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
    buf = newInputBuffer();
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

  while (size) {
    int rsz = pd_strand->connection->recv((void*)p,
					  (size_t) size,
					  deadline_secs, deadline_nanosecs);
    if (rsz > 0) {
      p += rsz;
      size -= rsz;
    }
    else {
      errorOnReceive(rsz,__FILE__,__LINE__,0);
      // never reaches here.
    }
  }
}

OMNI_NAMESPACE_END(omni)

