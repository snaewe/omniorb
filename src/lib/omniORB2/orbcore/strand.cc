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
  Revision 1.10.4.2  1999/09/16 16:04:25  sll
  Bug fix. Rope_iterator now use Strand::Sync::is_unused() to test if the
  strand is not in use. giopStream::is_unused() now remove unused giopStream
  as a side-effect. The net effect is that unused outgoing Rope is now
  deleted correctly.

  Revision 1.10.4.1  1999/09/15 20:18:28  sll
  Updated to use the new cdrStream abstraction.
  Marshalling operators for NetBufferedStream and MemBufferedStream are now
  replaced with just one version for cdrStream.
  Derived class giopStream implements the cdrStream abstraction over a
  network connection whereas the cdrMemoryStream implements the abstraction
  with in memory buffer.

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

#include <ropeFactory.h>

Strand::Strand(Rope *r)
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
  pd_refcount = 0;
  pd_seqNumber = 1;
  pd_reuse = 0;
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
Strand::decrRefCount(CORBA::Boolean held_rope_mutex,CORBA::Boolean deleteflag)
{
  Rope* r;
  if (!held_rope_mutex) {
    r = pd_rope;
    r->pd_lock.lock();
  }
  pd_refcount--;
  assert(pd_refcount >= 0);
  if (pd_refcount == 0 && deleteflag) {
    delete this;
  }
  if (!held_rope_mutex)
    r->pd_lock.unlock();
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

void
Strand::setReUseFlag()
{
  pd_reuse = 1;
}

void
Strand::raiseException(CORBA::ULong minor, CORBA::ULong completed)
{
  if (pd_reuse) {
    throw CORBA::TRANSIENT(minor,(CORBA::CompletionStatus)completed);
  }
  else {
    throw CORBA::COMM_FAILURE(minor,(CORBA::CompletionStatus)completed);
  }
}


Strand::Sync*
Strand::
Sync::getSync(Strand* s)
{
  return s->pd_head;
}

omni_mutex&
Strand::
Sync::getMutex(Strand* s)
{
  return s->pd_rope->pd_lock;
}

omni_mutex&
Strand::
Sync::getMutex(Rope* s)
{
  return s->pd_lock;
}

Strand*
Strand::
Sync::getStrand(Rope* s)
{
  return s->getStrand();
}

void
Strand::
Sync::setStrandIsDying(Strand* s)
{
  s->_setStrandIsDying();
}

Strand::
Sync::Sync(Strand *s) 
{
  // Caller of this function must ensure that pd_refcount is not zero.

  pd_strand = s;
  pd_next = s->pd_head;
  s->pd_head = this;
  s->incrRefCount(1);
}

Strand::
Sync::~Sync()
{
  if (!pd_strand) return;

  // remove this from the list in strand <pd_strand>
  Strand::Sync **p = &pd_strand->pd_head;
  while (*p && *p != this)
    p = &((*p)->pd_next);
  if (*p) {
    *p = (*p)->pd_next;
  }
  pd_strand->decrRefCount(1);

  // If this is the last sync object and the strand is dying, delete it.
  if (pd_strand->is_idle(1) && pd_strand->_strandIsDying())
    {
      delete pd_strand;
    }
  
  pd_strand = 0;
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

CORBA::Boolean
Strand::
Sync::garbageCollect()
{
  // The default is not to garbageCollect.
  // This function should be overloaded by the derived class to
  // determine whether it is appropriate to garbage collect this strand.
  return 0;
}

CORBA::Boolean
Strand::
Sync::is_unused()
{
  // The default is always indicate the strand is unused.
  // This function should be overloaded by the derived class to
  // determine whether it is really unused.
  return 1;
}

Rope::Rope(Anchor *a,
	   unsigned int maxStrands)
  : pd_lock()
{
  pd_head = 0;
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
Rope::getStrand()
{
  Strand *p;
  unsigned int n = 0;
  {
    Strand_iterator next(this,1);
    while ((p = next())) {
      if (!p->_strandIsDying()) {
	n++;
	Strand::Sync* q;
	if (!(q = Strand::Sync::getSync(p)) || q->is_unused())
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
  if (omniORB::traceLevel >= 20) {
    omniORB::log << "Rope::incrRefCount: old value = " << pd_refcount << "\n";
    omniORB::log.flush();
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
  if (omniORB::traceLevel >= 20) {
    omniORB::log << "Rope::decrRefCount: old value = " << pd_refcount << "\n";
    omniORB::log.flush();
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
    delete p;
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
	      // First close down all the strands before calling the dtor of the Rope
	      if (omniORB::traceLevel > 10) {
		omniORB::log << "Rope_iterator::operator() (): delete unused Rope.\n";
		omniORB::log.flush();
	      }
	      rp->pd_lock.lock();
	      Strand *p = rp->pd_head;
	      while (p)
		{
		  Strand::Sync* q;
		  if ((q = Strand::Sync::getSync(p)) && !q->is_unused()) {
		    // This is wrong, nobody should be using this strand.
		    throw omniORB::fatalException(__FILE__,__LINE__,
						  "Rope_iterator::operator() () tries to delete a strand that is not idle.");
		  }
		  p->shutdown();
		  Strand* np = p->pd_next;
		  delete p;
		  p = np;
		}
	      rp->pd_lock.unlock();
	      delete rp;
	      continue;
	    }
	}
      break;
    }
  return rp;
}

