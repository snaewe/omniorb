//				Package : omnithread
// omnithread/os2.cc     	Created : 6/95 tjr
//                              Ported  : 5/01 DLS
//
//    Copyright (C) 1995-1999 AT&T Laboratories Cambridge
//
//    This file is part of the omnithread library
//
//    The omnithread library is free software; you can redistribute it and/or
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
// Implementation of OMNI thread abstraction for OS/2 threads - compile
// using gcc/pgcc/emx.  This was shamelessly stolen and translated from
// the NT (and a little of the posix) implementation.

#include <stdlib.h>
#include <errno.h>
#include <omnithread.h>
#include <process.h>
#include <sys/time.h>

#define DB(x) // x 
//#include <iostream.h> or #include <iostream> if DB is on.

static void get_time_now(unsigned long* abs_sec, unsigned long* abs_nsec);

///////////////////////////////////////////////////////////////////////////
//
// Mutex
//
///////////////////////////////////////////////////////////////////////////


omni_mutex::omni_mutex(void)
{
    APIRET ulrc = DosCreateMutexSem(NULL,
                                    &crit,
                                    DC_SEM_SHARED,
                                    0);
}

omni_mutex::~omni_mutex(void)
{
    APIRET ulrc = DosCloseMutexSem(crit);
}

void
omni_mutex::lock(void)
{
    DosRequestMutexSem(crit,SEM_INDEFINITE_WAIT);
}

void
omni_mutex::unlock(void)
{
    DosReleaseMutexSem(crit);
}



///////////////////////////////////////////////////////////////////////////
//
// Condition variable
//
///////////////////////////////////////////////////////////////////////////


//
// Condition variables are tricky to implement using OS/2 synchronization
// primitives, since none of them have the atomic "release mutex and wait to be
// signalled" which is central to the idea of a condition variable.  To get
// around this the solution is to record which threads are waiting and
// explicitly wake up those threads.
//
// Here we implement a condition variable using a list of waiting threads
// (protected by a critical section), and a per-thread semaphore (which
// actually only needs to be a binary semaphore).
//
// To wait on the cv, a thread puts itself on the list of waiting threads for
// that cv, then releases the mutex and waits on its own personal semaphore.  A
// signalling thread simply takes a thread from the head of the list and kicks
// that thread's semaphore.  Broadcast is simply implemented by kicking the
// semaphore of each waiting thread.
//
// The only other tricky part comes when a thread gets a timeout from a timed
// wait on its semaphore.  Between returning with a timeout from the wait and
// entering the critical section, a signalling thread could get in, kick the
// waiting thread's semaphore and remove it from the list.  If this happens,
// the waiting thread's semaphore is now out of step so it needs resetting, and
// the thread should indicate that it was signalled rather than that it timed
// out.
//
// It is possible that the thread calling wait or timedwait is not a
// omni_thread. In this case we have to provide a temporary data structure,
// i.e. for the duration of the call, for the thread to link itself on the
// list of waiting threads. _internal_omni_thread_dummy provides such
// a data structure and _internal_omni_thread_helper is a helper class to
// deal with this special case for wait() and timedwait(). Once created,
// the _internal_omni_thread_dummy is cached for use by the next wait() or
// timedwait() call from a non-omni_thread. This is probably worth doing
// because creating a Semaphore is quite heavy weight.

class _internal_omni_thread_helper;

class _internal_omni_thread_dummy : public omni_thread {
public:
  inline _internal_omni_thread_dummy() : next(0) { }
  inline ~_internal_omni_thread_dummy() { }
  friend class _internal_omni_thread_helper;
private:
  _internal_omni_thread_dummy* next;
};

class _internal_omni_thread_helper {
public:
  inline _internal_omni_thread_helper()  {
    d = 0;
    t = omni_thread::self();
    if (!t) {
      omni_mutex_lock sync(cachelock);
      if (cache) {
	d = cache;
	cache = cache->next;
      }
      else {
	d = new _internal_omni_thread_dummy;
      }
      t = d;
    }
  }
  inline ~_internal_omni_thread_helper() {
    if (d) {
      omni_mutex_lock sync(cachelock);
      d->next = cache;
      cache = d;
    }
  }
  inline operator omni_thread* () { return t; }
  inline omni_thread* operator->() { return t; }

