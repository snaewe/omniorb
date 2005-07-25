// -*- Mode: C++; -*-
//                            Package   : omniORB
// invoker.h                  Created on: 11 Apr 2001
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
//	*** PROPRIETORY INTERFACE ***
//

/*
  $Log$
  Revision 1.1.2.8  2005/07/25 16:29:20  dgrisby
  Async worker could still add itself to the idle list more than once.

  Revision 1.1.2.7  2005/07/22 11:22:06  dgrisby
  Async worker could add itself to the idle list more than once.
  Thanks Hartmut Raschick

  Revision 1.1.2.6  2002/09/11 20:40:15  dgrisby
  Call thread interceptors from etherealiser queue.

  Revision 1.1.2.5  2002/09/10 23:17:11  dgrisby
  Thread interceptors.

  Revision 1.1.2.4  2002/02/25 11:18:02  dpg1
  Avoid thread churn in invoker.

  Revision 1.1.2.3  2002/02/13 17:40:52  dpg1
  Tweak to avoid destruction race in invoker.

  Revision 1.1.2.2  2002/02/13 16:02:40  dpg1
  Stability fixes thanks to Bastiaan Bakker, plus threading
  optimisations inspired by investigating Bastiaan's bug reports.

  Revision 1.1.2.1  2002/01/09 11:35:23  dpg1
  Remove separate omniAsyncInvoker library to save library overhead.

  Revision 1.1.2.7  2001/11/13 14:14:03  dpg1
  AsyncInvoker properly waits for threads to finish.

  Revision 1.1.2.6  2001/08/16 09:53:18  sll
  Added stdlib.h to give abort a prototype.

  Revision 1.1.2.5  2001/08/01 10:03:40  dpg1
  AyncInvoker no longer maintains its own dedicated thread queue.
  Derived classes must provide the implementation.

  Revision 1.1.2.4  2001/07/31 15:56:48  sll
  Make sure pd_nthreads is kept in sync with the actual no. of threads
  serving the Anytime tasks.

  Revision 1.1.2.3  2001/06/13 20:08:13  sll
  Minor update to make the ORB compiles with MSVC++.

  Revision 1.1.2.2  2001/05/01 16:03:16  sll
  Silly typo in a switch statement causes random failure due to corrupted
  link list.

  Revision 1.1.2.1  2001/04/19 09:47:54  sll
  New library omniAsyncInvoker.

*/

#include <omniORB4/CORBA.h>
#include <omniORB4/omniInterceptors.h>
#include <omniORB4/omniAsyncInvoker.h>
#include <interceptors.h>
#include <stdlib.h>

OMNI_USING_NAMESPACE(omni)

unsigned int omniAsyncInvoker::idle_timeout = 10;

class omniAsyncWorker;

class omniAsyncWorkerInfo
  : public omniInterceptors::createThread_T::info_T {
public:
  omniAsyncWorkerInfo(omniAsyncWorker* worker) :
    pd_worker(worker), pd_elmt(omniInterceptorP::createThread) {}

  void run();

private:
  omniAsyncWorker*        pd_worker;
  omniInterceptorP::elmT* pd_elmt;
};


class omniAsyncWorker : public omni_thread {
public:

  omniAsyncWorker(omniAsyncInvoker* pool, omniTask* task) :
    pd_pool(pool), pd_task(task), pd_next(0), pd_id(id()), pd_in_idle_queue(0)
  {
    pd_cond = new omni_tracedcondition(pool->pd_lock);
    start();
  }

  ~omniAsyncWorker() {

    if (omniORB::trace(10)) {
      omniORB::logger l;
      l << "AsyncInvoker: thread id = " << pd_id
	<< " has exited. Total threads = " << pd_pool->pd_totalthreads
	<< "\n";
    }

    delete pd_cond;
    pd_pool->pd_lock->lock();

    if (--pd_pool->pd_totalthreads == 0)
      pd_pool->pd_cond->signal();

    pd_pool->pd_lock->unlock();
  }

  void run(void*) {
    omniAsyncWorkerInfo info(this);
    info.run();
  }

