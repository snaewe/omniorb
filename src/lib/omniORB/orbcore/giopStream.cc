// -*- Mode: C++; -*-
//                            Package   : omniORB
// giopStream.cc              Created on: 13/1/99
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 1999 AT&T Laboratories, Cambridge
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
  Revision 1.1.4.3  2000/11/15 17:22:19  sll
  Consolidated all the state variables of a giopstream that is associated
  with a strand to a giopStreamInfo structure.
  Added char, wchar codeset convertor support to giopStream.

  Revision 1.1.4.2  2000/11/03 19:12:06  sll
  Use new marshalling functions for byte, octet and char. Use get_octet_array
  instead of get_char_array and put_octet_array instead of put_char_array.

  Revision 1.1.4.1  2000/09/27 17:30:28  sll
  *** empty log message ***

  Revision 1.1.2.9  2000/03/27 17:35:09  sll
  Redefined interface. Now do strand selection in this class.
  Also allow multiple calls on both the client and the server side.

  Revision 1.1.2.8  2000/02/14 18:06:45  sll
  Support GIOP 1.2 fragment interleaving. This requires minor access control
  changes to the relevant classes.

  Revision 1.1.2.7  1999/11/05 19:00:14  sll
  giopStream::is_unused() and giopStream::garbageCollect() should not
  delete the stream object if pd_nwaiting is not zero.

  Revision 1.1.2.6  1999/11/05 14:44:57  sll
  Fully initialise the return value of requestInfo::targetAddress() even if
  the addressing mode is only GIOP::ProfileAddr. This is done when the target
  object is unknown. There may be a MapTargetAddressToObjectFunction
  registered to look into targetAddress().

  Revision 1.1.2.5  1999/11/04 20:20:19  sll
  GIOP engines can now do callback to the higher layer to calculate total
  message size if necessary.
  Where applicable, changed to use the new server side descriptor-based stub.

  Revision 1.1.2.4  1999/10/05 20:35:33  sll
  Added support to GIOP 1.2 to recognise all TargetAddress mode.
  Now handles NEEDS_ADDRESSING_MODE and LOC_NEEDS_ADDRESSING_MODE.

  Revision 1.1.2.3  1999/10/02 18:24:32  sll
  Reformatted trace messages.

  Revision 1.1.2.2  1999/09/16 16:04:25  sll
  Bug fix. Rope_iterator now use Strand::Sync::is_unused() to test if the
  strand is not in use. giopStream::is_unused() now remove unused giopStream
  as a side-effect. The net effect is that unused outgoing Rope is now
  deleted correctly.

  Revision 1.1.2.1  1999/09/15 20:37:27  sll
  *** empty log message ***

*/

#include <omniORB4/CORBA.h>
#include <objectAdapter.h>
#include <scavenger.h>
#include <initialiser.h>
#include <giopStreamImpl.h>
#include <ropeFactory.h>
#include <tcpSocket.h>

#include <stdlib.h>

#define DIRECT_RCV_CUTOFF 1024
#define DIRECT_SND_CUTOFF 8192


#define PARANOID

#define LOGMESSAGE(level,prefix,message) do {\
   if (omniORB::trace(level)) {\
     omniORB::logger log;\
	log << " giopStream " ## prefix ## ": " ## message ## "\n";\
   }\
} while (0)

#define PTRACE(prefix,message) LOGMESSAGE(25,prefix,message)

#define GET_SYNC_MUTEX(s) s->pd_rope->pd_lock

giopStream::~giopStream()
{
  if (!pd_strand) return;

#ifdef PARANOID
  OMNIORB_ASSERT(!pd_rdlocked && !pd_wrlocked);
  OMNIORB_ASSERT(pd_inb_mkr == pd_inb_end);
  OMNIORB_ASSERT(pd_outb_mkr == pd_outb_end);
#endif

  // remove this from the list in strand <pd_strand>
  giopStream **p = &(streamInfo()->head);
  while (*p && *p != this)
    p = &((*p)->pd_next);
  if (*p) {
    *p = (*p)->pd_next;
  }
  pd_strand = 0;

}

giopStream*
giopStream::acquireClient(Rope* r, GIOP::Version v)
{
  // Acquire a giopStream for use as a GIOP Client.
  //
  // Theory of operation:
  //
  // The attributes of the rope determines if 2 or more calls can be in
  // progress on the same strand at the same time.
  //
  // 1. Only 1 call can be in progress per strand:
  //
  //    Scan the rope for idle strands. A strand is idle if its reference
  //    count is 0. When we have acquire a strand, we increment the
  //    reference count to claim ownership. If none is available and
  //    the number of strands has not exceeded r->pd_maxStrands, 
  //    call r->newStrand() to create a new one. If both methods fail
  //    to yield a strand, block waiting till a strand is freed.
  //
  // 2. More than 1 call is allowed per strand:
  //
  //    Same as above except that if there is no idle strand and we cannot
  //    create a new one, we do not block waiting for a free one. Instead
  //    we just choose one that is currently in use randomly and returns.

  giopStreamImpl* impl = giopStreamImpl::matchVersion(v);
  if (!impl) {
    impl = giopStreamImpl::maxVersion();
    v = impl->version();
  }

  omni_mutex_lock sync(r->pd_lock);

 again:

  Strand *s;
  unsigned int nbusy = 0;
  unsigned int ndying = 0;
  unsigned int nwrongver = 0;
  {
    Strand_iterator iter(r,1);
    while ((s = iter())) {
      if (s->_strandIsDying()) {
	ndying++;
      }
      else if (s->pd_giop_info->version.major != v.major ||
	       s->pd_giop_info->version.minor != v.minor) {

	// Wrong GIOP version. Each strand can only be used
	// for one GIOP version.
	// If ever we allow more than one GIOP version requests
	// to use one strand, make sure the client side interceptor
	// for codeset is updated to reflect this.
	nwrongver++;
      }
      else if (s->pd_refcount > 1) {
	// Strand_iterator::operator() returns the strand with
	// refcount incremented by 1, so we know the strand
	// is not claimed by any thread if refcount == 1.
	nbusy++;
      }
      else {
	break;
      }
    }
  }
  if (!s && (nbusy + ndying) < r->pd_maxStrands) {
    // Notice that we can have up to 
    //  r->pd_maxStrands * <no. of supported GIOP versions> strands created.
    //
    s = r->newStrand();
    s->pd_giop_info->version = v;
  }

  if (!s) {
    if (r->oneCallPerConnection() || ndying >= r->pd_maxStrands ) {
      r->waitForIdleStrand();
      goto again;
    }
    else {
      unsigned int n = rand() % r->pd_maxStrands;
      // Pick a random and non-dying strand
      Strand_iterator iter(r,1);
      Strand* q = 0;
      while (n >= 0 && (s = iter())) {
	if (!s->_strandIsDying() && 
	    s->pd_giop_info->version.major == v.major &&
	    s->pd_giop_info->version.minor == v.minor) {
	  n--;
	  if (!q) q = s;
	}
      }
      s = (s) ? s : q;
    }
  }

  s->incrRefCount(1);
  s->setClicks(INT_MAX);

  giopStream* p = s->pd_giop_info->head;
  while (p && p->pd_state != UnUsed) {
    p = p->pd_next;
  }

  if (!p) {
    p = new giopStream(s);
  }

  p->pd_state = OutputIdle;
  p->pd_impl = impl;

  p->pd_output_body_marshaller = 0;

  // the codeset convertors are filled in by the codeset interceptor
  // before a request is marshalled.
  p->pd_tcs_c = 0;
  p->pd_tcs_w = 0;

  return p;
}

