// -*- Mode: C++; -*-
//                            Package   : omniORB2
// Strand.cc                  Created on: 18/2/96
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
//	Implementation of the generic part of the Strand class
//	

/*
  $Log$
  Revision 1.10.2.2  1999/09/27 13:31:43  djr
  Updated logging to always issue omniORB: prefix.

  Revision 1.10.2.1  1999/09/21 20:37:17  sll
  -Simplified the scavenger code and the mechanism in which connections
   are shutdown. Now only one scavenger thread scans both incoming
   and outgoing connections. A separate thread do the actual shutdown.
  -omniORB::scanGranularity() now takes only one argument as there is
   only one scan period parameter instead of 2.
  -Trace messages in various modules have been updated to use the logger
   class.
  -ORBscanGranularity replaces -ORBscanOutgoingPeriod and
                                 -ORBscanIncomingPeriod.

  Revision 1.10  1999/08/30 16:51:59  sll
  Removed WrTestLock and heartbeat argument in WrLock.
  Replace with Sync::clicksSet, Sync::clicksDecrAndGet and Sync::clicksGet.

  Revision 1.9  1999/05/26 11:54:13  sll
  Replaced WrTimedLock with WrTestLock.
  New implementation of Strand_iterator.

  Revision 1.8  1999/03/11 16:25:56  djr
  Updated copyright notice

  Revision 1.7  1999/01/25 18:00:53  sll
  Fixed a bug in Sync::WrTimedLock and Sync::WrUnlock(Strand* s).
  Previously, WrTimedLock does not increment the reference count of the
  strand. Consequently, the strand could be deleted (when its reference count
  goes to zero) while a thread is still block inside WrTimedLock.

  Revision 1.6  1998/08/14 13:52:23  sll
  Added pragma hdrstop to control pre-compile header if the compiler feature
  is available.

  Revision 1.5  1998/04/07 19:37:39  sll
  Replace cerr with omniORB::log.

  Revision 1.4  1998/03/19 19:53:35  sll
  Bug fixed. WrTimedLock.

  Revision 1.3  1997/12/09 17:52:25  sll
  Updated to implement the new rope and strand interface.
  New member Sync::WrTimedLock()
  Rope_iterator::operator() () now garbage collect unused rope.

// Revision 1.2  1997/05/06  15:28:06  sll
// Public interface.
//
  */

#include <omniORB2/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <scavenger.h>
#include <ropeFactory.h>


#define LOGMESSAGE(level,prefix,message)  \
  omniORB::logs(level, "strand " prefix ": " message)


class omniORB_Ripper;

static omniORB_Ripper* ripper = 0;


Strand::Strand(Rope *r, CORBA::Boolean heapAllocated)
  : pd_rdcond(&r->pd_lock), pd_wrcond(&r->pd_lock)
{
  pd_head = 0;
  pd_rd_nwaiting = 0;
  pd_wr_nwaiting = 0;

  // enter this to the list in rope <r>
  pd_next = r->pd_head;
  r->pd_head = this;
  pd_rope = r;
  pd_dying = 0;
  pd_heapAllocated = heapAllocated;
  pd_refcount = 0;
  pd_seqNumber = 1;
  pd_clicks = (r->is_incoming() ? StrandScavenger::inIdleTimeLimit() :
	                          StrandScavenger::outIdleTimeLimit());
  pd_ripper_next = this;
  return;
}


Strand::~Strand()
{
  if (!pd_rope) return;
  
  // When this destructor is called, there should be no Strand::Sync 
  // objects remain on the Sync queue.
  assert(is_idle(1));

  // remove this from the list in rope <pd_rope>
  Strand **p = &pd_rope->pd_head;
  while (*p && *p != this)
    p = &((*p)->pd_next);
  if (*p) {
    *p = (*p)->pd_next;
  }

  pd_rope = 0;

  return;
}

void
Strand::incrRefCount(CORBA::Boolean held_rope_mutex)
{
  if (!held_rope_mutex)
    pd_rope->pd_lock.lock();
  assert(pd_refcount >= 0);
  pd_refcount++;
  if (!held_rope_mutex)
    pd_rope->pd_lock.unlock();
  return;
}

