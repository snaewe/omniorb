// -*- Mode: C++; -*-
//                            Package   : omniORB2
// Strand.cc                  Created on: 18/2/96
//                            Author    : Sai Lai Lo (sll)
//
// Copyright (C) Olivetti Research Limited, 1996
//
// Description:
//	Implementation of the generic part of the Strand class
//	

/*
  $Log$
  Revision 1.1  1997/01/08 17:26:01  sll
  Initial revision

  */

#include <omniORB2/CORBA.h>

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

Strand::
Sync::Sync(Strand *s,CORBA::Boolean rdLock,CORBA::Boolean wrLock) 
{
  // Caller of this function must ensure that pd_refcount is not zero.
  // This ensures that no other thread can destroy this strand
  // before this function grabs the MUTEX (s->pd_rope->pd_lock).

  pd_strand = 0;
  s->pd_rope->pd_lock.lock();

  if (s->StrandIsDying()) {
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
    s = r->getStrand();
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
  if (pd_strand->is_idle(1) && pd_strand->StrandIsDying())
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
Sync::setStrandDying()
{
  pd_strand->setStrandIsDying();
  return;
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
    if (p->is_idle(1)) {
      if (p->pd_heapAllocated)
	delete p;
      else
	p->~Strand();
    }
  }
  return;
}

Strand *

Rope::getStrand()
{
  Strand *p;
  unsigned int n = 0;
  {
    Strand_iterator next(this,1);
    while ((p = next())) {
      if (!p->StrandIsDying()) {
	n++;
	if (p->is_idle(1))
	  break;
      }
    }
  }
  if (!p) {
    if (n < pd_maxStrands) {
      p = newStrand();
    }
    else {
      Strand_iterator next(this,1);
      p = next();
    }
  }
  return p;
}

void

Rope::incrRefCount(CORBA::Boolean held_anchor_mutex)
{
  if (!held_anchor_mutex)
    pd_anchor->pd_lock.lock();
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
  pd_s = r->pd_head;
  pd_rope = r;
  pd_leave_mutex = held_rope_mutex;
  return;
}


Strand_iterator::~Strand_iterator()
{
  if (!pd_leave_mutex)
    ((Rope *)pd_rope)->pd_lock.unlock();
  return;
}

Strand *

Strand_iterator::operator() ()
{
  Strand *p = pd_s;
  if (pd_s)
    pd_s = pd_s->pd_next;
  return p;
}


Rope_iterator::Rope_iterator(const Anchor *a)
{
  ((Anchor *)a)->pd_lock.lock();
  pd_r = a->pd_head;
  pd_anchor = a;
  return;
}


Rope_iterator::~Rope_iterator()
{
  ((Anchor *)pd_anchor)->pd_lock.unlock();
  return;
}

Rope *

Rope_iterator::operator() ()
{
  Rope *p = pd_r;
  if (pd_r)
    pd_r = pd_r->pd_next;
  return p;
}