  static _internal_omni_thread_dummy* cache;
  static omni_mutex                   cachelock;

private:
  _internal_omni_thread_dummy* d;
  omni_thread*                 t;
};

_internal_omni_thread_dummy* _internal_omni_thread_helper::cache = 0;
omni_mutex                   _internal_omni_thread_helper::cachelock;


omni_condition::omni_condition(omni_mutex* m) : mutex(m)
{
    APIRET ulrc = DosCreateMutexSem(NULL,
                                    &crit,
                                    DC_SEM_SHARED,
                                    0);

    waiting_head = waiting_tail = NULL;
}


omni_condition::~omni_condition(void)
{
    DosReleaseMutexSem(crit);
//    DB( if (waiting_head != NULL) {
//	cerr << "omni_condition::~omni_condition: list of waiting threads "
//	     << "is not empty\n";
//    } )
}


void
omni_condition::wait(void)
{
    _internal_omni_thread_helper me;

    DosRequestMutexSem(crit,SEM_INDEFINITE_WAIT);

    me->cond_next = NULL;
    me->cond_prev = waiting_tail;
    if (waiting_head == NULL)
	waiting_head = me;
    else
	waiting_tail->cond_next = me;
    waiting_tail = me;
    me->cond_waiting = TRUE;

    DosReleaseMutexSem(crit);

    mutex->unlock();

    APIRET ulrc = DosWaitEventSem(me->cond_semaphore,SEM_INDEFINITE_WAIT);

    mutex->lock();

    if (ulrc != 0)
	throw omni_thread_fatal(ulrc);
}


int
omni_condition::timedwait(unsigned long abs_sec, unsigned long abs_nsec)
{
    _internal_omni_thread_helper me;

    DosRequestMutexSem(crit,SEM_INDEFINITE_WAIT);

    me->cond_next = NULL;
    me->cond_prev = waiting_tail;
    if (waiting_head == NULL)
	waiting_head = me;
    else
	waiting_tail->cond_next = me;
    waiting_tail = me;
    me->cond_waiting = TRUE;

    DosReleaseMutexSem(crit);

    mutex->unlock();

    unsigned long now_sec, now_nsec;

    get_time_now(&now_sec, &now_nsec);

    unsigned timeout;
    if ((abs_sec <= now_sec) && ((abs_sec < now_sec) || (abs_nsec < now_nsec)))
      timeout = 0;
    else {
      timeout = (abs_sec-now_sec) * 1000;

      if( abs_nsec < now_nsec )  timeout -= (now_nsec-abs_nsec) / 1000000;
      else                       timeout += (abs_nsec-now_nsec) / 1000000;
    }

    APIRET result = DosWaitEventSem(me->cond_semaphore,timeout);

    if (result == ERROR_TIMEOUT) {
	DosRequestMutexSem(crit,SEM_INDEFINITE_WAIT);

	if (me->cond_waiting) {
	    if (me->cond_prev != NULL)
		me->cond_prev->cond_next = me->cond_next;
	    else
		waiting_head = me->cond_next;
	    if (me->cond_next != NULL)
		me->cond_next->cond_prev = me->cond_prev;
	    else
		waiting_tail = me->cond_prev;
	    me->cond_waiting = FALSE;

	    DosReleaseMutexSem(crit);

	    mutex->lock();
	    return 0;
	}

	//
	// We timed out but another thread still signalled us.  Wait for
	// the semaphore (it _must_ have been signalled) to decrement it
	// again.  Return that we were signalled, not that we timed out.
	//

	DosReleaseMutexSem(crit);

        result = DosWaitEventSem(me->cond_semaphore,SEM_INDEFINITE_WAIT);
    }

    if (result != NO_ERROR)
	throw omni_thread_fatal(result);
    mutex->lock();
    return 1;
}


