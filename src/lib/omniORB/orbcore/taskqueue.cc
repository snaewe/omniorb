// -*- Mode: C++; -*-
//                            Package   : omniORB
// taskqueue.cc               Created on: 16/8/99
//                            Author    : David Riddoch (djr)
//
//    Copyright (C) 1996-1999 AT&T Research Cambridge
//
//    This file is part of the omniORB library.
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
  Revision 1.2  2000/07/04 15:22:50  dpg1
  Merge from omni3_develop.

  Revision 1.1.2.2  2000/02/07 15:26:59  dpg1
  Spurious \r characters removed.

  Revision 1.1.2.1  1999/09/22 14:27:09  djr
  Major rewrite of orbcore to support POA.

*/

#include <omniORB3/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <taskqueue.h>


class omniTaskQueueTask : public omniTaskQueue::Task {
public:
  virtual ~omniTaskQueueTask()  {}
  inline omniTaskQueueTask(omniTaskQueue::TaskFn fn, void* c)
    : pd_fn(fn), pd_cookie(c) {}
  virtual void doit()  { pd_fn(pd_cookie); }

private:
  omniTaskQueue::TaskFn pd_fn;
  void*                 pd_cookie;
};

//////////////////////////////////////////////////////////////////////

omniTaskQueue::~omniTaskQueue()  {}


omniTaskQueue::omniTaskQueue()
  : pd_cond(&pd_lock),
    pd_taskq(0),
    pd_taskqtail(0),
    pd_dying(0)
{
  start_undetached();
}


void
omniTaskQueue::insert(Task* t)
{
  OMNIORB_ASSERT(t);

  pd_lock.lock();
  int signal = !pd_taskq;
  t->pd_next = 0;
  if( pd_taskq )
    pd_taskqtail = (pd_taskqtail->pd_next = t);
  else
    pd_taskq = pd_taskqtail = t;
  pd_lock.unlock();

  if( signal )  pd_cond.signal();
}


void
omniTaskQueue::insert(void (*t)(void*), void* cookie)
{
  OMNIORB_ASSERT(t);

  insert(new omniTaskQueueTask(t, cookie));
}


void
omniTaskQueue::die()
{
  pd_lock.lock();
  pd_dying = 1;
  int signal = !pd_taskq;
  pd_lock.unlock();

  if( signal )  pd_cond.signal();
}


void*
omniTaskQueue::run_undetached(void*)
{
  while( 1 ) {
    pd_lock.lock();
    while( !pd_taskq ) {
      if( pd_dying ) {
        pd_lock.unlock();
        return 0;
      }
      pd_cond.wait();
    }
    Task* t = pd_taskq;
    pd_taskq = pd_taskq->pd_next;
    pd_lock.unlock();

    try { t->doit(); }
    catch(...) {}
    delete t;
  }

  return 0;
}

//////////////////////////////////////////////////////////////////////

omniTaskQueue::Task::~Task()  {}