giopStream*
giopStream::acquireServer(Strand* s)
{
  // Acquire a giopStream for use as a GIOP Server.
  //
  // Theory of operation:
  //
  // One or more threads may serve the same strand. However, only
  // one thread works on the same giopStream instance. This invariant
  // is enforced by this function.
  //
  // There may be one or more giopStream instances linked to this
  // strand. They can be in one of the following states:
  //
  //  1. Unused*
  //        - the instance has not been claimed by any thread
  //  2. InputFullyBuffered*
  //        - the instance has not been claimed by
  //          any thread and a complete message has
  //          been received.
  //  3. InputPartiallyBuffered**
  //        - a message has been partially received. The thread
  //          that is currently holding the read lock on this
  //          strand may append additional data to this instance.
  //          If no thread is holding the read lock, this instance
  //          can be claimed by the calling thread.
  //  4. Other states***
  //        - the instance is currently in used by another thread.
  //
  //  Note:
  //    *   the instance can be claimed by the calling thread
  //    **  the instance cannot be claimed by the calling thread unless
  //        no other thread is holding the read lock.
  //    *** the instance cannot be claimed by the calling thread
  //
  // This function also acquires the Read lock on the strand if the returned
  // giopStream instance is not InputFullyBuffered. (If it is 
  // InputFullyBuffered, there is no need to read from the strand, hence
  // no need for a Read lock.

  omni_mutex_lock sync(GET_SYNC_MUTEX(s));

  s->incrRefCount(1);
  s->setClicks(INT_MAX);

 again:
  // Scan the list to identify the 1st occurrance of an instance in
  // one of these states: UnUsed, InputFullyBuffered, InputPartiallyBuffered.
  giopStream* p = s->pd_giop_info->head;
  giopStream* up = 0; // 1st giopStream in unused state
  giopStream* fp = 0; // 1st giopStream in InputFullyBuffered state
  giopStream* pp = 0; // 1st giopStream in InputPartiallyBuffered state;
  while (p) {
    switch (p->pd_state) {
    case UnUsed:
      if (!up) up = p;
      break;
    case InputFullyBuffered:
      if (!fp) fp = p;
      break;
    case InputPartiallyBuffered:
      if (!pp) pp = p;
      break;
    default:
      break;
    }
    p = p->pd_next;
  }

  if (fp) {
    // This is good, no need to acquire a Read Lock on the strand because
    // the whole request message is already in buffer.
    p = fp;
  }
  else if (rdLockNonBlocking(s)) {
    // Now we have got the Read Lock, we are *the* thread to read from
    // the strand until we release the lock.
    // Choose a giopStream instance in the following order:
    // 1. InputPartiallyBuffered
    // 2. UnUsed
    // 3. None of the above
    if (pp) {
      p = pp;
    }
    else if (up) {
      up->pd_impl = 0;
      p = up;
    }
    else {
      p = new giopStream(s);
      p->pd_impl = 0;
    }
    p->pd_rdlocked = 1;
  }
  else {
    // Another thread is already reading from the strand, we let it does
    // the work and let it wake us up if there is some work to do.
    sleepOnRdLock(s);
    goto again;
  }

  p->pd_state = InputIdle;
  if (!p->pd_impl) {
#if 1
    p->pd_impl = giopStreamImpl::maxVersion();
#else 
    // Use the following if in future we want to restrict on the server
    // side the protocol version used on a connection to that used in
    // the first request.
    if (s->pd_giop_info->version.major) {
      p->pd_impl = giopStreamImpl::matchVersion(s->pd_giop_info->version);
    }
    else {
      p->pd_impl = giopStreamImpl::maxVersion();
    }
#endif
  }
  p->pd_output_body_marshaller = 0;

  // the codeset convertors are filled in by the codeset interceptor
  // before a request is marshalled.
  p->pd_tcs_c = 0;
  p->pd_tcs_w = 0;

  return p;
}

giopStream*
giopStream::findOnly(Strand* s,CORBA::ULong reqid)
{
  // Caller holds mutex GET_SYNC_MUTEX(s)
  giopStream* p = s->pd_giop_info->head;

  while (p) {
    if (p->pd_request_id == reqid) break;
    p = p->pd_next;
  }
  return p;
}

giopStream*
giopStream::findOrCreateBuffered(Strand* s,CORBA::ULong reqid,
				 CORBA::Boolean& new1 )
{
  omni_mutex_lock sync(GET_SYNC_MUTEX(s));

  giopStream* p = findOnly(s,reqid);

  if (!p) {
    p = new giopStream(s);
    p->pd_state = InputPartiallyBuffered;
    p->pd_request_id = reqid;
    new1 = 1;
  }
  else {
    if (p->pd_state != InputPartiallyBuffered) {
#ifdef PARANOID
      OMNIORB_ASSERT((p->pd_state == OutputRequestCompleted ||
	      p->pd_state == OutputRequest ||
	      p->pd_state == OutputLocateRequest));
#endif
      p->pd_state = InputPartiallyBuffered;
    }
    new1 = 0;
  }

  return p;
}