  void real_run() {

    if (omniORB::trace(10)) {
      omni_tracedmutex_lock sync(*pd_pool->pd_lock);
      omniORB::logger l;
      l << "AsyncInvoker: thread id = " << pd_id
	<< " has started. Total threads = " << pd_pool->pd_totalthreads
	<< "\n";
    }
    pd_pool->pd_lock->lock();

    while (pd_task || pd_pool->pd_keep_working) {

      if (!pd_task) {
	if ( !omniTaskLink::is_empty(pd_pool->pd_anytime_tq) ) {
	  pd_task = (omniTask*)pd_pool->pd_anytime_tq.next;
	  pd_task->deq();
	}
	else {
	  // Add to the idle queue
	  OMNIORB_ASSERT(!pd_in_idle_queue);
	  pd_next = pd_pool->pd_idle_threads;
	  pd_pool->pd_idle_threads = this;
	  pd_in_idle_queue = 1;

	  unsigned long abs_sec,abs_nanosec;
	  omni_thread::get_time(&abs_sec,&abs_nanosec,
				omniAsyncInvoker::idle_timeout);

	  int signalled = pd_cond->timedwait(abs_sec,abs_nanosec);

	  if (pd_in_idle_queue) {
	    // Remove from the idle queue
	    omniAsyncWorker** pp = &pd_pool->pd_idle_threads;
	    while (*pp && *pp != this) {
	      pp = &((*pp)->pd_next);
	    }
	    if (*pp) {
	      *pp = pd_next;
	    }
	    else {
	      if (omniORB::trace(1)) {
		omniORB::logger l;
		l << "AsyncInvoker: Warning: thread " << pd_id
		  << " thought it was in the idle queue but it was not.\n";
	      }
	    }
	    pd_next = 0;
	    pd_in_idle_queue = 0;
	  }
	  if (!signalled && !pd_task) {
	    // We have timed out and have not been assigned a task.
	    // Break out from the while loop and exit.
	    break;
	  }
	  // If signalled, we have been dequeued by the
	  // omniAsyncInvoker, and will have a task to process next
	  // time around the while loop.
	  continue;
	}
      }

      unsigned int immediate = (pd_task->category() ==
				omniTask::ImmediateDispatch);

      pd_pool->pd_lock->unlock();
      try {
	pd_task->execute();
      }
      catch(...) {
	omniORB::logs(1, "AsyncInvoker: Warning: unexpected exception "
		      "caught while executing a task.");
      }
      pd_task = 0;
      pd_pool->pd_lock->lock();

      if (immediate) {
	pd_pool->pd_nthreads++;
      }

      if (pd_pool->pd_nthreads > pd_pool->pd_maxthreads) {
	// No need to keep this thread
	break;
      }
    }

    pd_pool->pd_nthreads--;
    pd_pool->pd_lock->unlock();
  }

  friend class omniAsyncInvoker;

private:
  omniAsyncInvoker*     pd_pool;
  omniTask*             pd_task;
  omni_tracedcondition* pd_cond;
  omniAsyncWorker*      pd_next;
  int                   pd_id;
  CORBA::Boolean        pd_in_idle_queue;

  omniAsyncWorker();
  omniAsyncWorker(const omniAsyncWorker&);
  omniAsyncWorker& operator=(const omniAsyncWorker&);
};

void
omniAsyncWorkerInfo::run()
{
  if (pd_elmt) {
    omniInterceptors::createThread_T::interceptFunc f =
      (omniInterceptors::createThread_T::interceptFunc)pd_elmt->func;
    pd_elmt = pd_elmt->next;
    f(*this);
  }
  else
    pd_worker->real_run();
}



///////////////////////////////////////////////////////////////////////////
omniAsyncInvoker::omniAsyncInvoker(unsigned int max) {
  pd_keep_working = 1;
  pd_lock  = new omni_tracedmutex();
  pd_cond  = new omni_tracedcondition(pd_lock);
  pd_idle_threads = 0;
  pd_nthreads = 0;
  pd_maxthreads = max;
  pd_totalthreads = 0;
}

