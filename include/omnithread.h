//				Package : omnithread
// omnithread.h			Created : 7/94 tjr
//
// Copyright (C) Olivetti Research Ltd 1994, 1995
//
// Interface to OMNI thread abstraction.
//
// This file declares classes for threads and synchronisation objects
// (mutexes, condition variables and counting semaphores).
//
// Unless otherwise stated below, functions which return int return 0 if
// successful, non-zero error number if not.
//
// Wherever a seemingly arbitrary choice has had to be made as to the interface
// provided, the intention here has been to be as POSIX-like as possible.  This
// is why there is no semaphore timed wait, for example.
//

/* 
 * $Log$
 * Revision 1.4  1997/05/06 15:47:50  ewc
 * Fixed small Macro bug.
 *
 * Revision 1.3  1997/04/01  12:28:50  ewc
 * *** empty log message ***
 *
 * Revision 1.2  1995/08/17  10:14:55  tjr
 * new version of OMNI thread abstraction
 *
 */

#ifndef __omnithread_h_
#define __omnithread_h_

#ifndef NULL
#define NULL (void*)0
#endif

class omni_mutex;
class omni_condition;
class omni_semaphore;
class omni_thread;

//
// OMNI_THREAD_EXPOSE can be defined as public or protected to expose the
// implementation class - this may be useful for debugging.  Hopefully this
// won't change the underlying structure which the compiler generates so that
// this can work without recompiling the library.
//

#ifndef OMNI_THREAD_EXPOSE
#define OMNI_THREAD_EXPOSE private
#endif


//
// Include implementation-specific header file.
//
// This must define 4 CPP macros of the form OMNI_x_IMPLEMENTATION for mutex,
// condition variable, semaphore and thread.  Each should define any
// implementation-specific members of the corresponding classes.
//


#if defined(__arm__) && defined(__atmos__)
#include "omnithread/posix.h"

#elif defined(__alpha__) && defined(__osf1__)
#include "omnithread/posix.h"

#elif defined(__NT__)
#include "omnithread/nt.h"

#ifdef _MSC_VER

// Using MSVC++ to compile. If compiling library as a DLL,
// define _OMNITHREAD_DLL. If compiling as a statuc library, define
// _WINSTATIC
// If compiling an application that is to be statically linked to omnithread,
// define _WINSTATIC (if the application is  to be dynamically linked, 
// there is no need to define any of these macros).

#if defined (_OMNITHREAD_DLL) && defined(_WINSTATIC)
#error "Both _OMNITHREAD_DLL and _WINSTATIC are defined."
#elif defined(_OMNITHREAD_DLL)
#define _OMNITHREAD_NTDLL_ __declspec(dllexport)
#elif !defined(_WINSTATIC)
#define _OMNITHREAD_NTDLL_ __declspec(dllimport)
#elif defined(_WINSTATIC)
#define _OMNITHREAD_NTDLL_
#endif
 // _OMNITHREAD_DLL && _WINSTATIC

#else

// Not using MSVC++ to compile
#define _OMNITHREAD_NTDLL_

#endif
 // _MSC_VER
 
#elif defined(__sunos__) && (__OSVERSION__ == 5)
#ifdef UsePthread
#include "omnithread/posix.h"
#else
#include "omnithread/solaris.h"
#endif

#elif defined(__linux__)
#include "omnithread/posix.h"

#else
#error "No implementation header file"
#endif

#if !defined(__NT__)
#define _OMNITHREAD_NTDLL_
#endif

#if (!defined(OMNI_MUTEX_IMPLEMENTATION) || \
     !defined(OMNI_CONDITION_IMPLEMENTATION) || \
     !defined(OMNI_SEMAPHORE_IMPLEMENTATION) || \
     !defined(OMNI_THREAD_IMPLEMENTATION))
#error "Implementation header file incomplete"
#endif


///////////////////////////////////////////////////////////////////////////
//
// Mutex
//
///////////////////////////////////////////////////////////////////////////

