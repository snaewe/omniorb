//				Package : omnithread
// omnithread/solaris.cc	Created : 7/94 tjr
//
//    Copyright (C) 1994,1995,1996, 1997 Olivetti & Oracle Research Laboratory
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
// Implementation of OMNI thread abstraction for solaris threads.
//

#include <iostream.h>
#include <stdlib.h>
#include <errno.h>
#include "omnithread.h"

#define DB(x) // x 



///////////////////////////////////////////////////////////////////////////
//
// Mutex
//
///////////////////////////////////////////////////////////////////////////


omni_mutex::omni_mutex(void)
{
    int rc = mutex_init(&sol_mutex, USYNC_THREAD, 0);

    if (rc != 0)
	cerr << "omni_mutex::omni_mutex: mutex_init error " << rc << endl;
}


omni_mutex::~omni_mutex(void)
{
    int rc = mutex_destroy(&sol_mutex);

    if (rc != 0)
	cerr << "omni_mutex::~omni_mutex: mutex_destroy error " << rc << endl;
}


int omni_mutex::lock(void)
{
    return mutex_lock(&sol_mutex);
}


int omni_mutex::unlock(void)
{
    return mutex_unlock(&sol_mutex);
}



///////////////////////////////////////////////////////////////////////////
//
// Condition variable
//
///////////////////////////////////////////////////////////////////////////


omni_condition::omni_condition(omni_mutex* m) : mutex(m)
{
    int rc = cond_init(&sol_cond, USYNC_THREAD, 0);

    if (rc != 0)
	cerr << "omni_condition::omni_condition: cond_init error "
	     << rc << endl;
}


omni_condition::~omni_condition(void)
{
    int rc = cond_destroy(&sol_cond);

    if (rc != 0)
	cerr << "omni_condition::~omni_condition: cond_destroy error "
	     << rc << endl;
}


int omni_condition::wait(void)
{
    return cond_wait(&sol_cond, &mutex->sol_mutex);
}


int omni_condition::timed_wait(unsigned long secs, unsigned long nanosecs)
{
    timespec rqts = { secs, nanosecs };

    int rc = cond_timedwait(&sol_cond, &mutex->sol_mutex, &rqts);

    if (rc == ETIME)
	rc = ETIMEDOUT;

    return rc;
}


int omni_condition::signal(void)
{
    return cond_signal(&sol_cond);
}


int omni_condition::broadcast(void)
{
    return cond_broadcast(&sol_cond);
}



///////////////////////////////////////////////////////////////////////////
//
// Counting semaphore
//
///////////////////////////////////////////////////////////////////////////


omni_semaphore::omni_semaphore(unsigned int initial)
{
    int rc = sema_init(&sol_sem, initial, USYNC_THREAD, NULL);

    if (rc != 0)
	cerr << "omni_semaphore::omni_semaphore: sema_init error "
	     << rc << endl;
}


omni_semaphore::~omni_semaphore(void)
{
    int rc = sema_destroy(&sol_sem);

    if (rc != 0)
	cerr << "omni_semaphore::~omni_semaphore: sema_destroy error "
	     << rc << endl;
}


int omni_semaphore::wait(void)
{
    return sema_wait(&sol_sem);
}


