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
  Revision 1.1.6.4  2005/03/30 23:36:09  dgrisby
  Another merge from omni4_0_develop.

  Revision 1.1.6.3  2005/01/13 21:10:00  dgrisby
  New SocketCollection implementation, using poll() where available and
  select() otherwise. Windows specific version to follow.

  Revision 1.1.6.2  2005/01/06 23:10:29  dgrisby
  Big merge from omni4_0_develop.

  Revision 1.1.6.1  2003/03/23 21:02:14  dgrisby
  Start of omniORB 4.1.x development branch.

  Revision 1.1.4.24  2003/01/28 13:37:06  dgrisby
  Send GIOP dumps to logger. Thanks Matej Kenda.

  Revision 1.1.4.23  2002/11/12 16:40:11  dgrisby
  Fix incorrect delete.

  Revision 1.1.4.22  2002/02/25 11:17:13  dpg1
  Use tracedmutexes everywhere.

  Revision 1.1.4.21  2002/02/13 16:02:39  dpg1
  Stability fixes thanks to Bastiaan Bakker, plus threading
  optimisations inspired by investigating Bastiaan's bug reports.

  Revision 1.1.4.20  2001/10/17 16:33:28  dpg1
  New downcast mechanism for cdrStreams.

  Revision 1.1.4.19  2001/09/26 10:48:11  sll
  Fixed a bug which causes problems when, in a single recv(), the ORB read
  more than one GIOP messages into its buffer and the last of these messages
  is only partially read.

  Revision 1.1.4.18  2001/09/10 17:53:07  sll
  In inputMessage, if a strand is dying and has been orderly_closed, i.e. a
  GIOP CloseConnection has been received, set the retry flag in the
  CommFailure exception.

  Revision 1.1.4.17  2001/09/04 14:44:25  sll
  Added the boolean argument to notifyCommFailure to indicate if
  omniTransportLock is held by the caller.
  Make sure reference count rd_nwaiting and wr_nwaiting are correct when a
  timeout has occurred.

  Revision 1.1.4.16  2001/09/03 16:51:01  sll
  Added the deadline parameter and access functions. All member functions
  that previously had deadline arguments now use the per-object deadline
  implicitly.

  Revision 1.1.4.15  2001/08/06 15:51:28  sll
  In errorOnSend, make sure that the retry flag returns by notifyCommFailure
  is not overwritten if the send failed on TRANSIENT_ConnectFailed.

  Revision 1.1.4.14  2001/08/03 17:41:21  sll
  System exception minor code overhaul. When a system exeception is raised,
  a meaning minor code is provided.

  Revision 1.1.4.13  2001/07/31 16:20:29  sll
  New primitives to acquire read lock on a connection.

  Revision 1.1.4.12  2001/07/13 15:28:36  sll
  Added more tracing messages.

  Revision 1.1.4.11  2001/07/03 12:01:16  dpg1
  Minor correction to log message for platforms without C++ bool.

  Revision 1.1.4.10  2001/06/20 18:35:17  sll
  Upper case send,recv,connect,shutdown to avoid silly substutition by
  macros defined in socket.h to rename these socket functions
  to something else.

  Revision 1.1.4.9  2001/05/11 14:25:53  sll
  Added operator for omniORB::logger to report system exception status and
  minor code.

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
CORBA::ULong giopStream::directSendCutOff = 16384;
CORBA::ULong giopStream::directReceiveCutOff = 1024;
CORBA::ULong giopStream::bufferSize = 8192;

