// -*- Mode: C++; -*-
//                            Package   : omniORB2
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
  Revision 1.1.2.1  1999/09/15 20:37:27  sll
  *** empty log message ***

*/

#include <omniORB2/CORBA.h>
#include <scavenger.h>
#include <giopStreamImpl.h>

#define DIRECT_RCV_CUTOFF 1024
#define DIRECT_SND_CUTOFF 8192

#define STRAND_WRITE_LOCK()  assert(!pd_wrlocked); pd_wrlocked = 1; WrLock(1)
#define STRAND_WRITE_UNLOCK() assert(pd_wrlocked); pd_wrlocked = 0; WrUnlock(1)
#define STRAND_READ_LOCK()   assert(!pd_rdlocked); pd_rdlocked = 1; RdLock(1)
#define STRAND_READ_UNLOCK()   assert(pd_rdlocked); pd_rdlocked = 0; RdUnlock(1)

#define PARANOID

giopStream::~giopStream()
{
#ifdef PARANOID
  assert(!pd_rdlocked && !pd_wrlocked);
  assert(pd_inb_mkr == pd_inb_end);
  assert(pd_outb_mkr == pd_outb_end);
  assert(pd_nwaiting == 0);
#endif
}

giopStream*
giopStream::acquire(Rope* r, GIOP::Version v)
{
  // GIOP version 1.0, 1.1, 1.2
  // Role - GIOP Client
  // must acquire mutual exclusion on the strand return by the Rope
  // do not allow two calls in progress on the same strand.
  // Only one giopStream instance per strand.

  // GIOP 1.2 BiDir - rope marks this as BiDir.
  // allows more than one client to use the same strand concurrently
  // XXX Not supported yet.

  omni_mutex_lock sync(Strand::Sync::getMutex(r));

  Strand* s = Strand::Sync::getStrand(r);

  giopStream* p = (giopStream*) Strand::Sync::getSync(s);
  if (!p) {
    p = new giopStream(s);
  }

  // acquire mutual exclusion
  // Do not allow more than 1 call to progress on this strand at any moment.
  while (p->pd_nwaiting < 0) {
    p->pd_nwaiting--;
    p->pd_cond.wait();
    if (p->pd_nwaiting >= 0)
      p->pd_nwaiting--;
    else
      p->pd_nwaiting++;
  }
  p->pd_nwaiting = -p->pd_nwaiting - 1;

  p->pd_state = OutputIdle;
  p->pd_clicks = INT_MAX;
  p->pd_impl = giopStreamImpl::matchVersion(v);
  if (!p->pd_impl) p->pd_impl = giopStreamImpl::maxVersion();

  if (p->terminalError()) {
    // Well, we park on this giopStream and expect to be able to use it
    // Now we are told it has developed a terminal failure.
    // We throw a TRANSIENT and let the upper level deal with a retry.
    p->release();
    throw CORBA::TRANSIENT(0,CORBA::COMPLETED_NO);
  }

  return p;
}

giopStream*
giopStream::acquire(Strand* s)
{
  // GIOP version 1.0, 1.1, 1.2
  //   Role - GIOP Server
  //   Do not acquire mutual exclusion on return.
  //   More than one giopStream may park on the same strand.
  //   No 2 thread works on the same giopStream instance.
  //   May have more than one giopStream instance per strand.
  //
  // GIOP 1.2 BiDir- Never enter through this route!!
  //
  //  pd_state = InputIdle;


  omni_mutex_lock sync(Strand::Sync::getMutex(s));

  giopStream* p = (giopStream*) Strand::Sync::getSync(s);
  if (!p || p->pd_state != UnUsed) {
    p = new giopStream(s);
  }
  p->pd_impl = giopStreamImpl::maxVersion();
  p->pd_state = InputIdle;
  p->pd_clicks = INT_MAX;
  return p;
}

void
giopStream::release()
{
  // Must distinguish if this is GIOP Server or GIOP Client
  // If GIOP client, must release mutual exclusion and let another
  // other threads blocking in acquire to have a go.
  //
  // If GIOP server, may release this giopStream instance if it
  // is not the only one park on the strand.


  pd_impl->release(this);

  omni_mutex_lock sync(Strand::Sync::getMutex(pd_strand));

  if (pd_rdlocked) {
    STRAND_READ_UNLOCK();
  }

  if (pd_wrlocked) { 
    STRAND_WRITE_UNLOCK();
  }

  if (pd_nwaiting < 0) {
    pd_nwaiting = -pd_nwaiting - 1;
    if (pd_nwaiting > 0) {
      pd_cond.signal();
      pd_clicks = INT_MAX;
    }
    else {
      pd_clicks = StrandScavenger::outIdleTimeLimit();
    }
  }
  else {
    pd_clicks = StrandScavenger::inIdleTimeLimit();
  }

  // Error, we delete this object when it is safe to do so.
  if (terminalError()) {
    if (pd_nwaiting == 0)
      delete this;
  }
  else {
    pd_state = UnUsed;
  }
}