class _OMNITHREAD_NTDLL_ omni_mutex {

public:
    omni_mutex(void);
    ~omni_mutex(void);

    int lock(void);
    int unlock(void);
    int acquire(void) { return lock(); }
    int release(void) { return unlock(); }
	// the names lock and unlock are preferred over acquire and release
	// since we are attempting to be as POSIX-like as possible.

    static omni_mutex* create(void) { return new omni_mutex; }
	// for backwards compatibility

    friend class omni_condition;

OMNI_THREAD_EXPOSE:
    OMNI_MUTEX_IMPLEMENTATION
};


///////////////////////////////////////////////////////////////////////////
//
// Condition variable
//
///////////////////////////////////////////////////////////////////////////

class _OMNITHREAD_NTDLL_ omni_condition {

    omni_mutex* mutex;

public:
    omni_condition(omni_mutex* m);
	// constructor must be given a pointer to an existing mutex. The
	// condition variable is then linked to the mutex, so that there is an
	// implicit unlock and lock around wait() and timed_wait().

    ~omni_condition(void);

    int wait(void);
	// wait for the condition variable to be signalled.  The mutex is
	// implicitly released before waiting and locked again after waking up.
	// If wait() is called by multiple threads, a signal may wake up more
	// than one thread.  See POSIX threads documentation for details.

    int timed_wait(unsigned long secs, unsigned long nanosecs = 0);
	// timed_wait is given an absolute time to wait until.  To wait for a
	// relative time from now, use omni_thread::get_time. See POSIX threads
	// documentation for why absolute times are better than relative.
	// Returns 0 if successfully signalled, ETIMEDOUT if time expired.

    int signal(void);
	// if one or more threads have called wait(), signal wakes up at least
	// one of them, possibly more.  See POSIX threads documentation for
	// details.

    int broadcast(void);
	// broadcast is like signal but wakes all threads which have called
	// wait().

    static omni_condition* create(omni_mutex* m) { return new omni_condition(m); }
	// only for backwards compatibility

OMNI_THREAD_EXPOSE:
    OMNI_CONDITION_IMPLEMENTATION
};


///////////////////////////////////////////////////////////////////////////
//
// Counting semaphore
//
///////////////////////////////////////////////////////////////////////////

class _OMNITHREAD_NTDLL_ omni_semaphore {

public:
    omni_semaphore(unsigned int initial = 1);
    ~omni_semaphore(void);

    int wait(void);
	// if semaphore value is > 0 then decrement it and carry on. If it's
	// already 0 then block.

    int try_wait(void);
	// if semaphore value is > 0 then decrement it and return 0. If it's
	// already 0 then return EAGAIN.

    int post(void);
	// if any threads are blocked in wait(), wake one of them up. Otherwise
	// increment the value of the semaphore.

OMNI_THREAD_EXPOSE:
    OMNI_SEMAPHORE_IMPLEMENTATION
};


///////////////////////////////////////////////////////////////////////////
//
// Thread
//
///////////////////////////////////////////////////////////////////////////

class _OMNITHREAD_NTDLL_ omni_thread {

public:

    enum priority_t {
	PRIORITY_LOW,
	PRIORITY_NORMAL,
	PRIORITY_HIGH
    };

    enum state_t {
	STATE_INVALID,		// thread object was not created properly.
	STATE_NEW,		// object exists but thread hasn't started.
	STATE_RUNNING,		// thread is running.
	STATE_TERMINATED	// thread has terminated but storage has not
				// been reclaimed (i.e. waiting to be joined).
    };

    //
    // Constructors set up the thread object but the thread won't start until
    // start() is called. The create method can be used to construct and start
    // a thread in a single call.
    //

    omni_thread(void (*fn)(void*), void* arg = NULL,
		priority_t pri = PRIORITY_NORMAL);
    omni_thread(void* (*fn)(void*), void* arg = NULL,
		priority_t pri = PRIORITY_NORMAL);
	// these constructors create a thread which will run the given function
	// when start() is called.  The thread will be detached if given a
	// function with void return type, undetached if given a function
	// returning void*. If a thread is detached, storage for the thread is
	// reclaimed automatically on termination. Only an undetached thread
	// can be joined.