////////////////////////////////////////////////////////////////////////
giopStream::giopStream(giopStrand* strand) : 
  pd_strand(strand),
  pd_rdlocked(0),
  pd_wrlocked(0),
  pd_impl(0),
  pd_deadline_secs(0),
  pd_deadline_nanosecs(0),
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
void*
giopStream::ptrToClass(int* cptr)
{
  if (cptr == &giopStream::_classid) return (giopStream*)this;
  if (cptr == &cdrStream ::_classid) return (cdrStream*) this;

  return 0;
}
int giopStream::_classid;

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
  setDeadline(0,0);
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

    CORBA::Boolean hastimeout = 0;

    // Now blocks.
    if (!(pd_deadline_secs || pd_deadline_nanosecs))
      pd_strand->rdcond.wait();
    else {
      hastimeout = !(pd_strand->rdcond.timedwait(pd_deadline_secs,
						 pd_deadline_nanosecs));
    }

    if (pd_strand->rd_nwaiting >= 0)
      pd_strand->rd_nwaiting--;
    else
      pd_strand->rd_nwaiting++;

    if (hastimeout) {
      // Timeout. 
      errorOnReceive(0,__FILE__,__LINE__,0,1);
    }
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
  if (pd_strand->rd_nwaiting > 0) {
    if (pd_strand->rd_n_justwaiting == 0) {
      pd_strand->rdcond.signal();
    }
    else {
      // There are threads which have blocked on the read lock's condition
      // variable and do not try to acquire the read lock.  If we signal
      // just one thread, the thread got woken up may just be of this type.
      // The other threads which do want the read lock may not get
      // it. The end result is no thread gets the read lock and the threads
      // that want it are still blocked in the condition variable.
      // To prevent this from happening, we wake all threads up.
      wakeUpRdLock(pd_strand);
    }
  }
  pd_rdlocked = 0;
}

