//                            Package   : omnithread
// omnithread.h               Created on: 7/94 tjr
//
// Copyright (C) Olivetti Research Ltd 1994
//
// omnithread.h
//
// Interface to OMNI thread abstraction.
//
// This file defines virtual base classes for threads and synchronisation
// objects (mutexes, condition variables).
//

/* 
 $Log$
 Revision 1.1  1995/01/25 11:06:44  sll
 Initial revision

 */

#ifndef __omnithread_h_
#define __omnithread_h_

//
// This class and object ensure that we can execute some initialisation code
// before any user code is run. See Stroustrup page 352 for details (but note
// that his example code is wrong - definition of count is not in the header
// file).
//

class omni_thread_init_t {
    static int count;
    void implementation_initialise(void);
public:
    omni_thread_init_t(void);
};

static omni_thread_init_t omni_thread_init;


//
// Mutex
//

class omni_mutex {
public:
    static omni_mutex* create(void);
    virtual ~omni_mutex(void) {};
    virtual void acquire(void) = 0;
    virtual void release(void) = 0;
};


//
// Condition variable
//

class omni_condition {
public:
    static omni_condition* create(omni_mutex* m);
    virtual ~omni_condition(void) {};
    virtual void wait(void) = 0;
    virtual int timed_wait(unsigned long secs, unsigned long nanosecs = 0) = 0;
    virtual void signal(void) = 0;
    virtual void broadcast(void) = 0;
};

//
// Thread
//

typedef void* (*omni_thread_fn_t)(void*);
		// type of function which a new thread executes

class omni_thread {
    friend class omni_thread_init_t;

    static omni_mutex* list_mutex;
    static omni_thread* list_head;
    static omni_thread* list_tail;
    omni_thread* prev;
    omni_thread* next;

    static omni_mutex* next_id_mutex;
    static int next_id;
    int _id;

    static int _default_priority;
    static int _max_priority;

    static omni_thread* create_initial_thread(void);

protected:
    int _priority;
    int detached;
    int holding_list_mutex;

    omni_thread(int detached, int priority);
		// constructor cannot be called by user. Use create() instead.
    virtual ~omni_thread(void);
		// destructor cannot be called by user. Use exit() or cancel()
		// instead.

public:
    static omni_thread* create(omni_thread_fn_t, int detached, void* arg = (void*)0,
			       int priority = _default_priority);
		// create spawns a new thread executing the given function
		// with the given argument at the given priority. Returns a
		// pointer to the thread object on success, null pointer on
		// failure. If a thread is detached, storage for the thread
		// is reclaimed automatically on termination. A detached
		// thread cannot be joined with or synchronously cancelled.

    virtual int join(void**) = 0;
		// join causes the calling thread to wait for another's
		// completion, putting the return value in the variable of
		// type void* whose address is given. Returns 0 if
		// successful, -1 if not. Only non-detached threads may be
		// joined with.

    virtual int cancel(void) = 0;
		// cancel requests that a thread be cancelled. The thread
		// will not necessarily terminate at once. Storage for the
		// thread will not be reclaimed unless the thread is
		// detached or being joined with. Returns 0 if successful,
		// -1 if not.

    virtual int synchronous_cancel(void) = 0;
		// synchronous_cancel is like cancel, but only returns when
		// the thread has actually terminated. Only a non-detached
		// thread can be synchronously cancelled. Returns 0 if
		// successful, -1 if not.

    virtual int set_priority(int) = 0;
		// set the priority of the thread (from 0 to
		// max_priority). Returns 0 if successful, -1 if not.

    int priority(void) { return _priority; }
		// this thread's priority

    int id(void) { return _id; }
		// unique id (within the current process)

    static omni_thread* self(void);
		// returns the calling thread object.

    static void yield(void);
		// allows another thread to run.

    static void sleep(unsigned long secs, unsigned long nanosecs = 0);
		// sleeps for the given time.

    static void exit(void*);
		// causes the calling thread to terminate.

    static void cancel_other_threads(void);
		// calls cancel on all threads other than the calling thread.

    static int set_default_priority(int);
		// set default thread priority (default 0). Must be less
		// than or equal to the maximum. Returns 0 if successful,
		// -1 if not.

    static int default_priority(void) { return _default_priority; }
		// default priority for newly created threads

    static int set_max_priority(int);
		// set maximum thread priority. The range of allowable
		// priorities will be from 0 to this value. It defaults to
		// 0 (ie only a single priority). Returns 0 if successful,
		// -1 if not (ie if the underlying implementation can't
		// cope with that many priorities).

    static int max_priority(void) { return _max_priority; }
		// max thread priority
};

#endif
