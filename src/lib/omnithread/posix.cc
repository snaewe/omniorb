//				Package : omnithread
// omnithread/posix.cc		Created : 7/94 tjr
//
//    Copyright (C) 1994,1995,1996,1997 Olivetti & Oracle Research Laboratory
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
// Implementation of OMNI thread abstraction for posix threads
//
// The source below tests for the definition of the macros:
//     PthreadDraftVersion
//     PthreadSupportThreadPriority
//     NoNanoSleep
//     NeedPthreadInit
//
// As different draft versions of the pthread standard P1003.4a/P1003.1c
// define slightly different APIs, the macro 'PthreadDraftVersion'
// identifies the draft version supported by this particular platform.
//
// Some unix variants do not support thread priority unless a real-time
// kernel option is installed. The macro 'PthreadSupportThreadPriority',
// if defined, enables the use of thread priority. If it is not defined,
// setting or changing thread priority will be silently ignored.
//
// nanosleep() is defined in Posix P1003.4 since Draft 9 (?).
// Not all platforms support this standard. The macro 'NoNanoSleep'
// identifies platform that don't.
//

#include <iostream.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include "omnithread.h"

#ifdef __linux__
#include <pthread/mit/sys/timers.h>
#endif

#define DB(x) // x


#if (PthreadDraftVersion <= 6)
#define ERRNO(x) (((x) != 0) ? (errno) : 0)
#else
#define ERRNO(x) (x)
#endif



///////////////////////////////////////////////////////////////////////////
//
// Mutex
//
///////////////////////////////////////////////////////////////////////////


omni_mutex::omni_mutex(void)
{
#if (PthreadDraftVersion == 4)

    int rc = ERRNO(pthread_mutex_init(&posix_mutex,
				      pthread_mutexattr_default));
#else

    int rc = ERRNO(pthread_mutex_init(&posix_mutex, 0));

#endif

    if (rc != 0)
	cerr << "omni_mutex::omni_mutex: pthread_mutex_init error "
	     << rc << endl;
}


omni_mutex::~omni_mutex(void)
{
    int rc = ERRNO(pthread_mutex_destroy(&posix_mutex));

    if (rc != 0)
	cerr << "omni_mutex::~omni_mutex: pthread_mutex_destroy error "
	     << rc << endl;
}


int omni_mutex::lock(void)
{
    return ERRNO(pthread_mutex_lock(&posix_mutex));
}


int omni_mutex::unlock(void)
{
    return ERRNO(pthread_mutex_unlock(&posix_mutex));
}



///////////////////////////////////////////////////////////////////////////
//
// Condition variable
//
///////////////////////////////////////////////////////////////////////////


omni_condition::omni_condition(omni_mutex* m) : mutex(m)
{
#if (PthreadDraftVersion == 4)

    int rc = ERRNO(pthread_cond_init(&posix_cond, pthread_condattr_default));

#else

    int rc = ERRNO(pthread_cond_init(&posix_cond, 0));

#endif

    if (rc != 0)
	cerr << "omni_condition::omni_condition: pthread_cond_init error "
	     << rc << endl;
}


omni_condition::~omni_condition(void)
{
    int rc = ERRNO(pthread_cond_destroy(&posix_cond));

    if (rc != 0)
	cerr << "omni_condition::~omni_condition: pthread_cond_destroy error "
	     << rc << endl;
}


int omni_condition::wait(void)
{
    return ERRNO(pthread_cond_wait(&posix_cond, &mutex->posix_mutex));
}


int omni_condition::timed_wait(unsigned long secs, unsigned long nanosecs)
{
    timespec rqts = { secs, nanosecs };

    int rc = ERRNO(pthread_cond_timedwait(&posix_cond,
					  &mutex->posix_mutex, &rqts));
#if (PthreadDraftVersion <= 6)
    if (rc == EAGAIN)
	rc = ETIMEDOUT;
#endif

    return rc;
}


int omni_condition::signal(void)
{
    return ERRNO(pthread_cond_signal(&posix_cond));
}