void
giopStream::changeToFullyBuffered()
{
  OMNIORB_ASSERT(pd_state == InputPartiallyBuffered);

  omni_mutex_lock sync(GET_SYNC_MUTEX(pd_strand));
  pd_state = InputFullyBuffered;
  // Check if any thread is block waiting for a read lock, if so,
  // do a broadcast to wake up any thread that might look at this 
  // giopStream.
  wakeUpRdLock(pd_strand);
}

void
giopStream::transferReplyState(giopStream* dest)
{
  OMNIORB_ASSERT(pd_rdlocked);
  OMNIORB_ASSERT(pd_state == InputReplyHeader || pd_state == InputLocateReplyHeader);
  OMNIORB_ASSERT(dest->pd_state == OutputRequestCompleted ||
	 dest->pd_state == OutputRequest ||
	 dest->pd_state == OutputLocateRequest);

  dest->pd_rdlocked = 1;
  dest->pd_state = (pd_state == InputReplyHeader) ? InputReply 
                                                  : InputLocateReply;

  pd_rdlocked = 0;
  pd_state = OutputRequestCompleted;

  wakeUpRdLock(pd_strand);
}


void
giopStream::release()
{
  CORBA::Boolean remove = pd_impl->isBuffered(this);

  pd_impl->release(this);

  omni_mutex_lock sync(GET_SYNC_MUTEX(pd_strand));

  if (pd_rdlocked) {
    rdUnlock();
  }

  if (pd_wrlocked) { 
    wrUnlock();
  }

  // What clicks no. shall we set the strand clicks.

  Strand* s = pd_strand;

  if (!remove) {
    if (pd_state >= InputIdle) {
      pd_strand->setClicks(StrandScavenger::inIdleTimeLimit());
    }
    else {
      pd_strand->setClicks(StrandScavenger::outIdleTimeLimit());
    }
    pd_state = UnUsed;
  }
  else {
    delete this;
  }

  s->decrRefCount(1);
}

void
giopStream::deleteAll(Strand* s)
{
  giopStreamInfo* info = s->pd_giop_info;

  giopStream* p = info->head;
  while (p) {
    if (p->pd_impl && p->pd_impl->isBuffered(p))
      p->pd_impl->release(p);
    giopStream* q = p->pd_next;
    delete p;
    p = q;
  }
}

void
giopStream::deleteThis()
{
  omni_mutex_lock sync(GET_SYNC_MUTEX(pd_strand));

  if (pd_impl && pd_impl->isBuffered(this))
    pd_impl->release(this);
  delete this;
}

GIOP::Version
giopStream::version()
{
  return pd_impl->version();
}

giopStream::giopStream(Strand* s) : pd_impl(giopStreamImpl::maxVersion()),
				    pd_rdlocked(0),
				    pd_wrlocked(0),
                                    pd_next(0),
                                    pd_strand(s),
				    pd_request_id(0),
				    pd_state(UnUsed),
				    pd_inb_begin(0),
				    pd_outb_begin(0),
				    pd_input_msgbody_received(0),
				    pd_input_msgfrag_to_come(0),
				    pd_input_fragmented(0),
				    pd_input_buffers(0),
				    pd_input_current_buffer(0),
				    pd_input_hdr_end(0),
				    pd_output_msgfrag_size(0),
				    pd_output_msgsent_size(0),
				    pd_output_hdr_end(0),
				    pd_output_at_most_once(0),
				    pd_output_header_marshaller(0),
				    pd_output_body_marshaller(0)
{
  pd_next = s->pd_giop_info->head;
  s->pd_giop_info->head = this;
}

//////////////////////////////////////////////////////////////////////////////
//  The following implements the abstract functions defined in cdrStream.   //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

void
giopStream::get_octet_array(CORBA::Octet* b,int size,
			    omni::alignment_t align)
{
#ifdef PARANOID
  if (pd_state == InputReplyHeader ||
      pd_state == InputLocateReplyHeader ||
      pd_state == InputRequest ||
      pd_state == InputReply ||
      pd_state == InputLocateReply) {
    ;
  }
  else {
    throw omniORB::fatalException(__FILE__,__LINE__,
				  "giopStream::get_char_array() entered with the wrong state.");
  }
#endif

  Strand::sbuf s;
  if (size >= DIRECT_RCV_CUTOFF || 
      (size_t)size > pd_impl->max_input_buffer_size(this,align)) {

    pd_impl->copyInputData(this,b,size,align);
   }
  else {
  again:
    omni::ptr_arith_t p1 = omni::align_to((omni::ptr_arith_t)pd_inb_mkr,
					  align);
    omni::ptr_arith_t p2 = p1 + size;
    if ((void *)p2 > pd_inb_end) {
      fetchInputData(align,size);
      goto again;
    }
    pd_inb_mkr = (void*) p2;
    if (size)
      memcpy(b,(void*)p1,size);
  }
}

void
giopStream::skipInput(_CORBA_ULong size)
{
#ifdef PARANOID
  if (pd_state == InputReplyHeader ||
      pd_state == InputLocateReplyHeader ||
      pd_state == InputRequest ||
      pd_state == InputReply ||
      pd_state == InputLocateReply) {
    ;
  }
  else {
    throw omniORB::fatalException(__FILE__,__LINE__,
				  "giopStream::skipInput() entered with the wrong state.");
  }
#endif

  pd_impl->skipInputData(this,size);
}


CORBA::Boolean
giopStream::checkInputOverrun(CORBA::ULong itemSize, 
			      CORBA::ULong nItems,
			      omni::alignment_t align)
{
#ifdef PARANOID
  if (pd_state == InputReplyHeader ||
      pd_state == InputLocateReplyHeader ||
      pd_state == InputRequest ||
      pd_state == InputReply ||
      pd_state == InputLocateReply) {
    ;
  }
  else {
    throw omniORB::fatalException(__FILE__,__LINE__,
				  "giopStream::checkInputOverrun() entered with the wrong state.");
  }
#endif

  size_t avail = pd_impl->inputRemaining(this);
  omni::ptr_arith_t p1 = omni::align_to((omni::ptr_arith_t)pd_inb_mkr,align);
  p1 += itemSize*nItems;
  if (avail < (size_t)(p1 - (omni::ptr_arith_t)pd_inb_mkr))
    return 0;
  else
    return 1;
}

