// -*- Mode: C++; -*-
//                            Package   : omniORB
// giopStrand.cc              Created on: 16/01/2001
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
  Revision 1.1.4.1  2001/04/18 18:10:49  sll
  Big checkin with the brand new internal APIs.


  */

#include <omniORB4/CORBA.h>
#include <giopStream.h>
#include <giopStreamImpl.h>
#include <giopRope.h>
#include <giopStrand.h>
#include <GIOP_S.h>
#include <GIOP_C.h>
#include <initialiser.h>
#include <invoker.h>

OMNI_NAMESPACE_BEGIN(omni)

////////////////////////////////////////////////////////////////////////
class Scavenger : public omniTask {
public:
  Scavenger() : omniTask(omniTask::AnyTime) {}
  ~Scavenger() {}

  void execute();
  
  static void notify();

  static void terminate();

  static void initialise();

private:
  static CORBA::Boolean          shutdown;
  static omni_tracedmutex*       mutex;
  static omni_tracedcondition*	 cond;
  static Scavenger*              theTask;

  void removeIdle(StrandList& src,StrandList& dest);
};

////////////////////////////////////////////////////////////////////////
giopStrand::giopStrand(const giopAddress* addr) :
  biDir(0), address(addr), connection(0), gatekeeper_checked(0),first_use(1),
  tcs_selected(0), tcs_c(0), tcs_w(0), giopImpl(0),
  rdcond(omniTransportLock), rd_nwaiting(0),
  wrcond(omniTransportLock), wr_nwaiting(0),
  seqNumber(0), head(0), spare(0), pd_state(ACTIVE)
{
  version.major = version.minor = 0;
  resetIdleCounter(idleOutgoingBeats);
  Scavenger::notify();
  // Call scavenger::notify() to cause the scavenger thread to be
  // created if it hasn't been created already.
}

////////////////////////////////////////////////////////////////////////
giopStrand::giopStrand(giopConnection* conn) :
  biDir(0), address(0), connection(conn), gatekeeper_checked(0),first_use(0),
  tcs_selected(0), tcs_c(0), tcs_w(0), giopImpl(0),
  rdcond(omniTransportLock), rd_nwaiting(0),
  wrcond(omniTransportLock), wr_nwaiting(0),
  seqNumber(1), head(0), spare(0), pd_state(ACTIVE)
{
  version.major = version.minor = 0;
  resetIdleCounter(idleIncomingBeats);
  // Do not call scavenger::notify() because passive strands are not
  // looked after by the scavenger.
}

////////////////////////////////////////////////////////////////////////
giopStrand::~giopStrand()
{
  OMNIORB_ASSERT(pd_state == DYING);
  OMNIORB_ASSERT(giopStream::noLockWaiting(this));
  OMNIORB_ASSERT(giopStreamList::is_empty(servers));
  OMNIORB_ASSERT(giopStreamList::is_empty(clients));
  giopStream_Buffer* p = head;
  while (p) {
    giopStream_Buffer* q = p->next;
    giopStream_Buffer::deleteBuffer(p);
    p = q;
  }
  head = 0;
  p = spare;
  while (p) {
    giopStream_Buffer* q = p->next;
    giopStream_Buffer::deleteBuffer(p);
    p = q;
  }
  spare = 0;
}