int omni_condition::broadcast(void)
{
    return ERRNO(pthread_cond_broadcast(&posix_cond));
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


int omni_semaphore::wait(void)
{
    m.lock();

    while (value == 0) {

	int rc = c.wait();

	if (rc != 0) {
	    m.unlock();
	    return rc;
	}
    }

    value--;

    m.unlock();

    return 0;
}


int omni_semaphore::try_wait(void)
{
    m.lock();

    if (value == 0) {
	m.unlock();
	return EAGAIN;
    }

    value--;

    m.unlock();

    return 0;
}


int omni_semaphore::post(void)
{
    m.lock();

    if (value == 0) {
	c.signal();
    }

    value++;

    m.unlock();

    return 0;
}



///////////////////////////////////////////////////////////////////////////
//
// Thread
//
///////////////////////////////////////////////////////////////////////////


//
// static variables
//

int omni_thread::init_t::count = 0;

omni_mutex* omni_thread::next_id_mutex;
int omni_thread::next_id = 0;

static pthread_key_t self_key;

#ifdef PthreadSupportThreadPriority
static int lowest_priority;
static int normal_priority;
static int highest_priority;
#endif


//
// Initialisation function (gets called before any user code).
//

omni_thread::init_t::init_t(void)
{
    if (count++ != 0)	// only do it once however many objects get created.
	return;

    DB(cerr << "omni_thread::init: posix 1003.4a/1003.1c (draft "
       << PthreadDraftVersion << ") implementation initialising\n");

#ifdef NeedPthreadInit

    pthread_init();

#endif

#if (PthreadDraftVersion == 4)

    int rc = ERRNO(pthread_keycreate(&self_key, NULL));

    if (rc != 0) {
	cerr << "omni_thread::init: pthread_keycreate error " << rc << endl;
	::exit(1);
    }

#else

    int rc = ERRNO(pthread_key_create(&self_key, NULL));

    if (rc != 0) {
	cerr << "omni_thread::init: pthread_key_create error " << rc << endl;
	::exit(1);
    }

#endif

#ifdef PthreadSupportThreadPriority

#if defined(__osf1__) && defined(__alpha__)

    lowest_priority = PRI_OTHER_MIN;
    highest_priority = PRI_OTHER_MAX;

#elif defined(__sunos__) && (__OSVERSION__ == 5)

    // a bug in pthread_attr_setschedparam means lowest priority is 1 not 0

    lowest_priority  = 1;
    highest_priority = 3;

#else

    lowest_priority = sched_get_priority_min(SCHED_FIFO);
    highest_priority = sched_get_priority_max(SCHED_FIFO);

#endif

    switch (highest_priority - lowest_priority) {

    case 0:
    case 1:
	normal_priority = lowest_priority;
	break;

    default:
	normal_priority = lowest_priority + 1;
	break;
    }

#endif   /* PthreadSupportThreadPriority */

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

    t->posix_thread = pthread_self();

    DB(cerr << "initial thread " << t->id() << endl);

    rc = ERRNO(pthread_setspecific(self_key, (void*)t));

    if (rc != 0) {
	cerr << "omni_thread::init: pthread_setspecific error " << rc << endl;
	::exit(1);
    }

#ifdef PthreadSupportThreadPriority

#if (PthreadDraftVersion == 4)

    rc = ERRNO(pthread_setprio(t->posix_thread,
			       posix_priority(PRIORITY_NORMAL)));
    if (rc != 0) {
	cerr << "omni_thread::init: pthread_setprio error " << rc << endl;
	::exit(1);
    }

#elif (PthreadDraftVersion == 6)

    pthread_attr_t attr;
    pthread_attr_init(&attr);

    rc = ERRNO(pthread_attr_setprio(&attr, posix_priority(PRIORITY_NORMAL)));

    if (rc != 0) {
	cerr << "omni_thread::init: pthread_attr_setprio error " << rc << endl;
	::exit(1);
    }

    rc = ERRNO(pthread_setschedattr(t->posix_thread, attr));

    if (rc != 0) {
	cerr << "omni_thread::init: pthread_setschedattr error " << rc << endl;
	::exit(1);
    }

#else

    struct sched_param sparam;

    sparam.sched_priority = posix_priority(PRIORITY_NORMAL);

    rc = ERRNO(pthread_setschedparam(t->posix_thread, SCHED_OTHER, &sparam));

    if (rc != 0) {
	cerr << "omni_thread::init: pthread_setschedparam error "
	     << rc << endl;
	::exit(1);
    }

#endif   /* PthreadDraftVersion */

#endif   /* PthreadSupportThreadPriority */
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

	int rc = ERRNO(pthread_setspecific(self_key, me));

	if (rc != 0)
	    cerr << "omni_thread wrapper: pthread_setspecific error "
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

    // posix_thread is set up in initialisation routine or start().
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
    int rc;

    mutex.lock();

    if (_state != STATE_NEW) {
	mutex.unlock();
	DB(cerr << "omni_thread::start: thread not in \"new\" state\n");
	return EINVAL;
    }

    pthread_attr_t attr;

#if (PthreadDraftVersion == 4)
    pthread_attr_create(&attr);
#else
    pthread_attr_init(&attr);
#endif

#ifdef PthreadSupportThreadPriority

#if (PthreadDraftVersion <= 6)

    rc = ERRNO(pthread_attr_setprio(&attr, posix_priority(_priority)));

    if (rc != 0) {
	DB(cerr << "omni_thread::start: pthread_attr_setprio error "
	   << rc << endl);
	mutex.unlock();
	return rc;
    }

#else

    struct sched_param sparam;

    sparam.sched_priority = posix_priority(_priority);
    rc = ERRNO(pthread_attr_setschedparam(&attr, &sparam));

    if (rc != 0) {
	DB(cerr << "omni_thread::start: pthread_attr_setschedparam error "
	   << rc << endl);
	mutex.unlock();
	return rc;
    }

#endif	/* PthreadDraftVersion */

#endif	/* PthreadSupportThreadPriority */

#if defined(__osf1__) && defined(__alpha__)

    // omniORB requires a larger stack size than the default (21120)
    // on OSF/1

    rc = ERRNO(pthread_attr_setstacksize(&attr, 32768));

    if (rc != 0) {
	DB(cerr << "omni_thread::start: pthread_attr_setstacksize error "
	   << rc << endl);
	mutex.unlock();
	return rc;
    }

#endif	/* __osf1__ && __alpha__ */

#if (PthreadDraftVersion == 4)
    rc = ERRNO(pthread_create(&posix_thread, attr, wrapper, (void*)this));
#else
    rc = ERRNO(pthread_create(&posix_thread, &attr, wrapper, (void*)this));
#endif
    if (rc != 0) {
	DB(cerr << "omni_thread::start: pthread_create error " << rc << endl);
	mutex.unlock();
	return rc;
    }

#if (PthreadDraftVersion == 4)
    pthread_attr_delete(&attr);
#else
    pthread_attr_destroy(&attr);
#endif

    _state = STATE_RUNNING;

    if (detached) {

#if (PthreadDraftVersion <= 6)
	rc = ERRNO(pthread_detach(&posix_thread));
#else
	rc = ERRNO(pthread_detach(posix_thread));
#endif
	if (rc != 0) {
	    DB(cerr << "omni_thread::start: pthread_detach error "
	       << rc << endl);
	    mutex.unlock();
	    return rc;
	}
    }

    mutex.unlock();

    return 0;
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
// join - simply check error conditions & call pthread_join.
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

    DB(cerr << "omni_thread::join: doing pthread_join\n");

    int rc = ERRNO(pthread_join(posix_thread, status));

    if (rc != 0) return rc;

    DB(cerr << "omni_thread::join: pthread_join succeeded\n");

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

#ifdef PthreadSupportThreadPriority

#if (PthreadDraftVersion == 4)

    int rc = ERRNO(pthread_setprio(posix_thread, posix_priority(pri)));

    if (rc != 0) {
	mutex.unlock();
	return rc;
    }

#elif (PthreadDraftVersion == 6)

    pthread_attr_t attr;
    pthread_attr_init(&attr);

    int rc = ERRNO(pthread_attr_setprio(&attr, posix_priority(pri)));

    if (rc != 0) {
	mutex.unlock();
	return rc;
    }

    rc = ERRNO(pthread_setschedattr(posix_thread, attr));

    if (rc != 0) {
	mutex.unlock();
	return rc;
    }

#else

    struct sched_param sparam;

    sparam.sched_priority = posix_priority(pri);

    int rc = ERRNO(pthread_setschedparam(posix_thread, SCHED_OTHER, &sparam));

    if (rc != 0) {
	mutex.unlock();
	return rc;
    }

#endif   /* PthreadDraftVersion */

#endif   /* PthreadSupportThreadPriority */

    mutex.unlock();

    return 0;
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

    pthread_exit(return_value);
}

omni_thread* omni_thread::self(void)
{
    omni_thread* me;

#if (PthreadDraftVersion <= 6)

    int rc = ERRNO(pthread_getspecific(self_key, (void**)&me));

    if (rc != 0) {
	cerr << "omni_thread::self: pthread_getspecific error " << rc << endl;
	return (omni_thread*)NULL;
    }

#else

    me = (omni_thread *)pthread_getspecific(self_key);
    if (me == NULL) {
	cerr << "omni_thread::self: pthread_getspecific error" << endl;
	return (omni_thread*)NULL;
    }

#endif

    return me;
}

void omni_thread::yield(void)
{
#if (PthreadDraftVersion == 6)

    pthread_yield(NULL);

#elif (PthreadDraftVersion < 9)

    pthread_yield();

#else

    int rc = sched_yield();

    if (rc != 0) {
	cerr << "omni_thread::yield: sched_yield error " << errno << endl;
	::exit(1);
    }

#endif
}

int omni_thread::sleep(unsigned long secs, unsigned long nanosecs)
{
    timespec rqts = { secs, nanosecs };
#ifndef NoNanoSleep
    if (nanosleep(&rqts, (timespec*)NULL) != 0)
	return errno;
#else
#if defined(__osf1__) && defined(__alpha__)
    if (pthread_delay_np(&rqts) != 0)
	return errno;
#elif defined(__linux__)
    if (secs > 2000) {
	sleep(secs);
    } else {
	usleep(secs * 1000000 + (nanosecs / 1000));
    }
#else
    cerr << "Fatal: omni_thread::sleep is not supported." << endl;
    ::exit(1);
#endif
#endif
    return 0;
}

int omni_thread::get_time(unsigned long* abs_sec, unsigned long* abs_nsec,
			  unsigned long rel_sec, unsigned long rel_nsec)
{
    int rc = 0;
#if defined(__osf1__) && defined(__alpha__)
    timespec abs, rel;
    rel.tv_sec = rel_sec;
    rel.tv_nsec = rel_nsec;
    rc = ERRNO(pthread_get_expiration_np(&rel, &abs));
#else
    timespec abs;
#ifdef __linux__
    struct timeval tv;
    gettimeofday(&tv, NULL); 
    abs.tv_sec = tv.tv_sec;
    abs.tv_nsec = tv.tv_usec * 1000;
#else
    clock_gettime(CLOCK_REALTIME, &abs);
#endif
    abs.tv_nsec += rel_nsec;
    abs.tv_sec += rel_sec + abs.tv_nsec / 1000000000;
    abs.tv_nsec = abs.tv_nsec % 1000000000;
#endif
    *abs_sec = abs.tv_sec;
    *abs_nsec = abs.tv_nsec;
    return rc;
}


int omni_thread::posix_priority(priority_t pri)
{
    switch (pri) {

#ifdef PthreadSupportThreadPriority

    case PRIORITY_LOW:
	return lowest_priority;

    case PRIORITY_NORMAL:
	return normal_priority;

    case PRIORITY_HIGH:
	return highest_priority;

#endif

    default:
	return -1;
    }
}


//
// The inclusion of cancel() here is a dirty hack to keep omniORB happy.
// It should disappear in the near future.
//

int omni_thread::cancel(void)
{
#ifdef __linux__
    cerr << "omni_thread::cancel: no pthread_cancel on this platform\n";
    return ENOSYS;
#else
    if (this == self()) {
	DB(cerr << "omni_thread::cancel: can't cancel self\n");
	return EINVAL;
    }

    DB(cerr << "omni_thread::cancel: doing pthread_cancel on " << id()
       << "\n");

    int rc = ERRNO(pthread_cancel(posix_thread));

    if (rc != 0) {
	DB(cerr << "omni_thread::cancel: pthread_cancel failed\n");
	return rc;
    }

    return 0;
#endif
}