void
omni_condition::signal(void)
{
    DosRequestMutexSem(crit,SEM_INDEFINITE_WAIT);

    if (waiting_head != NULL) {
	omni_thread* t = waiting_head;
	waiting_head = t->cond_next;
	if (waiting_head == NULL)
	    waiting_tail = NULL;
	else
	    waiting_head->cond_prev = NULL;
	t->cond_waiting = FALSE;

        APIRET ulrc = DosPostEventSem(t->cond_semaphore);
	if (ulrc != NO_ERROR) {
	    DosReleaseMutexSem(crit);
	    throw omni_thread_fatal(ulrc);
	}
    }

    DosReleaseMutexSem(crit);
}


void
omni_condition::broadcast(void)
{
    APIRET ulrc;

    DosRequestMutexSem(crit,SEM_INDEFINITE_WAIT);

    while (waiting_head != NULL) {
	omni_thread* t = waiting_head;
	waiting_head = t->cond_next;
	if (waiting_head == NULL)
	    waiting_tail = NULL;
	else
	    waiting_head->cond_prev = NULL;
	t->cond_waiting = FALSE;

        ulrc = DosPostEventSem(t->cond_semaphore);
	if (ulrc != NO_ERROR) {
	    DosReleaseMutexSem(crit);
	    throw omni_thread_fatal(ulrc);
	}
    }

    DosReleaseMutexSem(crit);
}



///////////////////////////////////////////////////////////////////////////
//
// Counting semaphore
//
///////////////////////////////////////////////////////////////////////////


omni_semaphore::omni_semaphore(unsigned int initial) : c(&m)
{
    value = initial;
}

omni_semaphore::~omni_semaphore(void)
{
}

void
omni_semaphore::wait(void)
{
    omni_mutex_lock l(m);

    while (value == 0)
	c.wait();

    value--;
}

int
omni_semaphore::trywait(void)
{
    omni_mutex_lock l(m);

    if (value == 0)
	return 0;

    value--;
    return 1;
}

void
omni_semaphore::post(void)
{
    {
        omni_mutex_lock l(m);
        value++;
    }

    c.signal();
}


///////////////////////////////////////////////////////////////////////////
//
// Thread
//
///////////////////////////////////////////////////////////////////////////


//
// Static variables
//

int omni_thread::init_t::count = 0;

omni_mutex* omni_thread::next_id_mutex;
int omni_thread::next_id = 0;
static unsigned long *pself_tls_index;

//
// Initialization function (gets called before any user code).
//

omni_thread::init_t::init_t(void)
{
    ULONG ulrc;
    PTIB ptib = NULL;

    if (count++ != 0)	// only do it once however many objects get created.
	return;

    DB(cerr << "omni_thread::init: OS/2 implementation initializing\n");

    ulrc = DosAllocThreadLocalMemory(1,&pself_tls_index);
    //self_tls_index = TlsAlloc();

    if (pself_tls_index == NULL)
	throw omni_thread_fatal(ulrc);

    next_id_mutex = new omni_mutex;

    //
    // Create object for this (i.e. initial) thread.
    //

    omni_thread* t = new omni_thread;

    t->_state = STATE_RUNNING;

    ulrc = DosGetInfoBlocks(&ptib, NULL);
    if (ulrc == 0)
      t->nt_id = ptib->tib_ptib2->tib2_ultid;
    DB(cerr << "omni_thread::init: initial thread " << t->id() << " OS/2 thread id " << t->nt_id << endl);

    DB(cerr << "initial thread " << t->id() << " NT thread id " << t->nt_id << endl);

    pself_tls_index = (unsigned long*)t;

    ulrc = DosSetPriority(PRTYS_THREAD,
                          nt_priority(PRIORITY_NORMAL),
                          0,
                          t->nt_id);

}

//
// Wrapper for thread creation.
//