void
giopStream::fetchInputData(omni::alignment_t align,size_t required)
{
#ifdef PARANOID
  if (pd_state == InputReplyHeader ||
      pd_state == InputLocateReplyHeader ||
      pd_state == InputRequest ||
      pd_state == InputReply ||
      pd_state == InputLocateReply) {
    ;
  }
  else {
    throw omniORB::fatalException(__FILE__,__LINE__,
				  "giopStream::fetchInputData() entered with the wrong state.");
  }
#endif

  pd_impl->getInputData(this,align,required);
}

size_t
giopStream::maxFetchInputData(omni::alignment_t align) const
{
#ifdef PARANOID
  if (pd_state == InputReplyHeader ||
      pd_state == InputLocateReplyHeader ||
      pd_state == InputRequest ||
      pd_state == InputReply ||
      pd_state == InputLocateReply) {
    ;
  }
  else {
    throw omniORB::fatalException(__FILE__,__LINE__,
				  "giopStream::maxFetchInputData() entered with the wrong state.");
  }
#endif

  return pd_impl->max_input_buffer_size((giopStream*)this,align);
}

CORBA::Boolean
giopStream::checkOutputOverrun(CORBA::ULong itemSize,
			       CORBA::ULong nItems,
			       omni::alignment_t align)
{
#ifdef PARANOID
  if (pd_state == OutputRequest ||
      pd_state == OutputLocateRequest ||
      pd_state == OutputReply ||
      pd_state == OutputLocateReply) {
    ;
  }
  else {
    throw omniORB::fatalException(__FILE__,__LINE__,
				  "giopStream::checkOutputOverrun() entered with the wrong state.");
  }
#endif

  size_t avail = pd_impl->outputRemaining(this);
  if (avail != ULONG_MAX) {
    omni::ptr_arith_t p1=omni::align_to((omni::ptr_arith_t)pd_outb_mkr,align);
    p1 += itemSize*nItems;
    if (avail < (size_t)(p1 - (omni::ptr_arith_t)pd_outb_mkr))
      return 0;
  }
  return 1;
}