////////////////////////////////////////////////////////////////////////
GIOP_S*
giopStrand::acquireServer(giopServer* serv)
{
  // Theory of operation:
  //
  // One or more threads may serve the same strand. However, only
  // one thread works on the same GIOP_S instance. This invariant
  // is enforced by this function.
  //
  // There may be one or more GIOP_S instances linked to this
  // strand. They can be in one of the following states:
  //
  //  1. UnUsed*
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


  ASSERT_OMNI_TRACEDMUTEX_HELD(*omniTransportLock,0);

  omni_tracedmutex_lock sync(*omniTransportLock);

 again:
  // Scan the list to identify the 1st occurrance of an instance in
  // one of these states: UnUsed, InputFullyBuffered, InputPartiallyBuffered.

  GIOP_S* up = 0; // 1st GIOP_S in unused state
  GIOP_S* fp = 0; // 1st GIOP_S in InputFullyBuffered state
  GIOP_S* pp = 0; // 1st GIOP_S in InputPartiallyBuffered state;

  GIOP_S* sp;

  giopStreamList* p = servers.next;
  while (p != &servers) {
    sp = (GIOP_S*)p;
    switch (sp->state()) {
    case IOP_S::UnUsed:
      if (!up) up = sp;
      break;
    case IOP_S::InputFullyBuffered:
      if (!fp) fp = sp;
      break;
    case IOP_S::InputPartiallyBuffered:
      if (!pp) pp = sp;
      break;
    default:
      break;
    }
    p = p->next;
  }
  if (fp) {
    // This is good, no need to acquire a Read Lock on the strand because
    // the whole request message is already in buffer.
    sp = fp;
  }
  else if (giopStream::rdLockNonBlocking(this)) {
    // Now we have got the Read Lock, this is *the* thread to read from
    // the strand until we release the lock.
    // Choose a GIOP_S instance in the following order:
    // 1. InputPartiallyBuffered
    // 2. UnUsed
    // 3. None of the above
    if (pp) {
      sp = pp;
    }
    else if (up) {
      sp = up;
      sp->impl(0);
    }
    else {
      sp = new GIOP_S(this,serv);
      sp->giopStreamList::insert(servers);
    }
    sp->markRdLock();
  }
  else {
    // Another thread is already reading from the strand, we let it does
    // the work and let it wake us up if there is some work to do.
    giopStream::sleepOnRdLock(this);
    goto again;
  }

  sp->state(IOP_S::Idle);
  if (!sp->impl()) {
    sp->impl(giopStreamImpl::maxVersion());
  }
  // the codeset convertors are filled in by the codeset interceptor
  // before a request is unmarshalled.
  sp->TCS_C(0);
  sp->TCS_W(0);
  return sp;
}

////////////////////////////////////////////////////////////////////////
void
giopStrand::releaseServer(IOP_S* iop_s)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omniTransportLock,0);

  omni_tracedmutex_lock sync(*omniTransportLock);

  GIOP_S* giop_s = (GIOP_S*) iop_s;

  giop_s->rdUnLock();
  giop_s->wrUnLock();

  giopStrand* s = &((giopStrand&)(*(giopStream*)giop_s));
  giop_s->giopStreamList::remove();

  CORBA::Boolean remove = 0;

  if ( s->state() == giopStrand::DYING ) {
    remove = 1;

    // We have to go through the GIOP_S list and delete any ones
    // that are in the Unused and InputFullyBuffered states.
    // We can also delete the ones in InputPartiallyBuffered state
    // if no one is holding the Read lock. See the "theory of operation"
    // in acquireServer() for more info.
    CORBA::Boolean remove_partial = !giopStream::RdLockIsHeld(s);
    giopStreamList* p = servers.next;
    while (p != &servers) {
      GIOP_S* sp = (GIOP_S*)p;
      switch (sp->state()) {
      case IOP_S::InputPartiallyBuffered:
	if (! remove_partial) break;
	// else falls through
      case IOP_S::UnUsed:
      case IOP_S::InputFullyBuffered:
	p = p->next;
	sp->giopStreamList::remove();
	delete sp;
	continue;
      default:
	break;
      }
      p = p->next;
    }

    if ( giopStreamList::is_empty(s->clients) && 
	 giopStreamList::is_empty(s->servers) &&
	 giopStream::noLockWaiting(s)) {
      // get ride of the strand.

      // No other threads should be waiting for a read or write lock
      // on the strand. Otherwise, the GIOP_C or GIOP_S lists would not
      // be empty.
      s->StrandList::remove();
      s->Rope::Link::remove();
      if (s->connection) delete s->connection;
      delete s;
    }
  }
  else if ( !giopStreamList::is_empty(s->servers) ) {
    remove = 1;
  }
  else {
    giop_s->state(IOP_S::UnUsed);
    giop_s->giopStreamList::insert(s->servers);
    s->resetIdleCounter(idleIncomingBeats);
  }

  if (remove) delete giop_s;
}

////////////////////////////////////////////////////////////////////////
CORBA::ULong
giopStrand::newSeqNumber() {

  ASSERT_OMNI_TRACEDMUTEX_HELD(*omniTransportLock,1);
  seqNumber += 2;
  return seqNumber;
}