void
Strand::decrRefCount(CORBA::Boolean held_rope_mutex)
{
  if (!held_rope_mutex)
    pd_rope->pd_lock.lock();
  pd_refcount--;
  assert(pd_refcount >= 0);
  if (!held_rope_mutex)
    pd_rope->pd_lock.unlock();
  return;
}

CORBA::Boolean
Strand::is_idle(CORBA::Boolean held_rope_mutex)
{
  CORBA::Boolean idle;
  if (!held_rope_mutex)
    pd_rope->pd_lock.lock();
  idle = ((pd_refcount == 0)? 1: 0);
  if (!held_rope_mutex)
    pd_rope->pd_lock.unlock();
  return idle;
}

CORBA::Boolean
Strand::is_unused(CORBA::Boolean held_rope_mutex)
{
  CORBA::Boolean unused;
  if (!held_rope_mutex)
    pd_rope->pd_lock.lock();
  unused = ((pd_head == 0)? 1: 0);
  if (!held_rope_mutex)
    pd_rope->pd_lock.unlock();
  return unused;
}

Strand::
Sync::Sync(Strand *s,CORBA::Boolean rdLock,CORBA::Boolean wrLock) 
{
  // Caller of this function must ensure that pd_refcount is not zero.
  // This ensures that no other thread can destroy this strand
  // before this function grabs the MUTEX (s->pd_rope->pd_lock).

  pd_strand = 0;
  pd_secondHand = 0;
  s->pd_rope->pd_lock.lock();

  if (s->_strandIsDying()) {
    // If this strand or the rope it belongs is being shutdown, stop here
    s->pd_rope->pd_lock.unlock();
    throw CORBA::COMM_FAILURE(0,CORBA::COMPLETED_NO);
  }

  // enter this to the list in strand <s>
  pd_next = s->pd_head;
  s->pd_head = this;
  pd_strand = s;

  s->incrRefCount(1);

  // Acquire the locks on the strand
  // Always do the Read lock before the Write lock to avoid deadlock
  if (rdLock) {
    RdLock(1);
  }
  if (wrLock) {
    WrLock(1);
  }

  s->pd_rope->pd_lock.unlock();
  return;
}

Strand::
Sync::Sync(Rope *r,CORBA::Boolean rdLock,CORBA::Boolean wrLock)
{
  pd_strand = 0;
  r->pd_lock.lock();
  Strand *s;

  try {
    s = r->getStrand(pd_secondHand);
  }
  catch(...) {
    r->pd_lock.unlock();
    throw;
  }

  pd_next = s->pd_head;	
  s->pd_head = this;
  pd_strand = s;

  s->incrRefCount(1);

  // Acquire the locks on the strand
  // Always do the Read lock before the Write lock to avoid deadlock

  if (rdLock) {
    RdLock(1);
  }

  if (wrLock) {
    WrLock(1);
  }
  r->pd_lock.unlock();
  return;
}


Strand::
Sync::~Sync()
{
  if (!pd_strand) return;

  Rope *r = pd_strand->pd_rope;

  r->pd_lock.lock();

  // remove this from the list in strand <pd_strand>
  Strand::Sync **p = &pd_strand->pd_head;
  while (*p && *p != this)
    p = &((*p)->pd_next);
  if (*p) {
    *p = (*p)->pd_next;
  }
  pd_strand->decrRefCount(1);

  // If this is the last sync object and the strand is dying, delete it.
  if (pd_strand->is_idle(1) && strandIsDying())
    {
      if (pd_strand->pd_heapAllocated)
  	delete pd_strand;
      else
  	pd_strand->~Strand();
    }
  
  pd_strand = 0;
  r->pd_lock.unlock();
  return;
}

void
Strand::
Sync::RdLock(CORBA::Boolean held_rope_mutex)
{
  if (!held_rope_mutex)
    pd_strand->pd_rope->pd_lock.lock();

  while (pd_strand->pd_rd_nwaiting < 0) {
    pd_strand->pd_rd_nwaiting--;
    pd_strand->pd_rdcond.wait();
    if (pd_strand->pd_rd_nwaiting >= 0)
      pd_strand->pd_rd_nwaiting--;
    else
      pd_strand->pd_rd_nwaiting++;
  }
  pd_strand->pd_rd_nwaiting = -pd_strand->pd_rd_nwaiting - 1;

  if (!held_rope_mutex)
    pd_strand->pd_rope->pd_lock.unlock();
  return;
}