void
giopStream::put_octet_array(const CORBA::Octet* b, int size,
			    omni::alignment_t align)
{
#ifdef PARANOID
  if (pd_state == OutputRequest ||
      pd_state == OutputLocateRequest ||
      pd_state == OutputReply ||
      pd_state == OutputLocateReply) {
    ;
  }
  else {
    throw omniORB::fatalException(__FILE__,__LINE__,
				  "giopStream::put_char_array() entered with the wrong state.");
  }
#endif

  Strand::sbuf s;
  if (size >= DIRECT_SND_CUTOFF || 
      (size_t)size > pd_impl->max_reserve_buffer_size(this,align)) {

    pd_impl->copyOutputData(this,(void*)b,size,align);
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


CORBA::Boolean
giopStream::reserveOutputSpace(omni::alignment_t align,size_t required)
{
#ifdef PARANOID
  if (pd_state == OutputRequest ||
      pd_state == OutputLocateRequest ||
      pd_state == OutputReply ||
      pd_state == OutputLocateReply) {
    ;
  }
  else {
    throw omniORB::fatalException(__FILE__,__LINE__,
				  "giopStream::reserveOutputSpace() entered with the wrong state.");
  }
#endif

  pd_impl->getReserveSpace(this,align,required);

  // Either we have the space or we've thrown an exception in getReserveSpace.
  return 1;
}

size_t
giopStream::maxReserveOutputSpace(omni::alignment_t align) const
{
#ifdef PARANOID
  if (pd_state == OutputRequest ||
      pd_state == OutputLocateRequest ||
      pd_state == OutputReply ||
      pd_state == OutputLocateReply) {
    ;
  }
  else {
    throw omniORB::fatalException(__FILE__,__LINE__,
				  "giopStream::maxReserveOutputSpace() entered with the wrong state.");
  }
#endif

  size_t reqsize = pd_impl->outputRemaining(this);
  if (reqsize > pd_impl->max_reserve_buffer_size(this,align)) {
    reqsize = pd_impl->max_reserve_buffer_size(this,align);
  }
  return reqsize;
}

CORBA::ULong
giopStream::currentInputPtr() const
{
  return pd_impl->currentInputPtr(this);
}

CORBA::ULong
giopStream::currentOutputPtr() const
{
  return pd_impl->currentOutputPtr(this);
}

//////////////////////////////////////////////////////////////////////////////
//  The following are just direct calls to the real implementations.        //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

CORBA::Long 
giopStream::outputRequestMessageBegin(omniIOR* ior,
				      const char* opname,
				      size_t opnamesize,
				      CORBA::Boolean oneway,
				      CORBA::Boolean response_expected)
{
  {
    omni_mutex_lock sync(GET_SYNC_MUTEX(pd_strand));

#ifdef PARANOID
    if (pd_state != OutputIdle)
      throw omniORB::fatalException(__FILE__,__LINE__,
				    "giopStream::outputRequestMessageBegin() entered with the wrong state.");
#endif

    // Obtain exclusive write access to the strand
    wrLock();

    pd_state = OutputRequest;
    pd_strand->setClicks(StrandScavenger::clientCallTimeLimit());
  }

  return pd_impl->outputRequestMessageBegin(this,ior,opname,opnamesize,
					    oneway,response_expected);
}

CORBA::Long
giopStream::outputLocateMessageBegin(omniIOR* ior)
{
  {
    omni_mutex_lock sync(GET_SYNC_MUTEX(pd_strand));

#ifdef PARANOID
    if (pd_state != OutputIdle)
      throw omniORB::fatalException(__FILE__,__LINE__,
				    "giopStream::outputLocateMessageBegin() entered with the wrong state.");
#endif

    // Obtain exclusive write access to the strand
    wrLock();

    pd_state = OutputLocateRequest;
    pd_strand->setClicks(StrandScavenger::clientCallTimeLimit());
  }

  return pd_impl->outputLocateMessageBegin(this,ior);
}

void
giopStream::outputReplyMessageBegin(giopStream::requestInfo& f,
				    GIOP::ReplyStatusType status)
{
  {
    omni_mutex_lock sync(GET_SYNC_MUTEX(pd_strand));

#ifdef PARANOID
    if (pd_state != InputRequestCompleted)
      throw omniORB::fatalException(__FILE__,__LINE__,
				    "giopStream::outputReplyMessageBegin() entered with the wrong state.");
#endif

    // Obtain exclusive write access to the strand
    wrLock();

    pd_state = OutputReply;
  }

  pd_impl->outputReplyMessageBegin(this,f,status);
}

void
giopStream::outputLocateReplyMessageBegin(giopStream::requestInfo& f,
					  GIOP::LocateStatusType status)
{
  {
    omni_mutex_lock sync(GET_SYNC_MUTEX(pd_strand));

#ifdef PARANOID
    if (pd_state != InputRequestCompleted)
      throw omniORB::fatalException(__FILE__,__LINE__,
				    "giopStream::outputLocateReplyMessageBegin() entered with the wrong state.");
#endif

    // Obtain exclusive write access to the strand
    wrLock();

    pd_state = OutputLocateReply;
  }

  pd_impl->outputLocateReplyMessageBegin(this,f,status);
}

void
giopStream::outputMessageEnd()
{
#ifdef PARANOID
  if (pd_state == OutputRequest || 
      pd_state == OutputReply ||
      pd_state == OutputLocateRequest ||
      pd_state == OutputLocateReply) {
    ;
  }
  else {
    throw omniORB::fatalException(__FILE__,__LINE__,
				  "giopStream::outputMessageEnd() entered with the wrong state.");
  }
#endif

  pd_impl->outputMessageEnd(this);

  pd_output_body_marshaller = 0;

  {
    omni_mutex_lock sync(GET_SYNC_MUTEX(pd_strand));

    // Release exclusive write access to the strand
  

    if (pd_state == OutputRequest || 
	pd_state == OutputLocateRequest) {
      pd_state = OutputRequestCompleted;
    }
    else if (pd_state == OutputReply ||
	     pd_state == OutputLocateReply) {
      pd_state = giopStream::InputIdle;
      pd_strand->setClicks(StrandScavenger::inIdleTimeLimit());
    }

    wrUnlock();

  }
}

void
giopStream::SendMsgErrorMessage()
{
  {
    omni_mutex_lock sync(GET_SYNC_MUTEX(pd_strand));

#ifdef PARANOID
    if (pd_state == OutputRequest || 
	pd_state == OutputReply ||
	pd_state == OutputLocateRequest ||
	pd_state == OutputLocateReply) {

      throw omniORB::fatalException(__FILE__,__LINE__,
				    "giopStream::SendMsgErrorMessage() entered with the wrong state.");
    }
#endif

    // Obtain exclusive write access to the strand
    wrLock();
  }

  pd_impl->SendMsgErrorMessage(this);

  {
    omni_mutex_lock sync(GET_SYNC_MUTEX(pd_strand));

    // Release exclusive write access to the strand
    wrUnlock();
  }
}

void
giopStream::outputMessageBodyMarshaller(giopMarshaller& m)
{
#ifdef PARANOID
  if (pd_state != OutputIdle &&
      pd_state != InputRequestCompleted) {
    throw omniORB::fatalException(__FILE__,__LINE__,
				  "giopStream::outputMessageBodyMarshaller() entered with the wrong state.");
  }
#endif
  pd_output_body_marshaller = &m;
}

GIOP::MsgType
giopStream::inputRequestMessageBegin(giopStream::requestInfo& f)
{
  {
    omni_mutex_lock sync(GET_SYNC_MUTEX(pd_strand));

#ifdef PARANOID
    if (pd_state != InputIdle)
      throw omniORB::fatalException(__FILE__,__LINE__,
				    "giopStream::inputRequestMessageBegin() entered with the wrong state.");
#endif

    // No need to acquire read lock because:
    // 1. it is not necessary because the input message has been fully 
    //    buffered.
    // 2. The caller has acquired the instance from acquireServer() which
    //    has already obtained the read lock.
    if (!pd_impl->isBuffered(this))
      OMNIORB_ASSERT(pd_rdlocked);

    pd_strand->setClicks(StrandScavenger::inIdleTimeLimit());
    pd_state = InputRequest;
  }
    
   GIOP::MsgType rc = pd_impl->inputRequestMessageBegin(this,f,0);

  {
    omni_mutex_lock sync(GET_SYNC_MUTEX(pd_strand));
    if (!streamInfo()->version.major) {
      // Set the version no. based on the first request message.
      streamInfo()->version = pd_impl->version();
    }
    pd_strand->setClicks(StrandScavenger::serverCallTimeLimit());
  }

  return rc;
}

// Client side - receiving a reply
//
// Theory of operation:
//
// A reply can either be a GIOP Reply or a GIOP LocalReply message.  To
// indicate that the client is ready to receive a reply, the client invokes
// inputReplyMessageBegin() or inputLocateReplyMessageBegin().
//
// Depending on the attribute of the rope, the acquireClient() method,
// may allow more than 1 call to progress on the same strand at the same
// time. 
// In other words, multiple client threads may call input*ReplyMessageBegin()
// at the same time. Even though each thread is invoking the member function of
// different giopStream instances, all the instances share the same strand.
// Proper coordination must be done to ensure that each reply is routed to
// the right client.
// 
// Several things have to do right:
// 
// 1. Only one thread should be allowed to proceed to read from the strand
//    at any time. This is done by requiring the read lock to succeed.
//
// 2. The thread that is currently reading from the strand may find that
//    the replies coming in are not for its own request but for some other
//    threads. Therefore: 
//
//      a) the thread must find the giopStream the reply is destined to
//         and either:
//            i) in GIOP 1.2, buffer the reply (fragment) in the giopStream.
//            ii)in GIOP 1.0 and 1.1, signal the other thread that is using
//               the giopStream to wake up and unmarshal the reply.
//
//      b) with GIOP 1.0 and 1.1, the matter is made complicated by the
//         fact that it is not possible to know the identity of the reply
//         until the whole reply header has been unmarshalled. When the thread
//         discovers that the reply is not for its own request, it has to find
//         the giopStream and transfer the unmarshalled header to that
//         instance. It then has to transfer the read lock to the other thread
//         and signal it to unmarshal the rest of the reply message from the
//         strand. The thread can then block waiting for its turn to reaquire
//         the read lock.
//
//   The version specific treatment of reply messages is encapsulated into
//   the specific protocol implementation of the inputReplyMessageBegin()
//   and inputLocateReplyMessageBegin(). These methods returns a boolean
//   value to indicate if the header has successfully (value == TRUE)
//   unmarshalled. If not, value == FALSE, the caller has to try again.
//

GIOP::ReplyStatusType
giopStream::inputReplyMessageBegin()
{
  // To understand how this works, read the notes on "Client side - 
  // receiving a reply" above.
  GET_SYNC_MUTEX(pd_strand).lock();

 again:
  switch (pd_state) {
  case OutputRequestCompleted:
    {
      if (pd_rdlocked || rdLockNonBlocking(pd_strand)) {
	pd_rdlocked = 1;

	CORBA::Boolean rc;

	GET_SYNC_MUTEX(pd_strand).unlock();
	pd_state = InputReplyHeader;

	rc = pd_impl->inputReplyMessageBegin(this);

	GET_SYNC_MUTEX(pd_strand).lock();
	if (rc) {
	  pd_state = InputReply;
	  GET_SYNC_MUTEX(pd_strand).unlock();
	  return pd_reply_status.replyStatus;
	}
	else if (pd_state != OutputRequestCompleted) {
	  // Remember, the side effect of pd_impl->input..(), if it
	  // returns 0 is to reset the state of this giopStream back to
	  // OutputRequestCompleted. However, when we come to examine the
	  // state again here, it could have been changed under our feet!
	  // This could happen if between the time when pd_impl->input..()
	  // returns and we got the mutex lock, another thread has took the
	  // read lock and has read the reply message for this thread. In
	  // that case, the other thread would have changed the state
	  // of this giopStream to InputReply. No point to sleep on read lock,
	  // just go and try again.
	  goto again;
	}
      }
      OMNIORB_ASSERT(pd_rdlocked == 0);
      sleepOnRdLock(pd_strand);
      goto again;
      break;
    }
  case InputReply:
    {
      OMNIORB_ASSERT(pd_rdlocked);
      GET_SYNC_MUTEX(pd_strand).unlock();
      return pd_reply_status.replyStatus;
    }
  case InputPartiallyBuffered:
    {
      if (pd_rdlocked || rdLockNonBlocking(pd_strand)) {
	pd_rdlocked = 1;
	pd_state = InputReply;

	CORBA::Boolean rc;

	GET_SYNC_MUTEX(pd_strand).unlock();

	rc = pd_impl->inputReplyMessageBegin(this);

	OMNIORB_ASSERT(rc!=0);
	return pd_reply_status.replyStatus;
      }
      sleepOnRdLock(pd_strand);
      goto again;
    }
    break;
  case InputFullyBuffered:
    {
      OMNIORB_ASSERT(pd_rdlocked == 0);
      pd_state = InputReply;
      GET_SYNC_MUTEX(pd_strand).unlock();
      CORBA::Boolean rc = pd_impl->inputReplyMessageBegin(this);
      OMNIORB_ASSERT(rc!=0);
      return pd_reply_status.replyStatus;
    }
  default:
    {
      throw omniORB::fatalException(__FILE__,__LINE__,
				    "giopStream::inputReplyMessageBegin() entered with the wrong state.");
#ifdef NEED_DUMMY_RETURN
      // Not reached.
      return pd_reply_status.replyStatus;
#endif
    }
  }
}

GIOP::LocateStatusType
giopStream::inputLocateReplyMessageBegin()
{
  // To understand how this works, read the notes on "Client side - 
  // receiving a reply" above.
  GET_SYNC_MUTEX(pd_strand).lock();

 again:
  switch (pd_state) {
  case OutputRequestCompleted:
    {
      if (pd_rdlocked || rdLockNonBlocking(pd_strand)) {
	pd_rdlocked = 1;

	CORBA::Boolean rc;

	GET_SYNC_MUTEX(pd_strand).unlock();
	pd_state = InputLocateReplyHeader;

	rc = pd_impl->inputLocateReplyMessageBegin(this);

	GET_SYNC_MUTEX(pd_strand).lock();
	if (rc) {
	  pd_state = InputLocateReply;
	  GET_SYNC_MUTEX(pd_strand).unlock();
	  return pd_reply_status.locateReplyStatus;
	}
	else if (pd_state != OutputRequestCompleted) {
	  // Remember, the side effect of pd_impl->input..(), if it
	  // returns 0 is to reset the state of this giopStream back to
	  // OutputRequestCompleted. However, when we come to examine the
	  // state again here, it could have been changed under our feet!
	  // This could happen if between the time when pd_impl->input..()
	  // returns and we got the mutex lock, another thread has took the
	  // read lock and has read the reply message for this thread. In
	  // that case, the other thread would have changed the state
	  // of this giopStream to InputReply. No point to sleep on read lock,
	  // just go and try again.
	  goto again;
	}
      }
      OMNIORB_ASSERT(pd_rdlocked==0);
      sleepOnRdLock(pd_strand);
      goto again;
      break;
    }
  case InputLocateReply:
    {
      OMNIORB_ASSERT(pd_rdlocked);
      GET_SYNC_MUTEX(pd_strand).unlock();
      return pd_reply_status.locateReplyStatus;
    }
  case InputPartiallyBuffered:
    {
      if (pd_rdlocked || rdLockNonBlocking(pd_strand)) {
	pd_rdlocked = 1;
	pd_state = InputLocateReply;

	CORBA::Boolean rc;

	GET_SYNC_MUTEX(pd_strand).unlock();

	rc = pd_impl->inputLocateReplyMessageBegin(this);

	OMNIORB_ASSERT(rc!=0);
	return pd_reply_status.locateReplyStatus;
      }
      sleepOnRdLock(pd_strand);
      goto again;
    }
    break;
  case InputFullyBuffered:
    {
      OMNIORB_ASSERT(pd_rdlocked == 0);
      pd_state = InputLocateReply;
      GET_SYNC_MUTEX(pd_strand).unlock();
      CORBA::Boolean rc = pd_impl->inputLocateReplyMessageBegin(this);
      OMNIORB_ASSERT(rc!=0);
      return pd_reply_status.locateReplyStatus;
    }
  default:
    {
    throw omniORB::fatalException(__FILE__,__LINE__,
				  "giopStream::inputReplyMessageBegin() entered with the wrong state.");
#ifdef NEED_DUMMY_RETURN
      // Not reached.
      return pd_reply_status.locateReplyStatus;
#endif
    }
  }
}

void
giopStream::inputMessageEnd(CORBA::Boolean disgard,CORBA::Boolean error)
{

#ifdef PARANOID
  if (pd_state == InputReplyHeader ||
      pd_state == InputLocateReplyHeader || // Do we ever call this function in
					    // this state?
      pd_state == InputRequest ||
      pd_state == InputReply || 
      pd_state == InputLocateReply) {
    ;
  }
  else {
    throw omniORB::fatalException(__FILE__,__LINE__,
				  "giopStream::inputMessageEnd() entered with the wrong state.");
  }
#endif

  pd_impl->inputMessageEnd(this,disgard,error);

  {
    omni_mutex_lock sync(GET_SYNC_MUTEX(pd_strand));

    // Release exclusive read access to the strand
    if (!pd_impl->isBuffered(this)) rdUnlock();

    if (pd_state == InputRequest) {
      pd_state = InputRequestCompleted;
    }
    else if (pd_state == InputReply || pd_state == InputLocateReply) {
      pd_strand->setReUseFlag();
      pd_state = OutputIdle;
      pd_strand->setClicks(INT_MAX);
    }
  }
}

CORBA::Boolean
giopStream::terminalError() const
{
  return pd_strand->_strandIsDying();
}

void
giopStream::setTerminalError()
{
  pd_strand->_setStrandIsDying();
}


CORBA::Boolean 
giopStream::startSavingInputMessageBody()
{
#ifdef PARANOID
  if (pd_state == InputReplyHeader ||
      pd_state == InputLocateReplyHeader ||
      pd_state == InputRequest || 
      pd_state == InputReply ) {
    ;
  }
  else {
    throw omniORB::fatalException(__FILE__,__LINE__,
	  "giopStream::startSavingInputMessageBody() entered with the wrong state.");
  }
#endif
  return pd_impl->startSavingInputMessageBody(this);
}

CORBA::Boolean
giopStream::prepareCopyMessageBodyFrom(giopStream& s)
{
#ifdef PARANOID
  if (pd_state == OutputIdle || pd_state == InputRequestCompleted) {
    ;
  }
  else {
    throw omniORB::fatalException(__FILE__,__LINE__,
	  "giopStream::prepareCopyMessageBodyFrom() entered with the wrong state.");
  }
#endif

  return pd_impl->prepareCopyMessageBodyFrom(this,s);
}

void
giopStream::copyMessageBodyFrom(giopStream& s)
{
#ifdef PARANOID
  if (pd_state == OutputRequest || pd_state == OutputReply) {
    ;
  }
  else {
    throw omniORB::fatalException(__FILE__,__LINE__,
	  "giopStream::copyMessageBodyFrom() entered with the wrong state.");
  }
#endif

  pd_impl->copyMessageBodyFrom(this,s);
}




//////////////////////////////////////////////////////////////////////////////
//  Read/Write locks                                                        //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

omni_mutex& giopStream::getSyncMutex()
{
  return GET_SYNC_MUTEX(pd_strand);
}

void
giopStream::wrLock() {
  OMNIORB_ASSERT(!pd_wrlocked); 

  while (pd_strand->pd_wr_nwaiting < 0) {
    pd_strand->pd_wr_nwaiting--;
    pd_strand->pd_wrcond.wait();
    if (pd_strand->pd_wr_nwaiting >= 0)
      pd_strand->pd_wr_nwaiting--;
    else
      pd_strand->pd_wr_nwaiting++;
  }
  pd_strand->pd_wr_nwaiting = -pd_strand->pd_wr_nwaiting - 1;

  pd_wrlocked = 1;
}

void
giopStream::wrUnlock() {
  OMNIORB_ASSERT(pd_wrlocked); 

  OMNIORB_ASSERT(pd_strand->pd_wr_nwaiting < 0);
  pd_strand->pd_wr_nwaiting = -pd_strand->pd_wr_nwaiting - 1;
  if (pd_strand->pd_wr_nwaiting > 0)
    pd_strand->pd_wrcond.signal();

  pd_wrlocked = 0; 
}

void
giopStream::rdLock() {
  OMNIORB_ASSERT(!pd_rdlocked);

  while (pd_strand->pd_rd_nwaiting < 0) {
    pd_strand->pd_rd_nwaiting--;
    pd_strand->pd_rdcond.wait();
    if (pd_strand->pd_rd_nwaiting >= 0)
      pd_strand->pd_rd_nwaiting--;
    else
      pd_strand->pd_rd_nwaiting++;
  }
  pd_strand->pd_rd_nwaiting = -pd_strand->pd_rd_nwaiting - 1;

  pd_rdlocked = 1;

}

void
giopStream::rdUnlock() {
  OMNIORB_ASSERT(pd_rdlocked);

  OMNIORB_ASSERT(pd_strand->pd_rd_nwaiting < 0);
  pd_strand->pd_rd_nwaiting = -pd_strand->pd_rd_nwaiting - 1;
  if (pd_strand->pd_rd_nwaiting > 0)
    pd_strand->pd_rdcond.signal();

  pd_rdlocked = 0;

}


CORBA::Boolean
giopStream::rdLockNonBlocking(Strand* s)
{
  if (s->pd_rd_nwaiting < 0)
    return 0;
  else {
    s->pd_rd_nwaiting = -s->pd_rd_nwaiting - 1;
    return 1;
  }
}

void
giopStream::sleepOnRdLock(Strand* s)
{
  if (s->pd_rd_nwaiting < 0) {
    s->pd_rd_nwaiting--;
    s->pd_rdcond.wait();
    if (s->pd_rd_nwaiting >= 0)
      s->pd_rd_nwaiting--;
    else
      s->pd_rd_nwaiting++;
  }
}

void
giopStream::wakeUpRdLock(Strand* s)
{
  if (s->pd_rd_nwaiting < 0) {
#if 1
    s->pd_rdcond.broadcast();
#else
    // Do this if the platform's condition variable broadcast does not
    // work.
    for (int i= -s->pd_rd_nwaiting; i > 0; i--)
      s->pd_rdcond.signal();
#endif
  }
}


//////////////////////////////////////////////////////////////////////////////
//  giopStreamImpl                                                          //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

static giopStreamImpl* implHead = 0;
static giopStreamImpl* implMax = 0;
static GIOP::Version   implMaxVersion = { 255,255 };

giopStreamImpl::giopStreamImpl(const GIOP::Version& v)
{
  pd_version.major = v.major;
  pd_version.minor = v.minor;

  // Insert implementation to the last of the queue
  giopStreamImpl** pp = &implHead;
  while (*pp) pp = &((*pp)->pd_next);
  this->pd_next = 0;
  *pp = this;

  if (implMax) {
    CORBA::UShort ver1, ver2;
    ver1 = ((CORBA::UShort)v.major << 8) + v.minor;
    ver2 = ((CORBA::UShort)implMax->pd_version.major << 8) +
           implMax->pd_version.minor;
    if (ver1 <= ver2) return;
  }
  implMax = this;
}

giopStreamImpl::~giopStreamImpl()
{
}

giopStreamImpl*
giopStreamImpl::matchVersion(const GIOP::Version& v)
{
  giopStreamImpl* p = implHead;

  while (p) {
    if (p->pd_version.major == v.major && p->pd_version.minor == v.minor)
      break;
    p = p->pd_next;
  }
  return p;
}


giopStreamImpl*
giopStreamImpl::maxVersion()
{
  return implMax;
}

void
giopStreamImpl::release(giopStream* g)
{
  size_t leftover = (omni::ptr_arith_t)g->pd_inb_end - 
                    (omni::ptr_arith_t)g->pd_inb_mkr;
  if (g->pd_rdlocked && leftover && !g->terminalError()) {
      // XXX This should not be done if this is a buffered giopStream
    g->pd_strand->giveback_received(leftover);
  }
  g->pd_inb_end = g->pd_inb_mkr;

  leftover = (omni::ptr_arith_t)g->pd_outb_end - 
             (omni::ptr_arith_t)g->pd_outb_mkr;
  if (g->pd_wrlocked && leftover && !g->terminalError()) {
    // XXX This should not be done if this is a buffered giopStream
    g->pd_strand->giveback_reserved(leftover);
  }
  g->pd_outb_end = g->pd_outb_mkr;

}

CORBA::Boolean
giopStreamImpl::isBuffered(giopStream* g)
{
  return 0;
}

//////////////////////////////////////////////////////////////////////////////
//  giopStream::requestInfo                                                 //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

void
giopStream::
requestInfo::unmarshalIORAddressingInfo(cdrStream& s)
{
  GIOP::AddressingDisposition vp;
  CORBA::ULong   vl;

  resetKey();

  vp <<= s;
  if (vp == GIOP::KeyAddr) {
    vl <<= s;
    if (!s.checkInputOverrun(1,vl)) {
      throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);
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
      PTRACE("unmarshalRequestHeader","MARSHAL exception (corrupted targetAddress");
      throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);
    }

    // Extract the repository ID and the profile(s) from the addressinginfo.
    char* repoid = ta.ior.type_id._retn();
    CORBA::ULong nprofiles = ta.ior.profiles.length();
    IOP::TaggedProfile* profiles = ta.ior.profiles.get_buffer(1);
    omniIOR_var ior = new omniIOR(repoid,profiles,nprofiles,
				  ta.selected_profile_index);

    Rope* rope;
    CORBA::Boolean is_local;

    if (ropeFactory::iorToRope(ior,rope,is_local)) {
      if (is_local) {
	keysize(ior->iiop.object_key.length());
	memcpy((void*)key(),
	       (void*)ior->iiop.object_key.get_buffer(),
	       keysize());
      }
      else {
	rope->decrRefCount();
      }
    }

    // Return the repository ID and the profile(s) back to addressinginfo.
    ta.ior.type_id = ior->repositoryID._retn();
    ta.ior.profiles.replace(nprofiles,nprofiles,
			    ior->iopProfiles->get_buffer(1),1);

    // Reach here either we have got the key of the target object
    // or we have the target address info in targetAddress().
    //
    if (keysize() < 0) {
      // We couldn't decode the target address to a local object key. Unless
      // an interceptor can decode it further, this request will be rejected.
      PTRACE("unmarshalRequestHeader","ProfileAddr or ReferenceAddr addressing to unknown target.");
    }
  }
}