extern "C"
void APIENTRY
omni_thread_wrapper(void* ptr)
{
    omni_thread* me = (omni_thread*)ptr;

    DB(cerr << "omni_thread_wrapper: thread " << me->id()
       << " started\n");

    pself_tls_index = (unsigned long*)me;

    //
    // Now invoke the thread function with the given argument.
    //

    if (me->fn_void != NULL) {
	(*me->fn_void)(me->thread_arg);
	omni_thread::exit();
    }

    if (me->fn_ret != NULL) {
	void* return_value = (*me->fn_ret)(me->thread_arg);
	omni_thread::exit(return_value);
    }

    if (me->detached) {
	me->run(me->thread_arg);
	omni_thread::exit();
    } else {
	void* return_value = me->run_undetached(me->thread_arg);
	omni_thread::exit(return_value);
    }

    // should never get here.
}


//
// Constructors for omni_thread - set up the thread object but don't
// start it running.
//

// construct a detached thread running a given function.

omni_thread::omni_thread(void (*fn)(void*), void* arg, priority_t pri)
{
    common_constructor(arg, pri, 1);
    fn_void = fn;
    fn_ret = NULL;
}

// construct an undetached thread running a given function.

omni_thread::omni_thread(void* (*fn)(void*), void* arg, priority_t pri)
{
    common_constructor(arg, pri, 0);
    fn_void = NULL;
    fn_ret = fn;
}

// construct a thread which will run either run() or run_undetached().

omni_thread::omni_thread(void* arg, priority_t pri)
{
    common_constructor(arg, pri, 1);
    fn_void = NULL;
    fn_ret = NULL;
}

// common part of all constructors.

void
omni_thread::common_constructor(void* arg, priority_t pri, int det)
{
    _state = STATE_NEW;
    _priority = pri;

    next_id_mutex->lock();
    _id = next_id++;
    next_id_mutex->unlock();

    thread_arg = arg;
    detached = det;	// may be altered in start_undetached()

    APIRET ulrc = DosCreateEventSem(NULL, &cond_semaphore, 0L, 0);

    if (ulrc != NO_ERROR)
	throw omni_thread_fatal(ulrc);

    cond_next = cond_prev = NULL;
    cond_waiting = FALSE;

}


//
// Destructor for omni_thread.
//

omni_thread::~omni_thread(void)
{
    DosFreeThreadLocalMemory(pself_tls_index);
    DB(cerr << "destructor called for thread " << id() << endl);
    // Don't know if this will leak...
    _endthread();
}


//
// Start the thread
//

void
omni_thread::start(void)
{
APIRET ulrc;

    omni_mutex_lock l(mutex);

    if (_state != STATE_NEW)
	throw omni_thread_invalid();
    TID nt_id;

    nt_id = _beginthread(omni_thread_wrapper,
                         NULL,
                         16384,
                         this);
    if (nt_id == -1)
      throw omni_thread_fatal();
    _state = STATE_RUNNING;

    ulrc = DosSetPriority(PRTYS_THREAD,
                          nt_priority(_priority),
                          0,
                          nt_id);

}


//
// Start a thread which will run the member function run_undetached().
//

void
omni_thread::start_undetached(void)
{
    if ((fn_void != NULL) || (fn_ret != NULL))
	throw omni_thread_invalid();

    detached = 0;
    start();
}


//
// join - simply check error conditions & call WaitForSingleObject.
//

void
omni_thread::join(void** status)
{
    mutex.lock();

    if ((_state != STATE_RUNNING) && (_state != STATE_TERMINATED)) {
	mutex.unlock();
	throw omni_thread_invalid();
    }

    mutex.unlock();

    if (this == self())
	throw omni_thread_invalid();

    if (detached)
	throw omni_thread_invalid();

    DB(cerr << "omni_thread::join: doing DosWaitThread\n");

    DosWaitThread(&nt_id,DCWW_WAIT);

    DB(cerr << "omni_thread::join: DosWaitThread succeeded\n");

    if (status)
      *status = return_val;

    delete this;
}


//
// Change this thread's priority.
//