void
Strand::
Sync::RdUnlock(CORBA::Boolean held_rope_mutex)
{
  if (!held_rope_mutex)
    pd_strand->pd_rope->pd_lock.lock();

  assert(pd_strand->pd_rd_nwaiting < 0);
  pd_strand->pd_rd_nwaiting = -pd_strand->pd_rd_nwaiting - 1;
  if (pd_strand->pd_rd_nwaiting > 0)
    pd_strand->pd_rdcond.signal();

  if (!held_rope_mutex)
    pd_strand->pd_rope->pd_lock.unlock();

  return;
}

void
Strand::
Sync::WrLock(CORBA::Boolean held_rope_mutex)
{
  if (!held_rope_mutex)
    pd_strand->pd_rope->pd_lock.lock();

  while (pd_strand->pd_wr_nwaiting < 0) {
    pd_strand->pd_wr_nwaiting--;
    pd_strand->pd_wrcond.wait();
    if (pd_strand->pd_wr_nwaiting >= 0)
      pd_strand->pd_wr_nwaiting--;
    else
      pd_strand->pd_wr_nwaiting++;
  }
  pd_strand->pd_wr_nwaiting = -pd_strand->pd_wr_nwaiting - 1;

  if (!held_rope_mutex)
    pd_strand->pd_rope->pd_lock.unlock();
  return;

}

void
Strand::
Sync::WrUnlock(CORBA::Boolean held_rope_mutex)
{
  if (!held_rope_mutex)
    pd_strand->pd_rope->pd_lock.lock();

  assert(pd_strand->pd_wr_nwaiting < 0);
  pd_strand->pd_wr_nwaiting = -pd_strand->pd_wr_nwaiting - 1;
  if (pd_strand->pd_wr_nwaiting > 0)
    pd_strand->pd_wrcond.signal();

  if (!held_rope_mutex)
    pd_strand->pd_rope->pd_lock.unlock();

  return;
}

void
Strand::
Sync::clicksSet(int clicks, CORBA::Boolean held_rope_mutex)
{
  if (!held_rope_mutex)
    pd_strand->pd_rope->pd_lock.lock();

  pd_strand->pd_clicks = clicks;

  if (!held_rope_mutex)
    pd_strand->pd_rope->pd_lock.unlock();
}

int
Strand::
Sync::clicksDecrAndGet(Strand* s)
{
  s->pd_clicks--;
  return s->pd_clicks;
}
int
Strand::
Sync::clicksGet(Strand* s)
{
  return s->pd_clicks;
}


void
Strand::
Sync::setStrandIsDying()
{
  pd_strand->_setStrandIsDying();
  return;
}

CORBA::Boolean
Strand::
Sync::strandIsDying()
{
  return pd_strand->_strandIsDying();
}

CORBA::Boolean
Strand::
Sync::isReUsingExistingConnection() const
{
  return pd_secondHand;
}

Rope::Rope(Anchor *a,
	   unsigned int maxStrands,
	   CORBA::Boolean heapAllocated)
  : pd_lock()
{
  pd_head = 0;
  pd_heapAllocated = heapAllocated;
  pd_maxStrands = maxStrands;
  pd_anchor = a;
  pd_refcount = 0;
  pd_next = a->pd_head;
  a->pd_head = this;
  return;
}


Rope::~Rope()
{
  if (!pd_anchor) return;

  // When this destructor is called, there should be no Strand objects
  // remain on the strand queue
  assert(is_idle(1));

  // remove this from the list in anchor <pd_anchor>
  Rope **p = &pd_anchor->pd_head;
  while (*p && *p != this)
    p = &((*p)->pd_next);
  if (*p) {
    *p = (*p)->pd_next;
  }

  pd_anchor = 0;
}