giopStream::giopStream(Strand* s) : Strand::Sync(s),
				    pd_impl(giopStreamImpl::maxVersion()),
				    pd_rdlocked(0),
				    pd_wrlocked(0),
				    pd_cond(&Strand::Sync::getMutex(s)),
				    pd_nwaiting(0),
				    pd_clicks(INT_MAX),
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
				    pd_marshaller(0),
				    pd_request_id(0),
				    pd_state(UnUsed)
{
}

//////////////////////////////////////////////////////////////////////////////
//  The following implements the abstract functions defined in cdrStream.   //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

void
giopStream::get_char_array(CORBA::Char* b,int size,
			   omni::alignment_t align)
{
#ifdef PARANOID
  if (pd_state == InputRequest ||
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
  if (pd_state == InputRequest ||
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
  if (pd_state == InputRequest ||
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
  if (pd_state == InputRequest ||
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
  if (pd_state == InputRequest ||
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
giopStream::put_char_array(const CORBA::Char* b, int size,
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
giopStream::outputRequestMessageBegin(GIOPObjectInfo* f,
				      const char* opname,
				      size_t opnamesize,
				      CORBA::Boolean oneway,
				      CORBA::Boolean response_expected)
{
  {
    omni_mutex_lock sync(Strand::Sync::getMutex(pd_strand));

#ifdef PARANOID
    if (pd_state != OutputIdle)
      throw omniORB::fatalException(__FILE__,__LINE__,
				    "giopStream::outputRequestMessageBegin() entered with the wrong state.");
#endif

    // Obtain exclusive write access to the strand
    STRAND_WRITE_LOCK();

    pd_state = OutputRequest;
    pd_clicks = StrandScavenger::clientCallTimeLimit();
  }

  return pd_impl->outputRequestMessageBegin(this,f,opname,opnamesize,
					    oneway,response_expected);
}

CORBA::Long
giopStream::outputLocateMessageBegin(GIOPObjectInfo* f)
{
  {
    omni_mutex_lock sync(Strand::Sync::getMutex(pd_strand));

#ifdef PARANOID
    if (pd_state != OutputIdle)
      throw omniORB::fatalException(__FILE__,__LINE__,
				    "giopStream::outputLocateMessageBegin() entered with the wrong state.");
#endif

    // Obtain exclusive write access to the strand
    STRAND_WRITE_LOCK();

    pd_state = OutputLocateRequest;
    pd_clicks = StrandScavenger::clientCallTimeLimit();
  }

  return pd_impl->outputLocateMessageBegin(this,f);
}

void
giopStream::outputReplyMessageBegin(giopStream::requestInfo& f,
				    GIOP::ReplyStatusType status)
{
  {
    omni_mutex_lock sync(Strand::Sync::getMutex(pd_strand));

#ifdef PARANOID
    if (pd_state != InputRequestCompleted)
      throw omniORB::fatalException(__FILE__,__LINE__,
				    "giopStream::outputReplyMessageBegin() entered with the wrong state.");
#endif

    // Obtain exclusive write access to the strand
    STRAND_WRITE_LOCK();

    pd_state = OutputReply;
  }

  pd_impl->outputReplyMessageBegin(this,f,status);
}

void
giopStream::outputLocateReplyMessageBegin(giopStream::requestInfo& f,
					  GIOP::LocateStatusType status)
{
  {
    omni_mutex_lock sync(Strand::Sync::getMutex(pd_strand));

#ifdef PARANOID
    if (pd_state != InputRequestCompleted)
      throw omniORB::fatalException(__FILE__,__LINE__,
				    "giopStream::outputLocateReplyMessageBegin() entered with the wrong state.");
#endif

    // Obtain exclusive write access to the strand
    STRAND_WRITE_LOCK();

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

  {
    omni_mutex_lock sync(Strand::Sync::getMutex(pd_strand));

    // Release exclusive write access to the strand
  

    if (pd_state == OutputRequest || 
	pd_state == OutputLocateRequest) {
      pd_state = OutputRequestCompleted;
    }
    else if (pd_state == OutputReply ||
	     pd_state == OutputLocateReply) {
      pd_state = giopStream::InputIdle;
      pd_clicks = StrandScavenger::inIdleTimeLimit();
    }

    STRAND_WRITE_UNLOCK();

  }
}

void
giopStream::SendMsgErrorMessage()
{
  {
    omni_mutex_lock sync(Strand::Sync::getMutex(pd_strand));

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
    STRAND_WRITE_LOCK();
  }

  pd_impl->SendMsgErrorMessage(this);

  {
    omni_mutex_lock sync(Strand::Sync::getMutex(pd_strand));

    // Release exclusive write access to the strand
    STRAND_WRITE_UNLOCK();
  }
}

GIOP::MsgType
giopStream::inputRequestMessageBegin(giopStream::requestInfo& f)
{
  {
    omni_mutex_lock sync(Strand::Sync::getMutex(pd_strand));

#ifdef PARANOID
    if (pd_state != InputIdle)
      throw omniORB::fatalException(__FILE__,__LINE__,
				    "giopStream::inputRequestMessageBegin() entered with the wrong state.");
#endif

    // Obtain exclusive read access to the strand
    STRAND_READ_LOCK();

    pd_clicks = StrandScavenger::inIdleTimeLimit();
    pd_state = InputRequest;
  }
    
   GIOP::MsgType rc = pd_impl->inputRequestMessageBegin(this,f,0);

  {
    omni_mutex_lock sync(Strand::Sync::getMutex(pd_strand));
    pd_clicks = StrandScavenger::serverCallTimeLimit();
  }

  return rc;
}

GIOP::ReplyStatusType
giopStream::inputReplyMessageBegin(CORBA::ULong reqid)
{
  {
    omni_mutex_lock sync(Strand::Sync::getMutex(pd_strand));

#ifdef PARANOID
    if (pd_state != OutputRequestCompleted)
      throw omniORB::fatalException(__FILE__,__LINE__,
				    "giopStream::inputReplyMessageBegin() entered with the wrong state.");
#endif

    // Obtain exclusive read access to the strand
    STRAND_READ_LOCK();

    pd_state = InputReply;
  }

  return pd_impl->inputReplyMessageBegin(this,reqid,0);
}

GIOP::LocateStatusType
giopStream::inputLocateReplyMessageBegin(CORBA::ULong reqid)
{
  {
    omni_mutex_lock sync(Strand::Sync::getMutex(pd_strand));

#ifdef PARANOID
    if (pd_state != OutputRequestCompleted)
      throw omniORB::fatalException(__FILE__,__LINE__,
				    "giopStream::inputReplyMessageBegin() entered with the wrong state.");
#endif

    // Obtain exclusive read access to the strand
    STRAND_READ_LOCK();

    pd_state = InputLocateReply;
  }

  return pd_impl->inputLocateReplyMessageBegin(this,reqid,0);
}

void
giopStream::inputMessageEnd(CORBA::Boolean disgard,CORBA::Boolean error)
{

#ifdef PARANOID
  if (pd_state == InputRequest ||
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
    omni_mutex_lock sync(Strand::Sync::getMutex(pd_strand));

    // Release exclusive read access to the strand
    STRAND_READ_UNLOCK();

    if (pd_state == InputRequest) {
      pd_state = InputRequestCompleted;
    }
    else if (pd_state == InputReply || pd_state == InputLocateReply) {
      pd_strand->setReUseFlag();
      pd_state = OutputIdle;
      pd_clicks = INT_MAX;
    }
  }
}

CORBA::Boolean
giopStream::terminalError() const
{
  return pd_impl->terminalError(this);
}

void
giopStream::setTerminalError()
{
  pd_impl->terminalError(this);
}


CORBA::Boolean 
giopStream::startSavingInputMessageBody()
{
#ifdef PARANOID
  if (pd_state == InputRequest || pd_state == InputReply) {
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
//  Strand::Sync virtual functions                                          //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

CORBA::Boolean
giopStream::garbageCollect()
{
  giopStream* p = (giopStream*) Strand::Sync::getSync(pd_strand);

  CORBA::Boolean rc = 1;

  while (p) {

    if (--p->pd_clicks < 0) {
      switch (p->pd_state) {
      case UnUsed:
	{
	  giopStream* q = p;
	  p = (giopStream*) p->pd_next;
	  delete q;
	  continue;
	}	
      case OutputIdle:
      case InputIdle:
      case InputRequest:
	{
	  // do nothing. Default rc value is to return TRUE.
	  break;
	}
      default:
	{
	  rc = 0;     // Do not garbage collect this strand.
	  break;
	}
      }
    }
    else {
      rc = 0;
    }

    p = (giopStream*) p->pd_next;
  }
  if (omniORB::traceLevel > 20) {
    omniORB::log << "giopStream::garbageCollect: (" << (void*)this 
		 << ") " << ((rc)? "TRUE" : "FALSE") << "\n";
    omniORB::log.flush();
  }
  return rc;
}

CORBA::Boolean
giopStream::is_unused()
{
  return (pd_state == UnUsed) ? 1 : 0;
}

//////////////////////////////////////////////////////////////////////////////
//  giopStreamImpl                                                          //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

static giopStreamImpl* implHead = 0;
static giopStreamImpl* implMax = 0;

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
  if (g->pd_rdlocked && leftover && !terminalError(g)) {
      // XXX This should not be done if this is a buffered giopStream
    g->pd_strand->giveback_received(leftover);
  }
  g->pd_inb_end = g->pd_inb_mkr;

  leftover = (omni::ptr_arith_t)g->pd_outb_end - 
             (omni::ptr_arith_t)g->pd_outb_mkr;
  if (g->pd_wrlocked && leftover && !terminalError(g)) {
    // XXX This should not be done if this is a buffered giopStream
    g->pd_strand->giveback_reserved(leftover);
  }
  g->pd_outb_end = g->pd_outb_mkr;

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
    omni_giopStream11_initialiser_.attach();
    omni_giopStream12_initialiser_.attach();
  }

  void detach() { 
    omni_giopStream10_initialiser_.attach();
    omni_giopStream11_initialiser_.detach();
    omni_giopStream12_initialiser_.detach();
  }

};

static omni_giopStreamImpl_initialiser initialiser;

omniInitialiser& omni_giopStreamImpl_initialiser_ = initialiser;