int omni_semaphore::post(void)
{
    return sema_post(&sol_sem);
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

static thread_key_t self_key;


//
// Initialisation function (gets called before any user code).
//

omni_thread::init_t::init_t(void)
{
    if (count++ != 0)	// only do it once however many objects get created.
	return;

    DB(cerr << "omni_thread::init: solaris implementation initialising\n");

    int rc = thr_keycreate(&self_key, NULL);

    if (rc != 0) {
	cerr << "omni_thread::init: thr_keycreate error " << rc << endl;
	::exit(1);
    }

    next_id_mutex = new omni_mutex;

    //
    // Create object for this (i.e. initial) thread.
    //

    omni_thread* t = new omni_thread;

    if (t->_state != STATE_NEW) {
	cerr << "omni_thread::init: problem creating initial thread object\n";
	::exit(1);
    }

    t->_state = STATE_RUNNING;

    t->sol_thread = thr_self();

    DB(cerr << "initial thread " << t->id() << " sol_thread " << t->sol_thread
       << endl);

    rc = thr_setspecific(self_key, (void*)t);

    if (rc != 0) {
	cerr << "omni_thread::init: thr_setspecific error " << rc << endl;
	::exit(1);
    }

    rc = thr_setprio(t->sol_thread, sol_priority(PRIORITY_NORMAL));

    if (rc != 0) {
	cerr << "omni_thread::init: thr_setprio error " << rc << endl;
	::exit(1);
    }
}

//
// Wrapper for thread creation.
//

extern "C" {

    //
    // I'm not sure if you should be able to declare a static member function
    // as extern "C" like this but it seems to work.  If not, all you need
    // is a simple non-member function like this:
    //
    // static void* wrapper2(void* ptr) { return omni_thread::wrapper(ptr) }
    //

    void* omni_thread::wrapper(void* ptr)
    {
	omni_thread* me = (omni_thread*)ptr;

	DB(cerr << "omni_thread::wrapper: thread " << me->id()
	   << " started\n");

	int rc = thr_setspecific(self_key, me);

	if (rc != 0)
	    cerr << "omni_thread::wrapper: thr_setspecific error "
		 << rc << endl;

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

	return NULL;
    }
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

void omni_thread::common_constructor(void* arg, priority_t pri, int det)
{
    _state = STATE_NEW;
    _priority = pri;

    next_id_mutex->lock();
    _id = next_id++;
    next_id_mutex->unlock();

    thread_arg = arg;
    detached = det;	// may be altered in start_undetached()

    // sol_thread is set up in initialisation routine or start().
}


//
// Destructor for omni_thread.
//

omni_thread::~omni_thread(void)
{
    DB(cerr << "destructor called for thread " << id() << endl);
}


//
// Start the thread
//

int omni_thread::start(void)
{
    long flags = 0;

    if (detached)
	flags |= THR_DETACHED;

    mutex.lock();

    if (_state != STATE_NEW) {
	mutex.unlock();
	DB(cerr << "omni_thread::start: thread not in \"new\" state\n");
	return EINVAL;
    }

    int rc = thr_create(0, 0, wrapper, (void*)this, flags, &sol_thread);

    if (rc != 0) {
	mutex.unlock();
	return rc;
    }

    _state = STATE_RUNNING;

    rc = thr_setprio(sol_thread, sol_priority(_priority));

    mutex.unlock();
    return rc;
}

//
// Start a thread which will run the member function run_undetached().
//

int omni_thread::start_undetached(void)
{
    if ((fn_void != NULL) || (fn_ret != NULL))
	return EINVAL;

    detached = 0;
    return start();
}


//
// join - simply check error conditions & call thr_join.
//

int omni_thread::join(void** status)
{
    mutex.lock();

    if ((_state != STATE_RUNNING) && (_state != STATE_TERMINATED)) {
	mutex.unlock();
	DB(cerr << "omni_thread::join: thread not in running or "
	   << "terminated state\n");
	return EINVAL;
    }

    mutex.unlock();

    if (this == self()) {
	DB(cerr << "omni_thread::join: can't join with self\n");
	return EINVAL;
    }

    if (detached) {
	DB(cerr << "omni_thread::join: can't join with detached thread\n");
	return EINVAL;
    }

    DB(cerr << "omni_thread::join: doing thr_join\n");

    int rc = thr_join(sol_thread, (thread_t *)NULL, status);

    if (rc != 0) return rc;

    DB(cerr << "omni_thread::join: thr_join succeeded\n");

    delete this;

    return 0;
}


//
// Change this thread's priority.
//

int omni_thread::set_priority(priority_t pri)
{
    mutex.lock();

    if (_state != STATE_RUNNING) {
	mutex.unlock();
	DB(cerr << "omni_thread::set_priority: thread not in running state\n");
	return EINVAL;
    }

    _priority = pri;

    int rc = thr_setprio(sol_thread, sol_priority(pri));

    mutex.unlock();

    return rc;
}


//
// create - construct a new thread object and start it running.  Returns thread
// object if successful, null pointer if not.
//

// detached version

omni_thread* omni_thread::create(void (*fn)(void*), void* arg, priority_t pri)
{
    omni_thread* t = new omni_thread(fn, arg, pri);

    int rc = t->start();

    if (rc != 0) {
	cerr << "omni_thread::create: start error " << rc << endl;
	delete t;
	return (omni_thread*)NULL;
    }

    return t;
}

// undetached version

omni_thread* omni_thread::create(void* (*fn)(void*), void* arg, priority_t pri)
{
    omni_thread* t = new omni_thread(fn, arg, pri);

    int rc = t->start();

    if (rc != 0) {
	cerr << "omni_thread::create: start error " << rc << endl;
	delete t;
	return (omni_thread*)NULL;
    }

    return t;
}

// backwards-compatible version

omni_thread* omni_thread::create(void* (*fn)(void*), int det, void* arg)
{
    omni_thread* t = new omni_thread(fn, arg, PRIORITY_NORMAL);

    t->detached = det;

    int rc = t->start();

    if (rc != 0) {
	cerr << "omni_thread::create: start error " << rc << endl;
	delete t;
	return (omni_thread*)NULL;
    }

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

void omni_thread::exit(void* return_value)
{
    omni_thread* me = self();

    me->mutex.lock();

    if (me->_state != STATE_RUNNING)
	cerr << "omni_thread::exit: thread not in \"running\" state\n";

    me->_state = STATE_TERMINATED;

    me->mutex.unlock();

    DB(cerr << "omni_thread::exit: thread " << me->id() << " detached "
       << me->detached << " return value " << return_value << endl);

    if (me->detached)
	delete me;

    thr_exit(return_value);
}

omni_thread* omni_thread::self(void)
{
    omni_thread* me;

    int rc = thr_getspecific(self_key, (void**)&me);

    if (rc != 0) {
	cerr << "omni_thread::self: thr_getspecific error " << rc << endl;
	return (omni_thread*)NULL;
    }

    return me;
}

void omni_thread::yield(void)
{
    thr_yield();
}

int omni_thread::sleep(unsigned long secs, unsigned long nanosecs)
{
    timespec rqts = { secs, nanosecs };
    if (nanosleep(&rqts, (timespec*)NULL) != 0)
	return errno;
    return 0;
}

int omni_thread::get_time(unsigned long* abs_sec, unsigned long* abs_nsec,
			  unsigned long rel_sec, unsigned long rel_nsec)
{
    timespec abs;
    clock_gettime(CLOCK_REALTIME, &abs);
    abs.tv_nsec += rel_nsec;
    abs.tv_sec += rel_sec + abs.tv_nsec / 1000000000;
    abs.tv_nsec = abs.tv_nsec % 1000000000;
    *abs_sec = abs.tv_sec;
    *abs_nsec = abs.tv_nsec;
    return 0;
}


int omni_thread::sol_priority(priority_t pri)
{
    switch (pri) {

    case PRIORITY_LOW:
	return 0;

    case PRIORITY_NORMAL:
	return 1;

    case PRIORITY_HIGH:
	return 2;

    default:
	return -1;
    }
}