///////////////////////////////////////////////////////////////////////////
omniAsyncInvoker::~omniAsyncInvoker() {

  pd_lock->lock();
  pd_keep_working = 0;
  while (pd_idle_threads) {
    omniAsyncWorker* t = pd_idle_threads;
    pd_idle_threads = t->pd_next;
    t->pd_next = 0;
    t->pd_in_idle_queue = 0;
    t->pd_cond->signal();
  }
  // Wait for threads to exit
  while (pd_totalthreads) {
    pd_cond->wait();
  }
  pd_lock->unlock();

  delete pd_cond;
  delete pd_lock;
  omniORB::logs(10, "AsyncInvoker: deleted.");
}

///////////////////////////////////////////////////////////////////////////
int
omniAsyncInvoker::insert(omniTask* t) {

  switch (t->category()) {
  case omniTask::AnyTime:
    {
      omni_tracedmutex_lock sync(*pd_lock);

      if (pd_idle_threads) {
	omniAsyncWorker* w = pd_idle_threads;
	pd_idle_threads = w->pd_next;
	w->pd_next = 0;
	OMNIORB_ASSERT(w->pd_task == 0);
	w->pd_task = t;
	w->pd_in_idle_queue = 0;
	w->pd_cond->signal();
      }
      else {
	if (pd_nthreads < pd_maxthreads) {
	  try {
	    pd_nthreads++;
	    pd_totalthreads++;
	    omniAsyncWorker* w = new omniAsyncWorker(this,t);
	    OMNIORB_ASSERT(w);
	  }
	  catch (...) {
	    // Cannot start a new thread.
	    pd_nthreads--;
	    pd_totalthreads--;
	    omniORB::logs(5, "Exception trying to start new thread.");
	    t->enq(pd_anytime_tq);
	  }
	}
	else {
	  t->enq(pd_anytime_tq);
	}
      }
      break;
    }
  case omniTask::ImmediateDispatch:
    {
      omni_tracedmutex_lock sync(*pd_lock);

      if (pd_idle_threads) {
	omniAsyncWorker* w = pd_idle_threads;
	pd_idle_threads = w->pd_next;
	w->pd_next = 0;
	OMNIORB_ASSERT(w->pd_task == 0);
	w->pd_task = t;
	w->pd_in_idle_queue = 0;
	w->pd_cond->signal();
	pd_nthreads--;
      }
      else {
	try {
	  pd_totalthreads++;
	  omniAsyncWorker* w = new omniAsyncWorker(this,t);
	}
	catch(...) {
	  // Cannot start a new thread.
	  pd_totalthreads--;
	  omniORB::logs(5, "Exception trying to start new thread.");
	  return 0;
	}
      }
      break;
    }
  case omniTask::DedicatedThread:
    {
      return insert_dedicated(t);
    }
  }
  return 1;
}

///////////////////////////////////////////////////////////////////////////
int
omniAsyncInvoker::cancel(omniTask* t) {

  if (t->category() == omniTask::AnyTime) {
    omni_tracedmutex_lock sync(*pd_lock);
    omniTaskLink* l;

    for (l = pd_anytime_tq.next; l != &pd_anytime_tq; l =l->next) {
      if ((omniTask*)l == t) {
	l->deq();
	return 1;
      }
    }
  }
  else if (t->category() == omniTask::DedicatedThread) {
    return cancel_dedicated(t);
  }
  return 0;
}

///////////////////////////////////////////////////////////////////////////
//
// Default do-nothing implementations of dedicated thread functions

int
omniAsyncInvoker::work_pending()
{
  return 0;
}

void
omniAsyncInvoker::perform(unsigned long secs, unsigned long nanosecs)
{
  omniORB::logs(1, "omniAsyncInvoker::perform() not implemented. aborting...\n");
  abort();
}

int
omniAsyncInvoker::insert_dedicated(omniTask*)
{
  return 0;
}

int
omniAsyncInvoker::cancel_dedicated(omniTask*)
{
  return 0;
}


///////////////////////////////////////////////////////////////////////////
void
omniTaskLink::enq(omniTaskLink& head) {

  next = head.prev->next;
  head.prev->next = this;
  prev = head.prev;
  head.prev = this;
}


///////////////////////////////////////////////////////////////////////////
void
omniTaskLink::deq() {
  prev->next = next;
  next->prev = prev;
}

///////////////////////////////////////////////////////////////////////////
unsigned int
omniTaskLink::is_empty(omniTaskLink& head) {
  return (head.next == &head);
}