void
Rope::CutStrands(CORBA::Boolean held_rope_mutex)
{
  Strand_iterator next(this,held_rope_mutex);
  Strand *p;
  while ((p = next())) {
    p->shutdown();
    omni_thread::yield();
  }
  return;
}

Strand *
Rope::getStrand(CORBA::Boolean& secondHand)
{
  Strand *p;
  unsigned int n = 0;
  {
    Strand_iterator next(this,1);
    while ((p = next())) {
      if (!p->_strandIsDying()) {
	n++;
	if (p->is_unused(1)) {
	  secondHand = 1;
	  break;
	}
      }
    }
  }
  if (!p) {
    if (n < pd_maxStrands) {
      p = newStrand();
      secondHand = 0;
    }
    else {
      Strand_iterator next(this,1);
      p = next();
      secondHand = 1;
    }
  }
  return p;
}

void
Rope::incrRefCount(CORBA::Boolean held_anchor_mutex)
{
  if (!held_anchor_mutex)
    pd_anchor->pd_lock.lock();
  {
    if (omniORB::trace(20)) {
      omniORB::logger log;
      log << "strand Rope::incrRefCount: " << "old value = "
	<< pd_refcount << "\n";
    }
  }
  assert(pd_refcount >= 0);
  pd_refcount++;
  if (!held_anchor_mutex)
    pd_anchor->pd_lock.unlock();
  return;
}

void
Rope::decrRefCount(CORBA::Boolean held_anchor_mutex)
{
  if (!held_anchor_mutex)
    pd_anchor->pd_lock.lock();

  {
    if (omniORB::trace(20)) {
      omniORB::logger log;
      log << "strand Rope::decrRefCount: old value = " << pd_refcount << "\n";
    }
  }

  pd_refcount--;
  assert(pd_refcount >=0);
  if (!held_anchor_mutex)
    pd_anchor->pd_lock.unlock();
  return;
}

CORBA::Boolean
Rope::is_idle(CORBA::Boolean held_anchor_mutex)
{
  CORBA::Boolean idle;
  if (!held_anchor_mutex)
    pd_anchor->pd_lock.lock();
  idle = ((pd_refcount == 0)? 1: 0);
  if (!held_anchor_mutex)
    pd_anchor->pd_lock.unlock();
  return idle;
}


Anchor::Anchor() : pd_lock()
{
  pd_head = 0;
  return;
}


Anchor::~Anchor()
{
  //  assert(pd_head == 0);
  //  XXX we should really check here whether all the ropes has been
  //      shutdown.
  return;
}


Strand_iterator::Strand_iterator(const Rope *r,
				 CORBA::Boolean held_rope_mutex)
{
  if (!held_rope_mutex)
    ((Rope *)r)->pd_lock.lock();
  pd_rope = r;
  pd_leave_mutex = held_rope_mutex;
  pd_initialised = 0;
  pd_s = 0;
  return;
}


Strand_iterator::~Strand_iterator()
{
  if (pd_s) {
    pd_s->decrRefCount(1);
    pd_s = 0;                // Be paranoid
  }
  if (!pd_leave_mutex)
    ((Rope *)pd_rope)->pd_lock.unlock();
  return;
}

Strand *
Strand_iterator::operator() ()
{
  if (pd_s) {
    pd_s->decrRefCount(1);
    pd_s = pd_s->pd_next;
  }
  else if (!pd_initialised) {
    pd_s = pd_rope->pd_head;
    pd_initialised = 1;
  }
  while (pd_s && pd_s->is_idle(1) && pd_s->_strandIsDying()) {
    Strand *p=pd_s;
    pd_s = pd_s->pd_next;
    if (p->pd_heapAllocated)
      delete p;
    else
      p->~Strand();
  }
  if (pd_s) {
    pd_s->incrRefCount(1);
  }
  return pd_s;
}

Rope_iterator::Rope_iterator(const Anchor *a)
{
  ((Anchor *)a)->pd_lock.lock();
  pd_r = a->pd_head;
  pd_anchor = a;
  return;
}