////////////////////////////////////////////////////////////////////////
StrandList giopStrand::active;
StrandList giopStrand::active_timedout;
StrandList giopStrand::passive;
// Throughout the lifetime of a strand, it is a member of one and only one
// of the lists: 
//   active           - the ORB uses this connection in the role of a client
//                      it is 'active' in the sense that the connection was 
//                      initiated by this ORB
//   active_timedout  - the connection was previously active and has been
//                      idled for some time. It will be deleted soon.
//   passive          - the ORB uses this connection in the role of a server
//                      it is 'passive' because the connection was initiated
//                      by the remote party.
//

CORBA::ULong giopStrand::idleIncomingBeats = 36;
CORBA::ULong giopStrand::idleOutgoingBeats = 24;
CORBA::ULong giopStrand::scanPeriod = 5;
giopStrand::timeValue giopStrand::outgoingCallTimeOut = { 0, 0 };
giopStrand::timeValue giopStrand::incomingCallTimeOut = { 0, 0 };

////////////////////////////////////////////////////////////////////////
void
Scavenger::removeIdle(StrandList& src,StrandList& dest)
{
  StrandList* p = src.next;
  for (; p != &src; p = p->next) {
    giopStrand* s = (giopStrand*)p;
    if ( ( !s->biDir )  &&
	 ( giopStreamList::is_empty(s->clients) || 
	   ((GIOP_C*)s->clients.next)->state() == IOP_C::UnUsed ) )
      {
	if (--(s->idlebeats) <= 0) {
	  s->StrandList::remove();
	  s->Rope::Link::remove();
	  s->StrandList::insert(dest);
	}
      }
  }
}

////////////////////////////////////////////////////////////////////////
void
Scavenger::execute()
{
  unsigned long abs_sec,abs_nsec;
  omni_thread::get_time(&abs_sec,&abs_nsec);

  while (1) {
    {
      omni_tracedmutex_lock sync(*mutex);
      if ( shutdown || !giopStrand::scanPeriod ) {
	goto died;
      }
      omni_thread::get_time(&abs_sec,&abs_nsec,giopStrand::scanPeriod);
      cond->timedwait(abs_sec,abs_nsec);
    }

    StrandList shutdown_list;
    {
      // Collect all the strands that should be removed
      // We want to minimise the time we hold the omniTransportLock.
      // Therefore we remove the strands and keep it in the private
      // list for the time being.

      omni_tracedmutex_lock sync(*omniTransportLock);

      // Notice that only those strands that have no other threads touching
      // them will be scanned and may be removed.
      removeIdle(giopStrand::active_timedout,shutdown_list);
      removeIdle(giopStrand::active,shutdown_list);
    }

    {
      // Now goes through the list to delete them all
      StrandList* p = shutdown_list.next;
      while ( p != &shutdown_list ) {
	giopStrand* s = (giopStrand*)p;
	p = p->next;
	s->StrandList::remove();
	if (s->connection) delete s->connection;
	delete s;
      }
    }
  }

 died:
  {
    mutex->lock();
    if (shutdown) {
      mutex->unlock();
      delete cond;
      delete mutex;
    }
    theTask = 0;
    delete this;
  }
}

////////////////////////////////////////////////////////////////////////
void
Scavenger::notify()
{
  omni_tracedmutex_lock sycn(*mutex);
  if ( !shutdown && giopStrand::scanPeriod &&!theTask ) {
    theTask = new Scavenger();
    orbAsyncInvoker->insert(theTask);
  }
}

////////////////////////////////////////////////////////////////////////
void
Scavenger::terminate()
{

  mutex->lock();
  if (theTask) {
    shutdown = 1;
    cond->signal();
    mutex->unlock();
  }
  else {
    mutex->unlock();
    delete cond; cond = 0;
    delete mutex; mutex = 0;
  }
}

void
Scavenger::initialise()
{
  Scavenger::mutex = new omni_tracedmutex();
  Scavenger::cond  = new omni_tracedcondition(Scavenger::mutex);
}

////////////////////////////////////////////////////////////////////////
CORBA::Boolean        Scavenger::shutdown = 0;
omni_tracedmutex*     Scavenger::mutex = 0;
omni_tracedcondition* Scavenger::cond = 0;
Scavenger*            Scavenger::theTask = 0;