////////////////////////////////////////////////////////////////////////
void
giopStream::wrLock() {

  ASSERT_OMNI_TRACEDMUTEX_HELD(*omniTransportLock,1);

  OMNIORB_ASSERT(!pd_wrlocked); 

  while (pd_strand->wr_nwaiting < 0) {
    pd_strand->wr_nwaiting--;

    CORBA::Boolean hastimeout = 0;

    // Now blocks.
    if (!(pd_deadline_secs || pd_deadline_nanosecs))
      pd_strand->wrcond.wait();
    else {
      hastimeout = !(pd_strand->wrcond.timedwait(pd_deadline_secs,
						 pd_deadline_nanosecs));
    }

    if (pd_strand->wr_nwaiting >= 0)
      pd_strand->wr_nwaiting--;
    else
      pd_strand->wr_nwaiting++;

    if (hastimeout) {
      // Timeout. 
      errorOnReceive(0,__FILE__,__LINE__,0,1);
    }
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

  if ( pd_strand->state() == giopStrand::DYING && 
       pd_strand->rd_n_justwaiting ) {
    // There are threads just blocking on the read lock's condition
    // variable and do not try to acquire the read lock. They may never
    // have a chance to discover that the strand has in fact died.
    wakeUpRdLock(pd_strand);
  }
}

////////////////////////////////////////////////////////////////////////
CORBA::Boolean
giopStream::rdLockNonBlocking() {

  ASSERT_OMNI_TRACEDMUTEX_HELD(*omniTransportLock,1);

  OMNIORB_ASSERT(!pd_rdlocked);

  if (pd_strand->rd_nwaiting < 0)
    return 0;
  else {
    pd_strand->rd_nwaiting = -pd_strand->rd_nwaiting - 1;
    pd_rdlocked = 1;
    return 1;
  }
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
giopStream::sleepOnRdLock() {

  ASSERT_OMNI_TRACEDMUTEX_HELD(*omniTransportLock,1);

  if (pd_strand->rd_nwaiting < 0) {
    pd_strand->rd_nwaiting--;

    CORBA::Boolean hastimeout = 0;

    // Now blocks.
    if (!(pd_deadline_secs || pd_deadline_nanosecs))
      pd_strand->rdcond.wait();
    else {
      hastimeout = !(pd_strand->rdcond.timedwait(pd_deadline_secs,
						 pd_deadline_nanosecs));
    }

    if (pd_strand->rd_nwaiting >= 0)
      pd_strand->rd_nwaiting--;
    else
      pd_strand->rd_nwaiting++;

    if (hastimeout) {
      // Timeout. 
      errorOnReceive(0,__FILE__,__LINE__,0,1);
    }
  }
}

////////////////////////////////////////////////////////////////////////
void
giopStream::sleepOnRdLock(giopStrand* strand) {

  ASSERT_OMNI_TRACEDMUTEX_HELD(*omniTransportLock,1);

  if (strand->rd_nwaiting < 0) {
    strand->rd_nwaiting--;
    strand->rdcond.wait();
    if (strand->rd_nwaiting >= 0)
      strand->rd_nwaiting--;
    else
      strand->rd_nwaiting++;
  }
}

////////////////////////////////////////////////////////////////////////
void
giopStream::sleepOnRdLockAlways() {

  ASSERT_OMNI_TRACEDMUTEX_HELD(*omniTransportLock,1);

  if (pd_strand->rd_nwaiting < 0)
    pd_strand->rd_nwaiting--;
  else
    pd_strand->rd_nwaiting++;

  pd_strand->rd_n_justwaiting++;

  CORBA::Boolean hastimeout = 0;

  // Now blocks.
  if (!(pd_deadline_secs || pd_deadline_nanosecs))
    pd_strand->rdcond.wait();
  else {
      hastimeout = !(pd_strand->rdcond.timedwait(pd_deadline_secs,
						 pd_deadline_nanosecs));
  }

  pd_strand->rd_n_justwaiting--;

  if (pd_strand->rd_nwaiting >= 0)
    pd_strand->rd_nwaiting--;
  else
    pd_strand->rd_nwaiting++;

  if (hastimeout) {
    // Timeout. 
    errorOnReceive(0,__FILE__,__LINE__,0,1);
  }
}

////////////////////////////////////////////////////////////////////////
void
giopStream::wakeUpRdLock(giopStrand* strand) {

  ASSERT_OMNI_TRACEDMUTEX_HELD(*omniTransportLock,1);

#if 1
  strand->rdcond.broadcast();
#else
  // Do this if the platform's condition variable broadcast does not
  // work.
  int i = strand->rd_nwaiting;
  if (i < 0) i = -1;
  for ( ; i > 0; i--)
    strand->rdcond.signal();
#endif

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
giopStream::notifyCommFailure(CORBA::Boolean,
			      CORBA::ULong& minor,
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
  if (status != CORBA::COMPLETED_NO) retry = 0;
#ifndef OMNIORB_NO_EXCEPTION_LOGGING
  if( omniORB::traceExceptions ) {
    omniORB::logger l;
    l << "throw giopStream::CommFailure from "
      << omniExHelper::strip(filename) 	
      << ":" << linenumber 
      << "(" << (int)retry << ",";
    const char* description = minorCode2String(TRANSIENT_LookupTable,minor);
    if (!description)
      description = minorCode2String(COMM_FAILURE_LookupTable,minor);
    if (description)
      l << omniORB::logger::exceptionStatus(status,description);
    else
      l << omniORB::logger::exceptionStatus(status,minor);
    l << ")\n";
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
    OMNIORB_THROW(BAD_PARAM,BAD_PARAM_InternalInvariant,
		  (CORBA::CompletionStatus)completion());
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
    char* c = (char*)p;
    delete [] c;
    return;
  }
  p->next = pd_strand->spare;
  pd_strand->spare = p;
}

////////////////////////////////////////////////////////////////////////
void
giopStream::errorOnReceive(int rc, const char* filename, CORBA::ULong lineno,
			   giopStream_Buffer* buf,CORBA::Boolean heldlock) {

  CORBA::ULong minor;
  CORBA::Boolean retry;

  notifyCommFailure(heldlock,minor,retry);
  if (rc == 0) {
    // Timeout.
    // We do not use the return code from the function.
    if (buf && buf->end != buf->start) {
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
    notifyCommFailure(0,minor,retry);
    giopStream_Buffer::deleteBuffer(buf);
    CommFailure::_raise(minor,(CORBA::CompletionStatus)completion(),retry,
			filename,lineno);
    // never reaches here.
  }
  // Get the message size from the buffer
  CORBA::ULong msz;

  // check for 8 byte alignment 
  if (((long)hdr & 7) == 0)
    msz = *(CORBA::ULong*)(hdr + 8);
  else
    memcpy(&msz, hdr + 8, sizeof(CORBA::ULong));

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
giopStream::inputMessage() {

  OMNIORB_ASSERT(pd_rdlocked);


  if (pd_strand->state() == giopStrand::DYING) {
    CORBA::ULong minor;
    CORBA::Boolean retry;
    notifyCommFailure(0,minor,retry);
    CORBA::CompletionStatus status;
    if (pd_strand->orderly_closed) {
      status = CORBA::COMPLETED_NO;
    }
    else {
      status = (CORBA::CompletionStatus)completion();
    }
    CommFailure::_raise(minor,status,retry,__FILE__,__LINE__);
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

    int rsz = pd_strand->connection->Recv((void*)
					  ((omni::ptr_arith_t)buf+buf->last),
					  (size_t) (buf->end - buf->last),
					  pd_deadline_secs,
					  pd_deadline_nanosecs);
    if (rsz > 0) {
      buf->last += rsz;
    }
    else {
      errorOnReceive(rsz,__FILE__,__LINE__,buf,0);
      // never reaches here.
    }
  }

  if (omniORB::trace(25)) {
    omniORB::logger log;
    log << "inputMessage: from " 
	<< pd_strand->connection->peeraddress() << " "
	<< buf->last - buf->start << " bytes\n";
  }
  if (omniORB::trace(30)) {
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
      int rsz = pd_strand->connection->Recv((void*)
					    ((omni::ptr_arith_t)buf+buf->last),
					    (size_t) total,
					    pd_deadline_secs,
					    pd_deadline_nanosecs);
      if (rsz > 0) {
	if (omniORB::trace(25)) {
	  omniORB::logger log;
	  log << "inputMessage: (body) from " 
	      << pd_strand->connection->peeraddress() << " "
	      << rsz << " bytes\n";
	}
	if (omniORB::trace(30)) {
	  dumpbuf((unsigned char*)buf+buf->last,rsz);
	}
	buf->last += rsz;
	total -= rsz;
      }
      else {
	errorOnReceive(rsz,__FILE__,__LINE__,buf,0);
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

    int splitcount = 0;

    do {
      CORBA::ULong sz = buf->last - first;
      giopStream_Buffer* newbuf;
      if (sz >= 12) {
	CORBA::ULong msz = ensureSaneHeader(__FILE__,__LINE__,buf,first);
	if (msz <= sz) {
	  sz = msz;
	}
	else {
	  if (msz > giopStream::bufferSize)
	    msz = giopStream::bufferSize;
	  if (msz < sz) {
	    // Don't think this could happen because the code at present
	    // never allocate buffer bigger than giopStream::bufferSize
	    // and so we will never have received data bigger than can be
	    // accomodated in a buffer of size giopStream::bufferSize.
	    // However, this may well change in the future and we
	    // have to prepare for this. In this case, we allocate a
	    // buffer that is multiple of 8 bytes in size and can
	    // store all the data received so far.
	    msz = omni::align_to((omni::ptr_arith_t)sz,omni::ALIGN_8);
	  }
	}
	newbuf = giopStream_Buffer::newBuffer(msz);
      }
      else {
	// incomplete header, we don't know the size of the message.
	// allocate a normal buffer to accomodate the rest of the message
	newbuf = giopStream_Buffer::newBuffer();
      }
      memcpy((void*)((omni::ptr_arith_t)newbuf+newbuf->start),
	     (void*)((omni::ptr_arith_t)buf + first),
	     sz);
      newbuf->last += sz; 

      if (omniORB::trace(40)) {
	omniORB::logger log;
	log << "Split to new buffer\n";
      }
      splitcount++;

      *tail = newbuf;
      tail = &(newbuf->next);
      first += sz;
    } while (first != buf->last);
    buf->last = buf->start + buf->size;

    if (omniORB::trace(25)) {
      omniORB::logger log;
      log << "Split input data into " << splitcount+1 << " messages\n";
    }
  }
  return buf;
}

////////////////////////////////////////////////////////////////////////
giopStream_Buffer*
giopStream::inputChunk(CORBA::ULong maxsize) {

  OMNIORB_ASSERT(pd_rdlocked);

  giopStream_Buffer* buf;

  if (pd_strand->head) {
    // We are expecting a chunk of a message and yet what comes
    // in is another message. This indicates something seriously
    // wrong with the data sent by the other end.
    pd_strand->state(giopStrand::DYING);
    CORBA::ULong minor;
    CORBA::Boolean retry;
    notifyCommFailure(0,minor,retry);
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
    int rsz = pd_strand->connection->Recv((void*)
					  ((omni::ptr_arith_t)buf+buf->last),
					  (size_t) maxsize,
					  pd_deadline_secs,
					  pd_deadline_nanosecs);
    if (rsz > 0) {
      buf->last += rsz;
      maxsize -= rsz;
    }
    else {
      errorOnReceive(rsz,__FILE__,__LINE__,buf,0);
      // never reaches here.
    }
  }
  if (omniORB::trace(25)) {
    omniORB::logger log;
    log << "inputChunk: from " 
	<< pd_strand->connection->peeraddress() << " "
	<< buf->last - buf->start << " bytes\n";
  }
  if (omniORB::trace(30)) {
    dumpbuf((unsigned char*)buf+buf->start,buf->last - buf->start);
  }
  return buf;
}

////////////////////////////////////////////////////////////////////////
void
giopStream::inputCopyChunk(void* dest, CORBA::ULong size) {

  OMNIORB_ASSERT(pd_rdlocked);

  if (pd_strand->head) {
    // We are expecting a chunk of a message and yet what comes
    // in is another message. This indicates something seriously
    // wrong with the data sent by the other end.
    pd_strand->state(giopStrand::DYING);
    CORBA::ULong minor;
    CORBA::Boolean retry;
    notifyCommFailure(0,minor,retry);
    CommFailure::_raise(minor,(CORBA::CompletionStatus)completion(),retry,
			__FILE__,__LINE__);
    // never reaches here.
  }

  char* p = (char*) dest;

  if (omniORB::trace(25)) {
    omniORB::logger log;
    log << "inputCopyChunk: from " 
	<< pd_strand->connection->peeraddress() << " "
	<< size << " bytes\n";
  }

  while (size) {
    int rsz = pd_strand->connection->Recv((void*)p,(size_t) size,
					  pd_deadline_secs,
					  pd_deadline_nanosecs);
    if (rsz > 0) {
      if (omniORB::trace(30)) {
	dumpbuf((unsigned char*)p,rsz);
      }
      p += rsz;
      size -= rsz;

    }
    else {
      errorOnReceive(rsz,__FILE__,__LINE__,0,0);
      // never reaches here.
    }
  }
}

////////////////////////////////////////////////////////////////////////
void
giopStream::sendChunk(giopStream_Buffer* buf) {

  if (!pd_strand->connection) {
    OMNIORB_ASSERT(pd_strand->address);
      
    if (pd_strand->state() != giopStrand::DYING) {
      if (omniORB::trace(25)) {
	omniORB::logger log;
	log << "Client attempt to connect to "
	    << pd_strand->address->address() << "\n";
      }
      giopActiveConnection* c = pd_strand->address->Connect(pd_deadline_secs,
							 pd_deadline_nanosecs);
      if (c) pd_strand->connection = &(c->getConnection());
    }
    if (!pd_strand->connection) {
      errorOnSend(TRANSIENT_ConnectFailed,__FILE__,__LINE__,0);
    }
    if (omniORB::trace(20)) {
      omniORB::logger log;
      log << "Client opened connection to " 
	  << pd_strand->connection->peeraddress() << "\n";
    }
  }

  CORBA::ULong first = buf->start;
  size_t total;

  if (omniORB::trace(25)) {
    omniORB::logger log;
    log << "sendChunk: to " 
	<< pd_strand->connection->peeraddress() << " "
	<< buf->last - buf->start << " bytes\n";
  }

  if (omniORB::trace(30)) {
    dumpbuf((unsigned char*)buf+buf->start,buf->last-buf->start);
  }

  while ((total = buf->last - first)) {
    int ssz = pd_strand->connection->Send((void*)
					  ((omni::ptr_arith_t)buf+first),
					  total,
					  pd_deadline_secs,
					  pd_deadline_nanosecs);
    if (ssz > 0) {
      first += ssz;
    }
    else {
      errorOnSend(ssz,__FILE__,__LINE__,0);
      // never reaches here.
    }
  }
}

////////////////////////////////////////////////////////////////////////
void
giopStream::sendCopyChunk(void* buf,CORBA::ULong size) {

  if (!pd_strand->connection) {
    OMNIORB_ASSERT(pd_strand->address);
    if (pd_strand->state() != giopStrand::DYING) {
      giopActiveConnection* c = pd_strand->address->Connect(pd_deadline_secs,
							    pd_deadline_nanosecs);
      if (c) pd_strand->connection = &(c->getConnection());
    }
    if (!pd_strand->connection) {
      errorOnSend(TRANSIENT_ConnectFailed,__FILE__,__LINE__,0);
    }
    if (omniORB::trace(20)) {
      omniORB::logger log;
      log << "Client opened connection to " 
	  << pd_strand->connection->peeraddress() << "\n";
    }
  }

  if (omniORB::trace(25)) {
    omniORB::logger log;
    log << "sendCopyChunk: to " 
	<< pd_strand->connection->peeraddress() << " "
	<< size << " bytes\n";
  }

  if (omniORB::trace(30)) {
    dumpbuf((unsigned char*)buf,size);
  }

  while (size) {
    int ssz = pd_strand->connection->Send(buf,
					  size,
					  pd_deadline_secs,
					  pd_deadline_nanosecs);
    if (ssz > 0) {
      size -= ssz;
      buf = (void*)((omni::ptr_arith_t)buf + ssz);
    }
    else {
      errorOnSend(ssz,__FILE__,__LINE__,0);
      // never reaches here.
    }
  }

}

////////////////////////////////////////////////////////////////////////
void
giopStream::errorOnSend(int rc, const char* filename, CORBA::ULong lineno,
			CORBA::Boolean heldlock) {

  CORBA::ULong minor;
  CORBA::Boolean retry;

  notifyCommFailure(heldlock,minor,retry);
  if (rc == 0) {
    // Timeout.
    // We do not use the return code from the function.
    pd_strand->state(giopStrand::DYING);
    retry = 0;
    minor = TRANSIENT_CallTimedout;
  }
  else if (rc == TRANSIENT_ConnectFailed) {
    pd_strand->state(giopStrand::DYING);
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
  static omni_tracedmutex lock;
  omni_tracedmutex_lock sync(lock);
  unsigned i;
  char row[80];
  omniORB::logger l;

  const size_t dumplimit = 128;
  if (!omniORB::trace(40) && sz > dumplimit) {
    l << dumplimit << " bytes out of " << sz << "\n";
    sz = dumplimit;
  }
  else {
    l << "\n";
  }

  // Do complete rows of 16 octets.
  while( sz >= 16u ) {
    sprintf(row, "%02x%02x %02x%02x %02x%02x %02x%02x "
	         "%02x%02x %02x%02x %02x%02x %02x%02x ",
            (int) buf[0], (int) buf[1], (int) buf[2], (int) buf[3],
            (int) buf[4], (int) buf[5], (int) buf[6], (int) buf[7],
            (int) buf[8], (int) buf[9], (int) buf[10], (int) buf[11],
            (int) buf[12], (int) buf[13], (int) buf[14], (int) buf[15]);
    l << row;
    char* p = row;
    for( i = 0u; i < 16u; i++ )  *p++ = printable_char(*buf++);
    *p++ = '\0';
    l << row << "\n";
    sz -= 16u;
  }

  if( sz ) {
    unsigned row_pos = 0;
    // The final part-row.
    for( i = 0u; i < sz; i++ ) {
      sprintf(row+row_pos, (i & 1u) ? "%02x ":"%02x", (int) buf[i]);
      row_pos += (i%2 == 0)? 2 : 3;
    }
    for( ; i < 16u; i++ ) {
      sprintf(row+row_pos, (i & 1u) ? "   ":"  ");
      row_pos += (i%2 == 0)? 2 : 3;
    }
    for( i = 0u; i < sz; i++ ) {
      sprintf(row+row_pos, "%c", printable_char(buf[i]));
      row_pos++;
    }
    l << row << "\n";
  }
}

OMNI_NAMESPACE_END(omni)