void
omni_thread::set_priority(priority_t pri)
{
    omni_mutex_lock l(mutex);

    if (_state != STATE_RUNNING)
	throw omni_thread_invalid();

    _priority = pri;

    APIRET ulrc = DosSetPriority(PRTYS_THREAD,
                                 nt_priority(pri),
                                 0,
                                 nt_id);

    if (ulrc != NO_ERROR)
	throw omni_thread_fatal(ulrc);
}


//
// create - construct a new thread object and start it running.  Returns thread
// object if successful, null pointer if not.
//

// detached version

omni_thread*
omni_thread::create(void (*fn)(void*), void* arg, priority_t pri)
{
    omni_thread* t = new omni_thread(fn, arg, pri);
    t->start();
    return t;
}

// undetached version

omni_thread*
omni_thread::create(void* (*fn)(void*), void* arg, priority_t pri)
{
    omni_thread* t = new omni_thread(fn, arg, pri);
    t->start();
    return t;
}


//
// exit() _must_ lock the mutex even in the case of a detached thread.  This is
// because a thread may run to completion before the thread that created it has
// had a chance to get out of start().  By locking the mutex we ensure that the
// creating thread must have reached the end of start() before we delete the
// thread object.  Of course, once the call to start() returns, the user can
// still incorrectly refer to the thread object, but that's their problem.
//

void
omni_thread::exit(void* return_value)
{
    omni_thread* me = self();

    if (me)
      {
	me->mutex.lock();

	me->_state = STATE_TERMINATED;

	me->mutex.unlock();

	DB(cerr << "omni_thread::exit: thread " << me->id() << " detached "
	   << me->detached << " return value " << return_value << endl);

	if (me->detached) {
	  delete me;
	} else {
	  me->return_val = return_value;
	}
      }
    else
	DB(cerr << "omni_thread::exit: called with a non-omnithread. Exit quietly." << endl);
    DosExit(EXIT_THREAD,0);
}


omni_thread*
omni_thread::self(void)
{
    omni_thread *me;

    me = (omni_thread*)pself_tls_index;
    if (me == NULL) {
      DB(cerr << "omni_thread::self: called with a non-ominthread. NULL is returned." << endl);
    }
    return me;
}


void
omni_thread::yield(void)
{
    DosSleep(0);
}


#define MAX_SLEEP_SECONDS (ULONG)4294966	// (2**32-2)/1000

void
omni_thread::sleep(unsigned long secs, unsigned long nanosecs)
{
    if (secs <= MAX_SLEEP_SECONDS) {
	DosSleep(secs * 1000 + nanosecs / 1000000);
	return;
    }

    unsigned long no_of_max_sleeps = secs / MAX_SLEEP_SECONDS,
      i;

    for (i = 0; i < no_of_max_sleeps; i++)
	DosSleep(MAX_SLEEP_SECONDS * 1000);

    DosSleep((secs % MAX_SLEEP_SECONDS) * 1000 + nanosecs / 1000000);
}


void
omni_thread::get_time(unsigned long* abs_sec, unsigned long* abs_nsec,
		      unsigned long rel_sec, unsigned long rel_nsec)
{
    get_time_now(abs_sec, abs_nsec);
    *abs_nsec += rel_nsec;
    *abs_sec += rel_sec + *abs_nsec / 1000000000;
    *abs_nsec = *abs_nsec % 1000000000;
}


int
omni_thread::nt_priority(priority_t pri)
{
    switch (pri) {

    case PRIORITY_LOW:
	return PRTYC_IDLETIME;

    case PRIORITY_NORMAL:
	return PRTYC_REGULAR;

    case PRIORITY_HIGH:
	return PRTYC_TIMECRITICAL;
    }

    throw omni_thread_invalid();
    return 0;
}


static void
get_time_now(unsigned long* abs_sec, unsigned long* abs_nsec)
{
    struct timeval tv;
    gettimeofday(&tv, NULL); 

    *abs_sec = tv.tv_sec;
    *abs_nsec = tv.tv_usec * 1000;
}