    int start(void);
	// start() causes a thread created with one of the constructors to
	// start executing the appropriate function.

protected:

    omni_thread(void* arg = NULL, priority_t pri = PRIORITY_NORMAL);
	// this constructor is used in a derived class.  The thread will
	// execute the run() or run_undetached() member functions depending on
	// whether start() or start_undetached() is called respectively.

    int start_undetached(void);
	// can be used with the above constructor in a derived class to cause
	// the thread to be undetached.  In this case the thread executes the
	// run_undetached member function.

    virtual ~omni_thread(void);
	// destructor cannot be called by user (except via a derived class).
	// Use exit() or cancel() instead. This also means a thread object must
	// be allocated with new - it cannot be statically or automatically
	// allocated. The destructor of a class that inherits from omni_thread
	// shouldn't be public either (otherwise the thread object can be
	// destroyed while the underlying thread is still running).

public:

    int join(void**);
	// join causes the calling thread to wait for another's completion,
	// putting the return value in the variable of type void* whose address
	// is given (unless passed a null pointer). Only undetached threads
	// may be joined. Storage for the thread will be reclaimed.

    int set_priority(priority_t);
	// set the priority of the thread.

    static omni_thread* create(void (*fn)(void*), void* arg = NULL,
			       priority_t pri = PRIORITY_NORMAL);
    static omni_thread* create(void* (*fn)(void*), void* arg = NULL,
			       priority_t pri = PRIORITY_NORMAL);
	// create spawns a new thread executing the given function with the
	// given argument at the given priority. Returns a pointer to the
	// thread object on success, null pointer on failure. It simply
	// constructs a new thread object then calls start.

    static omni_thread* create(void* (*fn)(void*), int detached,
			       void* arg = NULL);
	// This form of create is for backwards compatibility only.

    static void exit(void* return_value = NULL);
	// causes the calling thread to terminate.

    static omni_thread* self(void);
	// returns the calling thread's omni_thread object.

    static void yield(void);
	// allows another thread to run.

    static int sleep(unsigned long secs, unsigned long nanosecs = 0);
	// sleeps for the given time.

    static int get_time(unsigned long* abs_sec, unsigned long* abs_nsec,
			unsigned long rel_sec = 0, unsigned long rel_nsec = 0);
	// calculates an absolute time in seconds and nanoseconds, suitable for
	// use in timed_waits on condition variables, which is the current time
	// plus the given relative offset.

private:

    virtual void run(void* arg) {}
    virtual void* run_undetached(void* arg) { return NULL; }
	// can be overridden in a derived class.  When constructed using the
	// the constructor omni_thread(void*, priority_t), these functions are
	// called by start() and start_undetached() respectively.

    void common_constructor(void* arg, priority_t pri, int det);
	// implements the common parts of the constructors.

    omni_mutex mutex;
	// used to protect any members which can change after construction,
	// i.e. the following 2 members:

    state_t _state;
    priority_t _priority;

    static omni_mutex* next_id_mutex;
    static int next_id;
    int _id;

    void (*fn_void)(void*);
    void* (*fn_ret)(void*);
    void* thread_arg;
    int detached;

public:

    priority_t priority(void) {

	// return this thread's priority.

	mutex.lock();
	priority_t pri = _priority;
	mutex.unlock();
	return pri;
    }

    state_t state(void) {

	// return thread state (invalid, new, running or terminated).

	mutex.lock();
	state_t s = _state;
	mutex.unlock();
	return s;
    }

    int id(void) { return _id; }
	// return unique thread id within the current process.


    // This class plus the instance of it declared below allows us to execute
    // some initialisation code before main() is called.

    class _OMNITHREAD_NTDLL_ init_t {
	static int count;
    public:
	init_t(void);
    };

    friend class init_t;

OMNI_THREAD_EXPOSE:
    OMNI_THREAD_IMPLEMENTATION
};

static omni_thread::init_t omni_thread_init;

#endif