/////////////////////////////////////////////////////////////////////////////
//            Module initialiser                                           //
/////////////////////////////////////////////////////////////////////////////

extern omniInitialiser& omni_giopStream10_initialiser_;
extern omniInitialiser& omni_giopStream11_initialiser_;
extern omniInitialiser& omni_giopStream12_initialiser_;

class omni_giopStreamImpl_initialiser : public omniInitialiser {
public:

  void attach() {
    omni_giopStream10_initialiser_.attach();
    if (implMaxVersion.minor >= 1)
      omni_giopStream11_initialiser_.attach();
    if (implMaxVersion.minor >= 2)
      omni_giopStream12_initialiser_.attach();
    {
      if (omniORB::trace(25)) {
	GIOP::Version v = giopStreamImpl::maxVersion()->version();
	omniORB::logger log;
	log << " Maximum supported GIOP version is " << (int)v.major 
	    << "." << (int)v.minor << "\n";
      }
    }
  }

  void detach() { 
    omni_giopStream10_initialiser_.attach();
    if (implMaxVersion.minor >= 1)
      omni_giopStream11_initialiser_.detach();
    if (implMaxVersion.minor >= 2)
      omni_giopStream12_initialiser_.detach();
  }

};

static omni_giopStreamImpl_initialiser initialiser;

omniInitialiser& omni_giopStreamImpl_initialiser_ = initialiser;

void
omniORB::maxGIOPVersion(CORBA::Char& major, CORBA::Char& minor)
{
  giopStreamImpl* maximpl = giopStreamImpl::maxVersion();

  if (maximpl) {
    GIOP::Version v = maximpl->version();
    major = v.major;
    minor = v.minor;
  }
  else {
    implMaxVersion.major = (major > 0) ? major : 1;
    implMaxVersion.minor = minor;
  }
}