Rope_iterator::Rope_iterator(ropeFactory* rf)
{
  rf->anchor()->pd_lock.lock();
  pd_r = rf->anchor()->pd_head;
  pd_anchor = rf->anchor();
  return;
}


Rope_iterator::~Rope_iterator()
{
  ((Anchor *)pd_anchor)->pd_lock.unlock();
  return;
}


/////////////////////////////////////////////////////////////////////////////
// omniORB_Ripper
//
// Instance of this class do the dirty work of calling real_shutdown on the
// strand. The call may actually block for considerable time.
// We do not want to hold up any thread that just want to indicate that
// the strand should be shutdown and does not want to wait till it
// is done.
//
class omniORB_Ripper : public omni_thread {
public:
  omniORB_Ripper() : pd_cond(&pd_mutex),
		     pd_isdying(0), 
		     pd_head(0) { start_undetached(); }

  void enqueue(Strand* p) {
    omni_mutex_lock sync(pd_mutex);
    if (p->pd_ripper_next != p) return;   // Already queued
    p->pd_ripper_next = pd_head;
    pd_head = p;
    p->incrRefCount(1);     // Note: caller has got the mutex on the strand
    pd_cond.signal();
  }

  CORBA::Boolean isPending(Strand* p) {
    omni_mutex_lock sync(pd_mutex);
    return (p->pd_ripper_next == p) ? 0 : 1;
  }

  void* run_undetached(void*) {

    LOGMESSAGE(15,"Ripper","start.");

    while (1) {
      
      Strand* p;

      {
	omni_mutex_lock sync(pd_mutex);

	while (!pd_head && !pd_isdying) {
	  pd_cond.wait();
	}

	if (pd_isdying && !pd_head) break;

	p = pd_head;
	pd_head = 0;
      }
      
      while (p) {
	Strand* np = p->pd_ripper_next;
	p->real_shutdown();
	p->decrRefCount();
	p = np;
      }

    }

    LOGMESSAGE(15,"Ripper","exit.");
    return 0;
  }

  void kill() {

    {
      omni_mutex_lock sync(pd_mutex);
      pd_isdying = 1;
      pd_cond.signal();
    }
    join(0);
  }

  virtual ~omniORB_Ripper() {}

private:
  omni_mutex     pd_mutex;
  omni_condition pd_cond;
  CORBA::Boolean pd_isdying;
  Strand* pd_head;
};


/////////////////////////////////////////////////////////////////////////////

void
Strand::shutdown()
{
  _setStrandIsDying();
  assert(ripper);
  ripper->enqueue(this);
}


/////////////////////////////////////////////////////////////////////////////

Rope *
Rope_iterator::operator() ()
{
  Rope *rp;
  while (1) 
    {
      rp = pd_r;
      if (rp) 
	{
	  pd_r = pd_r->pd_next;
	  if (rp->is_idle(1)) 
	    {
	      // This Rope is not used by any object reference
	      // First close down all the strands before calling
	      // the dtor of the Rope.
	      LOGMESSAGE(10,"Rope_iterator","delete unused Rope.");
	      CORBA::Boolean can_delete = 1;

	      {
		Strand_iterator next_strand(rp);
		Strand* p;
		while ((p = next_strand())) {
		  can_delete = 0;
		  if (p->is_unused(1)) {
		    p->_setStrandIsDying();
		  }
		  else {
		    LOGMESSAGE(0,"Rope_iterator","Detected Application error. An object reference returned to the application has been released but it is currently being used to do a remote call. This thread will now raise a omniORB::fatalException.");
		  }
		}
	      }
	      if (can_delete) {
		if (rp->pd_heapAllocated) delete rp;
		else rp->~Rope();
	      }
	      continue;
	    }
	}
      break;
    }
  return rp;
}


/////////////////////////////////////////////////////////////////////////////
//            Module initialiser                                           //
/////////////////////////////////////////////////////////////////////////////

class omni_strand_initialiser : public omniInitialiser {
public:

  void attach() {
    ripper = new omniORB_Ripper();
  }

  void detach() {
    ripper->kill();
    ripper = 0;
  }
};

static omni_strand_initialiser initialiser;

omniInitialiser& omni_strand_initialiser_ = initialiser;