////////////////////////////////////////////////////////////////////////
// Module initialiser
////////////////////////////////////////////////////////////////////////

class omni_giopStrand_initialiser : public omniInitialiser {
public:
  void attach() {
    Scavenger::initialise();
  }
  void detach() {
    Scavenger::terminate();
  }
};

static omni_giopStrand_initialiser initialiser;

omniInitialiser& omni_giopStrand_initialiser_ = initialiser;

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
void
giopStreamList::insert(giopStreamList& head)
{
  next = head.prev->next;
  head.prev->next = this;
  prev = head.prev;
  head.prev = this;
}

////////////////////////////////////////////////////////////////////////////
void
giopStreamList::remove()
{
  prev->next = next;
  next->prev = prev;
}

////////////////////////////////////////////////////////////////////////////
CORBA::Boolean
giopStreamList::is_empty(giopStreamList& head)
{
  return (head.next == &head);
}

OMNI_NAMESPACE_END(omni)

OMNI_USING_NAMESPACE(omni)


/////////////////////////////////////////////////////////////////////////////
void 
omniORB::scanGranularity(CORBA::ULong sec)
{
  giopStrand::scanPeriod = sec;
}

/////////////////////////////////////////////////////////////////////////////
CORBA::ULong
omniORB::scanGranularity()
{
  return giopStrand::scanPeriod;
}

/////////////////////////////////////////////////////////////////////////////
void 
omniORB::idleConnectionScanPeriod(omniORB::idleConnType direction,
				  CORBA::ULong sec)
{
  switch (direction)
    {
    case omniORB::idleIncoming:
      if (sec && giopStrand::scanPeriod)
	giopStrand::idleIncomingBeats = \
	  ((sec >= giopStrand::scanPeriod) ? 
	   sec : giopStrand::scanPeriod) / giopStrand::scanPeriod;
      else
        giopStrand::idleIncomingBeats = INT_MAX;
      break;
    case omniORB::idleOutgoing:
      if (sec && giopStrand::scanPeriod)
	giopStrand::idleOutgoingBeats = \
	  ((sec >= giopStrand::scanPeriod) ? 
	   sec : giopStrand::scanPeriod) / giopStrand::scanPeriod;
      else
        giopStrand::idleOutgoingBeats = INT_MAX;
      break;
    }
}

/////////////////////////////////////////////////////////////////////////////
CORBA::ULong 
omniORB::idleConnectionScanPeriod(omniORB::idleConnType direction)
{
  switch (direction)
    {
    case omniORB::idleIncoming:
      return ((giopStrand::idleIncomingBeats != INT_MAX) ? 
	      (giopStrand::idleIncomingBeats * giopStrand::scanPeriod) : 0);
    case omniORB::idleOutgoing:
    default:  // stop MSVC complaining
      return ((giopStrand::idleOutgoingBeats != INT_MAX) ? 
	      (giopStrand::idleOutgoingBeats * giopStrand::scanPeriod) : 0);
    }
}

/////////////////////////////////////////////////////////////////////////////
void 
omniORB::callTimeOutPeriod(omniORB::callTimeOutType direction,
			   CORBA::ULong sec, CORBA::ULong nanosec)
{
  switch (direction)
    {
    case omniORB::serverSide:
      giopStrand::incomingCallTimeOut.secs = sec;
      giopStrand::incomingCallTimeOut.nanosecs = nanosec;
      break;
    case omniORB::clientSide:
      giopStrand::outgoingCallTimeOut.secs = sec;
      giopStrand::outgoingCallTimeOut.nanosecs = nanosec;
      break;
    }
}

/////////////////////////////////////////////////////////////////////////////
CORBA::ULong 
omniORB::callTimeOutPeriod(omniORB::callTimeOutType direction,
			   CORBA::ULong* nanosec)
{
  switch (direction)
    {
    case omniORB::serverSide:
      *nanosec = giopStrand::incomingCallTimeOut.nanosecs;
      return giopStrand::incomingCallTimeOut.secs;
    case omniORB::clientSide:
    default:  // stop MSVC complaining
      *nanosec = giopStrand::outgoingCallTimeOut.nanosecs;
      return giopStrand::outgoingCallTimeOut.secs;
    }
}
